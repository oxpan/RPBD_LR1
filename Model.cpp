#pragma once
#include "Model.h"
#include "math.h"


Model::Model()
{
	
};

void Model::tryDB()
{
	try
	{
		std::wifstream odbcCon("connectionConfig.txt");
		std::wstring buf;
		if (odbcCon)
		{
			while (std::getline(odbcCon, buf, L'\t'))
			{
				if (buf.compare(L"dsn") == 0)
				{
					std::getline(odbcCon, buf);
					wcscpy_s(DataBaseConnection::dsn, buf.c_str());
				}
				if (buf.compare(L"user") == 0)
				{
					std::getline(odbcCon, buf);
					wcscpy_s(DataBaseConnection::user, buf.c_str());
				}
				if (buf.compare(L"pass") == 0)
				{
					std::getline(odbcCon, buf);
					wcscpy_s(DataBaseConnection::password, buf.c_str());
				}
			}
			odbcCon.close();
		}

		else { odbcCon.close(); throw - 3; }

		dbc = DataBaseConnection::getInstance();
		
		adMap.setDBC(dbc);
		pnMap.setDBC(dbc);
		pMap.setDBC(dbc);

		//��������� ������� ������ � ��
		Person p;
		Address add;
		PhoneNumber pn;

		pMap.setBuf(&p);
		adMap.setBuf(&add);
		pnMap.setBuf(&pn);

		try
		{
			pMap.findObj(1);
			adMap.findObj(1);
			pnMap.findObj(1);
		}
		catch (std::wstring msg)
		{
			try
			{
				pMap.createDB();
			}
			catch (int err)
			{
				throw - 1;
			}
		}
	}
	catch (int err)
	{
		try
		{
			adMap.setDBC(dbc);
			pnMap.setDBC(dbc);
			pMap.setDBC(dbc);
		}
		catch (int err) { throw err; }
		throw err;
	}
}

Person& Model::insertPerson(Person p)
{
	personTable.push_back(p);
	Person& res = personTable.back();
	sync(&res);
	return res;
}

bool Model::checkConnect()
{
	//���� ��� ����� ��� ������� �����������,
	//�� ��������� 1 ��� ���������������
	try {
		if (dbc == nullptr)
			dbc = DataBaseConnection::getInstance();
	}
	catch (...) { return false; }
	
	return dbc->checkConnection();
}

void Model::updatePerson(Person* pOld, Person* fio)
{
	wcscpy_s(pOld->lastName, fio->lastName);
	wcscpy_s(pOld->firstName, fio->firstName);
	wcscpy_s(pOld->fatherName, fio->fatherName);
}

int Model::getState(Person* p)
{
	bool bd = p->id > -1;
	if (!bd) 
	{
		if (!p->isSynced)
			return 2;
		else
			return 3;
	}
	else 
	{
		if (!p->isSynced)
			return 6;
		else
			return 5;
	}
};

void Model::download(Person* p)
{
	std::list<Address>::iterator it = addressTable.begin();
	//������ �����
	//��������� ��� � ��
	//����������� ��� �� id
	if (p->idAddress != -1) 
	{
		Address ad;
		ad.id = p->idAddress;
		adMap.setBuf(&ad);
		adMap.findObj(ad.id);
		addressTable.push_back(ad);
		p->address = &addressTable.back();
		p->address->isSynced = 1;
	}

	//��������
	//���� ������ id ���������
	for (int i = 0; i < p->idPhones.size(); i++) 
	{
		int id = p->idPhones[i];
		std::list<PhoneNumber>::iterator jt = phoneNumberTable.begin();		
		//������ ������� �� ��� id
		PhoneNumber pn;
		pn.id = id;
		pnMap.setBuf(&pn);
		pnMap.findObj(id);
		phoneNumberTable.push_back(pn);
		p->phoneNumbers.push_back(&phoneNumberTable.back());
		p->phoneNumbers.back()->isSynced = 1;
	}
};

void Model::upload(Person* p)
{
	//������������� ������
	if(p->address!=nullptr) 
		sync(p->address);
	//������������� ���������
	for(
		std::vector<PhoneNumber*>::iterator i = p->phoneNumbers.begin();
		i != p->phoneNumbers.end();
		++i)
		sync(*i);
};

PhoneNumber& Model::insertPhone(PhoneNumber pn)
{
	int q(0);
	PhoneNumber* res = &findPhone(pn, q);
	if (q == 1)
		return *res;
	phoneNumberTable.push_back(pn);
	sync(&phoneNumberTable.back());
	return phoneNumberTable.back();
}

PhoneNumber& Model::findPhone(PhoneNumber pn, int& ctr)
{
	//����� �������������
	syncAllPhones();
	int sd(0);
	int bd(0);
	PhoneNumber* res = &pn;
	for (std::list<PhoneNumber>::iterator i = phoneNumberTable.begin(); 
		i != phoneNumberTable.end(); 
		++i)
	{
		if ((*i).isEqual(&pn))
		{
			sd++;
			res = &(*i);
		}
	}

	if (checkConnect())
	{
		//����� ���������� � ����
		pnMap.setBuf(&pn);
		bd = pnMap.findObj();

		//���� ��������� ������� ���� ������ � ��
		//, �� ��������� ��� � ������ ����������
		if (bd == 1 && sd == 0)
		{
			phoneNumberTable.push_back(pn);
			res = &phoneNumberTable.back();
			res->isSynced = 1;
		}
	}
	if (sd == bd)
		ctr = bd;
	else if (bd == 0)
		ctr = sd;
	else
		ctr = bd;
	return *res;	
}

int Model::getState(PhoneNumber* pn)
{
	bool bd = pn->id > -1;
	if (!bd)
	{
		if (!pn->isSynced)
			return 2;
		else
			return 3;
	}
	else
	{
		if (!pn->isSynced)
			return 6;
		else
			return 5;
	}
}

void Model::sync(PhoneNumber* pn)
{
	int state = getState(pn);
	if ((state == 2 || state == 3 || state == 6) && checkConnect())
	{
		pnMap.setBuf(pn);
		pnMap.insertObj();
		if (state == 6) pn->isSynced = 1;
	}
	if (state == 2)
		pn->isSynced = 1;
}

void Model::syncAllPhones()
{
	for (std::list<PhoneNumber>::iterator i = phoneNumberTable.begin();
		i != phoneNumberTable.end();
		++i)
		sync(&(*i));
}

int Model::findReferences(PhoneNumber* pn)
{
	int state = getState(pn);
	int res(0);
	if (state == 3 || state == 5)
	{
		for (std::list<Person>::iterator i = personTable.begin(); i != personTable.end(); ++i)
			if ((*i).containPhoneNumber(pn))
				res++;
	};
	if (state == 5)
	{
		pnMap.setBuf(pn);
		res = res + pnMap.findReferences();
	};
	return res;
}

void Model::deletePhone(PhoneNumber* pn)
{
	switch (getState(pn))
	{
		case 5:
		{
			pnMap.setBuf(pn);
			pnMap.deleteObj();
		}
		case 2:
		case 3:
		{
			std::list<PhoneNumber>::iterator i = phoneNumberTable.begin();
			while (i != phoneNumberTable.end() && &(*i) != pn)
				++i;
			if(i!= phoneNumberTable.end())
				phoneNumberTable.erase(i);
			break;
		}
	}
}

Address& Model::findAddress(Address add, int& ctr)
{
	//����� �������������
	syncAllAddresses();
	int sd(0);
	int bd(0);
	Address* res = &add;
	for (std::list<Address>::iterator i = addressTable.begin();
		i != addressTable.end();
		++i)
	{
		if ((*i).isEqual(&add))
		{
			sd++;
			res = &(*i);
		}
	}
	if (checkConnect())
	{
		//����� ���������� � ����
		adMap.setBuf(&add);
		bd = adMap.findObj();

		//���� ��������� ������� ���� ������ � ��
		//, �� ��������� ��� � ������ ����������
		if (bd == 1 && sd == 0)
		{
			addressTable.push_back(add);
			res = &addressTable.back();
			res->isSynced = 1;
		}
	}
	if (sd == bd)
		ctr = bd;
	else if (bd == 0)
		ctr = sd;
	else
		ctr = bd;
	return *res;
}

Address& Model::insertAddress(Address add)
{
	int q(0);
	Address* res = &findAddress(add, q);
	if (q == 1)
		return *res;
	addressTable.push_back(add);
	sync(&addressTable.back());
	return addressTable.back();
}

int Model::getState(Address* add)
{
	bool bd = add->id > -1;
	if (!bd)
	{
		if (!add->isSynced)
			return 2;
		else
			return 3;
	}
	else
	{
		if (!add->isSynced)
			return 6;
		else
			return 5;
	}
}

void Model::sync(Address* add)
{
	int state = getState(add);
	if ((state == 2 || state == 3 || state == 6) && checkConnect())
	{
		adMap.setBuf(add);
		adMap.insertObj();
		if (state == 6) add->isSynced = 1;
	}
	if (state == 2)
		add->isSynced = 1;
}

void Model::syncAllAddresses()
{
	for (std::list<Address>::iterator i = addressTable.begin();
		i != addressTable.end();
		++i)
		sync(&(*i));
}

void Model::deleteAddress(Address* add)
{
	switch (getState(add))
	{
		case 5:
		{
			adMap.setBuf(add);
			adMap.deleteObj();
		}
		case 3: 
		{
			std::list<Address>::iterator i = addressTable.begin();
			while (i != addressTable.end() && &(*i) != add)
				++i;
			addressTable.erase(i);
			break;
		}
	}
}

int Model::findReferences(Address* add)
{
	int state = getState(add);
	int res(0);
	if (state == 3 || state == 5)
	{
		for (std::list<Person>::iterator i = personTable.begin(); i != personTable.end(); ++i) 
			if ((*i).address == add)
				res++;
	};
	if (state == 5) 
	{
		adMap.setBuf(add);
		res = res + adMap.findReferences();
	};
	return res;
}

void Model::syncAll()
{
	for (std::list<Person>::iterator i = personTable.begin(); i != personTable.end();++i) 
		sync(&(*i));	
};

void Model::sync( Person* per)
{
	int state = getState(per);
	if ( (state == 2 || state == 3) && checkConnect())
	{
		pMap.setBuf(per);
		upload(per);
		pMap.insertObj();
		if (state == 6) per->isSynced = 1;
	}
	if (state == 2)
		per->isSynced = 1;
	if (state == 6 && checkConnect())
	{
		pMap.setBuf(per);
		upload(per);
		pMap.updateObj();
		per->isSynced = 1;
	}
};

void Model::updatePerson(Person* pOld, Person pNew)
{
	Address* bufA = nullptr;	
	std::vector<PhoneNumber*> bufP;
	pOld->isSynced = 0;
	//���� ��������� ���
	if (!pOld->isEqual(&pNew)) 
		updatePerson(pOld, &pNew);	
	
	//<address>
		//�������������� ��������� �� ������, ����
		//� �������� �� ���� ������ �� ����� 
		Address empty;
		if (pOld->address == nullptr)
			pOld->address = &empty;
		
		if (pNew.address == nullptr)
			pNew.address = &empty;

		if (!pOld->address->isEqual(pNew.address))
		{
			bufA = pOld->address;
			Address* addN = &insertAddress(*pNew.address);
			pOld->address = addN;
			pOld->idAddress = pOld->address->id;
		}

		if (pOld->address == &empty)
			pOld->address = nullptr;

		if (pNew.address == &empty)
			pNew.address = nullptr;
	//</address>
		
	//<phoneNumber>
		PhoneNumber emptyy;
		for (int i = 0; i < pNew.phoneNumbers.size(); i++)
		{
			//��������� ��������� ��������� 
			//���� � ����� ������� ������ ���������
			if (i >= pOld->phoneNumbers.size())
			{
				pOld->phoneNumbers.push_back(&emptyy);
				pOld->idPhones.push_back(-1);
			}

			//���� �������� �� �������
			if (!pOld->phoneNumbers[i]->isEqual(pNew.phoneNumbers[i]))
			{
				bufP.push_back(pOld->phoneNumbers[i]);
				PhoneNumber* pnN = &insertPhone(*pNew.phoneNumbers[i]);
				pOld->phoneNumbers[i] = pnN;
				pOld->idPhones[i] = pnN->getId();
			}
		}
	//</phoneNumber>

	sync(pOld);

	//����� ��������� �����. ����� ��������� ���������� ������ �� ������ ������. 
	//���� �� �� �����, �� ������� ������.
	if (bufA != nullptr)
	{
		adMap.setBuf(bufA);
		if (findReferences(bufA) == 0)
			deleteAddress(bufA);
	}
	for (int i = 0; i < bufP.size(); i++) 
	{
		if (bufP[i] != nullptr)
		{
			pnMap.setBuf(bufP[i]);
			if (findReferences(bufP[i]) == 0)
				deletePhone(bufP[i]);
		}
	}	
}

void Model::deletePerson(Person* p)
{
	std::list<Person>::iterator i = personTable.begin();
	while (i != personTable.end() && &(*i) != p)
	{}
	//��������������, ���������� ��������� ��������� �� ������ ���������
	if (&(*i) == p)
	{
		//�������������� ���������� ����� ��������� �������
		Address* add = p->address;
		std::vector<PhoneNumber*> pn = p->phoneNumbers;
			
		int state = getState(p);
		//������ ����� ��������, ���� ������� ���� � ��
		if (state == 5 && checkConnect())
		{
			pMap.setBuf(p);
			pMap.deleteObj();
		}
		//������������� �����, ������� ��������� �������������� ��������
		if (state == 5 || state == 3)
		{
			personTable.erase(i);

			if (add != nullptr)
			{
				adMap.setBuf(add);
				if (findReferences(add) == 0)
					deleteAddress(add);
			}
			for (int i = 0; i < pn.size(); i++)
			{
				if (pn[i] != nullptr)
				{
					pnMap.setBuf(pn[i]);
					if (findReferences(pn[i]) == 0)
						deletePhone(pn[i]);
				}
			}
		}			
	}
}

//����� ������ ��������� �������� ������ 1 ��������.
//������������ ������ �� ������ � ���������� �������� ������������ ��������
//���� ����� ��������, �� ������� = 1
//���� ������� > 0 ������ ������������ ��������� ���������
//���� ������� == 0 ������ ������ �� �������
Person& Model::findPerson(Person p, bool isEmpty, int& ctr)
{
	//����� �������������
	syncAll();
	
	//����� �����
	Person* res = &p;
	
	ctr = 0;
	int i = 0;
	int sz = personTable.size();
	
	int sd = 0;
	int bd = 0;

	//���� � �� ���������� �� ���
	
	for (std::list<Person>::iterator i = personTable.begin(); i != personTable.end(); ++i) 
	{
		if (p.isEqual(&(*i))) 
		{
			res = &(*i);
			sd++;
		}
	}
	if (checkConnect())
	{
		//����� ���������� � ����
		pMap.setBuf(&p);
		if (isEmpty)
			bd = pMap.findObj(isEmpty);
		else		
			bd = abs( pMap.findObjj());
	
		//���� ��������� ������� ���� ������ � ��
		//, �� ��������� ��� � ������ ����������
		if (bd == 1 && sd == 0) 
		{
			//������� ��������
			personTable.push_back(p);
			//������ �� ��������������� ����������, ���� ������� ��������������
			if(!isEmpty)
				download(&personTable.back());			
			personTable.back().isSynced = 1;
			res = &personTable.back();
		}
	}
	if (sd == bd)
		ctr = bd;
	else if (bd == 0)
		ctr = sd;
	else
		ctr = bd;
	return *res;
}

Person& Model::findPerson(Person p, PhoneNumber pn, int& ctr) 
{
	//����� �������������
	syncAll();

	//����� �����
	Person* res = &p;

	ctr = 0;
	int i = 0;
	int sz = personTable.size();

	int sd = 0;
	int bd = 0;

	//���� � �� ���������� �� ���

	for (std::list<Person>::iterator i = personTable.begin(); i != personTable.end(); ++i)
	{
		if (p.isEqual(&(*i)) && (*i).containPhoneNumber(&pn))
		{
			res = &(*i);
			sd++;
		}
	}
	if (checkConnect())
	{
		//����� ���������� � ����
		pMap.setBuf(&p);
		bd = pMap.findObj(&pn);

		//���� ��������� ������� ���� ������ � ��
		//, �� ��������� ��� � ������ ����������
		if (bd == 1 && sd == 0)
		{
			//������� ��������
			personTable.push_back(p);
			//������ �� ��������������� ����������, ���� ������� ��������������
			download(&personTable.back());
			personTable.back().isSynced = 1;
			res = &personTable.back();
		}
	}
	if (sd == bd)
		ctr = bd;
	else if (bd == 0)
		ctr = sd;
	else
		ctr = bd;
	return *res;
}

Person& Model::findPerson(Person p, PhoneNumber pn, Address add, int& ctr)
{
	//����� �������������
	syncAll();

	//����� �����
	Person* res = &p;

	ctr = 0;
	int i = 0;
	int sz = personTable.size();

	int sd = 0;
	int bd = 0;

	//���� � �� ���������� �� ���

	for (std::list<Person>::iterator i = personTable.begin(); i != personTable.end(); ++i)
	{
		if (p.isEqual(&(*i)) && (*i).containPhoneNumber(&pn) && (*i).containAddress(&add))
		{
			res = &(*i);
			sd++;
		}
	}

	if (checkConnect())
	{
		//����� ���������� � ����
		pMap.setBuf(&p);
		bd = pMap.findObj(&pn, &add);

		//���� ��������� ������� ���� ������ � ��
		//, �� ��������� ��� � ������ ����������
		if (bd == 1 && sd == 0)
		{
			//������� ��������
			personTable.push_back(p);
			//������ �� ��������������� ����������, ���� ������� ��������������
			download(&personTable.back());
			personTable.back().isSynced = 1;
			res = &personTable.back();
		}
	}
	if (sd == bd)
		ctr = bd;
	else if (bd == 0)
		ctr = sd;
	else
		ctr = bd;
	return *res;
}

std::vector<Person*> Model::findBy4(std::vector<int> nums)
{
	//����� �������������
	syncAll();
	std::vector<Person*> res;
	//����� � ��
	if (!checkConnect())
	{
		//����������� �� ������� ���������
		for (std::list<Person>::iterator i = personTable.begin(); i != personTable.end(); ++i)
		{
			int buf(0);
			//�������� ������ �� ������� �� ������������ ����������� ��������
			for (int j = 0; j < (*i).phoneNumbers.size(); j++) 
			{
				if ((*i).phoneNumbers.at(j)->isContain(&nums))
					buf++;
			}
			if(buf>0)
				res.push_back(&(*i));
		}
		return res;
	}
	//����� � ��
	else 
	{
		Person empty;
		std::vector<Person> bd;
		pMap.setBuf(&empty);
		bd = pMap.findby4(&nums);
		
		//����� ����������� �� ��
		for (int i = 0; i < bd.size(); i++) 
		{
			std::list<Person>::iterator j = personTable.begin();
			bool flag = false;
			
			//���� ������ ������� �� ���������� � ��
			//���� ���� ������������ �� true,
			//������ ������ ���������� � ��
			while (j!= personTable.end())
			{
				bool reg = true;
				reg = reg && (*j).isEqual(&bd[i]);
				
				reg = reg && (*j).idPhones == bd[i].idPhones;

				reg = reg && (*j).idAddress == bd[i].idAddress;

				//�� ��� �������� ���� ������
				if (reg == true)
				{
					flag = reg;
					break;
				}
				j++;
			}
			
			//��������� ����������� ������
			//���� ������ ���� � ������ ��
			//������ ��� ����� ��� ���������
			if (flag)
				res.push_back(&(*j));
			//� �� ���������� �� ���� �������
			else
			{
				//������� ��������
				personTable.push_back(bd[i]);
				//������ �� ��������������� ����������, ���� ������� ��������������
				download(&personTable.back());
				personTable.back().isSynced = 1;
				res.push_back(&personTable.back());
			}
		}
	}
	return res;
}

std::vector<Person*> Model::find_List_FIO(Person p)
{
	//����� �������������
	syncAll();

	std::vector<Person*> res;

	//����� � ��
	if (!checkConnect())
	{
		//����������� �� ������� ���������
		for (std::list<Person>::iterator i = personTable.begin(); i != personTable.end(); ++i)
		{
			if ((*i).isEqual(&p))
				res.push_back(&(*i));
		}
		return res;
	}
	//����� � ��
	else
	{
		std::vector<Person> bd;
		pMap.setBuf(&p);
		bd = pMap.findListFIO();

		//����� ����������� �� ��
		for (int i = 0; i < bd.size(); i++)
		{
			std::list<Person>::iterator j = personTable.begin();
			bool flag = false;

			//���� ������ ������� �� ���������� � ��
			//���� ���� ������������ �� true,
			//������ ������ ���������� � ��
			while (j != personTable.end())
			{
				bool reg = true;
				reg = reg && (*j).isEqual(&bd[i]);

				reg = reg && (*j).idPhones == bd[i].idPhones;

				reg = reg && (*j).idAddress == bd[i].idAddress;

				//�� ��� �������� ���� ������
				if (reg == true)
				{
					flag = reg;
					break;
				}
				j++;
			}

			//��������� ����������� ������
			//���� ������ ���� � ������ ��
			//������ ��� ����� ��� ���������
			if (flag)
				res.push_back(&(*j));
			//� �� ���������� �� ���� �������
			else
			{
				//������� ��������
				personTable.push_back(bd[i]);
				//������ �� ��������������� ����������, ���� ������� ��������������
				download(&personTable.back());
				personTable.back().isSynced = 1;
				res.push_back(&personTable.back());
			}
		}
	}
	return res;
}

bool PhoneNumber::isContain(std::vector<int>* nums)
{
	bool res = true;

	SQLWCHAR cur =number[10];
	res = res && _wtoi(&cur) == nums->at(0);

	cur = number[11];
	res = res && _wtoi(&cur) == nums->at(1);

	cur = number[13];
	res = res && _wtoi(&cur) == nums->at(2);

	cur = number[14];
	res = res && _wtoi(&cur) == nums->at(3);

	return res;
}

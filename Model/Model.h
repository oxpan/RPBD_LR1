#pragma once
#include <windows.h>
#include <odbcinst.h>
#include <sqlext.h>
#include <sql.h>
#include <iostream>
#include <vector>
#include <unordered_map>

#define strSZ 20

class DataBaseConnection;

class Address
{
public:
	SQLWCHAR streetName[strSZ];	
	SQLINTEGER home;
	SQLINTEGER appartement;

private:
	DataBaseConnection* dbc;
	
	SQLINTEGER id;
	SQLINTEGER idStreet;

	SQLLEN idLen;
	SQLLEN idStreetLen;	
	SQLLEN streetNameLen;
	SQLLEN homeLen;
	SQLLEN appartementLen;

public:

	Address(int id = -1, int idStreet = -1)
	{
		this->id = id;
		this->idStreet = idStreet;
	}
	friend class AddressMapper;
	friend class Model;
};

class PhoneNumber
{
public:
	SQLWCHAR number[strSZ];
	SQLWCHAR typeName[strSZ];
	
private:
	DataBaseConnection* dbc;

	SQLINTEGER id;
	SQLLEN idLen;
	SQLINTEGER idType;
	SQLLEN idTypeLen;
	SQLLEN numberLen;
	SQLLEN typeNameLen;

public:
	
	PhoneNumber(int id =-1, int idType=-1)
	{
		this->id = id;
		this->idType = idType;
	}
	friend class PhoneMapper;
	friend class Model;
};

class Person
{
public:
	SQLWCHAR firstName[strSZ];	
	SQLWCHAR lastName[strSZ];
	SQLWCHAR fatherName[strSZ];
	Address* address;
	vector<PhoneNumber*> phoneNumbers;	

private:
	DataBaseConnection* dbc;

	SQLINTEGER id;
	SQLINTEGER idAddress;
	SQLINTEGER phoneCount;

	vector<SQLINTEGER> idPhones;
	
	SQLLEN nameLen;
	SQLLEN lastNameLen;
	SQLLEN fatherNameLen;
	SQLLEN idLen;
	SQLLEN idAddressLen;
	SQLLEN idPhone;

public:

	Person(int id=-1,int idAddress=-1)
	{
		phoneCount = 0;
		this->id = id;
		this->idAddress = idAddress;		
	}
	
	bool containPhoneNumber(PhoneNumber* pn) 
	{
		for (int i = 0; i < phoneNumbers.size(); i++) 
		{
			if (pn == phoneNumbers[i]) return true;
		}
		return false;
	}

	friend class PersonMapper;
	friend class Model;

};

class DataBaseConnection
{
protected:
	SQLHENV handleEnv;
	SQLRETURN retcode;
	SQLWCHAR* dsn;
	SQLWCHAR* user;
	SQLWCHAR* password;
	//������� ������ ����� get � �����������
	SQLHDBC hDBC;

	static DataBaseConnection* database_;
	
	DataBaseConnection() : status{1}
	{
		dsn = (SQLWCHAR*)L"Phonebook";
		user = (SQLWCHAR*)L"postgres";
		password = (SQLWCHAR*)L"123";

		retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &handleEnv);
		checkErr();

		retcode = SQLSetEnvAttr(handleEnv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
		checkErr();

		retcode = SQLAllocHandle(SQL_HANDLE_DBC, handleEnv, &hDBC);
		checkErr();

		//������ ������ ����� �����������
		retcode = SQLSetConnectAttr(hDBC, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)FALSE, 0);
		checkErr();
		
		retcode = SQLConnect(hDBC, dsn, SQL_NTS, user, SQL_NTS, password, SQL_NTS);
		checkErr();
		//���������� �������� ��������
	}

	void checkErr() 
	{
		if (retcode < 0) 
		{ 
			status = 0;
			throw "������ ����������� � ����\n"; 
		}
	}

public:	

	bool status;
	SQLHDBC* getHDBC() { return &hDBC; }
	DataBaseConnection(DataBaseConnection& other) = delete;
	void operator=(const DataBaseConnection&) = delete;
	static DataBaseConnection* getInstance();
	~DataBaseConnection() 
	{
		retcode = SQLDisconnect(hDBC);
		retcode = SQLFreeHandle(SQL_HANDLE_DBC,hDBC);
		retcode = SQLFreeHandle(SQL_HANDLE_ENV,handleEnv);
	}
};

DataBaseConnection* DataBaseConnection::database_ = nullptr;
DataBaseConnection* DataBaseConnection::getInstance() 
{
	if (database_ == nullptr) { database_ = new DataBaseConnection(); }
	return database_;
}

class Model 
{
private:
	unordered_map<int,Address> addressTable;
	//AddressMapper
	unordered_map<int,PhoneNumber> phoneNumberTable;
	//PhoneNumberMapper
	unordered_map<int, Person> personTable;
	//PersonMapper
	ConsoleApp* conApp;
public:
	Person& watchRecord() {};
	void removeRecord() {};
	void editRecord() {};
	void addRecord() {};
private:
	Person& findByAllAtributes() {};
	Person& findBy4() {};
	Person& findById() {};
};

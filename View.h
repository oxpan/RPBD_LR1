#pragma once
#include <iostream>
#include <sstream>
#include <cstring>

#include "Model.h"
#include "Controller.h"
using namespace std;

class Controller;

class ConsoleApp
{
private:
	bool runTimeProgram;// ������ � ������ ����� ���������
	int MainMenu;//������������ ��� ����� �������������
	Controller* con;
	Person* currentPerson;
public:
	ConsoleApp();
	void setController(Controller* con);
	
	//��������� ����
	void Menu();
	void MenuPC();	
	
	//����������� ����
	void run();
	void runPC();

	//������, ������������ �������������
	Person* findPerson();
	Person* addContact();
	void deleteContact();
	void findto4();
	void findList_FIO();
	void updateFIOcontacte();
	void addAddress();
	void addPhoneNumber();
	void deleteAddress();
	void updatePhoneNumber();

	//������� ��� �����
	void offlineStatus();
	void noTableConfig();
	void noConnectionConfig();
	void success();	
	void fail();
	void noRes();
	void clones();
	
	/*��������������� ������*/
	bool toRunMenuTwo();
	void drawPerson(Person* p);
	void drawAddress(Address* add);
	void drawPhoneNumber(PhoneNumber* pn);
	bool drawPhoneNumbers(std::vector<PhoneNumber*> args);
	bool checkFormat(const wchar_t* number);

	/*������ ��� ������ ������*/
	wchar_t* get_a_number();
	wchar_t* get_a_addressName();
	int get_a_type_number();
	int get_a_apartment();
	int get_a_numberhome();
	void inputContact(wchar_t*& lastnamecontact, wchar_t*& firstnamecontact, wchar_t*& fathernamecontact);
};
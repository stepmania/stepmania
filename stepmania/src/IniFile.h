/*
-----------------------------------------------------------------------------
 File: IniFile.h

 Desc: Wrapper for reading and writing an .ini file.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/


#ifndef _INIFILE_H_
#define _INIFILE_H_

#include <afxtempl.h>
//#include <iostream.h>


class IniFile  
{
	//all private variables
private:

	//stores pathname of ini file to read/write
	CString path;
	
	//all keys are of this time
	typedef CMapStringToString key;

	//list of keys in ini
	CArray<key, key> keys; 

	//corresponding list of keynames
	CArray<CString, CString> names; 


	
	//all private functions
private:

	//returns index of specified key, or -1 if not found
	int FindKey(CString keyname);


	//public variables
public:

	//will contain error info if one occurs
	//ended up not using much, just in ReadFile and GetValue
	CString error;


	//public functions
public:
	//default constructor
	IniFile();

	//constructor, can specify pathname here instead of using SetPath later
	IniFile(CString inipath);

	//default destructor
	virtual ~IniFile();

	//sets path of ini file to read and write from
	void SetPath(CString newpath);

	//reads ini file specified using IniFile::SetPath()
	//returns true if successful, false otherwise
	BOOL ReadFile();

	//writes data stored in class to ini file
	void WriteFile(); 

	//deletes all stored ini data
	void Reset();

	//returns number of keys currently in the ini
	int GetNumKeys();

	//returns a pointer to the key for direct modification
	CMapStringToString* GetKeyPointer( CString keyname );

	//returns number of values stored for specified key
	int GetNumValues( CString keyname );

	//gets value of [keyname] valuename = 
	//overloaded to return CString, int, and double,
	//returns "", or 0 if key/value not found.  Sets error member to show problem
	CString GetValue(CString keyname, CString valuename); 
	int GetValueI(CString keyname, CString valuename); 
	double GetValueF(CString keyname, CString valuename);

	//sets value of [keyname] valuename =.
	//specify the optional paramter as false (0) if you do not want it to create
	//the key if it doesn't exist. Returns true if data entered, false otherwise
	//overloaded to accept CString, int, and double
	BOOL SetValue(CString key, CString valuename, CString value, BOOL create = 1);
	BOOL SetValueI(CString key, CString valuename, int value, BOOL create = 1);
	BOOL SetValueF(CString key, CString valuename, double value, BOOL create = 1);

	//deletes specified value
	//returns true if value existed and deleted, false otherwise
	BOOL DeleteValue(CString keyname, CString valuename);

	//deletes specified key and all values contained within
	//returns true if key existed and deleted, false otherwise
	BOOL DeleteKey(CString keyname);
};


#endif

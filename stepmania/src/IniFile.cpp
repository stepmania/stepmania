#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: IniFile

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Adam Clauss
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "IniFile.h"
#include "RageUtil.h"
#include <fstream>
using namespace std;

//constructor, can specify pathname here instead of using SetPath later
IniFile::IniFile(CString inipath)
{
	path = inipath;
}

//default destructor
IniFile::~IniFile()
{

}

// sets path of ini file to read and write from
void IniFile::SetPath(CString newpath)
{
	path = newpath;
}

// reads ini file specified using IniFile::SetPath()
// returns true if successful, false otherwise
bool IniFile::ReadFile()
{
	ifstream file(path);

	if (file.bad())
	{
		error = "Unable to open ini file.";
		return 0;
	}

	CString keyname, valuename, value;
	CString temp;
	CString line;
	while (getline(file, line))
	{
		if(line.size() >= 3 &&
			line[0] == '\xef' &&
			line[1] == '\xbb' &&
			line[2] == '\xbf'
			)
		{
			/* Obnoxious NT marker for UTF-8.  Remove it. */
			line.erase(0, 3);
		}

		if (line == "")
			continue;

		StripCrnl(line);

		if (line.substr(0, 2) == "//" || line.substr(0) == "#")
			continue; /* comment */

		if (line[0] == '[' && line[line.GetLength()-1] == ']') //if a section heading
		{
			keyname = line;
			TrimLeft(keyname, "[");
			TrimRight(keyname, "]");
		}
		else //if a value
		{
			int iEqualIndex = line.Find("=");
			if( iEqualIndex != -1 )
			{
				valuename = line.Left(iEqualIndex);
				value = line.Right(line.GetLength()-valuename.GetLength()-1);
				SetValue(keyname,valuename,value);
			}
		}
	}
	return 1;
}

// writes data stored in class to ini file
void IniFile::WriteFile()
{
	FILE* fp = fopen( path, "w" );

	if( fp == NULL )
		return;

	for (keymap::const_iterator k = keys.begin(); k != keys.end(); ++k)
	{
		if (k->second.empty())
			continue;

		fprintf( fp, "[%s]\n", k->first.c_str() );

		for (key::const_iterator i = k->second.begin(); i != k->second.end(); ++i)
			fprintf( fp, "%s=%s\n", i->first.c_str(), i->second.c_str() );

		fprintf( fp, "\n" );
	}
	fclose( fp );
}

// deletes all stored ini data
void IniFile::Reset()
{
	keys.clear();
}

// returns number of keys currently in the ini
int IniFile::GetNumKeys() const
{
	return keys.size();
}

// returns number of values stored for specified key, or -1 if key not found
int IniFile::GetNumValues(const CString &keyname) const
{
	keymap::const_iterator k = keys.find(keyname);
	if (k == keys.end())
		return -1;

	return k->second.size();
}

// gets value of [keyname] valuename = 
// overloaded to return CString, int, and double
bool IniFile::GetValue(const CString &keyname, const CString &valuename, CString& value)
{
	keymap::const_iterator k = keys.find(keyname);
	if (k == keys.end())
	{
		error = "Unable to locate specified key.";
		return false;
	}

	key::const_iterator i = k->second.find(valuename);

	if (i == k->second.end())
	{
		error = "Unable to locate specified value.";
		return false;
	}

	value = i->second;
	return true;
}

// gets value of [keyname] valuename = 
// overloaded to return CString, int, and double
bool IniFile::GetValueI(const CString &keyname, const CString &valuename, int& value)
{
	CString sValue;
	bool bSuccess = GetValue(keyname,valuename,sValue);
	if( !bSuccess )
		return false;
	value = atoi(sValue);
	return true;
}

// gets value of [keyname] valuename = 
// overloaded to return CString, int, and double
bool IniFile::GetValueF(const CString &keyname, const CString &valuename, float& value)
{
	CString sValue;
	bool bSuccess = GetValue(keyname,valuename,sValue);
	if( !bSuccess )
		return false;
	value = (float)atof(sValue);
	return true;
}

// gets value of [keyname] valuename = 
// overloaded to return CString, int, and double
bool IniFile::GetValueB(const CString &keyname, const CString &valuename, bool& value)
{
	CString sValue;
	bool bSuccess = GetValue(keyname,valuename,sValue);
	if( !bSuccess )
		return false;
	value = atoi(sValue) != 0;
	return true;
}

// sets value of [keyname] valuename =.
// specify the optional paramter as false (0) if you do not want it to create
// the key if it doesn't exist. Returns true if data entered, false otherwise
// overloaded to accept CString, int, and double
bool IniFile::SetValue(const CString &keyname, const CString &valuename, const CString &value, bool create)
{
	if (!create && keys.find(keyname) == keys.end()) //if key doesn't exist
		return false; // stop entering this key

	// find value
	if (!create && keys[keyname].find(valuename) == keys[keyname].end())
		return false;

	keys[keyname][valuename] = value;
	return true;
}

// sets value of [keyname] valuename =.
// specify the optional paramter as false (0) if you do not want it to create
// the key if it doesn't exist. Returns true if data entered, false otherwise
// overloaded to accept CString, int, and double
bool IniFile::SetValueI(const CString &keyname, const CString &valuename, int value, bool create)
{
	return SetValue(keyname, valuename, ssprintf("%d",value), create);
}

// sets value of [keyname] valuename =.
// specify the optional paramter as false (0) if you do not want it to create
// the key if it doesn't exist. Returns true if data entered, false otherwise
// overloaded to accept CString, int, and double
bool IniFile::SetValueF(const CString &keyname, const CString &valuename, float value, bool create)
{
	return SetValue(keyname, valuename, ssprintf("%f",value), create);
}

// sets value of [keyname] valuename =.
// specify the optional paramter as false (0) if you do not want it to create
// the key if it doesn't exist. Returns true if data entered, false otherwise
// overloaded to accept CString, int, and double
bool IniFile::SetValueB(const CString &keyname, const CString &valuename, bool value, bool create)
{
	return SetValue(keyname, valuename, ssprintf("%d",value), create);
}

// deletes specified value
// returns true if value existed and deleted, false otherwise
bool IniFile::DeleteValue(const CString &keyname, const CString &valuename)
{
	keymap::iterator k = keys.find(keyname);
	if (k == keys.end())
		return false;

	key::iterator i = k->second.find(valuename);
	if(i == k->second.end())
		return false;

	k->second.erase(i);
	return true;
}

// deletes specified key and all values contained within
// returns true if key existed and deleted, false otherwise
bool IniFile::DeleteKey(const CString &keyname)
{
	keymap::iterator k = keys.find(keyname);
	if (k == keys.end())
		return false;

	keys.erase(k);
	return true;
}

const IniFile::key *IniFile::GetKey(const CString &keyname) const
{
	keymap::const_iterator i = keys.find(keyname);
	if(i == keys.end()) return NULL;
	return &i->second;
}

void IniFile::SetValue(const CString &keyname, const key &key)
{
	keys[keyname]=key;
}

void IniFile::RenameKey(const CString &from, const CString &to)
{
	if(keys.find(from) == keys.end())
		return;
	if(keys.find(to) != keys.end())
		return;

	keys[to] = keys[from];
	keys.erase(from);
}

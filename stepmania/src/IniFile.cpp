#include "stdafx.h"
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


/////////////////////////////////////////////////////////////////////
// Construction/Destruction
/////////////////////////////////////////////////////////////////////

//default constructor
IniFile::IniFile()
{
}

//constructor, can specify pathname here instead of using SetPath later
IniFile::IniFile(CString inipath)
{
	path = inipath;
}

//default destructor
IniFile::~IniFile()
{

}

/////////////////////////////////////////////////////////////////////
// Public Functions
/////////////////////////////////////////////////////////////////////

//sets path of ini file to read and write from
void IniFile::SetPath(CString newpath)
{
	path = newpath;
}

//reads ini file specified using IniFile::SetPath()
//returns true if successful, false otherwise
bool IniFile::ReadFile()
{
	CStdioFile file;
	CFileStatus status;
	if (!file.GetStatus(path,status))
		return 0;
	int curkey = -1, curval = -1;
	if (!file.Open(path, CFile::modeRead))
	{
		error = "Unable to open ini file.";
		return 0;
	}
	CString keyname, valuename, value;
	CString temp;
	CString line;
	while (file.ReadString(line))
	{
		if (line != "")
		{
			if (line[0] == '[' && line[line.GetLength()-1] == ']') //if a section heading
			{
				keyname = line;
				keyname.TrimLeft('[');
				keyname.TrimRight(']');
			}
			else //if a value
			{
				valuename = line.Left(line.Find("="));
				value = line.Right(line.GetLength()-valuename.GetLength()-1);
				SetValue(keyname,valuename,value);
			}
		}
	}
	file.Close();
	return 1;
}

//writes data stored in class to ini file
void IniFile::WriteFile()
{
	FILE* fp = fopen( path, "w" );
	for (int keynum = 0; keynum <= names.GetUpperBound(); keynum++)
	{
		if (keys[keynum].names.GetSize() != 0)
		{
			fprintf( fp, "[%s]\n", names[keynum] );
			for (int valuenum = 0; valuenum <= keys[keynum].names.GetUpperBound(); valuenum++)
				fprintf( fp, "%s=%s\n", keys[keynum].names[valuenum], keys[keynum].values[valuenum] );
			fprintf( fp, "\n" );
		}
	}
	fclose( fp );
}

//deletes all stored ini data
void IniFile::Reset()
{
	keys.SetSize(0);
	names.SetSize(0);
}

//returns number of keys currently in the ini
int IniFile::GetNumKeys()
{
	return keys.GetSize();
}

//returns number of values stored for specified key, or -1 if key found
int IniFile::GetNumValues(CString keyname)
{
	int keynum = FindKey(keyname);
	if (keynum == -1)
		return -1;
	else
		return keys[keynum].names.GetSize();
}

//gets value of [keyname] valuename = 
//overloaded to return CString, int, and double
bool IniFile::GetValue(CString keyname, CString valuename, CString& value)
{
	int keynum = FindKey(keyname), valuenum = FindValue(keynum,valuename);

	if (keynum == -1)
	{
		error = "Unable to locate specified key.";
		return false;
	}

	if (valuenum == -1)
	{
		error = "Unable to locate specified value.";
		return false;
	}

	value = keys[keynum].values[valuenum];
	return true;
}

//gets value of [keyname] valuename = 
//overloaded to return CString, int, and double
bool IniFile::GetValueI(CString keyname, CString valuename, int& value)
{
	CString sValue;
	bool bSuccess = GetValue(keyname,valuename,sValue);
	if( !bSuccess )
		return false;
	value = atoi(sValue);
	return true;
}

//gets value of [keyname] valuename = 
//overloaded to return CString, int, and double
bool IniFile::GetValueF(CString keyname, CString valuename, float& value)
{
	CString sValue;
	bool bSuccess = GetValue(keyname,valuename,sValue);
	if( !bSuccess )
		return false;
	value = (float)atof(sValue);
	return true;
}

//gets value of [keyname] valuename = 
//overloaded to return CString, int, and double
bool IniFile::GetValueB(CString keyname, CString valuename, bool& value)
{
	CString sValue;
	bool bSuccess = GetValue(keyname,valuename,sValue);
	if( !bSuccess )
		return false;
	value = atoi(sValue) != 0;
	return true;
}

//sets value of [keyname] valuename =.
//specify the optional paramter as false (0) if you do not want it to create
//the key if it doesn't exist. Returns true if data entered, false otherwise
//overloaded to accept CString, int, and double
bool IniFile::SetValue(CString keyname, CString valuename, CString value, bool create)
{
	int keynum = FindKey(keyname), valuenum = 0;
	//find key
	if (keynum == -1) //if key doesn't exist
	{
		if (!create) //and user does not want to create it,
			return 0; //stop entering this key
		names.SetSize(names.GetSize()+1);
		keys.SetSize(keys.GetSize()+1);
		keynum = names.GetSize()-1;
		names[keynum] = keyname;
	}

	//find value
	valuenum = FindValue(keynum,valuename);
	if (valuenum == -1)
	{
		if (!create)
			return 0;
		keys[keynum].names.SetSize(keys[keynum].names.GetSize()+1);
		keys[keynum].values.SetSize(keys[keynum].names.GetSize()+1);
		valuenum = keys[keynum].names.GetSize()-1;
		keys[keynum].names[valuenum] = valuename;
	}
	keys[keynum].values[valuenum] = value;
	return 1;
}

//sets value of [keyname] valuename =.
//specify the optional paramter as false (0) if you do not want it to create
//the key if it doesn't exist. Returns true if data entered, false otherwise
//overloaded to accept CString, int, and double
bool IniFile::SetValueI(CString keyname, CString valuename, int value, bool create)
{
	CString temp;
	temp.Format("%d",value);
	return SetValue(keyname, valuename, temp, create);
}

//sets value of [keyname] valuename =.
//specify the optional paramter as false (0) if you do not want it to create
//the key if it doesn't exist. Returns true if data entered, false otherwise
//overloaded to accept CString, int, and double
bool IniFile::SetValueF(CString keyname, CString valuename, float value, bool create)
{
	CString temp;
	temp.Format("%e",value);
	return SetValue(keyname, valuename, temp, create);
}

//sets value of [keyname] valuename =.
//specify the optional paramter as false (0) if you do not want it to create
//the key if it doesn't exist. Returns true if data entered, false otherwise
//overloaded to accept CString, int, and double
bool IniFile::SetValueB(CString keyname, CString valuename, bool value, bool create)
{
	CString temp;
	temp.Format("%d",value);
	return SetValue(keyname, valuename, temp, create);
}

//deletes specified value
//returns true if value existed and deleted, false otherwise
bool IniFile::DeleteValue(CString keyname, CString valuename)
{
	int keynum = FindKey(keyname), valuenum = FindValue(keynum,valuename);
	if (keynum == -1 || valuenum == -1)
		return 0;

	keys[keynum].names.RemoveAt(valuenum);
	keys[keynum].values.RemoveAt(valuenum);
	return 1;
}

//deletes specified key and all values contained within
//returns true if key existed and deleted, false otherwise
bool IniFile::DeleteKey(CString keyname)
{
	int keynum = FindKey(keyname);
	if (keynum == -1)
		return 0;
	keys.RemoveAt(keynum);
	names.RemoveAt(keynum);
	return 1;
}

//deletes specified key and all values contained within
//returns true if key existed and deleted, false otherwise
IniFile::key* IniFile::GetKey(CString keyname)
{
	int keynum = FindKey(keyname);
	if (keynum == -1)
		return NULL;
	return &keys[keynum];
}

/////////////////////////////////////////////////////////////////////
// Private Functions
/////////////////////////////////////////////////////////////////////

//returns index of specified key, or -1 if not found
int IniFile::FindKey(CString keyname)
{
	int keynum = 0;
	while ( keynum < keys.GetSize() && names[keynum] != keyname)
		keynum++;
	if (keynum == keys.GetSize())
		return -1;
	return keynum;
}

//returns index of specified value, in the specified key, or -1 if not found
int IniFile::FindValue(int keynum, CString valuename)
{
	if (keynum == -1)
		return -1;
	int valuenum = 0;
	while (valuenum < keys[keynum].names.GetSize() && keys[keynum].names[valuenum] != valuename)
		valuenum++;
	if (valuenum == keys[keynum].names.GetSize())
		return -1;
	return valuenum;
}

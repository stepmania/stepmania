/*
-----------------------------------------------------------------------------
 File: IniFile.h

 Desc: Wrapper for reading and writing an .ini file.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "stdafx.h"
#include "IniFile.h"
#include "fstream.h"


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
BOOL IniFile::ReadFile()
{
	CFile file;
	CFileStatus status;
	if( !file.GetStatus(path,status) )
		return FALSE;
	ifstream inifile;
	CString readinfo;
	inifile.open(path);
	int curkey = -1, curval = -1;
	if( inifile.fail() )
	{
		error = "Unable to open ini file.";
		return FALSE;
	}
	CString keyname, valuename, value;
	CString temp;
	while( getline(inifile,readinfo) )
	{
		if( readinfo != "" )
		{
			if( readinfo[0] == '[' && readinfo[readinfo.GetLength()-1] == ']' ) //if a section heading
			{
				keyname = readinfo;
				keyname.TrimLeft('[');
				keyname.TrimRight(']');
			}
			else //if a value
			{
				valuename = readinfo.Left(readinfo.Find("="));
				value = readinfo.Right(readinfo.GetLength()-valuename.GetLength()-1);
				SetValue(keyname,valuename,value);
			}
		}
	}
	inifile.close();
	return 1;
}

//writes data stored in class to ini file
void IniFile::WriteFile()
{
	ofstream inifile;
	inifile.open(path);

	// foreach key
	for( int keynum = 0; keynum <= names.GetUpperBound(); keynum++ )
	{
		inifile << '[' << names[keynum] << ']' << endl;

		CMapStringToString &map = keys[keynum];

		// for each value_name/value pair
		for( POSITION pos = map.GetStartPosition(); pos != NULL; )
		{
			CString value_name;
			CString value;
			map.GetNextAssoc( pos, value_name, value );

			inifile << value_name << "=" << value << endl;
		}
		inifile << endl;
	}

	inifile.close();
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

//returns a pointer to the key for direct modification
CMapStringToString* IniFile::GetKeyPointer( CString keyname )
{
	int keynum = FindKey(keyname);
	if (keynum == -1)
		return NULL;
	else
		return &keys[keynum];
}

//returns number of values stored for specified key, or -1 if key not found
int IniFile::GetNumValues(CString keyname)
{
	int keynum = FindKey(keyname);
	if (keynum == -1)
		return -1;
	else
		return keys[keynum].GetCount();
}

//gets value of [keyname] valuename = 
//overloaded to return CString, int, and double
CString IniFile::GetValue(CString keyname, CString valuename)
{
	int keynum = FindKey(keyname);//, valuenum = FindValue(keynum,valuename);
	if( keynum == -1 )
	{
		error = "Unable to locate specified key.";
		return "";
	}

	CMapStringToString &map = keys[keynum];
	CString value;
	if( !map.Lookup(valuename, value) )
	{
		error = "Unable to locate specified value.";
		return "";
	}
	return value;
}

//gets value of [keyname] valuename = 
//overloaded to return CString, int, and double
int IniFile::GetValueI(CString keyname, CString valuename)
{
	return atoi( GetValue(keyname,valuename) );
}

//gets value of [keyname] valuename = 
//overloaded to return CString, int, and double
double IniFile::GetValueF(CString keyname, CString valuename)
{
	return atof( GetValue(keyname, valuename) );
}

//sets value of [keyname] valuename =.
//specify the optional paramter as false (0) if you do not want it to create
//the key if it doesn't exist. Returns true if data entered, false otherwise
//overloaded to accept CString, int, and double
BOOL IniFile::SetValue(CString keyname, CString valuename, CString value, BOOL create)
{
	int keynum = FindKey(keyname);

	if( keynum == -1 ) //if key doesn't exist
	{
		if( !create ) //and user does not want to create it,
			return FALSE; //stop entering this key
		names.SetSize(names.GetSize()+1);
		keys.SetSize(keys.GetSize()+1);
		keynum = names.GetSize()-1;
		names[keynum] = keyname;
	}

	// insert value
	CMapStringToString &map = keys[keynum];
	CString oldvalue;
	if( !map.Lookup(valuename, oldvalue) && !create )
		return FALSE;
	map[valuename] = value;
	return TRUE;
}

//sets value of [keyname] valuename =.
//specify the optional paramter as false (0) if you do not want it to create
//the key if it doesn't exist. Returns true if data entered, false otherwise
//overloaded to accept CString, int, and double
BOOL IniFile::SetValueI(CString keyname, CString valuename, int value, BOOL create)
{
	CString temp;
	temp.Format("%d",value);
	return SetValue(keyname, valuename, temp, create);
}

//sets value of [keyname] valuename =.
//specify the optional paramter as false (0) if you do not want it to create
//the key if it doesn't exist. Returns true if data entered, false otherwise
//overloaded to accept CString, int, and double
BOOL IniFile::SetValueF(CString keyname, CString valuename, double value, BOOL create)
{
	CString temp;
	temp.Format("%e",value);
	return SetValue(keyname, valuename, temp, create);
}

//deletes specified value
//returns true if value existed and deleted, false otherwise
BOOL IniFile::DeleteValue(CString keyname, CString valuename)
{
	int keynum = FindKey(keyname);
	if( keynum == -1 )
		return FALSE;

	CMapStringToString &map = keys[keynum];
	return map.RemoveKey( valuename );
}

//deletes specified key and all values contained within
//returns true if key existed and deleted, false otherwise
BOOL IniFile::DeleteKey(CString keyname)
{
	int keynum = FindKey(keyname);
	if (keynum == -1)
		return 0;
	keys.RemoveAt(keynum);
	names.RemoveAt(keynum);
	return 1;
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

//overloaded from original getline to take CString
istream & IniFile:: getline(istream & is, CString & str)
{
    char buf[2048];
    is.getline(buf,2048);
    str = buf;
    return is;
}

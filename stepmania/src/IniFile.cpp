/*
-----------------------------------------------------------------------------
 File: IniFile.h

 Desc: Wrapper for reading and writing an .ini file.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "stdafx.h"
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
BOOL IniFile::ReadFile()
{
	CStdioFile file;
	if( !file.Open(path, CFile::modeRead) )
	{
		error = "Unable to open ini file.";
		return FALSE;
	}

	CString line;
	int curkey = -1, curval = -1;
	CString keyname, valuename, value;
	while( file.ReadString(line) )
	{
		if( line != "" )
		{
			if( line[0] == '[' && line[line.GetLength()-1] == ']' ) //if a section heading
			{
				keyname = line;
				keyname.TrimLeft('[');
				keyname.TrimRight(']');
			}
			else //if a value
			{
				int iIndexOfEqual = line.Find("=");
				if( iIndexOfEqual == -1 )	// this is a malformed line
					continue;
				valuename = line.Left( iIndexOfEqual );
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
	CStdioFile file;
	if( !file.Open(path, CFile::modeCreate | CFile::modeWrite ) )
	{
		error = "Unable to open ini for writing.";
		return;
	}

	// foreach key
	for( int keynum = 0; keynum <= names.GetUpperBound(); keynum++ )
	{
		CString sTemp;
		sTemp.Format( "[%s]\n", names[keynum] );
		file.WriteString( sTemp );

		CMapStringToString &map = keys[keynum];

		// for each value_name/value pair
		for( POSITION pos = map.GetStartPosition(); pos != NULL; )
		{
			CString value_name;
			CString value;
			map.GetNextAssoc( pos, value_name, value );

			sTemp.Format( "%s=%s\n", value_name, value );
			file.WriteString( sTemp );
		}
		file.WriteString( "\n" );
	}

	file.Close();
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
bool IniFile::GetValue(CString keyname, CString valuename, CString &value_out)
{
	int keynum = FindKey(keyname);//, valuenum = FindValue(keynum,valuename);
	if( keynum == -1 )
		return false;

	CMapStringToString &map = keys[keynum];
	return 1 == map.Lookup(valuename, value_out);
}

//gets value of [keyname] valuename = 
//overloaded to return CString, int, and double
bool IniFile::GetValueI(CString keyname, CString valuename, int &value_out)
{
	CString sValue;
	if( !GetValue(keyname, valuename, sValue) )
		return false;
	value_out = atoi(sValue);
	return true;
}

//gets value of [keyname] valuename = 
//overloaded to return CString, int, and double
bool IniFile::GetValueF(CString keyname, CString valuename, float &value_out)
{
	CString sValue;
	if( !GetValue(keyname, valuename, sValue) )
		return false;
	value_out = (float)atof(sValue);
	return true;
}

//gets value of [keyname] valuename = 
//overloaded to return CString, int, and double
bool IniFile::GetValueB(CString keyname, CString valuename, bool &value_out)
{
	CString sValue;
	if( !GetValue(keyname, valuename, sValue) )
		return false;
	value_out = sValue == "1";
	return true;
}

//sets value of [keyname] valuename =.
//specify the optional paramter as false (0) if you do not want it to create
//the key if it doesn't exist. Returns true if data entered, false otherwise
//overloaded to accept CString, int, and double
bool IniFile::SetValue(CString keyname, CString valuename, CString value, BOOL create)
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
bool IniFile::SetValueI(CString keyname, CString valuename, int value, BOOL create)
{
	CString temp;
	temp.Format("%d",value);
	return SetValue(keyname, valuename, temp, create);
}

//sets value of [keyname] valuename =.
//specify the optional paramter as false (0) if you do not want it to create
//the key if it doesn't exist. Returns true if data entered, false otherwise
//overloaded to accept CString, int, and double
bool IniFile::SetValueF(CString keyname, CString valuename, double value, BOOL create)
{
	CString temp;
	temp.Format("%e",value);
	return SetValue(keyname, valuename, temp, create);
}

//sets value of [keyname] valuename =.
//specify the optional paramter as false (0) if you do not want it to create
//the key if it doesn't exist. Returns true if data entered, false otherwise
//overloaded to accept CString, int, and double
bool IniFile::SetValueB(CString keyname, CString valuename, bool value, BOOL create)
{
	CString temp;
	temp.Format("%d",value);
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


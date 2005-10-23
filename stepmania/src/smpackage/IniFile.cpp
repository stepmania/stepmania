/*
-----------------------------------------------------------------------------
 File: IniFile.h

 Desc: Wrapper for reading and writing an .ini file.

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "stdafx.h"
#include "IniFile.h"
#include "RageUtil.h"


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


void IniFile::ReadBuf( const CString &buf )
{
	CString keyname;
	CStringArray lines;
	split(buf, "\n", lines, true);
	for( unsigned i = 0; i < lines.size(); ++i )
	{
		CString line = lines[i];

		if(line.GetLength() >= 3 &&
			line[0] == '\xef' &&
			line[1] == '\xbb' &&
			line[2] == '\xbf'
			)
		{
			/* Obnoxious NT marker for UTF-8.  Remove it. */
			line.Delete(0, 3);
		}

		StripCrnl(line);

		if( line.IsEmpty() )
			continue;
		if ((line.GetLength() >= 2 && line[0] == '/' && line[1] == '/') )
			continue; /* comment */
		if( line[0] == '#')
			continue; /* comment */

		if (line[0] == '[' && line[line.GetLength()-1] == ']') //if a section heading
		{
			keyname = line;
			keyname.Delete(0);
			keyname.Delete(keyname.GetLength()-1);
		}
		else //if a value
		{
			int iEqualIndex = line.Find("=");
			if( iEqualIndex != -1 )
			{
				CString valuename = line.Left(iEqualIndex);
				CString value = line.Right(line.GetLength()-valuename.GetLength()-1);
				SetValue(keyname,valuename,value);
			}
		}
	}
}

void IniFile::WriteBuf( CString &buf ) const
{
	buf = "";
	for (keymap::const_iterator k = keys.begin(); k != keys.end(); ++k)
	{
		if (k->second.empty())
			continue;

		buf += ssprintf( "[%s]\r\n", (const char *) k->first );

		for (key::const_iterator i = k->second.begin(); i != k->second.end(); ++i)
			buf += ssprintf( "%s=%s\r\n", (const char *)i->first, (const char *)i->second );

		buf += ssprintf( "\r\n" );
	}
}


// reads ini file specified using IniFile::SetPath()
// returns true if successful, false otherwise
bool IniFile::ReadFile()
{
	FILE *f = fopen(path, "rb");

	if (f == NULL)
	{
		error = ssprintf("Unable to open ini file: %s", strerror(errno));
		return 0;
	}

	int size = GetFileSizeInBytes(path);
	char *buf = new char[size+1];
	int ret = fread(buf, 1, size, f);
	if ( ret != size )
	{
		if( ferror(f) )
			error = ssprintf("Unable to read ini file: %s", strerror(errno));
		else
			error = ssprintf("Unexpected eof in INI");
		delete[] buf;
		fclose(f);	

		return false;
	}

	ReadBuf(buf);

	delete[] buf;
	fclose(f);	
	return true;
}

// writes data stored in class to ini file
void IniFile::WriteFile()
{
	CString buf;
	WriteBuf( buf );
	FILE* fp = fopen( path, "wb" );

	if( fp == NULL )
		return;
	
	fwrite(buf.GetBuffer(0), buf.GetLength(), 1, fp);
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
bool IniFile::GetValue(const CString &keyname, const CString &valuename, CString& value) const
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
bool IniFile::GetValueI(const CString &keyname, const CString &valuename, int& value) const
{
	CString sValue;
	if( !GetValue(keyname,valuename,sValue) )
		return false;
	sscanf( sValue, "%d", &value );
	return true;
}

bool IniFile::GetValueU(const CString &keyname, const CString &valuename, unsigned &value) const
{
	CString sValue;
	if( !GetValue(keyname,valuename,sValue) )
		return false;
	sscanf( sValue, "%u", &value );
	return true;
}

// gets value of [keyname] valuename = 
bool IniFile::GetValueF(const CString &keyname, const CString &valuename, float& value) const
{
	CString sValue;
	if( !GetValue(keyname,valuename,sValue) )
		return false;
	sscanf( sValue, "%f", &value );
	return true;
}

// gets value of [keyname] valuename = 
bool IniFile::GetValueB(const CString &keyname, const CString &valuename, bool& value) const
{
	CString sValue;
	if( !GetValue(keyname,valuename,sValue) )
		return false;
	value = atoi(sValue) != 0;
	return true;
}

// sets value of [keyname] valuename =.
// specify the optional paramter as false (0) if you do not want it to create
// the key if it doesn't exist. Returns true if data entered, false otherwise
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
bool IniFile::SetValueI(const CString &keyname, const CString &valuename, int value, bool create)
{
	return SetValue(keyname, valuename, ssprintf("%d",value), create);
}

bool IniFile::SetValueU(const CString &keyname, const CString &valuename, unsigned value, bool create)
{
	return SetValue(keyname, valuename, ssprintf("%u",value), create);
}

bool IniFile::SetValueF(const CString &keyname, const CString &valuename, float value, bool create)
{
	return SetValue(keyname, valuename, ssprintf("%f",value), create);
}

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

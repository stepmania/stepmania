#ifndef INIFILE_H
#define INIFILE_H
/*
-----------------------------------------------------------------------------
 Class: IniFile

 Desc: Wrapper for reading and writing an .ini file.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Adam Clauss
	Chris Danford
-----------------------------------------------------------------------------
*/

#include <map>

class IniFile  
{
public:
	// all keys are of this type

	typedef map<CString, CString> key;
	typedef map<CString, key> keymap;

	typedef keymap::const_iterator const_iterator;
	const_iterator begin() const { return keys.begin(); }
	const_iterator end() const { return keys.end(); }

private:
	//stores pathname of ini file to read/write
	CString path;

	// keys in ini
	keymap keys;


public:

	//will contain error info if one occurs
	//ended up not using much, just in ReadFile and GetValue
	CString error;

	//constructor, can specify pathname here instead of using SetPath later
	IniFile(CString inipath = "");

	//default destructor
	virtual ~IniFile();

	//sets path of ini file to read and write from
	void SetPath(CString newpath);
	CString GetPath() const { return path; }

	//reads ini file specified using IniFile::SetPath()
	//returns true if successful, false otherwise
	bool ReadFile();

	//writes data stored in class to ini file
	void WriteFile(); 

	//deletes all stored ini data
	void Reset();

	//returns number of keys currently in the ini
	int GetNumKeys() const;

	//returns number of values stored for specified key
	int GetNumValues(const CString &keyname) const;

	//gets value of [keyname] valuename = 
	//overloaded to return CString, int, and double,
	//returns "", or 0 if key/value not found.  Sets error member to show problem
	bool GetValue(const CString &key, const CString &valuename, CString& value);
	bool GetValueI(const CString &key, const CString &valuename, int& value);
	bool GetValueF(const CString &key, const CString &valuename, float& value);
	bool GetValueB(const CString &key, const CString &valuename, bool& value);

	//sets value of [keyname] valuename =.
	//specify the optional paramter as false (0) if you do not want it to create
	//the key if it doesn't exist. Returns true if data entered, false otherwise
	//overloaded to accept CString, int, and double
	bool SetValue(const CString &key, const CString &valuename, const CString &value, bool create = 1);
	bool SetValueI(const CString &key, const CString &valuename, int value, bool create = 1);
	bool SetValueF(const CString &key, const CString &valuename, float value, bool create = 1);
	bool SetValueB(const CString &key, const CString &valuename, bool value, bool create = 1);

	//deletes specified value
	//returns true if value existed and deleted, false otherwise
	bool DeleteValue(const CString &keyname, const CString &valuename);

	//deletes specified key and all values contained within
	//returns true if key existed and deleted, false otherwise
	bool DeleteKey(const CString &keyname);

	const key *GetKey(const CString &keyname) const;

	/* Rename a key. For example, call RenameKey("foo", "main") after
	 * reading an INI where [foo] is an alias to [main].  If to already
	 * exists, nothing happens. */
	void RenameKey(const CString &from, const CString &to);
};

#endif

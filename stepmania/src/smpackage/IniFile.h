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
using namespace std;

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
	mutable CString error;

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

	void ReadBuf( const CString &buf );
	void WriteBuf( CString &buf ) const;

	//deletes all stored ini data
	void Reset();

	//returns number of keys currently in the ini
	int GetNumKeys() const;

	//returns number of values stored for specified key
	int GetNumValues(const CString &keyname) const;

	//gets value of [keyname] valuename = 
	//returns "", or 0 if key/value not found.  Sets error member to show problem
	bool GetValue(const CString &key, const CString &valuename, CString& value) const;
	bool GetValueI(const CString &key, const CString &valuename, int& value) const;
	bool GetValueU(const CString &key, const CString &valuename, unsigned& value) const;
	bool GetValueF(const CString &key, const CString &valuename, float& value) const;
	bool GetValueB(const CString &key, const CString &valuename, bool& value) const;
	bool GetValue(const CString &key, const CString &valuename, int& value) const { return GetValueI(key, valuename, value); }
	bool GetValue(const CString &key, const CString &valuename, float& value) const { return GetValueF(key, valuename, value); }
	bool GetValue(const CString &key, const CString &valuename, bool& value) const { return GetValueB(key, valuename, value); }

	//sets value of [keyname] valuename =.
	//specify the optional paramter as false (0) if you do not want it to create
	//the key if it doesn't exist. Returns true if data entered, false otherwise
	bool SetValue(const CString &key, const CString &valuename, const CString &value, bool create = 1);
	bool SetValueI(const CString &key, const CString &valuename, int value, bool create = 1);
	bool SetValueU(const CString &key, const CString &valuename, unsigned value, bool create = 1);
	bool SetValueF(const CString &key, const CString &valuename, float value, bool create = 1);
	bool SetValueB(const CString &key, const CString &valuename, bool value, bool create = 1);
	bool SetValue(const CString &key, const CString &valuename, int value, bool create = 1) { return SetValueI(key, valuename, value, create); }

	//deletes specified value
	//returns true if value existed and deleted, false otherwise
	bool DeleteValue(const CString &keyname, const CString &valuename);

	//deletes specified key and all values contained within
	//returns true if key existed and deleted, false otherwise
	bool DeleteKey(const CString &keyname);

	const key *GetKey(const CString &keyname) const;
	void SetValue(const CString &keyname, const key &key);

	/* Rename a key. For example, call RenameKey("foo", "main") after
	 * reading an INI where [foo] is an alias to [main].  If to already
	 * exists, nothing happens. */
	void RenameKey(const CString &from, const CString &to);
};

#endif

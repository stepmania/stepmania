/* IniFile - Reading and writing .INI files. */

#ifndef INIFILE_H
#define INIFILE_H

#include <map>
using namespace std;

class RageFileBasic;

class IniKey : public map<CString, CString>
{
public:
	bool GetValue( const CString &valuename, CString& value ) const;
	bool GetValue( const CString &valuename, int& value ) const;
	bool GetValue( const CString &valuename, unsigned& value ) const;
	bool GetValue( const CString &valuename, float& value ) const;
	bool GetValue( const CString &valuename, bool& value ) const;

	bool SetValue( const CString &valuename, const CString &value );
	bool SetValue( const CString &valuename, int value );
	bool SetValue( const CString &valuename, unsigned value );
	bool SetValue( const CString &valuename, float value );
	bool SetValue( const CString &valuename, bool value );
};

class IniFile  
{
public:
	// all keys are of this type
	typedef IniKey key;
	typedef map<CString, key> keymap;

	typedef keymap::const_iterator const_iterator;
	const_iterator begin() const { return keys.begin(); }
	const_iterator end() const { return keys.end(); }

	typedef keymap::iterator iterator;
	iterator begin() { return keys.begin(); }
	iterator end() { return keys.end(); }

private:
	CString m_sPath;

	keymap keys;

	mutable CString m_sError;

public:
	/* Retrieve the filename of the last file loaded. */
	CString GetPath() const { return m_sPath; }
	const CString &GetError() const { return m_sError; }

	bool ReadFile( const CString &sPath );
	bool ReadFile( RageFileBasic &sFile );
	bool WriteFile( const CString &sPath );
	bool WriteFile( RageFileBasic &sFile );
	void Reset();

	int GetNumKeys() const;
	int GetNumValues( const CString &keyname ) const;

	bool GetValue( const CString &key, const CString &valuename, CString& value ) const;
	bool GetValue( const CString &key, const CString &valuename, int& value ) const;
	bool GetValue( const CString &key, const CString &valuename, unsigned& value ) const;
	bool GetValue( const CString &key, const CString &valuename, float& value ) const;
	bool GetValue( const CString &key, const CString &valuename, bool& value ) const;

	bool SetValue( const CString &key, const CString &valuename, const CString &value );
	bool SetValue( const CString &key, const CString &valuename, int value );
	bool SetValue( const CString &key, const CString &valuename, unsigned value );
	bool SetValue( const CString &key, const CString &valuename, float value );
	bool SetValue( const CString &key, const CString &valuename, bool value );

	bool DeleteKey( const CString &keyname );
	bool DeleteValue( const CString &keyname, const CString &valuename );

	const key *GetKey( const CString &keyname ) const;
	void SetValue( const CString &keyname, const key &key );

	/* Rename a key. For example, call RenameKey("foo", "main") after
	 * reading an INI where [foo] is an alias to [main].  If to already
	 * exists, nothing happens. */
	void RenameKey( const CString &from, const CString &to );
};

class IniFilePreserveOrder
{
public:
	// all keys are of this type

	typedef pair<CString, CString> key_value;
	typedef vector<key_value> key;
	typedef pair<CString, key> keymap_value;
	typedef vector<keymap_value> keymap;

	typedef keymap::const_iterator const_iterator;
	const_iterator begin() const { return keys.begin(); }
	const_iterator end() const { return keys.end(); }

	typedef keymap::iterator iterator;
	iterator begin() { return keys.begin(); }
	iterator end() { return keys.end(); }

private:
	CString m_sPath;

	keymap keys;

	mutable CString m_sError;

public:
	/* Retrieve the filename of the last file loaded. */
	CString GetPath() const { return m_sPath; }
	const CString &GetError() const { return m_sError; }

	bool ReadFile( const CString &sPath );
	bool WriteFile( const CString &sPath );

	bool SetValue( const CString &key, const CString &valuename, const CString &value );
};

#endif

/*
 * (c) 2001-2004 Adam Clauss, Chris Danford
 *
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

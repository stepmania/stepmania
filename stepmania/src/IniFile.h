/* IniFile - Reading and writing .INI files. */

#ifndef INIFILE_H
#define INIFILE_H

#include "XmlFile.h"
using namespace std;

class RageFileBasic;

class IniFile : public XNode
{
public:
	IniFile();

	/* Retrieve the filename of the last file loaded. */
	RString GetPath() const { return m_sPath; }
	const RString &GetError() const { return m_sError; }

	bool ReadFile( const RString &sPath );
	bool ReadFile( RageFileBasic &sFile );
	bool WriteFile( const RString &sPath ) const;
	bool WriteFile( RageFileBasic &sFile ) const;

	template <typename T>
	bool GetValue( const RString &sKey, const RString &sValueName, T& value ) const
	{
		const XNode* pNode = GetChild( sKey );
		if( pNode == NULL )
			return false;
		return pNode->GetAttrValue<T>( sValueName, value );
	}
	template <typename T>
	void SetValue( const RString &sKey, const RString &sValueName, const T &value )
	{
		XNode* pNode = GetChild( sKey );
		if( pNode == NULL )
			pNode = AppendChild( sKey );
		pNode->AppendAttr<T>( sValueName, value );
	}

	bool DeleteKey( const RString &keyname );
	bool DeleteValue( const RString &keyname, const RString &valuename );

	/* Rename a key. For example, call RenameKey("foo", "main") after
	 * reading an INI where [foo] is an alias to [main].  If to already
	 * exists, nothing happens. */
	bool RenameKey( const RString &from, const RString &to );

private:
	RString m_sPath;

	mutable RString m_sError;
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

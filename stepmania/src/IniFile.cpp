#include "global.h"
#include "IniFile.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageFile.h"

bool IniFile::ReadFile( const CString &sPath )
{
	m_sPath = sPath;
	CHECKPOINT_M( ssprintf("Reading '%s'",m_sPath.c_str()) );

	RageFile f;
	if( !f.Open( m_sPath ) )
	{
		LOG->Trace( "Reading '%s' failed: %s", m_sPath.c_str(), f.GetError().c_str() );
		m_sError = f.GetError();
		return 0;
	}

	return ReadFile( f );
}

bool IniFile::ReadFile( RageBasicFile &f )
{
	CString keyname;
	while( 1 )
	{
		CString line;
		switch( f.GetLine(line) )
		{
		case -1:
			m_sError = f.GetError();
			return false;
		case 0:
			return true; /* eof */
		}

		utf8_remove_bom( line );

		if( line == "" )
			continue;

		if( line.substr(0, 2) == "//" || line.substr(0) == "#" )
			continue; /* comment */

		if( line[0] == '[' && line[line.GetLength()-1] == ']'  )
		{
			/* New section. */
			keyname = line.substr(1, line.size()-2);
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

bool IniFile::WriteFile( const CString &sPath )
{
	RageFile f;
	if( !f.Open( sPath, RageFile::WRITE ) )
	{
		LOG->Trace( "Writing '%s' failed: %s", sPath.c_str(), f.GetError().c_str() );
		m_sError = f.GetError();
		return false;
	}

	return IniFile::WriteFile( f );
}

bool IniFile::WriteFile( RageBasicFile &f )
{
	for( keymap::const_iterator k = keys.begin(); k != keys.end(); ++k )
	{
		if( k->second.empty() )
			continue;

		if( f.PutLine( ssprintf("[%s]", k->first.c_str()) ) == -1 )
		{
			m_sError = f.GetError();
			return false;
		}


		for( key::const_iterator i = k->second.begin(); i != k->second.end(); ++i )
			f.PutLine( ssprintf("%s=%s", i->first.c_str(), i->second.c_str()) );

		if( f.PutLine( "" ) == -1 )
		{
			m_sError = f.GetError();
			return false;
		}
	}
	return true;
}

void IniFile::Reset()
{
	keys.clear();
}

int IniFile::GetNumKeys() const
{
	return keys.size();
}

int IniFile::GetNumValues( const CString &keyname ) const
{
	keymap::const_iterator k = keys.find(keyname);
	if( k == keys.end() )
		return -1;

	return k->second.size();
}

bool IniFile::GetValue( const CString &keyname, const CString &valuename, CString& value ) const
{
	keymap::const_iterator k = keys.find(keyname);
	if (k == keys.end())
		return false;

	key::const_iterator i = k->second.find(valuename);

	if( i == k->second.end() )
		return false;

	value = i->second;
	return true;
}

bool IniFile::SetValue( const CString &keyname, const CString &valuename, const CString &value )
{
	keys[keyname][valuename] = value;
	return true;
}

#define TYPE(T) \
	bool IniFile::GetValue( const CString &keyname, const CString &valuename, T &value ) const \
	{ \
		CString sValue; \
		if( !GetValue(keyname,valuename,sValue) ) \
			return false; \
		return FromString( sValue, value ); \
	} \
	bool IniFile::SetValue( const CString &keyname, const CString &valuename, T value ) \
	{ \
		return SetValue( keyname, valuename, ToString(value) ); \
	}

TYPE(int);
TYPE(unsigned);
TYPE(float);
TYPE(bool);

bool IniFile::DeleteValue(const CString &keyname, const CString &valuename)
{
	keymap::iterator k = keys.find(keyname);
	if( k == keys.end() )
		return false;

	key::iterator i = k->second.find(valuename);
	if( i == k->second.end() )
		return false;

	k->second.erase(i);
	return true;
}

bool IniFile::DeleteKey(const CString &keyname)
{
	keymap::iterator k = keys.find(keyname);
	if( k == keys.end() )
		return false;

	keys.erase(k);
	return true;
}

const IniFile::key *IniFile::GetKey(const CString &keyname) const
{
	keymap::const_iterator i = keys.find(keyname);
	if( i == keys.end() )
		return NULL;
	return &i->second;
}

void IniFile::SetValue(const CString &keyname, const key &key)
{
	keys[keyname] = key;
}

void IniFile::RenameKey(const CString &from, const CString &to)
{
	if( keys.find(from) == keys.end() )
		return;
	if( keys.find(to) != keys.end() )
		return;

	keys[to] = keys[from];
	keys.erase(from);
}










bool IniFilePreserveOrder::ReadFile( const CString &sPath )
{
	m_sPath = sPath;
	CHECKPOINT_M( ssprintf("Reading '%s'",m_sPath.c_str()) );

	RageFile f;
	if( !f.Open( m_sPath ) )
	{
		LOG->Trace( "Reading '%s' failed: %s", m_sPath.c_str(), f.GetError().c_str() );
		m_sError = f.GetError();
		return 0;
	}

	CString keyname;
	while( 1 )
	{
		CString line;
		switch( f.GetLine(line) )
		{
		case -1:
			m_sError = f.GetError();
			return false;
		case 0:
			return true; /* eof */
		}

		if( line.size() >= 3 &&
			line[0] == '\xef' &&
			line[1] == '\xbb' &&
			line[2] == '\xbf'
			)
		{
			/* Obnoxious NT marker for UTF-8.  Remove it. */
			line.erase(0, 3);
		}

		if( line == "" )
			continue;

		if( line.substr(0, 2) == "//" || line.substr(0) == "#" )
			continue; /* comment */

		if( line[0] == '[' && line[line.GetLength()-1] == ']'  )
		{
			/* New section. */
			keyname = line.substr(1, line.size()-2);
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

bool IniFilePreserveOrder::WriteFile( const CString &sPath )
{
	RageFile f;
	if( !f.Open( sPath, RageFile::WRITE ) )
	{
		LOG->Trace( "Writing '%s' failed: %s", sPath.c_str(), f.GetError().c_str() );
		m_sError = f.GetError();
		return false;
	}

	for( keymap::const_iterator k = keys.begin(); k != keys.end(); ++k )
	{
		if (k->second.empty())
			continue;

		if( f.PutLine( ssprintf("[%s]", k->first.c_str()) ) == -1 )
		{
			m_sError = f.GetError();
			return false;
		}


		for( key::const_iterator i = k->second.begin(); i != k->second.end(); ++i )
			f.PutLine( ssprintf("%s=%s", i->first.c_str(), i->second.c_str()) );

		if( f.PutLine( "" ) == -1 )
		{
			m_sError = f.GetError();
			return false;
		}
	}
	return true;
}

bool IniFilePreserveOrder::SetValue( const CString &keyname, const CString &valuename, const CString &value )
{
	int iIndexOfKey = -1;
	for( unsigned i=0; i<keys.size(); i++ )
	{
		if( keys[i].first == keyname )
		{
			iIndexOfKey = i;
			break;
		}
	}
	if( iIndexOfKey == -1 )
	{
		iIndexOfKey = keys.size();
		keys.resize( keys.size()+1 );
		keys.back().first = keyname;
	}

	key &k = keys[iIndexOfKey].second;

	int iIndexOfValue = -1;
	for( unsigned i=0; i<k.size(); i++ )
	{
		if( k[i].first == valuename )
		{
			iIndexOfValue = i;
			break;
		}
	}
	if( iIndexOfValue == -1 )
	{
		iIndexOfValue = k.size();
		k.resize( k.size()+1 );
		k.back().first = valuename;
	}

	k[iIndexOfValue].second = value;

	return true;
}



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

#include "global.h"
#include "IniFile.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageFile.h"
#include "Foreach.h"

IniFile::IniFile()
{
	m_sName = "IniFile";
}

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

bool IniFile::ReadFile( RageFileBasic &f )
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

		if( line.size() == 0 )
			continue;
		if( line[0] == '#' )
			continue; /* comment */
		if( line.size() > 1 && line[0] == '/' && line[1] == '/' )
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
				if( keyname.size() && valuename.size() )
					SetValue(keyname,valuename,value);
			}
		}
	}
}

bool IniFile::WriteFile( const CString &sPath ) const
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

bool IniFile::WriteFile( RageFileBasic &f ) const
{
	FOREACH_CONST_Child( this, pKey ) 
	{
		if( f.PutLine( ssprintf("[%s]", pKey->m_sName.c_str()) ) == -1 )
		{
			m_sError = f.GetError();
			return false;
		}

		FOREACH_CONST_Attr( pKey, pAttr )
		{
			if( f.PutLine( ssprintf("%s=%s", pAttr->m_sName.c_str(), pAttr->m_sValue.c_str()) ) == -1 )
			{
				m_sError = f.GetError();
				return false;
			}
		}

		if( f.PutLine( "" ) == -1 )
		{
			m_sError = f.GetError();
			return false;
		}
	}
	return true;
}

bool IniFile::GetValue( const CString &keyname, const CString &valuename, CString& value ) const
{
	const XNode* pNode = GetChild( keyname );
	if( pNode == NULL )
		return false;
	return pNode->GetAttrValue( valuename, value );
}

void IniFile::SetValue( const CString &keyname, const CString &valuename, const CString &value )
{
	XNode* pNode = GetChild( keyname );
	if( pNode == NULL )
		pNode = AppendChild( keyname );
	pNode->SetAttrValue( valuename, value );
}

bool IniFile::DeleteValue(const CString &keyname, const CString &valuename)
{
	XNode* pNode = GetChild( keyname );
	if( pNode == NULL )
		return false;
	XAttr* pAttr = pNode->GetAttr( valuename );
	if( pAttr == NULL )
		return false;
	return pNode->RemoveAttr( pAttr );
}


#define TYPE(T) \
	bool IniFile::GetValue( const CString &keyname, const CString &valuename, T &value ) const \
	{ \
		CString sValue; \
		if( !GetValue(keyname,valuename,sValue) ) \
			return false; \
		return FromString( sValue, value ); \
	} \
	void IniFile::SetValue( const CString &keyname, const CString &valuename, T value ) \
	{ \
		SetValue( keyname, valuename, ToString(value) ); \
	}

TYPE(int);
TYPE(unsigned);
TYPE(float);
TYPE(bool);


bool IniFile::DeleteKey(const CString &keyname)
{
	XNode* pNode = GetChild( keyname );
	if( pNode == NULL )
		return false;
	return RemoveChild( pNode );
}

bool IniFile::RenameKey(const CString &from, const CString &to)
{
	/* If to already exists, do nothing. */
	XNode* pTo = GetChild( to );
	if( pTo )
		return false;

	multimap<CString, XNode*>::iterator it = m_childs.find( from );
	if( it == m_childs.end() )
		return false;

	XNode* pNode = it->second;

	m_childs.erase( it );

	pNode->m_sName = to;
	m_childs.insert( make_pair(pNode->m_sName, pNode) );

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

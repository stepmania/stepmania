/* 
http://en.wikipedia.org/wiki/INI_file
 - names and values are trimmed on both sides
 - semicolons start a comment line
 - backslash followed by a newline doesn't break the line
*/
#include "global.h"
#include "IniFile.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageFile.h"


IniFile::IniFile(): XNode("IniFile")
{
}

bool IniFile::ReadFile( const RString &sPath )
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
	RString keyname;
	// keychild is used to cache the node that values are being added to. -Kyz
	XNode* keychild= nullptr;
	for(;;)
	{
		RString line;
		// Read lines until we reach a line that doesn't end in a backslash
		for(;;)
		{
			RString s;
			switch( f.GetLine(s) )
			{
			case -1:
				m_sError = f.GetError();
				return false;
			case 0:
				return true; // eof
			}

			utf8_remove_bom( s );

			line += s;

			if( line.empty() || line[line.size()-1] != '\\' )
			{
				break;
			}
			line.erase( line.end()-1 );
		}


		if( line.empty() )
			continue;
		switch(line[0])
		{
			case ';':
			case '#':
				continue; // comment
			case '/':
			case '-':
				if(line.size() > 1 && line[0] == line[1])
				{ continue; } // comment (Lua or C++ style)
				goto keyvalue;
			case '[':
				if(line[line.size()-1] == ']')
				{
					// New section.
					keyname = line.substr(1, line.size()-2);
					keychild= GetChild(keyname);
					if(keychild == nullptr)
					{
						keychild= AppendChild(keyname);
					}
					break;
				}
			default:
			keyvalue:
				if(keychild == nullptr)
				{ break; }
				// New value.
				size_t iEqualIndex = line.find("=");
				if( iEqualIndex != string::npos )
				{
					RString valuename = line.Left((int) iEqualIndex);
					RString value = line.Right(line.size()-valuename.size()-1);
					Trim(valuename);
					if(!valuename.empty())
					{
						SetKeyValue(keychild, valuename, value);
					}
				}
				break;
		}
	}
}

bool IniFile::WriteFile( const RString &sPath ) const
{
	RageFile f;
	if( !f.Open( sPath, RageFile::WRITE ) )
	{
		LOG->Warn( "Writing '%s' failed: %s", sPath.c_str(), f.GetError().c_str() );
		m_sError = f.GetError();
		return false;
	}

	bool bSuccess = IniFile::WriteFile( f );
	int iFlush = f.Flush();
	bSuccess &= (iFlush != -1);
	return bSuccess;
}

bool IniFile::WriteFile( RageFileBasic &f ) const
{
	FOREACH_CONST_Child( this, pKey ) 
	{
		if( f.PutLine( ssprintf("[%s]", pKey->GetName().c_str()) ) == -1 )
		{
			m_sError = f.GetError();
			return false;
		}

		FOREACH_CONST_Attr( pKey, pAttr )
		{
			const RString &sName = pAttr->first;
			const RString &sValue = pAttr->second->GetValue<RString>();

			// TODO: Are there escape rules for these?
			// take a cue from how multi-line Lua functions are parsed
			DEBUG_ASSERT( sName.find('\n') == sName.npos );
			DEBUG_ASSERT( sName.find('=') == sName.npos );

			if( f.PutLine( ssprintf("%s=%s", sName.c_str(), sValue.c_str()) ) == -1 )
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

bool IniFile::DeleteValue(const RString &keyname, const RString &valuename)
{
	XNode* pNode = GetChild( keyname );
	if( pNode == nullptr )
		return false;
	return pNode->RemoveAttr( valuename );
}


bool IniFile::DeleteKey(const RString &keyname)
{
	XNode* pNode = GetChild( keyname );
	if( pNode == nullptr )
		return false;
	return RemoveChild( pNode );
}

bool IniFile::RenameKey(const RString &from, const RString &to)
{
	// If to already exists, do nothing.
	if( GetChild(to) != nullptr )
		return false;

	XNode* pNode = GetChild( from );
	if( pNode == nullptr )
		return false;

	pNode->SetName( to );
	RenameChildInByName(pNode);

	return true;
}


/*
 * (c) 2001-2004 Adam Clauss, Chris Danford
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

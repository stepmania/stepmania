#include "global.h"
#include "XmlFileUtil.h"
#include "XmlFile.h"
#include "RageFile.h"
#include "RageFileDriverMemory.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "arch/Dialog/Dialog.h"

bool XmlFileUtil::LoadFromFileShowErrors( XNode &xml, RageFileBasic &f )
{
	RString sError;
	RString s;
	if( f.Read( s ) == -1 )
		sError = f.GetError();
	else
		Load( &xml, s, sError );
	if( sError.empty() )
		return true;

	RString sWarning = ssprintf( "XML: LoadFromFile failed: %s", sError.c_str() );
	LOG->Warn( sWarning );
	Dialog::OK( sWarning, "XML_PARSE_ERROR" );
	return false;
}


bool XmlFileUtil::LoadFromFileShowErrors( XNode &xml, const RString &sFile )
{
	RageFile f;
	if( !f.Open(sFile, RageFile::READ) )
	{
		LOG->Warn("Couldn't open %s for reading: %s", sFile.c_str(), f.GetError().c_str() );
		return false;
	}

	bool bSuccess = LoadFromFileShowErrors( xml, f );
	if( !bSuccess )
	{
		RString sWarning = ssprintf( "XML: LoadFromFile failed for file: %s", sFile.c_str() );
		LOG->Warn( sWarning );
		Dialog::OK( sWarning, "XML_PARSE_ERROR" );
	}
	return bSuccess;
}

static const char chXMLTagOpen		= '<';
static const char chXMLTagClose		= '>';
static const char chXMLQuestion		= '?';	// used in checking for meta tags: "<?TAG ... ?/>"
static const char chXMLTagPre		= '/';
static const char chXMLExclamation	= '!';
static const char chXMLDash			= '-';

static map<RString,RString> g_mapEntitiesToChars;
static map<char,RString> g_mapCharsToEntities;

// XXX not called
static void InitEntities()
{
	if( !g_mapEntitiesToChars.empty() )
		return;

	static struct Entity
	{
		char c;
		const char *pEntity;
	}
	const EntityTable[] =
	{
		{ '&',  "amp", },
		{ '\"', "quot", },
		{ '\'', "apos", },
		{ '<',  "lt", },
		{ '>',  "gt", } 
	};

	for( unsigned i = 0; i < ARRAYLEN(EntityTable); ++i )
	{
		const Entity &ent = EntityTable[i];
		g_mapEntitiesToChars[ent.pEntity] = RString(1, ent.c);
		g_mapCharsToEntities[ent.c] = ent.pEntity;
	}
}


// skip spaces
static void tcsskip( const RString &s, unsigned &i )
{
	i = s.find_first_not_of( " \t\r\n", i );
}

static bool XIsEmptyString( const RString &s )
{
	return s.find_first_not_of( "\r\n\t " ) == s.npos;
}

// put string of (psz~end) on ps string
static void SetString( const RString &s, int iStart, int iEnd, RString* ps, bool trim = false )
{
	if( trim )
	{
		while( iStart < iEnd && s[iStart] > 0 && isspace(s[iStart]) )
			iStart++;
		while( iEnd-1 >= iStart && s[iEnd-1] > 0 && isspace(s[iEnd-1]) )
			iEnd--;
	}

	int len = iEnd - iStart;
	if( len <= 0 )
		return;

	ps->assign( s, iStart, len );
}


// attr1="value1" attr2='value2' attr3=value3 />
//                                            ^- return pointer
// Desc   : loading attribute plain xml text
// Param  : pszAttrs - xml of attributes
//          pi = parser information
// Return : advanced string pointer. (error return npos)
namespace
{
unsigned LoadAttributes( XNode *pNode, const RString &xml, RString &sErrorOut, unsigned iOffset )
{
	while( iOffset < xml.size() )
	{
		tcsskip( xml, iOffset );
		if( iOffset >= xml.size()  )
			continue;

		// close tag
		if( iOffset < xml.size() &&
			(xml[iOffset] == chXMLTagClose || xml[iOffset] == chXMLTagPre || xml[iOffset] == chXMLQuestion || xml[iOffset] == chXMLDash) )
			return iOffset; // well-formed tag

		// XML Attr Name
		unsigned iEnd = xml.find_first_of( " =", iOffset );
		if( iEnd == xml.npos ) 
		{
			// error
			if( sErrorOut.empty() ) 
				sErrorOut = ssprintf( "<%s> attribute has error ", pNode->GetName().c_str() );
			return string::npos;
		}
		
		// XML Attr Name
		RString sName;
		SetString( xml, iOffset, iEnd, &sName );
		
		// add new attribute
		DEBUG_ASSERT( sName.size() );
		pair<XAttrs::iterator,bool> it = pNode->m_attrs.insert( make_pair(sName, RString()) );
		RString &sValue = it.first->second;
		iOffset = iEnd;
		
		// XML Attr Value
		tcsskip( xml, iOffset );

		if( iOffset < xml.size() && xml[iOffset] == '=' )
		{
			++iOffset;

			tcsskip( xml, iOffset );
			if( iOffset >= xml.size()  )
				continue;

			// if " or '
			// or none quote
			char quote = xml[iOffset];
			if( quote == '"' || quote == '\'' )
			{
				++iOffset;
				iEnd = xml.find( quote, iOffset );
			}
			else
			{
				//attr= value> 
				// none quote mode
				iEnd = xml.find_first_of( " >", iOffset );
			}

			if( iEnd == xml.npos ) 
			{
				// error
				if( sErrorOut.empty() ) 
					sErrorOut = ssprintf( "<%s> attribute text: couldn't find matching quote", sName.c_str() );
				return string::npos;
			}

			SetString( xml, iOffset, iEnd, &sValue, true );
			iOffset = iEnd;
			// ATTRVALUE 
			ReplaceEntityText( sValue, g_mapEntitiesToChars );

			if( quote == '"' || quote == '\'' )
				++iOffset;
		}
	}

	// not well-formed tag
	return string::npos;
}

}



// <TAG attr1="value1" attr2='value2' attr3=value3 >
// </TAG>
// or
// <TAG />
//        ^- return pointer
// Desc   : load xml plain text
// Param  : pszXml - plain xml text
//          pi = parser information
// Return : advanced string pointer  (error return npos)
unsigned XmlFileUtil::Load( XNode *pNode, const RString &xml, RString &sErrorOut, unsigned iOffset )
{
	pNode->Clear();

	// <
	iOffset = xml.find( chXMLTagOpen, iOffset );
	if( iOffset == string::npos )
		return string::npos;

	// </
	if( xml[iOffset+1] == chXMLTagPre )
		return iOffset;

	/* <!-- */
	if( !xml.compare(iOffset+1, 3, "!--") )
	{
		iOffset += 4;

		/* Find the close tag. */
		unsigned iEnd = xml.find( "-->", iOffset );
		if( iEnd == string::npos )
		{
			if( sErrorOut.empty() ) 
				sErrorOut = "Unterminated comment";

			return string::npos;
		}

		// Skip -->.
		iOffset = iEnd + 3;

		return Load( pNode, xml, sErrorOut, iOffset );
	}

	// XML Node Tag Name Open
	iOffset++;
	unsigned iTagEnd = xml.find_first_of( " \t\r\n/>", iOffset );
	SetString( xml, iOffset, iTagEnd, &pNode->m_sName );
	iOffset = iTagEnd;

	// Generate XML Attributte List
	iOffset = LoadAttributes( pNode, xml, sErrorOut, iOffset );
	if( iOffset == string::npos )
		return string::npos;

	// alone tag <TAG ... /> or <?TAG ... ?> or <!-- ... --> 
	// current pointer:   ^               ^              ^

	if( iOffset < xml.size() && (xml[iOffset] == chXMLTagPre || xml[iOffset] == chXMLQuestion || xml[iOffset] == chXMLDash) )
	{
		iOffset++;

		// skip over 2nd dash
		if( iOffset < xml.size() && xml[iOffset] == chXMLDash )
			iOffset++;

		if( iOffset == xml.size() || xml[iOffset] != chXMLTagClose )
		{
			// error: <TAG ... / >
			if( sErrorOut.empty() ) 
				sErrorOut = "Element must be closed.";

			// ill-formed tag
			return string::npos;
		}

		// well-formed tag
		++iOffset;

		// UGLY: We want to ignore all XML meta tags.  So, since the Node we 
		// just loaded is a meta tag, then Load ourself again using the rest 
		// of the file until we reach a non-meta tag.
		if( !pNode->GetName().empty() && (pNode->GetName()[0] == chXMLQuestion || pNode->GetName()[0] == chXMLExclamation) )
			iOffset = Load( pNode, xml, sErrorOut, iOffset );

		return iOffset;
	}

	// open/close tag <TAG ..> ... </TAG>
	//                        ^- current pointer
	if( XIsEmptyString(pNode->m_sValue) )
	{
		// Text Value 
		++iOffset;
		unsigned iEnd = xml.find( chXMLTagOpen, iOffset );
		if( iEnd == string::npos )
		{
			if( sErrorOut.empty() ) 
				sErrorOut = ssprintf( "%s must be closed with </%s>", pNode->GetName().c_str(), pNode->GetName().c_str() );
			// error cos not exist CloseTag </TAG>
			return string::npos;
		}
		
		SetString( xml, iOffset, iEnd, &pNode->m_sValue, true );

		iOffset = iEnd;
		// TEXTVALUE reference
		ReplaceEntityText( pNode->m_sValue, g_mapEntitiesToChars );
	}

	// generate child nodes
	while( iOffset < xml.size() )
	{
		XNode *node = new XNode;
		
		iOffset = Load( node, xml, sErrorOut, iOffset );
		if( !node->GetName().empty() )
		{
			DEBUG_ASSERT( node->GetName().size() );
			pNode->m_childs.insert( make_pair(node->GetName(), node) );
		}
		else
		{
			delete node;
		}

		// open/close tag <TAG ..> ... </TAG>
		//                             ^- current pointer
		// CloseTag case
		if( iOffset+1 < xml.size() && xml[iOffset] == chXMLTagOpen && xml[iOffset+1] == chXMLTagPre )
		{
			// </Close>
			iOffset += 2; // C
			
			tcsskip( xml, iOffset );
			if( iOffset >= xml.size()  )
				continue;

			unsigned iEnd = xml.find_first_of( " >", iOffset );
			if( iEnd == string::npos )
			{
				if( sErrorOut.empty() ) 
					sErrorOut = ssprintf( "it must be closed with </%s>", pNode->GetName().c_str() );
				// error
				return string::npos;
			}

			RString closename;
			SetString( xml, iOffset, iEnd, &closename );
			iOffset = iEnd+1;
			if( closename == pNode->GetName() )
			{
				// wel-formed open/close
				// return '>' or ' ' after pointer
				return iOffset;
			}
			else
			{
				// not welformed open/close
				if( sErrorOut.empty() ) 
					sErrorOut = ssprintf( "'<%s> ... </%s>' is not well-formed.", pNode->GetName().c_str(), closename.c_str() );
				return string::npos;
			}
		}
		else	// Alone child Tag Loaded
		{
			if( XIsEmptyString(pNode->m_sValue) && iOffset < xml.size() && xml[iOffset] != chXMLTagOpen )
			{
				// Text Value 
				unsigned iEnd = xml.find( chXMLTagOpen, iOffset );
				if( iEnd == string::npos ) 
				{
					// error cos not exist CloseTag </TAG>
					if( sErrorOut.empty() )  
						sErrorOut = ssprintf( "it must be closed with </%s>", pNode->GetName().c_str() );
					return string::npos;
				}
				
				SetString( xml, iOffset, iEnd, &pNode->m_sValue, true );

				iOffset = iEnd;
				//TEXTVALUE
				ReplaceEntityText( pNode->m_sValue, g_mapEntitiesToChars );
			}
		}
	}

	return iOffset;
}

namespace
{
bool GetAttrXML( RageFileBasic &f, const RString &sName, const RString &sValue )
{
	RString s(sValue);
	ReplaceEntityText( s, g_mapCharsToEntities );
	return f.Write(sName + "='" + s + "' ") != -1;
}

bool GetXMLInternal( const XNode *pNode, RageFileBasic &f, bool bWriteTabs, int &iTabBase )
{
	// tab
	if( f.Write("\r\n") == -1 )
		return false;
	if( bWriteTabs )
		for( int i = 0 ; i < iTabBase ; i++)
			if( f.Write("\t") == -1 )
				return false;

	// <TAG
	if( f.Write("<" + pNode->GetName()) == -1 )
		return false;

	// <TAG Attr1="Val1" 
	if( !pNode->m_attrs.empty() )
		if( f.Write(" ") == -1 )
			return false;
	FOREACH_CONST_Attr( pNode, p )
		if( !GetAttrXML(f, p->first, p->second) )
			return false;
	
	if( pNode->m_childs.empty() && pNode->m_sValue.empty() )
	{
		// <TAG Attr1="Val1"/> alone tag 
		if( f.Write("/>") == -1 )
			return false;
	}
	else
	{
		// <TAG Attr1="Val1"> and get child
		if( f.Write(">") == -1 )
			return false;
			
		if( !pNode->m_childs.empty() )
			iTabBase++;

		FOREACH_CONST_Child( pNode, p )
			if( !GetXMLInternal( pNode, f, bWriteTabs, iTabBase ) )
				return false;
		
		// Text Value
		if( !pNode->m_sValue.empty() )
		{
			if( !pNode->m_childs.empty() )
			{
				if( f.Write("\r\n") == -1 )
					return false;
				if( bWriteTabs )
					for( int i = 0 ; i < iTabBase ; i++)
						if( f.Write("\t") == -1 )
							return false;
			}
			RString s( pNode->m_sValue );
			ReplaceEntityText( s, g_mapCharsToEntities );
			if( f.Write(s) == -1 )
				return false;
		}

		// </TAG> CloseTag
		if( !pNode->m_childs.empty() )
		{
			if( f.Write("\r\n") == -1 )
				return false;
			if( bWriteTabs )
				for( int i = 0 ; i < iTabBase-1 ; i++)
					if( f.Write("\t") == -1 )
						return false;
		}
		if( f.Write("</" + pNode->GetName() + ">") == -1 )
			return false;

		if( !pNode->m_childs.empty() )
			iTabBase--;
	}
	return true;
}
}

bool XmlFileUtil::GetXML( const XNode *pNode, RageFileBasic &f, bool bWriteTabs )
{
	int iTabBase = 0;
	return GetXMLInternal( pNode, f, bWriteTabs, iTabBase );
}

RString XmlFileUtil::GetXML( const XNode *pNode )
{
	RageFileObjMem f;
	int iTabBase = 0;
	GetXMLInternal( pNode, f, true, iTabBase );
	return f.GetString();
}

bool XmlFileUtil::SaveToFile( const XNode *pNode, RageFileBasic &f, const RString &sStylesheet, bool bWriteTabs )
{
	f.PutLine( "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" );
	if( !sStylesheet.empty() )
		f.PutLine( "<?xml-stylesheet type=\"text/xsl\" href=\"" + sStylesheet + "\"?>" );
	int iTabBase = 0;
	if( !GetXMLInternal(pNode, f, bWriteTabs, iTabBase) )
		return false;
	f.PutLine( "" );
	if( f.Flush() == -1 )
		return false;
	return true;
}

bool XmlFileUtil::SaveToFile( const XNode *pNode, const RString &sFile, const RString &sStylesheet, bool bWriteTabs )
{
	RageFile f;
	if( !f.Open(sFile, RageFile::WRITE) )
	{
		LOG->Warn( "Couldn't open %s for writing: %s", sFile.c_str(), f.GetError().c_str() );
		return false;
	}

	return SaveToFile( pNode, f, sStylesheet, bWriteTabs );
}

/*
 * (c) 2001-2004 Chris Danford
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

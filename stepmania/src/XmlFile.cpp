// Adapted from http://www.codeproject.com/cpp/xmlite.asp.
// On 2004-02-09 Cho, Kyung-Min gave us permission to use and modify this 
// library.
//
// XmlFile : XML Lite Parser Library
// by Cho, Kyung Min: bro@shinbiro.com 2002-10-30

#include "global.h"
#include "XmlFile.h"
#include "RageFile.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "DateTime.h"
#include "Foreach.h"
#include "RageFileDriverMemory.h"

static inline long XStr2Int( const char* str, long default_value = 0 )
{
	return str ? atol(str) : default_value;
}

static const char chXMLTagOpen		= '<';
static const char chXMLTagClose		= '>';
static const char chXMLQuestion		= '?';	// used in checking for meta tags: "<?TAG ... ?/>"
static const char chXMLTagPre		= '/';
static const char chXMLExclamation	= '!';
static const char chXMLDash			= '-';


static map<RString,RString> g_mapEntitiesToChars;
static map<char,RString> g_mapCharsToEntities;

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

static struct RunInitEntities
{
	RunInitEntities() { InitEntities(); }
} g_RunInitEntities;

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

XNode::XNode( const XNode &cpy ):
	m_sName( cpy.m_sName ),
	m_sValue( cpy.m_sValue ),
	m_attrs( cpy.m_attrs )
{
	FOREACH_CONST_Child( &cpy, c )
		this->AppendChild( new XNode(*c) );
}

XNode::~XNode()
{
	Clear();
}

void XNode::Clear()
{
	FOREACH_Child( this, p )
		SAFE_DELETE( p );
	m_childs.clear();
	m_attrs.clear();
}
	
// attr1="value1" attr2='value2' attr3=value3 />
//                                            ^- return pointer
// Desc   : loading attribute plain xml text
// Param  : pszAttrs - xml of attributes
//          pi = parser information
// Return : advanced string pointer. (error return npos)
unsigned XNode::LoadAttributes( const RString &xml, PARSEINFO *pi, unsigned iOffset )
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
			if( pi->error_string.empty() ) 
				pi->error_string = ssprintf( "<%s> attribute has error ", m_sName.c_str() );
			return string::npos;
		}
		
		// XML Attr Name
		RString sName;
		SetString( xml, iOffset, iEnd, &sName );
		
		// add new attribute
		DEBUG_ASSERT( sName.size() );
		pair<XAttrs::iterator,bool> it = m_attrs.insert( make_pair(sName, RString()) );
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
				if( pi->error_string.empty() ) 
					pi->error_string = ssprintf( "<%s> attribute text: couldn't find matching quote", sName.c_str() );
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

// <TAG attr1="value1" attr2='value2' attr3=value3 >
// </TAG>
// or
// <TAG />
//        ^- return pointer
// Desc   : load xml plain text
// Param  : pszXml - plain xml text
//          pi = parser information
// Return : advanced string pointer  (error return npos)
unsigned XNode::Load( const RString &xml, PARSEINFO *pi, unsigned iOffset )
{
	Clear();

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
			if( pi->error_string.empty() ) 
				pi->error_string = "Unterminated comment";

			return string::npos;
		}

		// Skip -->.
		iOffset = iEnd + 3;

		return Load( xml, pi, iOffset );
	}

	// XML Node Tag Name Open
	iOffset++;
	unsigned iTagEnd = xml.find_first_of( " \t\r\n/>", iOffset );
	SetString( xml, iOffset, iTagEnd, &m_sName );
	iOffset = iTagEnd;

	// Generate XML Attributte List
	iOffset = LoadAttributes( xml, pi, iOffset );
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
			if( pi->error_string.empty() ) 
				pi->error_string = "Element must be closed.";

			// ill-formed tag
			return string::npos;
		}

		// well-formed tag
		++iOffset;

		// UGLY: We want to ignore all XML meta tags.  So, since the Node we 
		// just loaded is a meta tag, then Load ourself again using the rest 
		// of the file until we reach a non-meta tag.
		if( !m_sName.empty() && (m_sName[0] == chXMLQuestion || m_sName[0] == chXMLExclamation) )
			iOffset = Load( xml, pi, iOffset );

		return iOffset;
	}

	// open/close tag <TAG ..> ... </TAG>
	//                        ^- current pointer
	if( XIsEmptyString( m_sValue ) )
	{
		// Text Value 
		++iOffset;
		unsigned iEnd = xml.find( chXMLTagOpen, iOffset );
		if( iEnd == string::npos )
		{
			if( pi->error_string.empty() ) 
				pi->error_string = ssprintf( "%s must be closed with </%s>", m_sName.c_str(), m_sName.c_str() );
			// error cos not exist CloseTag </TAG>
			return string::npos;
		}
		
		SetString( xml, iOffset, iEnd, &m_sValue, true );

		iOffset = iEnd;
		// TEXTVALUE reference
		ReplaceEntityText( m_sValue, g_mapEntitiesToChars );
	}

	// generate child nodes
	while( iOffset < xml.size() )
	{
		XNode *node = new XNode;
		
		iOffset = node->Load( xml, pi, iOffset );
		if( !node->m_sName.empty() )
		{
			DEBUG_ASSERT( node->m_sName.size() );
			m_childs.insert( make_pair(node->m_sName, node) );
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
				if( pi->error_string.empty() ) 
					pi->error_string = ssprintf( "it must be closed with </%s>", m_sName.c_str() );
				// error
				return string::npos;
			}

			RString closename;
			SetString( xml, iOffset, iEnd, &closename );
			iOffset = iEnd+1;
			if( closename == this->m_sName )
			{
				// wel-formed open/close
				// return '>' or ' ' after pointer
				return iOffset;
			}
			else
			{
				// not welformed open/close
				if( pi->error_string.empty() ) 
					pi->error_string = ssprintf( "'<%s> ... </%s>' is not well-formed.", m_sName.c_str(), closename.c_str() );
				return string::npos;
			}
		}
		else	// Alone child Tag Loaded
		{
			if( XIsEmptyString( m_sValue ) && iOffset < xml.size() && xml[iOffset] != chXMLTagOpen )
			{
				// Text Value 
				unsigned iEnd = xml.find( chXMLTagOpen, iOffset );
				if( iEnd == string::npos ) 
				{
					// error cos not exist CloseTag </TAG>
					if( pi->error_string.empty() )  
						pi->error_string = ssprintf( "it must be closed with </%s>", m_sName.c_str() );
					return string::npos;
				}
				
				SetString( xml, iOffset, iEnd, &m_sValue, true );

				iOffset = iEnd;
				//TEXTVALUE
				ReplaceEntityText( m_sValue, g_mapEntitiesToChars );
			}
		}
	}

	return iOffset;
}

// Desc   : convert plain xml text from parsed xml attirbute
// Return : converted plain string
bool XNode::GetAttrXML( RageFileBasic &f, DISP_OPT &opt, const RString &sName, const RString &sValue ) const
{
	RString s(sValue);
	if( opt.reference_value )
		ReplaceEntityText( s, g_mapCharsToEntities );
	return f.Write(sName + "='" + s + "' ") != -1;
}

// Desc   : convert plain xml text from parsed xml node
// Return : converted plain string
bool XNode::GetXML( RageFileBasic &f, DISP_OPT &opt ) const
{
	// tab
	if( opt.newline )
	{
		if( f.Write("\r\n") == -1 )
			return false;
		if( opt.write_tabs )
			for( int i = 0 ; i < opt.tab_base ; i++)
				if( f.Write("\t") == -1 )
					return false;
	}

	// <TAG
	if( f.Write("<" + m_sName) == -1 )
		return false;

	// <TAG Attr1="Val1" 
	if( !m_attrs.empty() )
		if( f.Write(" ") == -1 )
			return false;
	FOREACH_CONST_Attr( this, p )
		if( !GetAttrXML(f, opt, p->first, p->second) )
			return false;
	
	if( m_childs.empty() && m_sValue.empty() )
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
			
		if( opt.newline && !m_childs.empty() )
		{
			opt.tab_base++;
		}

		FOREACH_CONST_Child( this, p )
			if( !p->GetXML( f, opt ) )
				return false;
		
		// Text Value
		if( !m_sValue.empty() )
		{
			if( opt.newline && !m_childs.empty() )
			{
				if( opt.newline )
					if( f.Write("\r\n") == -1 )
						return false;
				if( opt.write_tabs )
					for( int i = 0 ; i < opt.tab_base ; i++)
						if( f.Write("\t") == -1 )
							return false;
			}
			RString s( m_sValue );
			if( opt.reference_value )
					ReplaceEntityText( s, g_mapCharsToEntities );
			if( f.Write(s) == -1 )
				return false;
		}

		// </TAG> CloseTag
		if( opt.newline && !m_childs.empty() )
		{
			if( f.Write("\r\n") == -1 )
				return false;
			if( opt.write_tabs )
				for( int i = 0 ; i < opt.tab_base-1 ; i++)
					if( f.Write("\t") == -1 )
						return false;
		}
		if( f.Write("</" + m_sName + ">") == -1 )
			return false;

		if( opt.newline )
		{
			if( !m_childs.empty() )
				opt.tab_base--;
		}
	}
	return true;
}

// Desc   : convert plain xml text from parsed xml node
// Return : converted plain string
RString XNode::GetXML() const
{
	RageFileObjMem f;
	DISP_OPT opt;
	GetXML( f, opt );
	return f.GetString();
}

void XNode::GetValue( RString &out ) const	{ out = m_sValue; }
void XNode::GetValue( int &out ) const		{ out = atoi(m_sValue); }
void XNode::GetValue( float &out ) const	{ out = StringToFloat(m_sValue); }
void XNode::GetValue( bool &out ) const		{ out = atoi(m_sValue) != 0; }
void XNode::GetValue( unsigned &out ) const	{ out = 0; sscanf(m_sValue,"%u",&out); }
void XNode::GetValue( DateTime &out ) const	{ out.FromString( m_sValue ); }

bool XNode::GetAttrValue( const RString &sName, RString &out ) const	{ const RString* pAttr=GetAttr(sName); if(pAttr==NULL) return false; out = *pAttr; return true; }
bool XNode::GetAttrValue( const RString &sName, int &out ) const		{ const RString* pAttr=GetAttr(sName); if(pAttr==NULL) return false; out = atoi(*pAttr); return true; }
bool XNode::GetAttrValue( const RString &sName, float &out ) const	{ const RString* pAttr=GetAttr(sName); if(pAttr==NULL) return false; out = StringToFloat(*pAttr); return true; }
bool XNode::GetAttrValue( const RString &sName, bool &out ) const		{ const RString* pAttr=GetAttr(sName); if(pAttr==NULL) return false; out = atoi(*pAttr) != 0; return true; }
bool XNode::GetAttrValue( const RString &sName, unsigned &out ) const	{ const RString* pAttr=GetAttr(sName); if(pAttr==NULL) return false; out = 0; sscanf(*pAttr,"%u",&out); return true; }
bool XNode::GetAttrValue( const RString &sName, DateTime &out ) const	{ const RString* pAttr=GetAttr(sName); if(pAttr==NULL) return false; out.FromString( *pAttr ); return true; }

void XNode::SetValue( int v )				{ m_sValue = ssprintf("%d",v); }
void XNode::SetValue( float v )				{ m_sValue = ssprintf("%f",v); }
void XNode::SetValue( bool v )				{ m_sValue = ssprintf("%d",v); }
void XNode::SetValue( unsigned v )			{ m_sValue = ssprintf("%u",v); }
void XNode::SetValue( const DateTime &v )	{ m_sValue = v.GetString(); }

const RString *XNode::GetAttr( const RString &attrname ) const
{
	map<RString, RString>::const_iterator it = m_attrs.find( attrname );
	if( it != m_attrs.end() )
		return &it->second;
	return NULL;
}

RString *XNode::GetAttr( const RString &attrname )
{
	map<RString, RString>::iterator it = m_attrs.find( attrname );
	if( it != m_attrs.end() )
		return &it->second;
	return NULL;
}

// Desc   : Find child with name and return child
// Return : NULL return if no child.
XNode *XNode::GetChild( const RString &sName )
{
	multimap<RString, XNode*>::iterator it = m_childs.find( sName );
	if( it != m_childs.end() )
	{
		DEBUG_ASSERT( sName == it->second->m_sName );
		return it->second;
	}
	return NULL;
}

const XNode *XNode::GetChild( const RString &sName ) const
{
	multimap<RString, XNode*>::const_iterator it = m_childs.find( sName );
	if( it != m_childs.end() )
	{
		DEBUG_ASSERT( sName == it->second->m_sName );
		return it->second;
	}
	return NULL;
}

XNode *XNode::AppendChild( const RString &sName, const RString &value )		{ XNode *p = new XNode; p->m_sName = sName; p->m_sValue = value; return AppendChild( p ); }
XNode *XNode::AppendChild( const RString &sName, float value )				{ XNode *p = new XNode; p->m_sName = sName; p->SetValue( value ); return AppendChild( p ); }
XNode *XNode::AppendChild( const RString &sName, int value )				{ XNode *p = new XNode; p->m_sName = sName; p->SetValue( value ); return AppendChild( p ); }
XNode *XNode::AppendChild( const RString &sName, unsigned value )			{ XNode *p = new XNode; p->m_sName = sName; p->SetValue( value ); return AppendChild( p ); }
XNode *XNode::AppendChild( const RString &sName, const DateTime &value )	{ XNode *p = new XNode; p->m_sName = sName; p->SetValue( value ); return AppendChild( p ); }

XNode *XNode::AppendChild( XNode *node )
{
	DEBUG_ASSERT( node->m_sName.size() );

	/* Hinted insert: optimize for alphabetical inserts, for the copy ctor. */
	m_childs.insert( m_childs.end(), pair<RString,XNode*>(node->m_sName,node) );
	return node;
}

// detach node and delete object
bool XNode::RemoveChild( XNode *node )
{
	FOREACHMM( RString, XNode*, m_childs, p )
	{
		if( p->second == node )
		{
			SAFE_DELETE( p->second );
			m_childs.erase( p );
			return true;
		}
	}
	return false;
}


// detach attribute
bool XNode::RemoveAttr( const RString &sName )
{
	return m_attrs.erase(sName) > 0;
}

void XNode::AppendAttr( const RString &sName, const RString &sValue )
{
	pair<XAttrs::iterator,bool> ret = m_attrs.insert( make_pair(sName,sValue) );
	if( !ret.second )
		ret.first->second = sValue; // already existed
}

void XNode::AppendAttr( const RString &sName, float value ){ AppendAttr(sName,ssprintf("%f",value)); }
void XNode::AppendAttr( const RString &sName, int value )	{ AppendAttr(sName,ssprintf("%d",value)); }
void XNode::AppendAttr( const RString &sName, unsigned value )	{ AppendAttr(sName,ssprintf("%u",value)); }

bool XNode::SaveToFile( RageFileBasic &f, DISP_OPT &opt ) const
{
	f.PutLine( "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" );
	if( !opt.stylesheet.empty() )
		f.PutLine( "<?xml-stylesheet type=\"text/xsl\" href=\"" + opt.stylesheet + "\"?>" );
	if( !this->GetXML(f, opt) )
		return false;
	if( f.Flush() == -1 )
		return false;
	return true;
}

bool XNode::SaveToFile( const RString &sFile, DISP_OPT &opt ) const
{
	RageFile f;
	if( !f.Open(sFile, RageFile::WRITE) )
	{
		LOG->Warn( "Couldn't open %s for writing: %s", sFile.c_str(), f.GetError().c_str() );
		return false;
	}

	return SaveToFile( f, opt );
}

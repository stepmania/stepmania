#include "global.h"
#include "XmlFile.h"
#include "RageFile.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "DateTime.h"
#include "Foreach.h"
#include "arch/Dialog/Dialog.h"
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


static map<CString,CString> g_mapEntitiesToChars;
static map<char,CString> g_mapCharsToEntities;

static void InitEntities()
{
	if( !g_mapEntitiesToChars.empty() )
		return;

	struct Entity
	{
		char c;
		const char *pEntity;
	}
	static const EntityTable[] =
	{
		{ '&',  "amp", },
		{ '\"', "quot", },
		{ '\'', "apos", },
		{ '<',  "lt", },
		{ '>',  "gt", } 
	};

	for( unsigned i = 0; i < ARRAYSIZE(EntityTable); ++i )
	{
		const Entity &ent = EntityTable[i];
		g_mapEntitiesToChars[ent.pEntity] = CString(1, ent.c);
		g_mapCharsToEntities[ent.c] = ent.pEntity;
	}
}

struct RunInitEntities
{
	RunInitEntities() { InitEntities(); }
} static g_RunInitEntities;

// skip spaces
static char* tcsskip( const char* psz )
{
	while( psz && isspace(*psz) )
		++psz;

	return (char*)psz;
}

static bool XIsEmptyString( const CString &s )
{
	return s.find_first_not_of( "\r\n\t " ) == s.npos;
}

// put string of (psz~end) on ps string
static void SetString( const char* psz, const char* end, CString* ps, bool trim = false )
{
	if( trim )
	{
		while( psz < end && isspace(*psz) )
			psz++;
		while( end-1 >= psz && isspace(end[-1]) )
			end--;
	}

	int len = end - psz;
	if( len <= 0 )
		return;

	ps->assign( psz, len );
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
	
	FOREACH_Attr( this, p2 )
		SAFE_DELETE( p2 );
	m_attrs.clear();
}
	
// attr1="value1" attr2='value2' attr3=value3 />
//                                            ^- return pointer
// Desc   : loading attribute plain xml text
// Param  : pszAttrs - xml of attributes
//          pi = parser information
// Return : advanced string pointer. (error return NULL)
const char* XNode::LoadAttributes( const char* xml, PARSEINFO *pi /*= &piDefault*/)
{
	while( xml && *xml )
	{
		xml = tcsskip( xml );
		if( !xml )
			continue;

		// close tag
		if( *xml == chXMLTagClose || *xml == chXMLTagPre || *xml == chXMLQuestion || *xml == chXMLDash )
			return xml; // well-formed tag

		// XML Attr Name
		const char* pEnd = strpbrk( xml, " =" );
		if( pEnd == NULL ) 
		{
			// error
			if( !pi->error_occur ) 
			{
				pi->error_occur = true;
				pi->error_pointer = xml;
				pi->error_code = PIE_ATTR_NO_VALUE;
				pi->error_string = ssprintf( "<%s> attribute has error ", m_sName.c_str() );
			}
			return NULL;
		}
		
		XAttr *attr = new XAttr;

		// XML Attr Name
		SetString( xml, pEnd, &attr->m_sName );
		
		// add new attribute
		DEBUG_ASSERT( attr->m_sName.size() );
		m_attrs.insert( make_pair(attr->m_sName, attr) );
		xml = pEnd;
		
		// XML Attr Value
		xml = tcsskip( xml );
		if( !xml )
			continue;

		//if( xml = strchr( xml, '=' ) )
		if( *xml == '=' )
		{
			xml = tcsskip( ++xml );
			if( !xml )
				continue;
			// if " or '
			// or none quote
			int quote = *xml;
			if( quote == '"' || quote == '\'' )
			{
				pEnd = strchr( ++xml, quote );
			}
			else
			{
				//attr= value> 
				// none quote mode
				pEnd = strpbrk( xml, " >" );
			}

			bool trim = pi->trim_value;
			SetString( xml, pEnd, &attr->m_sValue, trim );
			xml = pEnd;
			// ATTRVALUE 
			if( pi->entity_value )
				ReplaceEntityText( attr->m_sValue, g_mapEntitiesToChars );

			if( quote == '"' || quote == '\'' )
				xml++;
		}
	}

	// not well-formed tag
	return NULL;
}

// <TAG attr1="value1" attr2='value2' attr3=value3 >
// </TAG>
// or
// <TAG />
//        ^- return pointer
// Desc   : load xml plain text
// Param  : pszXml - plain xml text
//          pi = parser information
// Return : advanced string pointer  (error return NULL)
const char* XNode::Load( const char* xml, PARSEINFO *pi /*= &piDefault*/ )
{
	Clear();

	// <
	xml = strchr( xml, chXMLTagOpen );
	if( xml == NULL )
		return NULL;

	// </
	if( xml[1] == chXMLTagPre )
		return xml;

	/* <!-- */
	if( !strncmp(xml+1, "!--", 3) )
	{
		xml += 4;

		/* Find the close tag. */
		char *pEnd = strstr( xml, "-->" );
		if( pEnd == NULL )
		{
			if( !pi->error_occur ) 
			{
				pi->error_occur = true;
				pi->error_pointer = xml;
				pi->error_code = PIE_ALONE_NOT_CLOSED;
				pi->error_string = "Unterminated comment";
			}

			return NULL;
		}

		// Skip -->.
		xml = pEnd + 3;

		return Load( xml, pi );
	}

	// XML Node Tag Name Open
	xml++;
	const char* pTagEnd = strpbrk( xml, " \t\r\n/>" );
	SetString( xml, pTagEnd, &m_sName );

	xml = pTagEnd;
	// Generate XML Attributte List
	xml = LoadAttributes( xml, pi );
	if( xml == NULL )
		return NULL;

	// alone tag <TAG ... /> or <?TAG ... ?> or <!-- ... --> 
	// current pointer:   ^               ^              ^

	if( *xml == chXMLTagPre || *xml == chXMLQuestion || *xml == chXMLDash )
	{
		xml++;

		// skip over 2nd dash
		if( *xml == chXMLDash )
			xml++;

		if( *xml != chXMLTagClose )
		{
			// error: <TAG ... / >
			if( !pi->error_occur ) 
			{
				pi->error_occur = true;
				pi->error_pointer = xml;
				pi->error_code = PIE_ALONE_NOT_CLOSED;
				pi->error_string = "Element must be closed.";
			}

			// ill-formed tag
			return NULL;
		}

		// well-formed tag
		++xml;

		// UGLY: We want to ignore all XML meta tags.  So, since the Node we 
		// just loaded is a meta tag, then Load ourself again using the rest 
		// of the file until we reach a non-meta tag.
		if( !m_sName.empty() && (m_sName[0] == chXMLQuestion || m_sName[0] == chXMLExclamation) )
			xml = Load( xml, pi );

		return xml;
	}

	// open/close tag <TAG ..> ... </TAG>
	//                        ^- current pointer
	if( XIsEmptyString( m_sValue ) )
	{
		// Text Value 
		const char* pEnd = strchr( ++xml, chXMLTagOpen );
		if( pEnd == NULL ) 
		{
			if( !pi->error_occur ) 
			{
				pi->error_occur = true;
				pi->error_pointer = xml;
				pi->error_code = PIE_NOT_CLOSED;
				pi->error_string = ssprintf( "%s must be closed with </%s>", m_sName.c_str(), m_sName.c_str() );
			}
			// error cos not exist CloseTag </TAG>
			return NULL;
		}
		
		bool trim = pi->trim_value;
		SetString( xml, pEnd, &m_sValue, trim );

		xml = pEnd;
		// TEXTVALUE reference
		if( pi->entity_value )
			ReplaceEntityText( m_sValue, g_mapEntitiesToChars );
	}

	// generate child nodes
	while( xml && *xml )
	{
		XNode *node = new XNode;
		
		xml = node->Load( xml,pi );
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
		if( xml && xml[0] == chXMLTagOpen && xml[1] == chXMLTagPre )
		{
			// </Close>
			xml+=2; // C
			
			xml = tcsskip( xml );
			if( xml == NULL )
				return NULL;

			const char* pEnd = strpbrk( xml, " >" );
			if( pEnd == NULL ) 
			{
				if( !pi->error_occur ) 
				{
					pi->error_occur = true;
					pi->error_pointer = xml;
					pi->error_code = PIE_NOT_CLOSED;
					pi->error_string = ssprintf( "it must be closed with </%s>", m_sName.c_str() );
				}
				// error
				return NULL;
			}

			CString closename;
			SetString( xml, pEnd, &closename );
			xml = pEnd+1;
			if( closename == this->m_sName )
			{
				// wel-formed open/close
				// return '>' or ' ' after pointer
				return xml;
			}
			else
			{
				// not welformed open/close
				if( !pi->error_occur ) 
				{
					pi->error_occur = true;
					pi->error_pointer = xml;
					pi->error_code = PIE_NOT_NESTED;
					pi->error_string = ssprintf( "'<%s> ... </%s>' is not well-formed.", m_sName.c_str(), closename.c_str() );
				}
				return NULL;
			}
		}
		else	// Alone child Tag Loaded
		{
			if( xml && XIsEmptyString( m_sValue ) && *xml !=chXMLTagOpen )
			{
				// Text Value 
				const char* pEnd = strchr( xml, chXMLTagOpen );
				if( pEnd == NULL ) 
				{
					// error cos not exist CloseTag </TAG>
					if( !pi->error_occur )  
					{
						pi->error_occur = true;
						pi->error_pointer = xml;
						pi->error_code = PIE_NOT_CLOSED;
						pi->error_string = ssprintf( "it must be closed with </%s>", m_sName.c_str() );
					}
					return NULL;
				}
				
				bool trim = pi->trim_value;
				SetString( xml, pEnd, &m_sValue, trim );

				xml = pEnd;
				//TEXTVALUE
				if( pi->entity_value )
					ReplaceEntityText( m_sValue, g_mapEntitiesToChars );
			}
		}
	}

	return xml;
}

// Desc   : convert plain xml text from parsed xml attirbute
// Return : converted plain string
bool XAttr::GetXML( RageFileBasic &f, DISP_OPT *opt ) const
{
	CString s(m_sValue);
	if( opt && opt->reference_value )
		ReplaceEntityText( s, g_mapCharsToEntities );
	return f.Write(m_sName + "='" + s + "' ") != -1;
}

// Desc   : convert plain xml text from parsed xml node
// Return : converted plain string
bool XNode::GetXML( RageFileBasic &f, DISP_OPT *opt ) const
{
	// tab
	if( opt && opt->newline )
	{
		if( f.Write("\r\n") == -1 )
			return false;
		if( opt->write_tabs )
			for( int i = 0 ; i < opt->tab_base ; i++)
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
		if( !p->GetXML(f, opt) )
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
			
		if( opt && opt->newline && !m_childs.empty() )
		{
			opt->tab_base++;
		}

		FOREACH_CONST_Child( this, p )
			if( !p->GetXML( f, opt ) )
				return false;
		
		// Text Value
		if( !m_sValue.empty() )
		{
			if( opt && opt->newline && !m_childs.empty() )
			{
				if( opt && opt->newline )
					if( f.Write("\r\n") == -1 )
						return false;
				if( opt->write_tabs )
					for( int i = 0 ; i < opt->tab_base ; i++)
						if( f.Write("\t") == -1 )
							return false;
			}
			CString s( m_sValue );
			if( opt && opt->reference_value )
					ReplaceEntityText( s, g_mapCharsToEntities );
			if( f.Write(s) == -1 )
				return false;
		}

		// </TAG> CloseTag
		if( opt && opt->newline && !m_childs.empty() )
		{
			if( f.Write("\r\n") == -1 )
				return false;
			if( opt->write_tabs )
				for( int i = 0 ; i < opt->tab_base-1 ; i++)
					if( f.Write("\t") == -1 )
						return false;
		}
		if( f.Write("</" + m_sName + ">") == -1 )
			return false;

		if( opt && opt->newline )
		{
			if( !m_childs.empty() )
				opt->tab_base--;
		}
	}
	return true;
}

// Desc   : convert plain xml text from parsed xml node
// Return : converted plain string
CString XNode::GetXML() const
{
	RageFileObjMem f;
	GetXML( f, NULL );
	return f.GetString();
}

void XNode::GetValue( CString &out ) const	{ out = m_sValue; }
void XNode::GetValue( int &out ) const		{ out = atoi(m_sValue); }
void XNode::GetValue( float &out ) const	{ out = strtof(m_sValue, NULL); }
void XNode::GetValue( bool &out ) const		{ out = atoi(m_sValue) != 0; }
void XNode::GetValue( unsigned &out ) const	{ out = 0; sscanf(m_sValue,"%u",&out); }
void XNode::GetValue( DateTime &out ) const	{ out.FromString( m_sValue ); }

void XAttr::GetValue( CString &out ) const	{ out = m_sValue; }
void XAttr::GetValue( int &out ) const		{ out = atoi(m_sValue); }
void XAttr::GetValue( float &out ) const	{ out = strtof(m_sValue, NULL); }
void XAttr::GetValue( bool &out ) const		{ out = atoi(m_sValue) != 0; }
void XAttr::GetValue( unsigned &out ) const	{ out = 0; sscanf(m_sValue,"%u",&out); }
void XAttr::GetValue( DateTime &out ) const	{ out.FromString( m_sValue ); }

void XNode::SetValue( int v )				{ m_sValue = ssprintf("%d",v); }
void XNode::SetValue( float v )				{ m_sValue = ssprintf("%f",v); }
void XNode::SetValue( bool v )				{ m_sValue = ssprintf("%d",v); }
void XNode::SetValue( unsigned v )			{ m_sValue = ssprintf("%u",v); }
void XNode::SetValue( const DateTime &v )	{ m_sValue = v.GetString(); }

const XAttr *XNode::GetAttr( const CString &attrname ) const
{
	multimap<CString, XAttr*>::const_iterator it = m_attrs.find( attrname );
	if( it != m_attrs.end() )
	{
		DEBUG_ASSERT( attrname == it->second->m_sName );
		return it->second;
	}
	return NULL;
}

XAttr *XNode::GetAttr( const CString &attrname )
{
	multimap<CString, XAttr*>::iterator it = m_attrs.find( attrname );
	if( it != m_attrs.end() )
	{
		DEBUG_ASSERT( attrname == it->second->m_sName );
		return it->second;
	}
	return NULL;
}

// Desc   : Find child with name and return child
// Return : NULL return if no child.
XNode *XNode::GetChild( const CString &sName )
{
	multimap<CString, XNode*>::iterator it = m_childs.find( sName );
	if( it != m_childs.end() )
	{
		DEBUG_ASSERT( sName == it->second->m_sName );
		return it->second;
	}
	return NULL;
}

const XNode *XNode::GetChild( const CString &sName ) const
{
	multimap<CString, XNode*>::const_iterator it = m_childs.find( sName );
	if( it != m_childs.end() )
	{
		DEBUG_ASSERT( sName == it->second->m_sName );
		return it->second;
	}
	return NULL;
}

XNode *XNode::AppendChild( const CString &sName, const CString &value )		{ XNode *p = new XNode; p->m_sName = sName; p->m_sValue = value; return AppendChild( p ); }
XNode *XNode::AppendChild( const CString &sName, float value )				{ XNode *p = new XNode; p->m_sName = sName; p->SetValue( value ); return AppendChild( p ); }
XNode *XNode::AppendChild( const CString &sName, int value )				{ XNode *p = new XNode; p->m_sName = sName; p->SetValue( value ); return AppendChild( p ); }
XNode *XNode::AppendChild( const CString &sName, unsigned value )			{ XNode *p = new XNode; p->m_sName = sName; p->SetValue( value ); return AppendChild( p ); }
XNode *XNode::AppendChild( const CString &sName, const DateTime &value )	{ XNode *p = new XNode; p->m_sName = sName; p->SetValue( value ); return AppendChild( p ); }

XNode *XNode::AppendChild( XNode *node )
{
	DEBUG_ASSERT( node->m_sName.size() );
	m_childs.insert( pair<CString,XNode*>(node->m_sName,node) );
	return node;
}

// detach node and delete object
bool XNode::RemoveChild( XNode *node )
{
	FOREACHMM( CString, XNode*, m_childs, p )
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


// add attribute
XAttr *XNode::AppendAttr( XAttr *attr )
{
	DEBUG_ASSERT( attr->m_sName.size() );
	m_attrs.insert( make_pair(attr->m_sName,attr) );
	return attr;
}

// detach attribute and delete object
bool XNode::RemoveAttr( XAttr *attr )
{
	FOREACHMM( CString, XAttr*, m_attrs, p )
	{
		if( p->second == attr )
		{
			SAFE_DELETE( p->second );
			m_attrs.erase( p );
			return true;
		}
	}
	return false;
}

XAttr *XNode::AppendAttr( const CString &sName, const CString &sValue )
{
	XAttr *pAttr = new XAttr;
	pAttr->m_sName = sName;
	pAttr->m_sValue = sValue;
	return AppendAttr( pAttr );
}

XAttr *XNode::AppendAttr( const CString &sName, float value ){ return AppendAttr(sName,ssprintf("%f",value)); }
XAttr *XNode::AppendAttr( const CString &sName, int value )	{ return AppendAttr(sName,ssprintf("%d",value)); }
XAttr *XNode::AppendAttr( const CString &sName, unsigned value )	{ return AppendAttr(sName,ssprintf("%u",value)); }

void XNode::SetAttrValue( const CString &sName, const CString &sValue )
{
	XAttr* pAttr = GetAttr( sName );
	if( pAttr )
		pAttr->m_sValue = sValue;
	else
		AppendAttr( sName, sValue );
}


bool XNode::LoadFromFile( const CString &sFile )
{
	RageFile f;
	if( !f.Open(sFile, RageFile::READ) )
	{
		LOG->Warn("Couldn't open %s for reading: %s", sFile.c_str(), f.GetError().c_str() );
		return false;
	}

	bool bSuccess = LoadFromFile( f );
	if( !bSuccess )
	{
		CString sWarning = ssprintf( "XML: LoadFromFile failed for file: %s", sFile.c_str() );
		LOG->Warn( sWarning );
		Dialog::OK( sWarning, "XML_PARSE_ERROR" );
	}
	return bSuccess;
}

bool XNode::LoadFromFile( RageFileBasic &f )
{
	PARSEINFO pi;
	CString s;
	if( f.Read( s ) == -1 )
	{
		pi.error_occur = true;
		pi.error_pointer = NULL;
		pi.error_code = PIE_READ_ERROR;
		pi.error_string = f.GetError();
		
		goto error;
	}
	this->Load( s, &pi );
	if( pi.error_occur )
		goto error;
	return true;

error:
	CString sWarning = ssprintf( "XML: LoadFromFile failed: %s", pi.error_string.c_str() );
	LOG->Warn( sWarning );
	Dialog::OK( sWarning, "XML_PARSE_ERROR" );
	return false;
}

bool XNode::SaveToFile( RageFileBasic &f, DISP_OPT *opt ) const
{
	f.PutLine( "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" );
	if( !opt->stylesheet.empty() )
		f.PutLine( "<?xml-stylesheet type=\"text/xsl\" href=\"" + opt->stylesheet + "\"?>" );
	if( !this->GetXML(f, opt) )
		return false;
	if( f.Flush() == -1 )
		return false;
	return true;
}

bool XNode::SaveToFile( const CString &sFile, DISP_OPT *opt ) const
{
	RageFile f;
	if( !f.Open(sFile, RageFile::WRITE) )
	{
		LOG->Warn("Couldn't open %s for writing: %s", sFile.c_str(), f.GetError().c_str() );
		return false;
	}

	return SaveToFile( f, opt );
}

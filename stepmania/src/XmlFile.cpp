#include "global.h"
// XmlFile.cpp: implementation of the XmlFile class.
//
//////////////////////////////////////////////////////////////////////

#include "XmlFile.h"
#include <iostream>
#include <sstream>
#include <string>


static const TCHAR chXMLTagOpen		= '<';
static const TCHAR chXMLTagClose	= '>';
static const TCHAR chXMLTagPre	= '/';
static const TCHAR chXMLEscape = '\\';	// for value field escape


static const XENTITY x_EntityTable[] = {
		{ '&', ("&amp;"), 5 } ,
		{ '\"', ("&quot;"), 6 } ,
		{ '\'', ("&apos;"), 6 } ,
		{ '<', ("&lt;"), 4 } ,
		{ '>', ("&gt;"), 4 } 
	};

PARSEINFO piDefault;
DISP_OPT optDefault;
XENTITYS entityDefault((LPXENTITY)x_EntityTable, sizeof(x_EntityTable)/sizeof(x_EntityTable[0]) );
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


//========================================================
// Name   : _tcschrs
// Desc   : same with _tcspbrk 
// Param  :
// Return :
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPTSTR _tcschrs( LPCTSTR psz, LPCTSTR pszchs )
{
	while( psz && *psz )
	{
		if( strchr( pszchs, *psz ) )
			return (LPTSTR)psz;
		psz++;
	}
	return NULL;
}

//========================================================
// Name   : _tcsskip
// Desc   : skip space
// Param  : 
// Return : skiped string
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPTSTR _tcsskip( LPCTSTR psz )
{
	while( psz && *psz == ' ' ) psz++;
		
	return (LPTSTR)psz;
}

//========================================================
// Name   : _tcsechr
// Desc   : similar with _tcschr with escape process
// Param  : escape - will be escape character
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPTSTR _tcsechr( LPCTSTR psz, int ch, int escape )
{
	LPTSTR pch = (LPTSTR)psz;
	LPTSTR prev_escape = NULL;
	while( pch && *pch )
	{
		if( *pch == escape && prev_escape == NULL )
			prev_escape = pch;
		else
		{
			prev_escape = NULL;
			if( *pch == ch ) return (LPTSTR)pch;
		}
		pch++;
	}
	return pch;
}

//========================================================
// Name   : _tcselen
// Desc   : similar with _tcslen with escape process
// Param  : escape - will be escape character
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
int _tcselen( int escape, LPTSTR srt, LPTSTR end = NULL ) 
{
	int len = 0;
	LPTSTR pch = srt;
	if( end==NULL ) end = (LPTSTR)sizeof(long);
	LPTSTR prev_escape = NULL;
	while( pch && *pch && pch<end )
	{
		if( *pch == escape && prev_escape == NULL )
			prev_escape = pch;
		else
		{
			prev_escape = NULL;
			len++;
		}
		pch++;
	}
	return len;
}

//========================================================
// Name   : _tcsecpy
// Desc   : similar with _tcscpy with escape process
// Param  : escape - will be escape character
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
void _tcsecpy( LPTSTR psz, int escape, LPTSTR srt, LPTSTR end = NULL )
{
	LPTSTR pch = srt;
	if( end==NULL ) end = (LPTSTR)sizeof(long);
	LPTSTR prev_escape = NULL;
	while( pch && *pch && pch<end )
	{
		if( *pch == escape && prev_escape == NULL )
			prev_escape = pch;
		else
		{
			prev_escape = NULL;
			*psz++ = *pch;
		}

		pch++;
	}

	*psz = '\0';
}

//========================================================
// Name   : _tcsepbrk
// Desc   : similar with _tcspbrk with escape process
// Param  : escape - will be escape character
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPTSTR _tcsepbrk( LPCTSTR psz, LPCTSTR chset, int escape )
{
	LPTSTR pch = (LPTSTR)psz;
	LPTSTR prev_escape = NULL;
	while( pch && *pch )
	{
		if( *pch == escape && prev_escape == NULL )
			prev_escape = pch;
		else
		{
			prev_escape = NULL;
			if( _tcschr( chset, *pch ) )
				return (LPTSTR)pch;		
		}
		pch++;
	}
	return pch;
}

//========================================================
// Name   : _SetString
// Desc   : put string of (psz~end) on ps string
// Param  : trim - will be trim?
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
void _SetString( LPTSTR psz, LPTSTR end, CString* ps, bool trim = FALSE, int escape = 0 )
{
	//trim
	if( trim )
	{
		while( psz && psz < end && _istspace(*psz) ) psz++;
		while( (end-1) && psz < (end-1) && _istspace(*(end-1)) ) end--;
	}
	int len = end - psz;
	if( len <= 0 ) return;
	if( escape )
	{
		len = _tcselen( escape, psz, end );
//		LPTSTR pss = ps->GetBufferSetLength( len );
		char* szTemp = new char[len];
		_tcsecpy( szTemp, escape, psz, end );
		*ps = szTemp;
	}
	else
	{
//		LPTSTR pss = ps->GetBufferSetLength(len + 1 );
		char* szTemp = new char[len+1];
		memcpy( szTemp, psz, len );
		szTemp[len] = '\0';
		*ps = szTemp;
	}
}

_tagXMLNode::~_tagXMLNode()
{
	Close();
}

void _tagXMLNode::Close()
{
    unsigned i;

	for( i = 0 ; i < childs.size(); i ++)
	{
		LPXNode p = childs[i];
		if( p )
		{
			delete p; childs[i] = NULL;
		}
	}
	childs.clear();
	
	for( i = 0 ; i < attrs.size(); i ++)
	{
		LPXAttr p = attrs[i];
		if( p )
		{
			delete p; attrs[i] = NULL;
		}
	}
	attrs.clear();
}
	
// attr1="value1" attr2='value2' attr3=value3 />
//                                            ^- return pointer
//========================================================
// Name   : LoadAttributes
// Desc   : loading attribute plain xml text
// Param  : pszAttrs - xml of attributes
//          pi = parser information
// Return : advanced string pointer. (error return NULL)
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPTSTR _tagXMLNode::LoadAttributes( LPCTSTR pszAttrs , LPPARSEINFO pi /*= &piDefault*/)
{
	LPTSTR xml = (LPTSTR)pszAttrs;

	while( xml && *xml )
	{
		xml = _tcsskip( xml );
		if( !xml )
			continue;

		// close tag
		if( *xml == chXMLTagClose || *xml == chXMLTagPre )
			// wel-formed tag
			return xml;

		// XML Attr Name
		TCHAR* pEnd = _tcspbrk( xml, " =" );
		if( pEnd == NULL ) 
		{
			// error
			if( pi->erorr_occur == false ) 
			{
				pi->erorr_occur = true;
				pi->error_pointer = xml;
				pi->error_code = PIE_ATTR_NO_VALUE;
				pi->error_string.Format( ("<%s> attribute has error "), name.c_str() );
			}
			return NULL;
		}
		
		LPXAttr attr = new XAttr;
		attr->parent = this;

		// XML Attr Name
		_SetString( xml, pEnd, &attr->name );
		
		// add new attribute
		attrs.push_back( attr );
		xml = pEnd;
		
		// XML Attr Value
		xml = _tcsskip( xml );
		if( !xml )
			continue;

		//if( xml = _tcschr( xml, '=' ) )
		if( *xml == '=' )
		{
			xml = _tcsskip( ++xml );
			if( !xml )
				continue;
			// if " or '
			// or none quote
			int quote = *xml;
			if( quote == '"' || quote == '\'' )
				pEnd = _tcsechr( ++xml, quote, chXMLEscape );
			else
			{
				//attr= value> 
				// none quote mode
				//pEnd = _tcsechr( xml, ' ', '\\' );
				pEnd = _tcsepbrk( xml, (" >"), chXMLEscape );
			}

			bool trim = pi->trim_value;
			TCHAR escape = pi->escape_value;
			//_SetString( xml, pEnd, &attr->value, trim, chXMLEscape );	
			_SetString( xml, pEnd, &attr->value, trim, escape );
			xml = pEnd;
			// ATTRVALUE 
			if( pi->entity_value && pi->entitys )
				attr->value = pi->entitys->Ref2Entity(attr->value);

			if( quote == '"' || quote == '\'' )
				xml++;
		}
	}

	// not wel-formed tag
	return NULL;
}

// <TAG attr1="value1" attr2='value2' attr3=value3 >
// </TAG>
// or
// <TAG />
//        ^- return pointer
//========================================================
// Name   : Load
// Desc   : load xml plain text
// Param  : pszXml - plain xml text
//          pi = parser information
// Return : advanced string pointer  (error return NULL)
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPTSTR _tagXMLNode::Load( LPCTSTR pszXml, LPPARSEINFO pi /*= &piDefault*/ )
{
	// Close it
	Close();

	LPTSTR xml = (LPTSTR)pszXml;

	xml = _tcschr( xml, chXMLTagOpen );
	if( xml == NULL )
		return NULL;

	// Close Tag
	if( *(xml+1) == chXMLTagPre ) // </Close
		return xml;

	// XML Node Tag Name Open
	xml++;
	TCHAR* pTagEnd = _tcspbrk( xml, " />" );
	_SetString( xml, pTagEnd, &name );
	xml = pTagEnd;
	// Generate XML Attributte List
	xml = LoadAttributes( xml, pi );
	if( xml == NULL )
		return NULL;

	// alone tag <TAG ... />
	if( *xml == chXMLTagPre )
	{
		xml++;
		if( *xml == chXMLTagClose )
			// wel-formed tag
			return ++xml;
		else
		{
			// error: <TAG ... / >
			if( pi->erorr_occur == false ) 
			{
				pi->erorr_occur = true;
				pi->error_pointer = xml;
				pi->error_code = PIE_ALONE_NOT_CLOSED;
				pi->error_string = ("Element must be closed.");
			}
			// not wel-formed tag
			return NULL;
		}
	}
	else
	// open/close tag <TAG ..> ... </TAG>
	//                        ^- current pointer
	{
		// text value가 없으툈E넣도록한다.
		//if( this->value.IsEmpty() || this->value == ("") )
		if( XIsEmptyString( value ) )
		{
			// Text Value 
			TCHAR* pEnd = _tcsechr( ++xml, chXMLTagOpen, chXMLEscape );
			if( pEnd == NULL ) 
			{
				if( pi->erorr_occur == false ) 
				{
					pi->erorr_occur = true;
					pi->error_pointer = xml;
					pi->error_code = PIE_NOT_CLOSED;
					pi->error_string.Format(("%s must be closed with </%s>"), name.c_str() );
				}
				// error cos not exist CloseTag </TAG>
				return NULL;
			}
			
			bool trim = pi->trim_value;
			TCHAR escape = pi->escape_value;
			//_SetString( xml, pEnd, &value, trim, chXMLEscape );
			_SetString( xml, pEnd, &value, trim, escape );

			xml = pEnd;
			// TEXTVALUE reference
			if( pi->entity_value && pi->entitys )
				value = pi->entitys->Ref2Entity(value);
		}

		// generate child nodes
		while( xml && *xml )
		{
			LPXNode node = new XNode;
			node->parent = this;
			
			xml = node->Load( xml,pi );
			if( node->name.IsEmpty() == FALSE )
			{
				childs.push_back( node );
			}
			else
			{
				delete node;
			}

			// open/close tag <TAG ..> ... </TAG>
			//                             ^- current pointer
			// CloseTag case
			if( xml && *xml && *(xml+1) && *xml == chXMLTagOpen && *(xml+1) == chXMLTagPre )
			{
				// </Close>
				xml+=2; // C
				
				xml = _tcsskip( xml );
				if( xml == NULL )
					return NULL;

				CString closename;
				TCHAR* pEnd = _tcspbrk( xml, " >" );
				if( pEnd == NULL ) 
				{
					if( pi->erorr_occur == false ) 
					{
						pi->erorr_occur = true;
						pi->error_pointer = xml;
						pi->error_code = PIE_NOT_CLOSED;
						pi->error_string.Format(("it must be closed with </%s>"), name.c_str() );
					}
					// error
					return NULL;
				}
				_SetString( xml, pEnd, &closename );
				if( closename == this->name )
				{
					// wel-formed open/close
					xml = pEnd+1;
					// return '>' or ' ' after pointer
					return xml;
				}
				else
				{
					xml = pEnd+1;
					// not welformed open/close
					if( pi->erorr_occur == false ) 
					{
						pi->erorr_occur = true;
						pi->error_pointer = xml;
						pi->error_code = PIE_NOT_NESTED;
						pi->error_string.Format(("'<%s> ... </%s>' is not well-formed."), name.c_str(), closename.c_str() );

					}
					return NULL;
				}
			}
			else	// Alone child Tag Loaded
					// else 해야하는햨E말아야하는햨E의심간다.
			{
				
				//if( xml && this->value.IsEmpty() && *xml !=chXMLTagOpen )
				if( xml && XIsEmptyString( value ) && *xml !=chXMLTagOpen )
				{
					// Text Value 
					TCHAR* pEnd = _tcsechr( xml, chXMLTagOpen, chXMLEscape );
					if( pEnd == NULL ) 
					{
						// error cos not exist CloseTag </TAG>
						if( pi->erorr_occur == false )  
						{
							pi->erorr_occur = true;
							pi->error_pointer = xml;
							pi->error_code = PIE_NOT_CLOSED;
							pi->error_string.Format(("it must be closed with </%s>"), name.c_str() );
						}
						return NULL;
					}
					
					bool trim = pi->trim_value;
					TCHAR escape = pi->escape_value;
					//_SetString( xml, pEnd, &value, trim, chXMLEscape );
					_SetString( xml, pEnd, &value, trim, escape );

					xml = pEnd;
					//TEXTVALUE
					if( pi->entity_value && pi->entitys )
						value = pi->entitys->Ref2Entity(value);
				}
			}
		}
	}

	return xml;
}

//========================================================
// Name   : GetXML
// Desc   : convert plain xml text from parsed xml attirbute
// Param  :
// Return : converted plain string
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
CString _tagXMLAttr::GetXML( LPDISP_OPT opt /*= &optDefault*/ )
{
	std::ostringstream os;
	//os << (LPCTSTR)name << "='" << (LPCTSTR)value << "' ";
	os << (LPCTSTR)name << "='"
		<< (LPCTSTR)(opt->reference_value&&opt->entitys?opt->entitys->Entity2Ref(value):value) << "' ";
	return os.str().c_str();
}

//========================================================
// Name   : GetXML
// Desc   : convert plain xml text from parsed xml node
// Param  :
// Return : converted plain string
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
CString _tagXMLNode::GetXML( LPDISP_OPT opt /*= &optDefault*/ )
{
	std::ostringstream os;

	// tab
	if( opt && opt->newline )
	{
		if( opt && opt->newline )
			os << "\r\n";
		for( int i = 0 ; i < opt->tab_base ; i++)
			os << '\t';
	}

	// <TAG
	os << '<' << (LPCTSTR)name;

	// <TAG Attr1="Val1" 
	if( attrs.empty() == false ) os << ' ';
	for( unsigned i = 0 ; i < attrs.size(); i++ )
	{
		os << (LPCTSTR)attrs[i]->GetXML(opt);
	}
	
	if( childs.empty() && value.IsEmpty() )
	{
		// <TAG Attr1="Val1"/> alone tag 
		os << "/>";	
	}
	else
	{
		// <TAG Attr1="Val1"> and get child
		os << '>';
		if( opt && opt->newline && !childs.empty() )
		{
			opt->tab_base++;
		}

		for( unsigned i = 0 ; i < childs.size(); i++ )
			os << (LPCTSTR)childs[i]->GetXML( opt );
		
		// Text Value
		if( value != ("") )
		{
			if( opt && opt->newline && !childs.empty() )
			{
				if( opt && opt->newline )
					os << "\r\n";
				for( int i = 0 ; i < opt->tab_base ; i++)
					os << '\t';
			}
			os << (LPCTSTR)(opt->reference_value&&opt->entitys?opt->entitys->Entity2Ref(value):value);
		}

		// </TAG> CloseTag
		if( opt && opt->newline && !childs.empty() )
		{
			os << "\r\n";
			for( int i = 0 ; i < opt->tab_base-1 ; i++)
				os << '\t';
		}
		os << "</" << (LPCTSTR)name << '>';

		if( opt && opt->newline )
		{
			if( !childs.empty() )
				opt->tab_base--;
		}
	}
	
	return os.str().c_str();
}

//========================================================
// Name   : GetAttr
// Desc   : get attribute with attribute name
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPXAttr	_tagXMLNode::GetAttr( LPCTSTR attrname )
{
	for( unsigned i = 0 ; i < attrs.size(); i++ )
	{
		LPXAttr attr = attrs[i];
		if( attr )
		{
			if( attr->name == attrname )
				return attr;
		}
	}
	return NULL;
}

//========================================================
// Name   : GetAttrs
// Desc   : find attributes with attribute name, return its list
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
XAttrs _tagXMLNode::GetAttrs( LPCTSTR name )
{
	XAttrs attrs;
	for( unsigned i = 0 ; i < attrs.size(); i++ )
	{
		LPXAttr attr = attrs[i];
		if( attr )
		{
			if( attr->name == name )
				attrs.push_back( attr );
		}
	}
	return attrs;
}

//========================================================
// Name   : GetAttrValue
// Desc   : get attribute with attribute name, return its value
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPCTSTR	_tagXMLNode::GetAttrValue( LPCTSTR attrname )
{
	LPXAttr attr = GetAttr( attrname );
	return attr ? (LPCTSTR)attr->value : NULL;
}

XNodes _tagXMLNode::GetChilds()
{
	return childs;
}

//========================================================
// Name   : GetChilds
// Desc   : Find childs with name and return childs list
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
XNodes _tagXMLNode::GetChilds( LPCTSTR name )
{
	XNodes nodes;
	for( unsigned i = 0 ; i < childs.size(); i++ )
	{
		LPXNode node = childs[i];
		if( node )
		{
			if( node->name == name )
				nodes.push_back( node );
		}
	}
	return nodes;	
}

//========================================================
// Name   : GetChild
// Desc   : get child node with index
// Param  :
// Return : NULL return if no child.
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPXNode _tagXMLNode::GetChild( int i )
{
	if( i >= 0 && i < (int)childs.size() )
		return childs[i];
	return NULL;
}

//========================================================
// Name   : GetChildCount
// Desc   : get child node count
// Param  :
// Return : 0 return if no child
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-12-26
//========================================================
int	_tagXMLNode::GetChildCount()
{
	return childs.size();
}

//========================================================
// Name   : GetChild
// Desc   : Find child with name and return child
// Param  :
// Return : NULL return if no child.
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPXNode	_tagXMLNode::GetChild( LPCTSTR name )
{
	for( unsigned i = 0 ; i < childs.size(); i++ )
	{
		LPXNode node = childs[i];
		if( node )
		{
			if( node->name == name )
				return node;
		}
	}
	return NULL;
}

//========================================================
// Name   : GetChildValue
// Desc   : Find child with name and return child's value
// Param  :
// Return : NULL return if no child.
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPCTSTR	_tagXMLNode::GetChildValue( LPCTSTR name )
{
	LPXNode node = GetChild( name );
	return (node != NULL)? (LPCTSTR)node->value : NULL;
}

LPXAttr _tagXMLNode::GetChildAttr( LPCTSTR name, LPCTSTR attrname )
{
	LPXNode node = GetChild(name);
	return node ? node->GetAttr(attrname) : NULL;
}

LPCTSTR _tagXMLNode::GetChildAttrValue( LPCTSTR name, LPCTSTR attrname )
{
	LPXAttr attr = GetChildAttr( name, attrname );
	return attr ? (LPCTSTR)attr->value : NULL;
}


//========================================================
// Name   : GetChildIterator
// Desc   : get child nodes iterator
// Param  :
// Return : NULL return if no childs.
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
XNodes::iterator _tagXMLNode::GetChildIterator( LPXNode node )
{
	XNodes::iterator it = childs.begin();
	for( ; it != childs.end() ; ++(it) )
	{
		if( *it == node )
			return it;
	}
	return childs.end();
}

//========================================================
// Name   : AppendChild
// Desc   : add node
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPXNode	_tagXMLNode::AppendChild( LPCTSTR name /*= NULL*/, LPCTSTR value /*= NULL*/ )
{
	return AppendChild( CreateNode( name, value ) );
}

LPXNode	_tagXMLNode::AppendChild( LPCTSTR name /*= NULL*/, float value /*= NULL*/ )
{
	return AppendChild( name, ssprintf("%f",value) );
}

LPXNode	_tagXMLNode::AppendChild( LPCTSTR name /*= NULL*/, int value /*= NULL*/ )
{
	return AppendChild( name, ssprintf("%d",value) );
}

//========================================================
// Name   : AppendChild
// Desc   : add node
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPXNode _tagXMLNode::AppendChild( LPXNode node )
{
	node->parent = this;
	childs.push_back( node );
	return node;
}

//========================================================
// Name   : RemoveChild
// Desc   : detach node and delete object
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
bool _tagXMLNode::RemoveChild( LPXNode node )
{
	XNodes::iterator it = GetChildIterator( node );
	if( it != childs.end() )
	{
		delete *it;
		childs.erase( it );
		return true;
	}
	return false;
}

//========================================================
// Name   : GetAttr
// Desc   : get attribute with index in attribute list
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPXAttr _tagXMLNode::GetAttr( int i )
{
	if( i >= 0 && i < (int)attrs.size() )
		return attrs[i];
	return NULL;
}

//========================================================
// Name   : GetAttrIterator
// Desc   : get attribute iterator
// Param  : 
// Return : std::vector<LPXAttr>::iterator
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
XAttrs::iterator _tagXMLNode::GetAttrIterator( LPXAttr attr )
{
	XAttrs::iterator it = attrs.begin();
	for( ; it != attrs.end() ; ++(it) )
	{
		if( *it == attr )
			return it;
	}
	return attrs.end();
}

//========================================================
// Name   : AppendAttr
// Desc   : add attribute
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPXAttr _tagXMLNode::AppendAttr( LPXAttr attr )
{
	attr->parent = this;
	attrs.push_back( attr );
	return attr;
}

//========================================================
// Name   : RemoveAttr
// Desc   : detach attribute and delete object
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
bool _tagXMLNode::RemoveAttr( LPXAttr attr )
{
	XAttrs::iterator it = GetAttrIterator( attr );
	if( it != attrs.end() )
	{
		delete *it;
		attrs.erase( it );
		return true;
	}
	return false;
}

//========================================================
// Name   : CreateNode
// Desc   : Create node object and return it
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPXNode _tagXMLNode::CreateNode( LPCTSTR name /*= NULL*/, LPCTSTR value /*= NULL*/ )
{
	LPXNode node = new XNode;
	node->name = name;
	node->value = value;
	return node;
}

//========================================================
// Name   : CreateAttr
// Desc   : create Attribute object and return it
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPXAttr _tagXMLNode::CreateAttr( LPCTSTR name /*= NULL*/, LPCTSTR value /*= NULL*/ )
{
	LPXAttr attr = new XAttr;
	attr->name = name;
	attr->value = value;
	return attr;
}

//========================================================
// Name   : AppendAttr
// Desc   : add attribute
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPXAttr _tagXMLNode::AppendAttr( LPCTSTR name /*= NULL*/, LPCTSTR value /*= NULL*/ )
{
	return AppendAttr( CreateAttr( name, value ) );
}

//========================================================
// Name   : DetachChild
// Desc   : no delete object, just detach in list
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPXNode _tagXMLNode::DetachChild( LPXNode node )
{
	XNodes::iterator it = GetChildIterator( node );
	if( it != childs.end() )
	{
		childs.erase( it );
		return node;
	}
	return NULL;
}

//========================================================
// Name   : DetachAttr
// Desc   : no delete object, just detach in list
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPXAttr _tagXMLNode::DetachAttr( LPXAttr attr )
{
	XAttrs::iterator it = GetAttrIterator( attr );
	if( it != attrs.end() )
	{
		attrs.erase( it );
		return attr;
	}
	return NULL;
}

_tagXMLEntitys::_tagXMLEntitys( LPXENTITY entities, int count )
{
	for( int i = 0; i < count; i++)
		push_back( entities[i] );
}

LPXENTITY _tagXMLEntitys::GetEntity( int entity )
{
	for( unsigned i = 0 ; i < size(); i ++ )
	{
		if( at(i).entity == entity )
			return LPXENTITY(&at(i));
	}
	return NULL;
}

LPXENTITY _tagXMLEntitys::GetEntity( LPTSTR entity )
{
	for( unsigned i = 0 ; i < size(); i ++ )
	{
		LPTSTR ref = (LPTSTR)at(i).ref;
		LPTSTR ps = entity;
		while( ref && *ref )
			if( *ref++ != *ps++ )
				break;
		if( ref && !*ref )	// found!
			return LPXENTITY(&at(i));
	}
	return NULL;
}

int _tagXMLEntitys::GetEntityCount( LPCTSTR str )
{
	int nCount = 0;
	LPTSTR ps = (LPTSTR)str;
	while( ps && *ps )
		if( GetEntity( *ps++ ) ) nCount ++;
	return nCount;
}

int _tagXMLEntitys::Ref2Entity( LPCTSTR estr, LPTSTR str, int strlen )
{
	LPTSTR pes = (LPTSTR)estr;
	LPTSTR ps = str;
	LPTSTR ps_end = ps+strlen;
	while( pes && *pes && ps < ps_end )
	{
		LPXENTITY ent = GetEntity( pes );
		if( ent )
		{
			// copy entity meanning char
			*ps = ent->entity;
			pes += ent->ref_len;
		}
		else
			*ps = *pes++;	// default character copy
		ps++;
	}
	*ps = '\0';
	
	// total copied characters
	return ps-str;	
}

int _tagXMLEntitys::Entity2Ref( LPCTSTR str, LPTSTR estr, int estrlen )
{
	LPTSTR ps = (LPTSTR)str;
	LPTSTR pes = (LPTSTR)estr;
	LPTSTR pes_end = pes+estrlen;
	while( ps && *ps && pes < pes_end )
	{
		LPXENTITY ent = GetEntity( *ps );
		if( ent )
		{
			// copy entity string
			LPTSTR ref = (LPTSTR)ent->ref;
			while( ref && *ref )
				*pes++ = *ref++;
		}
		else
			*pes++ = *ps;	// default character copy
		ps++;
	}
	*pes = '\0';
	
	// total copied characters
	return pes-estr;
}

CString _tagXMLEntitys::Ref2Entity( LPCTSTR estr )
{
	CString es;
	if( estr )
	{
		int len = _tcslen(estr);
//		LPTSTR esbuf = es.GetBufferSetLength( len+1 );
		char* szTemp = new char[len+1];
		if( szTemp )
			Ref2Entity( estr, szTemp, len );
		es = szTemp;
	}
	return es;
}

CString _tagXMLEntitys::Entity2Ref( LPCTSTR str )
{
	CString s;
	if( str )
	{
		int nEntityCount = GetEntityCount(str);
		if( nEntityCount == 0 )
			return CString(str);
		int len = _tcslen(str) + nEntityCount*10 ;
		//LPTSTR sbuf = s.GetBufferSetLength( len+1 );
		char* szTemp = new char[len+1];
		if( szTemp )
			Entity2Ref( str, szTemp, len );
		s = szTemp;
	}
	return s;
}

CString XRef2Entity( LPCTSTR estr )
{
	return entityDefault.Ref2Entity( estr );
}

CString XEntity2Ref( LPCTSTR str )
{
	return entityDefault.Entity2Ref( str );
}

// XmlFile.h: interface for the XmlFile class.
//
// Adapted from http://www.codeproject.com/cpp/xmlite.asp
// by Chris.  The CodeProject FAQ says "all developers may freely 
// use the code in their own applications".
//
// XmlFile : XML Lite Parser Library
// by bro ( Cho,Kyung Min: bro@shinbiro.com ) 2002-10-30
// History.
// 2002-10-29 : First Coded. Parsing XMLElelement and Attributes.
//              get xml parsed string ( looks good )
// 2002-10-30 : Get Node Functions, error handling ( not completed )
// 2002-12-06 : Helper Funtion string to long
// 2002-12-12 : Entity Helper Support
// 2003-04-08 : Close, 
// 2003-07=23 : add property escape_value. (now no escape on default)
//              fix escape functions
//////////////////////////////////////////////////////////////////////

#include <vector>
#include <RageUtil.h>

// TCHAR defines
#define LPTSTR char*
#define LPCTSTR const char*
#define _tcschr strchr
#ifndef FALSE
#   define FALSE false
#endif
#define _istspace isspace
#define _tcspbrk strpbrk
#define IsEmpty empty
#define _tcslen strlen
#define _tcsecpy strlen

struct _tagXMLAttr;
typedef _tagXMLAttr XAttr, *LPXAttr;
typedef std::vector<LPXAttr> XAttrs;
struct _tagXMLNode;
typedef _tagXMLNode XNode, *LPXNode;
typedef std::vector<LPXNode> XNodes, *LPXNodes;

// Entity Encode/Decode Support
typedef struct _tagXmlEntity
{
	TCHAR entity;					// entity ( & " ' < > )
	TCHAR ref[10];					// entity reference ( &amp; &quot; etc )
	int ref_len;					// entity reference length
}XENTITY,*LPXENTITY;

typedef struct _tagXMLEntitys : public std::vector<XENTITY>
{
	LPXENTITY GetEntity( int entity );
	LPXENTITY GetEntity( LPTSTR entity );	
	int GetEntityCount( LPCTSTR str );
	int Ref2Entity( LPCTSTR estr, LPTSTR str, int strlen );
	int Entity2Ref( LPCTSTR str, LPTSTR estr, int estrlen );
	CString Ref2Entity( LPCTSTR estr );
	CString Entity2Ref( LPCTSTR str );	

	_tagXMLEntitys(){};
	_tagXMLEntitys( LPXENTITY entities, int count );
}XENTITYS,*LPXENTITYS;
extern XENTITYS entityDefault;
CString XRef2Entity( LPCTSTR estr );
CString XEntity2Ref( LPCTSTR str );	

typedef enum 
{
	PIE_PARSE_WELFORMED	= 0,
	PIE_ALONE_NOT_CLOSED,
	PIE_NOT_CLOSED,
	PIE_NOT_NESTED,
	PIE_ATTR_NO_VALUE
}PCODE;

// Parse info.
typedef struct _tagParseInfo
{
	bool		trim_value;			// [set] do trim when parse?
	bool		entity_value;		// [set] do convert from reference to entity? ( &lt; -> < )
	LPXENTITYS	entitys;			// [set] entity table for entity decode
	TCHAR		escape_value;		// [set] escape value (default '\\')

	LPTSTR		xml;				// [get] xml source
	bool		erorr_occur;		// [get] is occurance of error?
	LPTSTR		error_pointer;		// [get] error position of xml source
	PCODE		error_code;			// [get] error code
	CString		error_string;		// [get] error string

	_tagParseInfo() { trim_value = true; entity_value = true; entitys = &entityDefault; xml = NULL; erorr_occur = false; error_pointer = NULL; error_code = PIE_PARSE_WELFORMED; escape_value = 0; }
}PARSEINFO,*LPPARSEINFO;
extern PARSEINFO piDefault;

// display optional environment
typedef struct _tagDispOption
{
	bool newline;			// newline when new tag
	bool reference_value;	// do convert from entity to reference ( < -> &lt; )
	LPXENTITYS	entitys;	// entity table for entity encode

	int tab_base;			// internal usage
	_tagDispOption() { newline = true; reference_value = true; entitys = &entityDefault; tab_base = 0; }
}DISP_OPT, *LPDISP_OPT;
extern DISP_OPT optDefault;

// XAttr : Attribute Implementation
typedef struct _tagXMLAttr
{
	CString name;
	CString	value;
	
	_tagXMLNode*	parent;

	CString GetXML( LPDISP_OPT opt = &optDefault );
}XAttr, *LPXAttr;

// XMLNode structure
typedef struct _tagXMLNode
{
	// name and value
	CString name;
	CString	value;

	// internal variables
	LPXNode	parent;		// parent node
	XNodes	childs;		// child node
	XAttrs	attrs;		// attributes

	// Load/Save XML
	LPTSTR	Load( LPCTSTR pszXml, LPPARSEINFO pi = &piDefault );
	LPTSTR	LoadAttributes( LPCTSTR pszAttrs, LPPARSEINFO pi = &piDefault );
	CString GetXML( LPDISP_OPT opt = &optDefault );

	// in own attribute list
	LPXAttr	GetAttr( LPCTSTR attrname ); 
	LPCTSTR	GetAttrValue( LPCTSTR attrname ); 
	XAttrs	GetAttrs( LPCTSTR name ); 

	// in one level child nodes
	LPXNode	GetChild( LPCTSTR name ); 
	LPCTSTR	GetChildValue( LPCTSTR name ); 
	XNodes	GetChilds( LPCTSTR name ); 
	XNodes	GetChilds(); 

	LPXAttr GetChildAttr( LPCTSTR name, LPCTSTR attrname );
	LPCTSTR GetChildAttrValue( LPCTSTR name, LPCTSTR attrname );
	
	// modify DOM 
	int		GetChildCount();
	LPXNode GetChild( int i );
	XNodes::iterator GetChildIterator( LPXNode node );
	LPXNode CreateNode( LPCTSTR name = NULL, LPCTSTR value = NULL );
	LPXNode	AppendChild( LPCTSTR name = NULL, LPCTSTR value = NULL );
	LPXNode	AppendChild( LPCTSTR name, float value );
	LPXNode	AppendChild( LPCTSTR name, int value );
	LPXNode	AppendChild( LPXNode node );
	bool	RemoveChild( LPXNode node );
	LPXNode DetachChild( LPXNode node );


	LPXAttr GetAttr( int i );
	XAttrs::iterator GetAttrIterator( LPXAttr node );
	LPXAttr CreateAttr( LPCTSTR anem = NULL, LPCTSTR value = NULL );
	LPXAttr AppendAttr( LPCTSTR name = NULL, LPCTSTR value = NULL );
	LPXAttr	AppendAttr( LPXAttr attr );
	bool	RemoveAttr( LPXAttr attr );
	LPXAttr DetachAttr( LPXAttr attr );

	// operator overloads
	LPXNode operator [] ( int i ) { return GetChild(i); }

	_tagXMLNode() { parent = NULL; }
	~_tagXMLNode();

	void Close();
}XNode, *LPXNode;

// Helper Funtion
inline long XStr2Int( LPCTSTR str, long default_value = 0 )
{
	return str ? atol(str) : default_value;
}

inline bool XIsEmptyString( LPCTSTR str )
{
	CString s(str);
	TrimLeft( s );
	TrimRight( s );

	return ( s.empty() || s == "" );
}


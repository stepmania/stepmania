#ifndef XmlFile_H
#define XmlFile_H
// XmlFile.h: interface for the XmlFile class.
//
// Adapted from http://www.codeproject.com/cpp/xmlite.asp.
// On 2004-02-09 Cho,Kyung Min gave us permission to use and modify this 
// library.
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
	LPXENTITY GetEntity( char* entity );	
	int GetEntityCount( const char* str );
	int Ref2Entity( const char* estr, char* str, int strlen );
	int Entity2Ref( const char* str, char* estr, int estrlen );
	CString Ref2Entity( const char* estr );
	CString Entity2Ref( const char* str );	

	_tagXMLEntitys(){};
	_tagXMLEntitys( LPXENTITY entities, int count );
}XENTITYS,*LPXENTITYS;
extern XENTITYS entityDefault;
CString XRef2Entity( const char* estr );
CString XEntity2Ref( const char* str );	

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

	char*		xml;				// [get] xml source
	bool		erorr_occur;		// [get] is occurance of error?
	char*		error_pointer;		// [get] error position of xml source
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
	void GetValue(CString &out) const	{ out = value; }
	void GetValue(int &out) const		{ out = atoi(value); }
	void GetValue(float &out) const		{ out = (float) atof(value); }
	void GetValue(bool &out) const		{ out = atoi(value) != 0; }
	
	_tagXMLNode*	parent;

	CString GetXML( LPDISP_OPT opt = &optDefault );
}XAttr, *LPXAttr;

// XMLNode structure
typedef struct _tagXMLNode
{
	// name and value
	CString name;
	CString	value;
	void GetValue(CString &out) const	{ out = value; }
	void GetValue(int &out) const		{ out = atoi(value); }
	void GetValue(float &out) const		{ out = (float) atof(value); }
	void GetValue(bool &out) const		{ out = atoi(value) != 0; }

	// internal variables
	LPXNode	parent;		// parent node
	XNodes	childs;		// child node
	XAttrs	attrs;		// attributes

	// Load/Save XML
	char*	Load( const char* pszXml, LPPARSEINFO pi = &piDefault );
	char*	LoadAttributes( const char* pszAttrs, LPPARSEINFO pi = &piDefault );
	CString GetXML( LPDISP_OPT opt = &optDefault );

	bool LoadFromFile( CString sFile, LPPARSEINFO pi = &piDefault );
	bool SaveToFile( CString sFile, LPDISP_OPT opt = &optDefault );

	// in own attribute list
	const LPXAttr	GetAttr( const char* attrname ) const; 
	LPXAttr	GetAttr( const char* attrname ); 
	const char*	GetAttrValue( const char* attrname ); 
	bool GetAttrValue(const char* name,CString &out) const	{ const XAttr* pAttr=GetAttr(name); if(pAttr==NULL) return false; pAttr->GetValue(out); return true; }
	bool GetAttrValue(const char* name,int &out) const		{ const XAttr* pAttr=GetAttr(name); if(pAttr==NULL) return false; pAttr->GetValue(out); return true; }
	bool GetAttrValue(const char* name,float &out) const	{ const XAttr* pAttr=GetAttr(name); if(pAttr==NULL) return false; pAttr->GetValue(out); return true; }
	bool GetAttrValue(const char* name,bool &out) const		{ const XAttr* pAttr=GetAttr(name); if(pAttr==NULL) return false; pAttr->GetValue(out); return true; }
	XAttrs	GetAttrs( const char* name ); 

	// in one level child nodes
	const LPXNode	GetChild( const char* name ) const; 
	LPXNode	GetChild( const char* name ); 
	const char*	GetChildValue( const char* name ); 
	bool GetChildValue(const char* name,CString &out) const	{ const XNode* pChild=GetChild(name); if(pChild==NULL) return false; pChild->GetValue(out); return true; }
	bool GetChildValue(const char* name,int &out) const		{ const XNode* pChild=GetChild(name); if(pChild==NULL) return false; pChild->GetValue(out); return true; }
	bool GetChildValue(const char* name,float &out) const	{ const XNode* pChild=GetChild(name); if(pChild==NULL) return false; pChild->GetValue(out); return true; }
	bool GetChildValue(const char* name,bool &out) const	{ const XNode* pChild=GetChild(name); if(pChild==NULL) return false; pChild->GetValue(out); return true; }
	XNodes	GetChilds( const char* name ); 
	XNodes	GetChilds(); 

	LPXAttr GetChildAttr( const char* name, const char* attrname );
	const char* GetChildAttrValue( const char* name, const char* attrname );
	
	// modify DOM 
	int		GetChildCount();
	LPXNode GetChild( int i );
	XNodes::iterator GetChildIterator( LPXNode node );
	LPXNode CreateNode( const char* name = NULL, const char* value = NULL );
	LPXNode	AppendChild( const char* name = NULL, const char* value = NULL );
	LPXNode	AppendChild( const char* name, float value );
	LPXNode	AppendChild( const char* name, int value );
	LPXNode	AppendChild( LPXNode node );
	bool	RemoveChild( LPXNode node );
	LPXNode DetachChild( LPXNode node );


	LPXAttr GetAttr( int i );
	XAttrs::iterator GetAttrIterator( LPXAttr node );
	LPXAttr CreateAttr( const char* anem = NULL, const char* value = NULL );
	LPXAttr AppendAttr( const char* name = NULL, const char* value = NULL );
	LPXAttr AppendAttr( const char* name, float value );
	LPXAttr AppendAttr( const char* name, int value );
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
inline long XStr2Int( const char* str, long default_value = 0 )
{
	return str ? atol(str) : default_value;
}

inline bool XIsEmptyString( const char* str )
{
	CString s(str);
	TrimLeft( s );
	TrimRight( s );

	return ( s.empty() || s == "" );
}

#endif

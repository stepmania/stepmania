#ifndef XmlFile_H
#define XmlFile_H
// XmlFile.h: interface for the XmlFile class.
//
// Adapted from http://www.codeproject.com/cpp/xmlite.asp.
// On 2004-02-09 Cho, Kyung-Min gave us permission to use and modify this 
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

#include <map>
struct DateTime;
class RageFileBasic;

struct XAttr;
typedef multimap<CString,XAttr*> XAttrs;
struct XNode;
typedef multimap<CString,XNode*> XNodes;

#define FOREACH_Attr( pNode, Var ) \
	XAttrs::iterator Var##Iter; \
	XAttr *Var = NULL; \
	for( Var##Iter = (pNode)->m_attrs.begin(), Var = Var##Iter->second; \
		Var##Iter != (pNode)->m_attrs.end(); \
		++Var##Iter, Var = Var##Iter->second )

#define FOREACH_CONST_Attr( pNode, Var ) \
	XAttrs::const_iterator Var##Iter; \
	const XAttr *Var = NULL; \
	for( Var##Iter = (pNode)->m_attrs.begin(), Var = Var##Iter->second; \
		Var##Iter != (pNode)->m_attrs.end(); \
		++Var##Iter, Var = Var##Iter->second )

#define FOREACH_Child( pNode, Var ) \
	XNodes::iterator Var##Iter; \
	XNode *Var = NULL; \
	for( Var##Iter = (pNode)->m_childs.begin(), Var = Var##Iter->second; \
		Var##Iter != (pNode)->m_childs.end(); \
		++Var##Iter, Var = Var##Iter->second )

#define FOREACH_CONST_Child( pNode, Var ) \
	XNodes::const_iterator Var##Iter; \
	const XNode *Var = NULL; \
	for( Var##Iter = (pNode)->m_childs.begin(), Var = Var##Iter->second; \
		Var##Iter != (pNode)->m_childs.end(); \
		++Var##Iter, Var = Var##Iter->second )


// Entity Encode/Decode Support
struct XENTITY
{
	char entity;					// entity ( & " ' < > )
	char ref[10];					// entity reference ( &amp; &quot; etc )
	int ref_len;					// entity reference length
};

struct XENTITYS : public vector<XENTITY>
{
	const XENTITY *GetEntity( char entity ) const;
	const XENTITY *GetEntity( const char* entity ) const;
	int GetEntityCount( const char* str ) const;
	int Ref2Entity( const char* estr, char* str, int strlen ) const;
	int Entity2Ref( const char* str, char* estr, int estrlen ) const;
	CString Ref2Entity( const char* estr ) const;
	CString Entity2Ref( const char* str ) const;

	XENTITYS(){};
	XENTITYS( const XENTITY *entities, int count );
};
extern XENTITYS entityDefault;

enum PCODE
{
	PIE_PARSE_WELL_FORMED = 0,
	PIE_ALONE_NOT_CLOSED,
	PIE_NOT_CLOSED,
	PIE_NOT_NESTED,
	PIE_ATTR_NO_VALUE,
	PIE_READ_ERROR
};

// Parse info.
struct PARSEINFO
{
	bool		trim_value;			// [set] do trim when parse?
	bool		entity_value;		// [set] do convert from reference to entity? ( &lt; -> < )
	XENTITYS	*entitys;			// [set] entity table for entity decode
	char		escape_value;		// [set] escape value (default '\\')

	char*		xml;				// [get] xml source
	bool		error_occur;		// [get] is occurance of error?
	const char*	error_pointer;		// [get] error position of xml source
	PCODE		error_code;			// [get] error code
	CString		error_string;		// [get] error string

	PARSEINFO() { trim_value = true; entity_value = true; entitys = &entityDefault; xml = NULL; error_occur = false; error_pointer = NULL; error_code = PIE_PARSE_WELL_FORMED; escape_value = 0; }
};

// display optional environment
struct DISP_OPT
{
	bool newline;			// newline when new tag
	bool reference_value;	// do convert from entity to reference ( < -> &lt; )
	XENTITYS	*entitys;	// entity table for entity encode
	CString stylesheet;		// empty string = no stylesheet
	bool write_tabs;		// if false, don't write tab indent characters

	int tab_base;			// internal usage
	DISP_OPT()
	{
		newline = true;
		reference_value = true;
		entitys = &entityDefault;
		stylesheet = "";
		write_tabs = true;
		tab_base = 0;
	}
};

// XAttr : Attribute Implementation
struct XAttr
{
	CString m_sName;	// a duplicate of the m_sName in the parent's map
	CString	m_sValue;
	void GetValue( CString &out ) const;
	void GetValue( int &out ) const;
	void GetValue( float &out ) const;
	void GetValue( bool &out ) const;
	void GetValue( unsigned &out ) const;
	void GetValue( DateTime &out ) const;
	
	bool GetXML( RageFileBasic &f, DISP_OPT *opt ) const;
};

// XMLNode structure
struct XNode
{
	CString m_sName;	// a duplicate of the m_sName in the parent's map
	CString	m_sValue;
	XNodes	m_childs;		// child node
	XAttrs	m_attrs;		// attributes

	void GetValue( CString &out ) const;
	void GetValue( int &out ) const;
	void GetValue( float &out ) const;
	void GetValue( bool &out ) const;
	void GetValue( unsigned &out ) const;
	void GetValue( DateTime &out ) const;
	void SetValue( int v );
	void SetValue( float v );
	void SetValue( bool v );
	void SetValue( unsigned v );
	void SetValue( const DateTime &v );

	// Load/Save XML
	const char *Load( const char* pszXml, PARSEINFO *pi );
	const char *LoadAttributes( const char* pszAttrs, PARSEINFO *pi );
	bool GetXML( RageFileBasic &f, DISP_OPT *opt ) const;
	CString GetXML() const;

	bool LoadFromFile( const CString &sFile );
	bool LoadFromFile( RageFileBasic &f );
	bool SaveToFile( const CString &sFile, DISP_OPT *opt ) const;
	bool SaveToFile( RageFileBasic &f, DISP_OPT *opt ) const;

	// in own attribute list
	const XAttr *GetAttr( const CString &sAttrName ) const; 
	XAttr *GetAttr( const CString &sAttrName ); 
	bool GetAttrValue( const CString &sName, CString &out ) const  { const XAttr* pAttr=GetAttr(sName); if(pAttr==NULL) return false; pAttr->GetValue(out); return true; }
	bool GetAttrValue( const CString &sName, int &out ) const      { const XAttr* pAttr=GetAttr(sName); if(pAttr==NULL) return false; pAttr->GetValue(out); return true; }
	bool GetAttrValue( const CString &sName, float &out ) const    { const XAttr* pAttr=GetAttr(sName); if(pAttr==NULL) return false; pAttr->GetValue(out); return true; }
	bool GetAttrValue( const CString &sName, bool &out ) const     { const XAttr* pAttr=GetAttr(sName); if(pAttr==NULL) return false; pAttr->GetValue(out); return true; }
	bool GetAttrValue( const CString &sName, unsigned &out ) const { const XAttr* pAttr=GetAttr(sName); if(pAttr==NULL) return false; pAttr->GetValue(out); return true; }
	bool GetAttrValue( const CString &sName, DateTime &out ) const { const XAttr* pAttr=GetAttr(sName); if(pAttr==NULL) return false; pAttr->GetValue(out); return true; }

	// in one level child nodes
	const XNode *GetChild( const CString &sName ) const; 
	XNode *GetChild( const CString &sName ); 
	bool GetChildValue( const CString &sName, CString &out ) const  { const XNode* pChild=GetChild(sName); if(pChild==NULL) return false; pChild->GetValue(out); return true; }
	bool GetChildValue( const CString &sName, int &out ) const      { const XNode* pChild=GetChild(sName); if(pChild==NULL) return false; pChild->GetValue(out); return true; }
	bool GetChildValue( const CString &sName, float &out ) const    { const XNode* pChild=GetChild(sName); if(pChild==NULL) return false; pChild->GetValue(out); return true; }
	bool GetChildValue( const CString &sName, bool &out ) const     { const XNode* pChild=GetChild(sName); if(pChild==NULL) return false; pChild->GetValue(out); return true; }
	bool GetChildValue( const CString &sName, unsigned &out ) const { const XNode* pChild=GetChild(sName); if(pChild==NULL) return false; pChild->GetValue(out); return true; }
	bool GetChildValue( const CString &sName, DateTime &out ) const { const XNode* pChild=GetChild(sName); if(pChild==NULL) return false; pChild->GetValue(out); return true; }

	// modify DOM 
	XNode *AppendChild( const CString &sName = CString(), const CString &value = CString() );
	XNode *AppendChild( const CString &sName, float value );
	XNode *AppendChild( const CString &sName, int value );
	XNode *AppendChild( const CString &sName, unsigned value );
	XNode *AppendChild( const CString &sName, const DateTime &value );
	XNode *AppendChild( XNode *node );
	bool RemoveChild( XNode *node );

	XAttr *AppendAttr( const CString &sName = CString(), const CString &sValue = CString() );
	XAttr *AppendAttr( const CString &sName, float value );
	XAttr *AppendAttr( const CString &sName, int value );
	XAttr *AppendAttr( const CString &sName, unsigned value );
	XAttr *AppendAttr( const CString &sName, const DateTime &value );
	XAttr *AppendAttr( XAttr *pAttr );
	bool RemoveAttr( XAttr *pAttr );

	// creates the attribute if it doesn't already exist
	void SetAttrValue( const CString &sName, const CString &sValue );

	XNode() { }
	~XNode();

	void Clear();
};

#endif

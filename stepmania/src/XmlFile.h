/* XmlFile - Simple XML reading and writing. */

#ifndef XmlFile_H
#define XmlFile_H

#include <map>
struct DateTime;
class RageFileBasic;

struct XAttr;
typedef map<CString,XAttr*> XAttrs;
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

	char*		xml;				// [get] xml source
	bool		error_occur;		// [get] is occurance of error?
	const char*	error_pointer;		// [get] error position of xml source
	PCODE		error_code;			// [get] error code
	CString		error_string;		// [get] error string

	PARSEINFO() { trim_value = true; entity_value = true; xml = NULL; error_occur = false; error_pointer = NULL; error_code = PIE_PARSE_WELL_FORMED; }
};

// display optional environment
struct DISP_OPT
{
	bool newline;			// newline when new tag
	bool reference_value;	// do convert from entity to reference ( < -> &lt; )
	CString stylesheet;		// empty string = no stylesheet
	bool write_tabs;		// if false, don't write tab indent characters

	int tab_base;			// internal usage
	DISP_OPT()
	{
		newline = true;
		reference_value = true;
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
	unsigned Load( const CString &sXml, PARSEINFO *pi, unsigned iOffset = 0 );
	unsigned LoadAttributes( const CString &sAttrs, PARSEINFO *pi, unsigned iOffset );
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
	XNode( const XNode &cpy );
	~XNode();

	void Clear();
};

#endif

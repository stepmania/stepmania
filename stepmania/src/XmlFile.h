/* XmlFile - Simple XML reading and writing. */

#ifndef XML_FILE_H
#define XML_FILE_H

#include <map>
struct DateTime;
class RageFileBasic;

typedef map<RString,RString> XAttrs;
class XNode;
typedef multimap<RString,XNode*> XNodes;

#define FOREACH_Attr( pNode, Var ) \
	for( XAttrs::iterator Var = (pNode)->m_attrs.begin(); \
		Var != (pNode)->m_attrs.end(); \
		++Var )

#define FOREACH_CONST_Attr( pNode, Var ) \
	for( XAttrs::const_iterator Var = (pNode)->m_attrs.begin(); \
		Var != (pNode)->m_attrs.end(); \
		++Var )

#define FOREACH_Child( pNode, Var ) \
	XNode *Var = NULL; \
	for( XNodes::iterator Var##Iter = (pNode)->m_childs.begin(); \
		Var = (Var##Iter != (pNode)->m_childs.end())? Var##Iter->second:NULL, \
		Var##Iter != (pNode)->m_childs.end(); \
		++Var##Iter )

#define FOREACH_CONST_Child( pNode, Var ) \
	const XNode *Var = NULL; \
	for( XNodes::const_iterator Var##Iter = (pNode)->m_childs.begin(); \
		Var = (Var##Iter != (pNode)->m_childs.end())? Var##Iter->second:NULL, \
		Var##Iter != (pNode)->m_childs.end(); \
		++Var##Iter )

// display optional environment
struct XMLDisplayOptions
{
	RString stylesheet;	// empty string = no stylesheet
	bool write_tabs;	// if false, don't write tab indent characters

	int tab_base;		// internal usage
	XMLDisplayOptions()
	{
		stylesheet = "";
		write_tabs = true;
		tab_base = 0;
	}
};

// XMLNode structure
class XNode
{
public:
	RString m_sName;	// a duplicate of the m_sName in the parent's map
	RString	m_sValue;
	XNodes	m_childs;	// child node
	XAttrs	m_attrs;	// attributes

	void GetValue( RString &out ) const;
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
	unsigned Load( const RString &sXml, RString &sErrorOut, unsigned iOffset = 0 );
	unsigned LoadAttributes( const RString &sAttrs, RString &sErrorOut, unsigned iOffset );
	bool GetXML( RageFileBasic &f, XMLDisplayOptions &opt ) const;
	bool GetAttrXML( RageFileBasic &f, XMLDisplayOptions &opt, const RString &sName, const RString &sValue ) const;
	RString GetXML() const;

	bool SaveToFile( const RString &sFile, XMLDisplayOptions &opt ) const;
	bool SaveToFile( RageFileBasic &f, XMLDisplayOptions &opt ) const;

	// in own attribute list
	const RString *GetAttr( const RString &sAttrName ) const; 
	RString *GetAttr( const RString &sAttrName ); 
	bool GetAttrValue( const RString &sName, RString &out ) const;
	bool GetAttrValue( const RString &sName, int &out ) const;
	bool GetAttrValue( const RString &sName, float &out ) const;
	bool GetAttrValue( const RString &sName, bool &out ) const;
	bool GetAttrValue( const RString &sName, unsigned &out ) const;
	bool GetAttrValue( const RString &sName, DateTime &out ) const;

	// in one level child nodes
	const XNode *GetChild( const RString &sName ) const; 
	XNode *GetChild( const RString &sName ); 
	bool GetChildValue( const RString &sName, RString &out ) const  { const XNode* pChild=GetChild(sName); if(pChild==NULL) return false; pChild->GetValue(out); return true; }
	bool GetChildValue( const RString &sName, int &out ) const      { const XNode* pChild=GetChild(sName); if(pChild==NULL) return false; pChild->GetValue(out); return true; }
	bool GetChildValue( const RString &sName, float &out ) const    { const XNode* pChild=GetChild(sName); if(pChild==NULL) return false; pChild->GetValue(out); return true; }
	bool GetChildValue( const RString &sName, bool &out ) const     { const XNode* pChild=GetChild(sName); if(pChild==NULL) return false; pChild->GetValue(out); return true; }
	bool GetChildValue( const RString &sName, unsigned &out ) const { const XNode* pChild=GetChild(sName); if(pChild==NULL) return false; pChild->GetValue(out); return true; }
	bool GetChildValue( const RString &sName, DateTime &out ) const { const XNode* pChild=GetChild(sName); if(pChild==NULL) return false; pChild->GetValue(out); return true; }

	// modify DOM 
	XNode *AppendChild( const RString &sName = RString(), const RString &value = RString() );
	XNode *AppendChild( const RString &sName, float value );
	XNode *AppendChild( const RString &sName, int value );
	XNode *AppendChild( const RString &sName, unsigned value );
	XNode *AppendChild( const RString &sName, const DateTime &value );
	XNode *AppendChild( XNode *node );
	bool RemoveChild( XNode *node );

	void AppendAttr( const RString &sName = RString(), const RString &sValue = RString() );
	void AppendAttr( const RString &sName, float value );
	void AppendAttr( const RString &sName, int value );
	void AppendAttr( const RString &sName, unsigned value );
	void AppendAttr( const RString &sName, const DateTime &value );
	bool RemoveAttr( const RString &sName );

	XNode() { }
	XNode( const XNode &cpy );
	~XNode();

	void Clear();
};

#endif

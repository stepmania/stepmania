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
	bool GetAttrXML( RageFileBasic &f, const RString &sName, const RString &sValue ) const;
	bool GetXML( RageFileBasic &f, bool bWriteTabs = true ) const;
	RString GetXML() const;

	bool SaveToFile( const RString &sFile, const RString &sStylesheet = "", bool bWriteTabs = true ) const;
	bool SaveToFile( RageFileBasic &f, const RString &sStylesheet = "", bool bWriteTabs = true ) const;

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
	template <typename T>
	bool GetChildValue( const RString &sName, T &out ) const { const XNode *pChild=GetChild(sName); if(pChild==NULL) return false; pChild->GetValue(out); return true; }

	// modify DOM
	template <typename T>
	XNode *AppendChild( const RString &sName, T value )	{ XNode *p=new XNode(sName); p->SetValue(value); return AppendChild(p); }
	XNode *AppendChild( const RString &sName )		{ XNode *p=new XNode(sName); return AppendChild(p); }
	XNode *AppendChild( XNode *node );
	bool RemoveChild( XNode *node );

	void AppendAttr( const RString &sName, const RString &sValue = RString() );
	void AppendAttr( const RString &sName, float value );
	void AppendAttr( const RString &sName, int value );
	void AppendAttr( const RString &sName, unsigned value );
	void AppendAttr( const RString &sName, const DateTime &value );
	bool RemoveAttr( const RString &sName );

	XNode() { }
	explicit XNode( const RString &sName ) { m_sName = sName; }
	XNode( const XNode &cpy );
	~XNode();

	void Clear();

private:
	bool GetXMLInternal( RageFileBasic &f, bool bWriteTabs, int &iTabBase ) const;
};

#endif

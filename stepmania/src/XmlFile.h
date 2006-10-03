/* XmlFile - Simple XML reading and writing. */

#ifndef XML_FILE_H
#define XML_FILE_H

#include <map>
struct DateTime;
class RageFileBasic;

class XNodeValue
{
public:
	RString	m_sValue;

	void GetValue( RString &out ) const;
	void GetValue( int &out ) const;
	void GetValue( float &out ) const;
	void GetValue( bool &out ) const;
	void GetValue( unsigned &out ) const;

	template<typename T>
	T GetValue() const { T val; GetValue(val); return val; }

	void SetValue( const RString &v );
	void SetValue( int v );
	void SetValue( float v );
	void SetValue( unsigned v );
};

typedef map<RString,XNodeValue*> XAttrs;
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
	XNodeValue m_Value;
	XNodes	m_childs;	// child node
	XAttrs	m_attrs;	// attributes

	void SetName( const RString &sName ) { m_sName = sName; }
	const RString &GetName() const { return m_sName; }
	const RString &GetValue() const { return m_Value.m_sValue; }

	template <typename T>
	void GetValue( T &out ) const { m_Value.GetValue(out); }
	template <typename T>
	void SetValue( const T val ) { m_Value.SetValue(val); }

	// in own attribute list
	const XNodeValue *GetAttr( const RString &sAttrName ) const; 
	XNodeValue *GetAttr( const RString &sAttrName ); 
	template <typename T>
	bool GetAttrValue( const RString &sName, T &out ) const	{ const XNodeValue *pAttr=GetAttr(sName); if(pAttr==NULL) return false; pAttr->GetValue(out); return true; }

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
	bool RemoveChild( XNode *node, bool bDelete = true );

	XNodeValue *AppendAttr( const RString &sName, XNodeValue *pValue, bool bOverwrite = true );
	XNodeValue *AppendAttr( const RString &sName );
	template <typename T>
	XNodeValue *AppendAttr( const RString &sName, T value ) { XNodeValue *pVal = AppendAttr( sName ); pVal->SetValue( value ); return pVal; }
	bool RemoveAttr( const RString &sName );

	XNode() { }
	explicit XNode( const RString &sName ) { m_sName = sName; }
	XNode( const XNode &cpy );
	~XNode();

	void Clear();

private:
	XNode &operator=( const XNode &cpy ); // don't use
};

#endif

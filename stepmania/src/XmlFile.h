/* XmlFile - Simple XML reading and writing. */

#ifndef XML_FILE_H
#define XML_FILE_H

#include <map>
struct DateTime;
class RageFileBasic;
struct lua_State;

class XNodeValue
{
public:
	virtual ~XNodeValue() { }
	virtual XNodeValue *Copy() const = 0;

	virtual void GetValue( RString &out ) const = 0;
	virtual void GetValue( int &out ) const = 0;
	virtual void GetValue( float &out ) const = 0;
	virtual void GetValue( bool &out ) const = 0;
	virtual void GetValue( unsigned &out ) const = 0;
	virtual void PushValue( lua_State *L ) const = 0;

	template<typename T>
	T GetValue() const { T val; GetValue(val); return val; }

	virtual void SetValue( const RString &v ) = 0;
	virtual void SetValue( int v ) = 0;
	virtual void SetValue( float v ) = 0;
	virtual void SetValue( unsigned v ) = 0;
	virtual void SetValueFromStack( lua_State *L ) = 0;
};

class XNodeStringValue: public XNodeValue
{
public:
	RString	m_sValue;

	XNodeValue *Copy() const { return new XNodeStringValue( *this ); }

	void GetValue( RString &out ) const;
	void GetValue( int &out ) const;
	void GetValue( float &out ) const;
	void GetValue( bool &out ) const;
	void GetValue( unsigned &out ) const;
	void PushValue( lua_State *L ) const;

	void SetValue( const RString &v );
	void SetValue( int v );
	void SetValue( float v );
	void SetValue( unsigned v );
	void SetValueFromStack( lua_State *L );
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

class XNode
{
public:
	RString m_sName;	// a duplicate of the m_sName in the parent's map
	XNodes	m_childs;	// child node
	XAttrs	m_attrs;	// attributes

	void SetName( const RString &sName ) { m_sName = sName; }
	const RString &GetName() const { return m_sName; }

	static const RString TEXT_ATTRIBUTE;
	template <typename T>
	void GetTextValue( T &out ) const { GetAttrValue(TEXT_ATTRIBUTE, out); }

	// in own attribute list
	const XNodeValue *GetAttr( const RString &sAttrName ) const; 
	XNodeValue *GetAttr( const RString &sAttrName ); 
	template <typename T>
	bool GetAttrValue( const RString &sName, T &out ) const	{ const XNodeValue *pAttr=GetAttr(sName); if(pAttr==NULL) return false; pAttr->GetValue(out); return true; }
	bool PushAttrValue( lua_State *L, const RString &sName ) const;

	// in one level child nodes
	const XNode *GetChild( const RString &sName ) const;
	XNode *GetChild( const RString &sName );
	template <typename T>
	bool GetChildValue( const RString &sName, T &out ) const { const XNode *pChild=GetChild(sName); if(pChild==NULL) return false; pChild->GetTextValue(out); return true; }
	bool PushChildValue( lua_State *L, const RString &sName ) const;

	// modify DOM
	template <typename T>
	XNode *AppendChild( const RString &sName, T value )	{ XNode *p=AppendChild(sName); p->AppendAttr(XNode::TEXT_ATTRIBUTE, value); return p; }
	XNode *AppendChild( const RString &sName )		{ XNode *p=new XNode(sName); return AppendChild(p); }
	XNode *AppendChild( XNode *node );
	bool RemoveChild( XNode *node, bool bDelete = true );

	XNodeValue *AppendAttrFrom( const RString &sName, XNodeValue *pValue, bool bOverwrite = true );
	XNodeValue *AppendAttr( const RString &sName );
	template <typename T>
	XNodeValue *AppendAttr( const RString &sName, T value ) { XNodeValue *pVal = AppendAttr( sName ); pVal->SetValue( value ); return pVal; }
	bool RemoveAttr( const RString &sName );

	XNode();
	explicit XNode( const RString &sName );
	XNode( const XNode &cpy );
	~XNode() { Free(); }

	void Clear();

private:
	void Free();
	XNode &operator=( const XNode &cpy ); // don't use
};

#endif

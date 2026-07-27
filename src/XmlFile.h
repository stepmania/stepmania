/* XmlFile - Simple XML reading and writing. */

#ifndef XML_FILE_H
#define XML_FILE_H

#include <map>
#include <vector>


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

typedef std::map<RString,XNodeValue*> XAttrs;
class XNode;
typedef std::vector<XNode*> XNodes;
/** @brief Loop through each node. */
#define FOREACH_Attr( pNode, Var ) \
	for( XAttrs::iterator Var = (pNode)->m_attrs.begin(); \
		Var != (pNode)->m_attrs.end(); \
		++Var )
/** @brief Loop through each node, using a constant iterator. */
#define FOREACH_CONST_Attr( pNode, Var ) \
	for( XAttrs::const_iterator Var = (pNode)->m_attrs.begin(); \
		Var != (pNode)->m_attrs.end(); \
		++Var )
/** @brief Loop through each child. */
#define FOREACH_Child( pNode, Var ) \
	XNode *Var = nullptr; \
	for( XNodes::iterator Var##Iter = (pNode)->GetChildrenBegin(); \
		Var = (Var##Iter != (pNode)->GetChildrenEnd())? *Var##Iter:nullptr, \
		Var##Iter != (pNode)->GetChildrenEnd(); \
		++Var##Iter )
/** @brief Loop through each child, using a constant iterator. */
#define FOREACH_CONST_Child( pNode, Var ) \
	const XNode *Var = nullptr; \
	for( XNodes::const_iterator Var##Iter = (pNode)->GetChildrenBegin(); \
		Var = (Var##Iter != (pNode)->GetChildrenEnd())? *Var##Iter:nullptr, \
		Var##Iter != (pNode)->GetChildrenEnd(); \
		++Var##Iter )

class XNode
{
private:
	XNodes	m_childs;	// child nodes
	std::multimap<RString, XNode*> m_children_by_name;

public:
	RString m_sName;
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
	bool GetAttrValue( const RString &sName, T &out ) const	{ const XNodeValue *pAttr=GetAttr(sName); if(pAttr== nullptr) return false; pAttr->GetValue(out); return true; }
	bool PushAttrValue( lua_State *L, const RString &sName ) const;

	XNodes::iterator GetChildrenBegin() { return m_childs.begin(); }
	XNodes::const_iterator GetChildrenBegin() const { return m_childs.begin(); }
	XNodes::iterator GetChildrenEnd() { return m_childs.end(); }
	XNodes::const_iterator GetChildrenEnd() const { return m_childs.end(); }
	bool ChildrenEmpty() const { return m_childs.empty(); }

	// in one level child nodes
	const XNode *GetChild( const RString &sName ) const;
	XNode *GetChild( const RString &sName );
	template <typename T>
	bool GetChildValue( const RString &sName, T &out ) const { const XNode *pChild=GetChild(sName); if(pChild== nullptr) return false; pChild->GetTextValue(out); return true; }
	bool PushChildValue( lua_State *L, const RString &sName ) const;

	// modify DOM
	template <typename T>
	XNode *AppendChild( const RString &sName, T value )	{ XNode *p=AppendChild(sName); p->AppendAttr(XNode::TEXT_ATTRIBUTE, value); return p; }
	XNode *AppendChild( const RString &sName )		{ XNode *p=new XNode(sName); return AppendChild(p); }
	XNode *AppendChild( XNode *node );
	bool RemoveChild( XNode *node, bool bDelete = true );
	void RemoveChildFromByName(XNode *node);
	void RenameChildInByName(XNode* node);

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

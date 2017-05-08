/* XmlFile - Simple XML reading and writing. */

#ifndef XML_FILE_H
#define XML_FILE_H

#include <map>
#include <unordered_map>
struct DateTime;
class RageFileBasic;
struct lua_State;

class XNodeValue
{
public:
	virtual ~XNodeValue() { }
	virtual XNodeValue *Copy() const = 0;

	virtual void GetValue( std::string &out ) const = 0;
	virtual void GetValue( int &out ) const = 0;
	virtual void GetValue( float &out ) const = 0;
	virtual void GetValue( bool &out ) const = 0;
	virtual void GetValue( unsigned &out ) const = 0;
	virtual void PushValue( lua_State *L ) const = 0;

	template<typename T>
	T GetValue() const { T val; GetValue(val); return val; }

	virtual void SetValue( std::string const &v ) = 0;
	virtual void SetValue( int v ) = 0;
	virtual void SetValue( float v ) = 0;
	virtual void SetValue( unsigned v ) = 0;
	virtual void SetValueFromStack( lua_State *L ) = 0;
};

class XNodeStringValue: public XNodeValue
{
public:
	std::string	m_sValue;

	XNodeValue *Copy() const { return new XNodeStringValue( *this ); }

	void GetValue( std::string &out ) const;
	void GetValue( int &out ) const;
	void GetValue( float &out ) const;
	void GetValue( bool &out ) const;
	void GetValue( unsigned &out ) const;
	void PushValue( lua_State *L ) const;

	void SetValue( std::string const &v );
	void SetValue( int v );
	void SetValue( float v );
	void SetValue( unsigned v );
	void SetValueFromStack( lua_State *L );
};

typedef std::map<std::string,XNodeValue*> XAttrs;
class XNode;
typedef std::vector<XNode*> XNodes;

class XNode
{
private:
	XNodes	m_childs;	// child nodes
	std::unordered_multimap<std::string, XNode*> m_children_by_name;

public:
	std::string m_sName;
	XAttrs	m_attrs;	// attributes

	void SetName( std::string const &sName ) { m_sName = sName; }
	std::string const GetName() const { return m_sName; }

	static std::string const TEXT_ATTRIBUTE;
	template <typename T>
	void GetTextValue( T &out ) const { GetAttrValue(TEXT_ATTRIBUTE, out); }

	// in own attribute list
	const XNodeValue *GetAttr( const std::string &sAttrName ) const;
	XNodeValue *GetAttr( const std::string &sAttrName );
	template <typename T>
	bool GetAttrValue( std::string const &sName, T &out ) const
	{
		const XNodeValue *pAttr=GetAttr(sName);
		if(pAttr==nullptr)
		{
			return false;
		}
		pAttr->GetValue(out);
		return true;
	}
	bool PushAttrValue( lua_State *L, std::string const &sName ) const;

	bool ChildrenEmpty() const { return m_childs.empty(); }

	XNodes::iterator begin()
	{
		return m_childs.begin();
	}
	XNodes::const_iterator begin() const
	{
		return m_childs.begin();
	}
	XNodes::iterator end()
	{
		return m_childs.end();
	}
	XNodes::const_iterator end() const
	{
		return m_childs.end();
	}

	// in one level child nodes
	const XNode *GetChild( std::string const &sName ) const;
	XNode *GetChild( std::string const &sName );
	template <typename T>
	bool GetChildValue( std::string const &sName, T &out ) const
	{
		const XNode *pChild=GetChild(sName);
		if(pChild==nullptr)
		{
			return false;
		}
		pChild->GetTextValue(out);
		return true;
	}
	bool PushChildValue( lua_State *L, std::string const &sName ) const;

	// modify DOM
	template <typename T>
	XNode *AppendChild( std::string const &sName, T value )
	{
		XNode *p=AppendChild(sName);
		p->AppendAttr(XNode::TEXT_ATTRIBUTE, value);
		return p;
	}
	XNode *AppendChild( std::string const &sName )
	{
		XNode *p=new XNode(sName);
		return AppendChild(p);
	}
	XNode *AppendChild( XNode *node );
	bool RemoveChild( XNode *node, bool bDelete = true );
	void RemoveChildFromByName(XNode *node);
	void RenameChildInByName(XNode* node);

	XNodeValue *AppendAttrFrom( std::string const &sName, XNodeValue *pValue, bool bOverwrite = true );
	XNodeValue *AppendAttr( std::string const &sName );
	template <typename T>
	XNodeValue *AppendAttr( std::string const &sName, T value )
	{
		XNodeValue *pVal = AppendAttr( sName );
		pVal->SetValue( value );
		return pVal;
	}
	bool RemoveAttr( std::string const &sName );

	XNode();
	explicit XNode( std::string const &sName );
	XNode( const XNode &cpy );
	~XNode() { Free(); }

	void Clear();

private:
	void Free();
	XNode &operator=( const XNode &cpy ); // don't use
};

#endif

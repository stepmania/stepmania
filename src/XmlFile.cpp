// Adapted from http://www.codeproject.com/cpp/xmlite.asp.
// On 2004-02-09 Cho, Kyung-Min gave us permission to use and modify this
// library.
//
// XmlFile : XML Lite Parser Library
// by Cho, Kyung Min: bro@shinbiro.com 2002-10-30

#include "global.h"
#include "XmlFile.h"
#include "RageFile.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "DateTime.h"
#include "LuaManager.h"

std::string const XNode::TEXT_ATTRIBUTE = "__TEXT__";

XNode::XNode()
{
}

XNode::XNode( std::string const &sName )
{
	m_sName = sName;
}

XNode::XNode( const XNode &cpy ):
	m_sName( cpy.m_sName )
{
	for (auto const &pAttr: cpy.m_attrs)
	{
		this->AppendAttrFrom( pAttr.first, pAttr.second->Copy() );
	}
	for (auto const *c: cpy)
	{
		this->AppendChild( new XNode(*c) );
	}
}

void XNode::Clear()
{
	Free();
}

void XNode::Free()
{
	for (auto *p: *this)
	{
		delete p;
	}
	for (auto &pAttr: this->m_attrs)
	{
		delete pAttr.second;
	}
	m_childs.clear();
	m_children_by_name.clear();
	m_attrs.clear();
}

void XNodeStringValue::GetValue( std::string &out ) const		{ out = m_sValue; }
void XNodeStringValue::GetValue( int &out ) const		{ out = StringToInt(m_sValue); }
void XNodeStringValue::GetValue( float &out ) const		{ out = StringToFloat(m_sValue); }
void XNodeStringValue::GetValue( bool &out ) const		{ out = StringToInt(m_sValue) != 0; }
void XNodeStringValue::GetValue( unsigned &out ) const		{ out = strtoul(m_sValue.c_str(),nullptr,0); }
void XNodeStringValue::PushValue( lua_State *L ) const
{
	LuaHelpers::Push( L, m_sValue );
}

void XNodeStringValue::SetValue( std::string const &v )		{ m_sValue = v; }
void XNodeStringValue::SetValue( int v )			{ m_sValue = fmt::sprintf("%d",v); }
void XNodeStringValue::SetValue( float v )			{ m_sValue = fmt::sprintf("%f",v); }
void XNodeStringValue::SetValue( unsigned v )			{ m_sValue = fmt::sprintf("%u",v); }
void XNodeStringValue::SetValueFromStack( lua_State *L )
{
	LuaHelpers::Pop( L, m_sValue );
}

const XNodeValue *XNode::GetAttr( std::string const &attrname ) const
{
	XAttrs::const_iterator it = m_attrs.find( attrname );
	if( it != m_attrs.end() )
		return it->second;
	return nullptr;
}

bool XNode::PushAttrValue( lua_State *L, std::string const &sName ) const
{
	const XNodeValue *pAttr = GetAttr(sName);
	if( pAttr == nullptr )
	{
		lua_pushnil( L );
		return false;
	}
	pAttr->PushValue( L );
	return true;
}

XNodeValue *XNode::GetAttr( std::string const &attrname )
{
	XAttrs::iterator it = m_attrs.find( attrname );
	if( it != m_attrs.end() )
		return it->second;
	return nullptr;
}

XNode *XNode::GetChild( std::string const &sName )
{
	auto by_name= m_children_by_name.find(sName);
	if(by_name != m_children_by_name.end() &&
		sName == by_name->second->GetName())
	{
		return by_name->second;
	}
	return nullptr;
}

bool XNode::PushChildValue( lua_State *L, std::string const &sName ) const
{
	const XNode *pChild = GetChild(sName);
	if( pChild == nullptr )
	{
		lua_pushnil( L );
		return false;
	}
	pChild->GetAttr(XNode::TEXT_ATTRIBUTE)->PushValue( L );
	return true;
}

const XNode *XNode::GetChild( std::string const &sName ) const
{
	auto by_name= m_children_by_name.find(sName);
	if(by_name != m_children_by_name.end() &&
		sName == by_name->second->GetName())
	{
		return by_name->second;
	}
	return nullptr;
}

XNode *XNode::AppendChild( XNode *node )
{
	DEBUG_ASSERT( node->m_sName.size() );
	m_children_by_name.insert(std::make_pair(node->m_sName, node));
	m_childs.push_back( node );
	return node;
}

// detach node and delete object
bool XNode::RemoveChild(XNode *node, bool bDelete)
{
	XNodes::iterator it = find( m_childs.begin(), m_childs.end(), node );
	if( it == m_childs.end() )
		return false;
	RemoveChildFromByName(node);
	if(bDelete)
	{ delete node; }
	m_childs.erase( it );
	return true;
}

void XNode::RemoveChildFromByName(XNode* node)
{
	auto by_name= m_children_by_name.find(node->m_sName);
	if(by_name != m_children_by_name.end() &&
		node->GetName() == by_name->second->GetName())
	{
		for(; by_name != m_children_by_name.end(); ++by_name)
		{
			if(by_name->second == node)
			{
				m_children_by_name.erase(by_name);
				break;
			}
		}
	}
}

void XNode::RenameChildInByName(XNode* node)
{
	RemoveChildFromByName(node);
	m_children_by_name.insert(std::make_pair(node->m_sName, node));
}


// detach attribute
bool XNode::RemoveAttr( std::string const &sName )
{
	XAttrs::iterator it = m_attrs.find( sName );
	if( it == m_attrs.end() )
		return false;

	delete it->second;
	m_attrs.erase( it );
	return true;
}

/* If bOverwrite is true and a node already exists with that name, the old value will be deleted.
 * If bOverwrite is false and a node already exists with that name, the new value will be deleted. */
XNodeValue *XNode::AppendAttrFrom( std::string const &sName, XNodeValue *pValue, bool bOverwrite )
{
	DEBUG_ASSERT( sName.size() );
	std::pair<XAttrs::iterator,bool> ret = m_attrs.insert( std::make_pair(sName, (XNodeValue *) nullptr) );
	if( !ret.second ) // already existed
	{
		if( bOverwrite )
		{
			delete ret.first->second;
		}
		else
		{
			delete pValue;
			pValue = ret.first->second;
		}
	}

	ret.first->second = pValue;

	return ret.first->second;
};

XNodeValue *XNode::AppendAttr( std::string const &sName )
{
	DEBUG_ASSERT( sName.size() );
	std::pair<XAttrs::iterator,bool> ret = m_attrs.insert( std::make_pair(sName, (XNodeValue *) nullptr) );
	if( ret.second )
		ret.first->second = new XNodeStringValue;
	return ret.first->second; // already existed
}


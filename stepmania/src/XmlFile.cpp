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
#include "Foreach.h"
#include "LuaManager.h"

XNode::XNode()
{
	m_pValue = new XNodeStringValue;
}

XNode::XNode( const RString &sName )
{
	m_sName = sName;
	m_pValue = new XNodeStringValue;
}

XNode::XNode( const XNode &cpy ):
	m_sName( cpy.m_sName )
{
	m_pValue = cpy.m_pValue->Copy();
	FOREACH_CONST_Attr( &cpy, pAttr )
		this->AppendAttrFrom( pAttr->first, pAttr->second->Copy() );
	FOREACH_CONST_Child( &cpy, c )
		this->AppendChild( new XNode(*c) );
}

XNode::~XNode()
{
	Free();
}

void XNode::Clear()
{
	Free();
	m_pValue = new XNodeStringValue;
}

void XNode::Free()
{
	FOREACH_Child( this, p )
		SAFE_DELETE( p );
	FOREACH_Attr( this, pAttr )
		SAFE_DELETE( pAttr->second );
	m_childs.clear();
	m_attrs.clear();

	SAFE_DELETE( m_pValue );
}
	
void XNodeStringValue::GetValue( RString &out ) const		{ out = m_sValue; }
void XNodeStringValue::GetValue( int &out ) const		{ out = atoi(m_sValue); }
void XNodeStringValue::GetValue( float &out ) const		{ out = StringToFloat(m_sValue); }
void XNodeStringValue::GetValue( bool &out ) const		{ out = atoi(m_sValue) != 0; }
void XNodeStringValue::GetValue( unsigned &out ) const		{ out = strtoul(m_sValue,NULL,0); }
void XNodeStringValue::PushValue( lua_State *L ) const
{
	LuaHelpers::Push( L, m_sValue );
}

void XNodeStringValue::SetValue( const RString &v )		{ m_sValue = v; }
void XNodeStringValue::SetValue( int v )			{ m_sValue = ssprintf("%d",v); }
void XNodeStringValue::SetValue( float v )			{ m_sValue = ssprintf("%f",v); }
void XNodeStringValue::SetValue( unsigned v )			{ m_sValue = ssprintf("%u",v); }
void XNodeStringValue::SetValueFromStack( lua_State *L )
{
	LuaHelpers::Pop( L, m_sValue );
}

const XNodeValue *XNode::GetAttr( const RString &attrname ) const
{
	XAttrs::const_iterator it = m_attrs.find( attrname );
	if( it != m_attrs.end() )
		return it->second;
	return NULL;
}

bool XNode::PushAttrValue( lua_State *L, const RString &sName ) const
{
	const XNodeValue *pAttr = GetAttr(sName);
	if( pAttr == NULL )
	{
		lua_pushnil( L );
		return false;
	}
	pAttr->PushValue( L );
	return true;
}

XNodeValue *XNode::GetAttr( const RString &attrname )
{
	XAttrs::iterator it = m_attrs.find( attrname );
	if( it != m_attrs.end() )
		return it->second;
	return NULL;
}

XNode *XNode::GetChild( const RString &sName )
{
	multimap<RString, XNode*>::iterator it = m_childs.find( sName );
	if( it != m_childs.end() )
	{
		DEBUG_ASSERT( sName == it->second->m_sName );
		return it->second;
	}
	return NULL;
}

bool XNode::PushChildValue( lua_State *L, const RString &sName ) const
{
	const XNode *pChild = GetChild(sName);
	if( pChild == NULL )
	{
		lua_pushnil( L );
		return false;
	}
	pChild->m_pValue->PushValue( L );
	return true;
}

const XNode *XNode::GetChild( const RString &sName ) const
{
	multimap<RString, XNode*>::const_iterator it = m_childs.find( sName );
	if( it != m_childs.end() )
	{
		DEBUG_ASSERT( sName == it->second->m_sName );
		return it->second;
	}
	return NULL;
}

XNode *XNode::AppendChild( XNode *node )
{
	DEBUG_ASSERT( node->m_sName.size() );

	/* Hinted insert: optimize for alphabetical inserts, for the copy ctor. */
	m_childs.insert( m_childs.end(), pair<RString,XNode*>(node->m_sName,node) );
	return node;
}

// detach node and delete object
bool XNode::RemoveChild( XNode *node, bool bDelete )
{
	FOREACHMM( RString, XNode*, m_childs, p )
	{
		if( p->second == node )
		{
			if( bDelete )
				SAFE_DELETE( p->second );
			m_childs.erase( p );
			return true;
		}
	}
	return false;
}


// detach attribute
bool XNode::RemoveAttr( const RString &sName )
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
XNodeValue *XNode::AppendAttrFrom( const RString &sName, XNodeValue *pValue, bool bOverwrite )
{
	DEBUG_ASSERT( sName.size() );
	pair<XAttrs::iterator,bool> ret = m_attrs.insert( make_pair(sName, (XNodeValue *) NULL) );
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

XNodeValue *XNode::AppendAttr( const RString &sName )
{
	DEBUG_ASSERT( sName.size() );
	pair<XAttrs::iterator,bool> ret = m_attrs.insert( make_pair(sName, (XNodeValue *) NULL) );
	if( ret.second )
		ret.first->second = new XNodeStringValue;
	return ret.first->second; // already existed
}


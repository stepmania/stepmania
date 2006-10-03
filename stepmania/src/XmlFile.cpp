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

XNode::XNode( const XNode &cpy ):
	m_Value( cpy.m_Value ),
	m_attrs( cpy.m_attrs )
{
	FOREACH_CONST_Child( &cpy, c )
		this->AppendChild( new XNode(*c) );
}

XNode::~XNode()
{
	Clear();
}

void XNode::Clear()
{
	FOREACH_Child( this, p )
		SAFE_DELETE( p );
	FOREACH_Attr( this, pAttr )
		SAFE_DELETE( pAttr->second );
	m_childs.clear();
	m_attrs.clear();
}
	
void XNodeValue::GetValue( RString &out ) const		{ out = m_sValue; }
void XNodeValue::GetValue( int &out ) const		{ out = atoi(m_sValue); }
void XNodeValue::GetValue( float &out ) const		{ out = StringToFloat(m_sValue); }
void XNodeValue::GetValue( bool &out ) const		{ out = atoi(m_sValue) != 0; }
void XNodeValue::GetValue( unsigned &out ) const	{ out = strtoul(m_sValue,NULL,0); }

void XNodeValue::SetValue( const RString &v )		{ m_sValue = v; }
void XNodeValue::SetValue( int v )			{ m_sValue = ssprintf("%d",v); }
void XNodeValue::SetValue( float v )			{ m_sValue = ssprintf("%f",v); }
void XNodeValue::SetValue( unsigned v )			{ m_sValue = ssprintf("%u",v); }

const XNodeValue *XNode::GetAttr( const RString &attrname ) const
{
	XAttrs::const_iterator it = m_attrs.find( attrname );
	if( it != m_attrs.end() )
		return it->second;
	return NULL;
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
	XAttrs::iterator begin = m_attrs.lower_bound( sName );
	XAttrs::iterator end = m_attrs.upper_bound( sName );
	if( begin == end )
		return false;
	m_attrs.erase( begin, end );
	return true;
}

/* If bOverwrite is true and a node already exists with that name, the old value will be deleted.
 * If bOverwrite is false and a node already exists with that name, the new value will be deleted. */
XNodeValue *XNode::AppendAttr( const RString &sName, XNodeValue *pValue, bool bOverwrite )
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
		ret.first->second = new XNodeValue();
	return ret.first->second; // already existed
}


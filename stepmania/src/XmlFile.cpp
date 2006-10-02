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
	m_sName( cpy.m_sName ),
	m_sValue( cpy.m_sValue ),
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
	m_childs.clear();
	m_attrs.clear();
}
	
void XNode::GetValue( RString &out ) const	{ out = m_sValue; }
void XNode::GetValue( int &out ) const		{ out = atoi(m_sValue); }
void XNode::GetValue( float &out ) const	{ out = StringToFloat(m_sValue); }
void XNode::GetValue( bool &out ) const		{ out = atoi(m_sValue) != 0; }
void XNode::GetValue( unsigned &out ) const	{ out = strtoul(m_sValue,NULL,0); }
void XNode::GetValue( DateTime &out ) const	{ out.FromString( m_sValue ); }

bool XNode::GetAttrValue( const RString &sName, RString &out ) const	{ const RString* pAttr=GetAttr(sName); if(pAttr==NULL) return false; out = *pAttr; return true; }
bool XNode::GetAttrValue( const RString &sName, int &out ) const	{ const RString* pAttr=GetAttr(sName); if(pAttr==NULL) return false; out = atoi(*pAttr); return true; }
bool XNode::GetAttrValue( const RString &sName, float &out ) const	{ const RString* pAttr=GetAttr(sName); if(pAttr==NULL) return false; out = StringToFloat(*pAttr); return true; }
bool XNode::GetAttrValue( const RString &sName, bool &out ) const	{ const RString* pAttr=GetAttr(sName); if(pAttr==NULL) return false; out = atoi(*pAttr) != 0; return true; }
bool XNode::GetAttrValue( const RString &sName, unsigned &out ) const	{ const RString* pAttr=GetAttr(sName); if(pAttr==NULL) return false; out = 0; sscanf(*pAttr,"%u",&out); return true; }
bool XNode::GetAttrValue( const RString &sName, DateTime &out ) const	{ const RString* pAttr=GetAttr(sName); if(pAttr==NULL) return false; out.FromString( *pAttr ); return true; }

void XNode::SetValue( const RString &v )		{ m_sValue = v; }
void XNode::SetValue( int v )				{ m_sValue = ssprintf("%d",v); }
void XNode::SetValue( float v )				{ m_sValue = ssprintf("%f",v); }
void XNode::SetValue( unsigned v )			{ m_sValue = ssprintf("%u",v); }
void XNode::SetValue( const DateTime &v )		{ m_sValue = v.GetString(); }

const RString *XNode::GetAttr( const RString &attrname ) const
{
	map<RString, RString>::const_iterator it = m_attrs.find( attrname );
	if( it != m_attrs.end() )
		return &it->second;
	return NULL;
}

RString *XNode::GetAttr( const RString &attrname )
{
	map<RString, RString>::iterator it = m_attrs.find( attrname );
	if( it != m_attrs.end() )
		return &it->second;
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
bool XNode::RemoveChild( XNode *node )
{
	FOREACHMM( RString, XNode*, m_childs, p )
	{
		if( p->second == node )
		{
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
	return m_attrs.erase(sName) > 0;
}

void XNode::AppendAttr( const RString &sName, const RString &sValue )
{
	DEBUG_ASSERT( sName.size() );
	pair<XAttrs::iterator,bool> ret = m_attrs.insert( make_pair(sName,sValue) );
	if( !ret.second )
		ret.first->second = sValue; // already existed
}

void XNode::AppendAttr( const RString &sName, float value )	{ AppendAttr(sName,ssprintf("%f",value)); }
void XNode::AppendAttr( const RString &sName, int value )	{ AppendAttr(sName,ssprintf("%d",value)); }
void XNode::AppendAttr( const RString &sName, unsigned value )	{ AppendAttr(sName,ssprintf("%u",value)); }


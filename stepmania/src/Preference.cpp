#include "global.h"
#include "Preference.h"
#include "XmlFile.h"
#include "RageLog.h"
#include "LuaManager.h"
#include "MessageManager.h"
#include "SubscriptionManager.h"

static SubscriptionManager<IPreference> m_Subscribers;

IPreference::IPreference( const RString& sName ):
	m_sName( sName ),
	m_bIsStatic( false )
{
	m_Subscribers.Subscribe( this );
}

IPreference::~IPreference()
{
	m_Subscribers.Unsubscribe( this );
}

IPreference *IPreference::GetPreferenceByName( const RString &sName )
{
	FOREACHS( IPreference*, *m_Subscribers.m_pSubscribers, p )
	{
		if( !(*p)->GetName().CompareNoCase( sName ) )
			return *p;
	}

	return NULL;
}

void IPreference::LoadAllDefaults()
{
	FOREACHS_CONST( IPreference*, *m_Subscribers.m_pSubscribers, p )
		(*p)->LoadDefault();
}

void IPreference::ReadAllPrefsFromNode( const XNode* pNode, bool bIsStatic )
{
	ASSERT( pNode );
	FOREACHS_CONST( IPreference*, *m_Subscribers.m_pSubscribers, p )
		(*p)->ReadFrom( pNode, bIsStatic );
}

void IPreference::SavePrefsToNode( XNode* pNode )
{
	FOREACHS_CONST( IPreference*, *m_Subscribers.m_pSubscribers, p )
		(*p)->WriteTo( pNode );
}

void IPreference::ReadAllDefaultsFromNode( const XNode* pNode )
{
	if( pNode == NULL )
		return;
	FOREACHS_CONST( IPreference*, *m_Subscribers.m_pSubscribers, p )
		(*p)->ReadDefaultFrom( pNode );
}

void IPreference::PushValue( lua_State *L ) const
{
	if( LOG )
		LOG->Trace( "The preference value \"%s\" is of a type not supported by Lua", m_sName.c_str() );

	lua_pushnil( L );
}

void IPreference::SetFromStack( lua_State *L )
{
	if( LOG )
		LOG->Trace( "The preference value \"%s\" is of a type not supported by Lua", m_sName.c_str() );

	lua_pop( L, 1 );
}
#define READFROM_AND_WRITETO( type ) \
	template<> RString PrefToString( const type &v ) \
	{ \
		return ::ToString( v ); \
	} \
	template<> void PrefFromString( const RString &s, type &v ) \
	{ \
		::FromString( s, v ); \
	} \
	template<> void PrefSetFromStack( lua_State *L, type &v ) \
	{ \
		LuaHelpers::Pop( L, v ); \
	} \
	template<> void PrefPushValue( lua_State *L, const type &v ) \
	{ \
		LuaHelpers::Push( L, v ); \
	}

READFROM_AND_WRITETO( int )
READFROM_AND_WRITETO( float )
READFROM_AND_WRITETO( bool )
READFROM_AND_WRITETO( RString )

void IPreference::ReadFrom( const XNode* pNode, bool bIsStatic )
{
	RString sVal;
	if( pNode->GetAttrValue(m_sName, sVal) )
	{
		FromString( sVal );
		m_bIsStatic = bIsStatic;
	}
}

void IPreference::WriteTo( XNode* pNode ) const
{
	if( !m_bIsStatic )
		pNode->AppendAttr( m_sName, ToString() );
}

/* Load our value from the node, and make it the new default. */
void IPreference::ReadDefaultFrom( const XNode* pNode )
{
	RString sVal;
	if( !pNode->GetAttrValue(m_sName, sVal) )
		return;
	SetDefaultFromString( sVal );
}

void BroadcastPreferenceChanged( const RString& sPreferenceName )
{
	if( MESSAGEMAN )
		MESSAGEMAN->Broadcast( sPreferenceName+"Changed" );
}

/*
 * (c) 2001-2004 Chris Danford, Chris Gomez
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

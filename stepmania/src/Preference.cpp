#include "global.h"
#include "Preference.h"
#include "PrefsManager.h"
#include "IniFile.h"
#include "RageLog.h"
#include "LuaFunctions.h"
#include "LuaManager.h"
#include "MessageManager.h"

static const CString PrefsGroupNames[] = {
	"Debug",
	"Editor",
	"Options",
};
XToString( PrefsGroup, NUM_PREFS_GROUPS );

IPreference::IPreference( PrefsGroup PrefsGroup, const CString& sName ):
	m_PrefsGroup( PrefsGroup ),
	m_sName( sName )
{
	PrefsManager::Subscribe( this );
}

IPreference::~IPreference()
{
	PrefsManager::Unsubscribe( this );
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

#define READFROM_AND_WRITETO( type, cast ) \
	template<> void Preference<type>::FromString( const CString &s ) \
	{ \
		::FromString( s, (cast)m_currentValue ); \
	} \
	template<> CString Preference<type>::ToString() const \
	{ \
		return ::ToString( (cast)m_currentValue ); \
	} \
	template<> void Preference<type>::PushValue( lua_State *L ) const \
	{ \
		LuaHelpers::PushStack( (cast)m_currentValue, L ); \
	} \
	template<> void Preference<type>::SetFromStack( lua_State *L ) \
	{ \
		LuaHelpers::PopStack( (cast)m_currentValue, L ); \
	}

READFROM_AND_WRITETO( int, int& )
READFROM_AND_WRITETO( float, float& )
READFROM_AND_WRITETO( bool, bool& )
READFROM_AND_WRITETO( CString, CString& )
READFROM_AND_WRITETO( PrefsManager::BackgroundMode, int& )
READFROM_AND_WRITETO( PrefsManager::BannerCache, int& )
READFROM_AND_WRITETO( PrefsManager::MusicWheelUsesSections, int& )
READFROM_AND_WRITETO( PrefsManager::MarvelousTiming, int& )
READFROM_AND_WRITETO( CoinMode, int& )
READFROM_AND_WRITETO( Premium, int& )
READFROM_AND_WRITETO( PrefsManager::Maybe, int& )
READFROM_AND_WRITETO( PrefsManager::CharacterOption, int& )
READFROM_AND_WRITETO( PrefsManager::CourseSortOrders, int& )
READFROM_AND_WRITETO( PrefsManager::GetRankingName, int& )
READFROM_AND_WRITETO( PrefsManager::ScoringTypes, int& )
READFROM_AND_WRITETO( PrefsManager::BoostAppPriority, int& )
READFROM_AND_WRITETO( RageSoundReader_Resample::ResampleQuality, int& )

void IPreference::ReadFrom( const IniFile &ini )
{
	CString sVal;
	if( ini.GetValue( PrefsGroupToString(m_PrefsGroup), m_sName, sVal ) )
		FromString( sVal );
}

void IPreference::WriteTo( IniFile &ini ) const
{
	ini.SetValue( PrefsGroupToString(m_PrefsGroup), m_sName, ToString() );
}

void BroadcastPreferenceChanged( const CString& sPreferenceName )
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

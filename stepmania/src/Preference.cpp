#include "global.h"
#include "Preference.h"
#include "PrefsManager.h"
#include "IniFile.h"

static const CString PrefsGroupNames[NUM_PREFS_GROUPS] = {
	"Debug",
	"Editor",
	"Options",
};
XToString( PrefsGroup );

void SubscribePreference( IPreference *p )
{
	PrefsManager::Subscribe( p );
}

void UnsubscribePreference( IPreference *p )
{
	PrefsManager::Unsubscribe( p );
}

#define READFROM_AND_WRITETO( type ) \
	void Preference<type>::ReadFrom( const IniFile &ini ) \
	{ \
		ini.GetValue( PrefsGroupToString(m_PrefsGroup), m_sName, m_currentValue ); \
	} \
	void Preference<type>::WriteTo( IniFile &ini ) const \
	{ \
		ini.SetValue( PrefsGroupToString(m_PrefsGroup), m_sName, m_currentValue ); \
	} \

READFROM_AND_WRITETO( CString )
READFROM_AND_WRITETO( int )
READFROM_AND_WRITETO( float )
READFROM_AND_WRITETO( bool )

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

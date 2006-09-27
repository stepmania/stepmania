#include "global.h"
#include "HelpDisplay.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "ThemeManager.h"
#include "ActorUtil.h"

REGISTER_ACTOR_CLASS( HelpDisplay )

HelpDisplay::HelpDisplay()
{
	m_iCurTipIndex = 0;
	m_fSecsUntilSwitch = 0;
	m_fSecsBetweenSwitches = 1;
}

void HelpDisplay::Load( const RString &sType )
{
	RunCommands( THEME->GetMetricA(sType,"TipOnCommand") );
	m_fSecsUntilSwitch = THEME->GetMetricF(sType,"TipShowTime");
}

void HelpDisplay::SetTips( const vector<RString> &arrayTips, const vector<RString> &arrayTipsAlt )
{ 
	ASSERT( arrayTips.size() == arrayTipsAlt.size() );

	if( arrayTips == m_arrayTips && arrayTipsAlt == m_arrayTipsAlt )
		return;

	SetText( "" );

	m_arrayTips = arrayTips;
	m_arrayTipsAlt = arrayTipsAlt;

	m_iCurTipIndex = 0;
	m_fSecsUntilSwitch = 0;
	Update( 0 );
}


void HelpDisplay::Update( float fDeltaTime )
{
	float fHibernate = m_fHibernateSecondsLeft;

	BitmapText::Update( fDeltaTime );

	if( m_arrayTips.empty() )
		return;

	m_fSecsUntilSwitch -= max( fDeltaTime - fHibernate, 0 );
	if( m_fSecsUntilSwitch > 0 )
		return;

	// time to switch states
	m_fSecsUntilSwitch = m_fSecsBetweenSwitches;
	SetText( m_arrayTips[m_iCurTipIndex], m_arrayTipsAlt[m_iCurTipIndex] );
	m_iCurTipIndex++;
	m_iCurTipIndex = m_iCurTipIndex % m_arrayTips.size();
}


#include "LuaBinding.h"
#include "FontCharAliases.h"
class LunaHelpDisplay: public Luna<HelpDisplay>
{
public:
	static int settips( T* p, lua_State *L )
	{
		luaL_checktype( L, 1, LUA_TTABLE );
		lua_pushvalue( L, 1 );
		vector<RString> arrayTips;
		LuaHelpers::ReadArrayFromTable( arrayTips, L );
		lua_pop( L, 1 );
		for( unsigned i = 0; i < arrayTips.size(); ++i )
			FontCharAliases::ReplaceMarkers( arrayTips[i] );
		if( lua_gettop(L) > 1 && !lua_isnil( L, 2 ) )
		{
			vector<RString> arrayTipsAlt;
			luaL_checktype( L, 2, LUA_TTABLE );
			lua_pushvalue( L, 2 );
			LuaHelpers::ReadArrayFromTable( arrayTipsAlt, L );
			lua_pop( L, 1 );
			for( unsigned i = 0; i < arrayTipsAlt.size(); ++i )
				FontCharAliases::ReplaceMarkers( arrayTipsAlt[i] );

			p->SetTips( arrayTips, arrayTipsAlt );
		}
		else
			p->SetTips( arrayTips );

		return 0;
	}

	static int gettips( T* p, lua_State *L )
	{
		vector<RString> arrayTips, arrayTipsAlt;
		p->GetTips( arrayTips, arrayTipsAlt );

		LuaHelpers::CreateTableFromArray( arrayTips, L );
		LuaHelpers::CreateTableFromArray( arrayTipsAlt, L );

		return 2;
	}
	static int setsecsbetweenswitches( T* p, lua_State *L ) { p->SetSecsBetweenSwitches( FArg(1) ); return 0; }

	LunaHelpDisplay()
	{
		LUA->Register( Register );

		ADD_METHOD( settips );
		ADD_METHOD( gettips );
		ADD_METHOD( setsecsbetweenswitches );
	}
};

LUA_REGISTER_DERIVED_CLASS( HelpDisplay, BitmapText )

/*
 * (c) 2001-2003 Chris Danford, Glenn Maynard
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

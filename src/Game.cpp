#include "global.h"
#include "Game.h"
#include "RageLog.h"

TapNoteScore Game::MapTapNoteScore( TapNoteScore tns ) const
{
	switch( tns )
	{
	case TNS_W1: return m_mapW1To;
	case TNS_W2: return m_mapW2To;
	case TNS_W3: return m_mapW3To;
	case TNS_W4: return m_mapW4To;
	case TNS_W5: return m_mapW5To;
	default: return tns;
	}
}

static const Game::PerButtonInfo g_CommonButtonInfo[] =
{
	{ GameButtonType_INVALID }, // GAME_BUTTON_MENULEFT
	{ GameButtonType_INVALID }, // GAME_BUTTON_MENURIGHT
	{ GameButtonType_INVALID }, // GAME_BUTTON_MENUUP
	{ GameButtonType_INVALID }, // GAME_BUTTON_MENUDOWN
	{ GameButtonType_INVALID }, // GAME_BUTTON_START
	{ GameButtonType_INVALID }, // GAME_BUTTON_SELECT
	{ GameButtonType_INVALID }, // GAME_BUTTON_BACK
	{ GameButtonType_INVALID }, // GAME_BUTTON_COIN
	{ GameButtonType_INVALID }, // GAME_BUTTON_OPERATOR
	{ GameButtonType_INVALID }, // GAME_BUTTON_EFFECT_UP
	{ GameButtonType_INVALID }, // GAME_BUTTON_EFFECT_DOWN
};

const Game::PerButtonInfo *Game::GetPerButtonInfo( GameButton gb ) const
{
	COMPILE_ASSERT( GAME_BUTTON_NEXT == ARRAYLEN(g_CommonButtonInfo) );
	if( gb < GAME_BUTTON_NEXT )
		return &g_CommonButtonInfo[gb];
	else
		return &m_PerButtonInfo[gb-GAME_BUTTON_NEXT];
}

// lua start
#include "LuaBinding.h"

class LunaGame: public Luna<Game>
{
public:
	static int GetName( T* p, lua_State *L )			{ lua_pushstring( L, p->m_szName ); return 1; }

	LunaGame()
	{
		ADD_METHOD( GetName );
	}
};

LUA_REGISTER_CLASS( Game )
// lua end

/*
 * (c) 2001-2002 Chris Danford
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

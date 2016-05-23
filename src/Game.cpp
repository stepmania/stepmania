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
	{ GameButtonType_Menu }, // GAME_BUTTON_MENULEFT
	{ GameButtonType_Menu }, // GAME_BUTTON_MENURIGHT
	{ GameButtonType_Menu }, // GAME_BUTTON_MENUUP
	{ GameButtonType_Menu }, // GAME_BUTTON_MENUDOWN
	{ GameButtonType_Menu }, // GAME_BUTTON_START
	{ GameButtonType_Menu }, // GAME_BUTTON_SELECT
	{ GameButtonType_Menu }, // GAME_BUTTON_BACK
	{ GameButtonType_Menu }, // GAME_BUTTON_COIN
	{ GameButtonType_Menu }, // GAME_BUTTON_OPERATOR
	{ GameButtonType_Menu }, // GAME_BUTTON_EFFECT_UP
	{ GameButtonType_Menu }, // GAME_BUTTON_EFFECT_DOWN
};

const Game::PerButtonInfo *Game::GetPerButtonInfo( GameButton gb ) const
{
	COMPILE_ASSERT( GAME_BUTTON_NEXT == ARRAYLEN(g_CommonButtonInfo) );
	if( gb < GAME_BUTTON_NEXT )
		return &g_CommonButtonInfo[gb];
	else
		return &m_PerButtonInfo[gb-GAME_BUTTON_NEXT];
}

TapNoteScore Game::GetMapJudgmentTo( TapNoteScore tns ) const
{
	switch(tns)
	{
		case TNS_W1: return m_mapW1To;
		case TNS_W2: return m_mapW2To; 
		case TNS_W3: return m_mapW3To;
		case TNS_W4: return m_mapW4To;
		case TNS_W5: return m_mapW5To;
		default: return TapNoteScore_Invalid;
	}
}

// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the Game. */ 
class LunaGame: public Luna<Game>
{
public:
	static int GetName( T* p, lua_State *L )			{ lua_pushstring( L, p->m_szName ); return 1; }
	static int CountNotesSeparately( T* p, lua_State *L )	{ lua_pushboolean( L, p->m_bCountNotesSeparately ); return 1; }
	DEFINE_METHOD( GetMapJudgmentTo, GetMapJudgmentTo(Enum::Check<TapNoteScore>(L, 1)) )
	DEFINE_METHOD(GetSeparateStyles, m_PlayersHaveSeparateStyles);

	LunaGame()
	{
		ADD_METHOD( GetName );
		ADD_METHOD( CountNotesSeparately );
		ADD_METHOD( GetMapJudgmentTo );
		ADD_METHOD( GetSeparateStyles );
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

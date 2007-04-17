#include "global.h"
#include "ScreenProfileLoad.h"
#include "GameState.h"
#include "MemoryCardManager.h"
#include "ScreenManager.h"

REGISTER_SCREEN_CLASS( ScreenProfileLoad );

void ScreenProfileLoad::BeginScreen()
{
	m_bHaveProfileToLoad = GAMESTATE->HaveProfileToLoad();
	if( !m_bHaveProfileToLoad )
	{
		/* We have nothing to load, so just lock the cards. */
		FOREACH_PlayerNumber( pn )
			MEMCARDMAN->LockCard( pn );
	}

	ScreenWithMenuElements::BeginScreen();
}

void ScreenProfileLoad::Input( const InputEventPlus &input )
{
}

void ScreenProfileLoad::Continue()
{
	if( m_bHaveProfileToLoad )
	{
		GAMESTATE->LoadProfiles();
		SCREENMAN->ZeroNextUpdate();
	}

	StartTransitioningScreen( SM_GoToNextScreen );
}

// lua start
#include "LuaBinding.h"

class LunaScreenProfileLoad: public Luna<ScreenProfileLoad>
{
public:
	static int Continue( T* p, lua_State *L )
	{
		LUA->YieldLua();
		p->Continue();
		LUA->UnyieldLua();
		return 0;
	}
	static int HaveProfileToLoad( T* p, lua_State *L )
	{
		LuaHelpers::Push( L, p->m_bHaveProfileToLoad );
		return 1;
	}
	
	LunaScreenProfileLoad()
	{
  		ADD_METHOD( Continue );
  		ADD_METHOD( HaveProfileToLoad );
	}
};

LUA_REGISTER_DERIVED_CLASS( ScreenProfileLoad, ScreenWithMenuElements )
// lua end

/*
 * (c) 2007 Glenn Maynard
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

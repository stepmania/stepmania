#include "global.h"
#include "ScreenProfileSave.h"
#include "GameState.h"
#include "ScreenManager.h"

REGISTER_SCREEN_CLASS( ScreenProfileSave );

void ScreenProfileSave::BeginScreen()
{
	ScreenWithMenuElements::BeginScreen();
}

bool ScreenProfileSave::Input( const InputEventPlus &input )
{
	return false;
}

void ScreenProfileSave::Continue()
{
	GAMESTATE->SavePlayerProfiles();
	SCREENMAN->ZeroNextUpdate();

	StartTransitioningScreen( SM_GoToNextScreen );
}

// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the ScreenProfileSave. */ 
class LunaScreenProfileSave: public Luna<ScreenProfileSave>
{
public:
	static int Continue( T* p, lua_State *L )
	{
		LUA->YieldLua();
		p->Continue();
		LUA->UnyieldLua();
		COMMON_RETURN_SELF;
	}
	static int HaveProfileToSave( T* p, lua_State *L )
	{
		LuaHelpers::Push( L, GAMESTATE->HaveProfileToSave() );
		return 1;
	}
	
	LunaScreenProfileSave()
	{
  		ADD_METHOD( Continue );
  		ADD_METHOD( HaveProfileToSave );
	}
};

LUA_REGISTER_DERIVED_CLASS( ScreenProfileSave, ScreenWithMenuElements )
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
 * 
 * (c) 2016- Electromuis, Anton Grootes
 * This branch of https://github.com/stepmania/stepmania
 * will from here on out be released as GPL v3 (wich converts from the previous MIT license)
 */

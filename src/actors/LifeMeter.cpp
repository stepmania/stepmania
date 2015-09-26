#include "global.h"
#include "LifeMeter.h"
#include "LifeMeterBar.h"
#include "LifeMeterBattery.h"
#include "LifeMeterTime.h"

LifeMeter *LifeMeter::MakeLifeMeter( LifeType t )
{
	switch( t )
	{
	case LifeType_Bar:     return new LifeMeterBar;
	case LifeType_Battery: return new LifeMeterBattery;
	case LifeType_Time:    return new LifeMeterTime;
	default:
		FAIL_M(ssprintf("Unrecognized LifeMeter type: %i", t));
	}
}

// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the LifeMeter. */ 
class LunaLifeMeter: public Luna<LifeMeter>
{
public:
	static int GetLife( T* p, lua_State *L )
	{
		LuaHelpers::Push( L, p->GetLife() );
		return 1;
	}
	static int IsInDanger( T* p, lua_State *L ) { LuaHelpers::Push( L, p->IsInDanger() ); return 1; }
	static int IsHot( T* p, lua_State *L ) { LuaHelpers::Push( L, p->IsHot() ); return 1; }
	static int IsFailing( T* p, lua_State *L ) { LuaHelpers::Push( L, p->IsFailing() ); return 1; }

	LunaLifeMeter()
	{
		ADD_METHOD( GetLife );
		ADD_METHOD( IsInDanger );
		ADD_METHOD( IsHot );
		ADD_METHOD( IsFailing );
	}
};

LUA_REGISTER_DERIVED_CLASS( LifeMeter, ActorFrame )
// lua end

/*
 * (c) 2005 Glenn Maynard
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

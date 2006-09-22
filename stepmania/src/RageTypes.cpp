#include "global.h"
#include "RageTypes.h"
#include "LuaManager.h"

void RageColor::PushTable( lua_State *L ) const
{
	lua_newtable( L );
	int iTable = lua_gettop(L);

	lua_pushnumber( L, r );
	lua_rawseti( L, iTable, 1 );
	lua_pushnumber( L, g );
	lua_rawseti( L, iTable, 2 );
	lua_pushnumber( L, b );
	lua_rawseti( L, iTable, 3 );
	lua_pushnumber( L, a );
	lua_rawseti( L, iTable, 4 );
}

void RageColor::FromStack( lua_State *L, int iPos )
{
	lua_pushvalue( L, iPos );
	int iFrom = lua_gettop( L );

	lua_rawgeti( L, iFrom, 1 );
	r = lua_tonumber( L, -1 );
	lua_rawgeti( L, iFrom, 2 );
	g = lua_tonumber( L, -1 );
	lua_rawgeti( L, iFrom, 3 );
	b = lua_tonumber( L, -1 );
	lua_rawgeti( L, iFrom, 4 );
	a = lua_tonumber( L, -1 );
	lua_pop( L, 5 );
}

bool LuaHelpers::FromStack( lua_State *L, RageColor &Object, int iOffset )
{
	Object.FromStack( L, iOffset );
	return true;
}

/*
 * Copyright (c) 2006 Glenn Maynard
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

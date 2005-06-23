#include "global.h"
#include "LuaBinding.h"

void CreateMethodsTable( lua_State *L, const CString &szName )
{
	lua_pushstring( L, szName );
	lua_rawget( L, LUA_GLOBALSINDEX );
	if( !lua_isnil(L, -1) )
		return;

	lua_pop( L, 1 );
	lua_newtable( L );
	lua_pushstring( L, szName );
	lua_pushvalue( L, -2 );
	lua_rawset( L, LUA_GLOBALSINDEX );
}

/*
 * Get a userdata, and check that it's either szType or a type
 * derived from szType, by walking the __index chain.
 */
bool CheckLuaObjectType( lua_State *L, int narg, const char *szType )
{
	int iTop = lua_gettop(L);
	lua_pushvalue( L, narg );

	narg = lua_gettop(L);
	while(1)
	{
		/* If the object on the stack has no metatable, it has no type; fail. */
		if( !lua_getmetatable(L, narg) )
		{
			lua_settop( L, iTop );
			return false;
		}
		int iMetatable = lua_gettop(L);

		/* Look up the type name. */
		lua_pushstring( L, "__type" );
		lua_rawget( L, iMetatable );
		const char *szActualType = lua_tostring( L, -1 );

		if( szActualType != NULL && !strcmp(szActualType, szType) )
		{
			/* The type matches. */
			lua_settop( L, iTop );
			return true;
		}

		/* The type doesn't match.  Does the metatable have __index? */
		lua_pushstring( L, "__index" );
		lua_rawget( L, iMetatable );
		if( lua_isnil(L, -1) )
		{
			/* There's no __index.  The type doesn't match. */
			lua_settop( L, iTop );
			return false;
		}

		/* Start over with __index. */
		lua_replace( L, narg );
		lua_pop( L, 1 );
	}
}

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

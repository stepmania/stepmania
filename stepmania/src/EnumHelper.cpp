#include "global.h"
#include "EnumHelper.h"
#include "LuaManager.h"
#include "RageUtil.h"

int CheckEnum( lua_State *L, LuaReference &table, int iPos, int iInvalid, const char *szType )
{
	if( lua_isnil(L, iPos) )
		return iInvalid;

	iPos = LuaHelpers::AbsIndex( L, iPos );

	table.PushSelf( L );
	lua_pushvalue( L, iPos );
	lua_gettable( L, -2 );

	// If the result is nil, then a string was passed that is not a member of this enum.  Throw
	// an error.  To specify the invalid value, pass nil.  That way, typos will throw an error,
	// and not silently result in nil, or an out-of-bounds value.
	if( unlikely(lua_isnil(L, -1)) )
	{
		// XXX: show string if a string, otherwise the type
		lua_pushvalue( L, iPos );
		RString sGot;
		LuaHelpers::Pop( L, sGot );
		LuaHelpers::Push( ssprintf("Expected %s; got \"%s\"", szType, sGot.c_str() ), L );
		lua_error( L );
	}
	int iRet = lua_tointeger( L, -1 );
	lua_pop( L, 2 );
	return iRet;
}

/*
 * (c) 2004 Chris Danford
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

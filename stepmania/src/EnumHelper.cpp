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
		RString sGot;
		if( lua_isstring(L, iPos) )
		{
			/* We were given a string, but it wasn't a valid value for this enum.  Show
			 * the string. */
			lua_pushvalue( L, iPos );
			LuaHelpers::Pop( L, sGot );
			sGot = ssprintf( "\"%s\"", sGot.c_str() );
		}
		else
		{
			/* We didn't get a string.  Show the type. */
			luaL_pushtype( L, iPos );
			LuaHelpers::Pop( L, sGot );
		}
		LuaHelpers::Push( L, ssprintf("Expected %s; got %s", szType, sGot.c_str() ) );
		lua_error( L );
	}
	int iRet = lua_tointeger( L, -1 );
	lua_pop( L, 2 );
	return iRet;
}

// szNameArray is of size iMax; pNameCache is of size iMax+2.
const RString &EnumToString( int iVal, int iMax, const char **szNameArray, auto_ptr<RString> *pNameCache )
{
	if( unlikely(pNameCache[0].get() == NULL) )
	{
		for( int i = 0; i < iMax; ++i )
		{
			auto_ptr<RString> ap( new RString( szNameArray[i] ) );
			pNameCache[i] = ap;
		}

		auto_ptr<RString> ap( new RString );
		pNameCache[iMax+1] = ap;
	}

	// iMax+1 is "Invalid".  iMax+0 is the NUM_ size value, which can not be converted
	// to a string.
	ASSERT_M( iVal >= 0 && (iVal < iMax || iVal == iMax+1), ssprintf("%i, %i", iVal, iMax)  );
	return *pNameCache[iVal];
}

/*
 * (c) 2006 Glenn Maynard
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

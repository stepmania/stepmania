#ifndef LUA_FUNCTIONS_H
#define LUA_FUNCTIONS_H

#include "RageUtil.h" /* for ssprintf */

extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

/* Argument helpers: */
#define LUA_ASSERT( expr, err ) if( !(expr) ) { LUA->Fail( err ); }

/* Require exactly "need" arguments. */
#define REQ_ARGS(func, need) { \
	const int args = lua_gettop(L); \
	LUA_ASSERT( args == need, ssprintf( func " requires exactly %i argument%s, got %i", need, need == 1? "":"s", args) ); \
}

/* argument n must be of type "type" */
#define REQ_ARG(func, n, type) { \
	LUA_ASSERT( lua_is##type(L, n), ssprintf("Argument %i to " func " must be %s", n, #type) ); }
/* argument n must be a number between minimum...maximum */
#define REQ_ARG_NUMBER_RANGE(func, n, minimum, maximum) { \
	REQ_ARG(func, n, number); \
	const int val = (int) lua_tonumber( L, n ); \
	LUA_ASSERT( val >= minimum && val <= maximum, ssprintf("Argument %i to " func " must be an integer between %i and %i (got %i)", n,  minimum, maximum, val) ); \
}
#define LUA_RETURN( expr, L ) { LuaHelpers::Push( expr, L ); return 1; }

/* Helpers to create common functions: */
/* Functions that take no arguments: */
#define LuaFunction_NoArgs( func, call ) \
int LuaFunc_##func( lua_State *L ) { \
	REQ_ARGS( #func, 0 ); \
	LUA_RETURN( call, L ); \
} \
LuaFunction( func ); /* register it */

#define LuaFunction_Int( func, call ) \
int LuaFunc_##func( lua_State *L ) { \
	REQ_ARGS( #func, 1 ); \
	REQ_ARG( #func, 1, number ); \
	const int a1 = int(lua_tonumber( L, 1 )); \
	LUA_RETURN( call, L ); \
} \
LuaFunction( func ); /* register it */

#define LuaFunction_IntInt( func, call ) \
int LuaFunc_##func( lua_State *L ) { \
	REQ_ARGS( #func, 2 ); \
	REQ_ARG( #func, 1, number ); \
	REQ_ARG( #func, 2, number ); \
	const int a1 = int(lua_tonumber( L, 1 )); \
	const int a2 = int(lua_tonumber( L, 2 )); \
	LUA_RETURN( call, L ); \
} \
LuaFunction( func ); /* register it */

#define LuaFunction_Float( func, call ) \
int LuaFunc_##func( lua_State *L ) { \
	REQ_ARGS( #func, 1 ); \
	REQ_ARG( #func, 1, number ); \
	const float a1 = float(lua_tonumber( L, 1 )); \
	LUA_RETURN( call, L ); \
} \
LuaFunction( func ); /* register it */

#define LuaFunction_Str( func, call ) \
int LuaFunc_##func( lua_State *L ) { \
	REQ_ARGS( #func, 1 ); \
	REQ_ARG( #func, 1, string ); \
	CString str; \
	LuaHelpers::PopStack( str, NULL ); \
	LUA_RETURN( call, L ); \
} \
LuaFunction( func ); /* register it */

#define LuaFunction_StrStr( func, call ) \
int LuaFunc_##func( lua_State *L ) { \
	REQ_ARGS( #func, 2 ); \
	REQ_ARG( #func, 1, string ); \
	REQ_ARG( #func, 2, string ); \
	CString str1; \
	CString str2; \
	LuaHelpers::PopStack( str2, NULL ); \
	LuaHelpers::PopStack( str1, NULL ); \
	LUA_RETURN( call, L ); \
} \
LuaFunction( func ); /* register it */

/* Functions that take a single PlayerNumber argument: */
#define LuaFunction_PlayerNumber( func, call ) \
int LuaFunc_##func( lua_State *L ) { \
	REQ_ARGS( #func, 1 ); \
	REQ_ARG_NUMBER_RANGE( #func, 1, 1, NUM_PLAYERS ); \
	const PlayerNumber pn = (PlayerNumber) (int(lua_tonumber( L, -1 ))-1); \
	LUA_RETURN( call, L ); \
} \
LuaFunction( func ); /* register it */

/* Linked list of functions we make available to Lua. */
struct LuaFunctionList
{
	LuaFunctionList( CString name, lua_CFunction func );
	CString name;
	lua_CFunction func;
	LuaFunctionList *next;
};
extern LuaFunctionList *g_LuaFunctionList;
#define LuaFunction( func ) static LuaFunctionList g_##func( #func, LuaFunc_##func )

#endif

/*
 * (c) 2004 Glenn Maynard
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

#ifndef LUA_FUNCTIONS_H
#define LUA_FUNCTIONS_H

#include "LuaHelpers.h"

extern "C"
{
#include <lua.h>
#include <lualib.h>
}

/* Argument helpers: */
#define LUA_ASSERT( expr, err ) if( !(expr) ) { Lua::Fail( L, err ); }
/* require exactly n arguments */
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
#define LUA_RETURN( expr ) { Lua::PushStack( L, expr ); return 1; }

/* Helpers to create common functions: */
/* Functions that take no arguments: */
#define LuaFunction_NoArgs( func, call ) \
int LuaFunc_##func( lua_State *L ) { \
	REQ_ARGS( #func, 0 ); \
	LUA_RETURN( call ); \
} \
LuaFunction( func ); /* register it */

#define LuaFunction_IntInt( func, call ) \
int LuaFunc_##func( lua_State *L ) { \
	REQ_ARGS( #func, 2 ); \
	REQ_ARG( #func, 1, number ); \
	REQ_ARG( #func, 2, number ); \
	const int a1 = int(lua_tonumber( L, 1 )); \
	const int a2 = int(lua_tonumber( L, 2 )); \
	LUA_RETURN( call ); \
} \
LuaFunction( func ); /* register it */

#define LuaFunction_Str( func, call ) \
int LuaFunc_##func( lua_State *L ) { \
	REQ_ARGS( #func, 1 ); \
	REQ_ARG( #func, 1, string ); \
	CString str; \
	Lua::PopStack( L, str ); \
	LUA_RETURN( call ); \
} \
LuaFunction( func ); /* register it */

/* Functions that take a single PlayerNumber argument: */
#define LuaFunction_PlayerNumber( func, call ) \
int LuaFunc_##func( lua_State *L ) { \
	REQ_ARGS( #func, 1 ); \
	REQ_ARG_NUMBER_RANGE( #func, 1, 1, NUM_PLAYERS ); \
	const PlayerNumber pn = (PlayerNumber) (int(lua_tonumber( L, -1 ))-1); \
	LUA_RETURN( call ); \
} \
LuaFunction( func ); /* register it */


/* Linked list of functions we make available to Lua. */
struct LuaFunctionList
{
	LuaFunctionList( CString name, lua_CFunction func );
	CString name;
	lua_CFunction func;
	LuaFunctionList *next;
} extern *g_LuaFunctionList;
#define LuaFunction( func ) static LuaFunctionList g_##func( #func, LuaFunc_##func )

#endif
/*
 * Copyright (c) 2004 by the person(s) listed below.  All rights reserved.
 *	Glenn Maynard
 */

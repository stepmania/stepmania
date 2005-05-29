#ifndef LUA_FUNCTIONS_H
#define LUA_FUNCTIONS_H

#include "LuaBinding.h"

extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#define LUA_RETURN( expr, L ) { LuaHelpers::Push( expr, L ); return 1; }

/* Helpers to create common functions: */
#define LuaFunction( func, call ) \
int LuaFunc_##func( lua_State *L ) { \
	LUA_RETURN( call, L ); \
} \
RegisterLuaFunction( func );
#define LuaFunction_NoArgs LuaFunction /* compat */

#define LuaFunction_Int( func, call ) \
int LuaFunc_##func( lua_State *L ) { \
	const int a1 = IArg(1); \
	LUA_RETURN( call, L ); \
} \
RegisterLuaFunction( func );

#define LuaFunction_Float( func, call ) \
int LuaFunc_##func( lua_State *L ) { \
	const float a1 = FArg(1); \
	LUA_RETURN( call, L ); \
} \
RegisterLuaFunction( func );

#define LuaFunction_Str( func, call ) \
int LuaFunc_##func( lua_State *L ) { \
	CString str = SArg(1); \
	LUA_RETURN( call, L ); \
} \
RegisterLuaFunction( func );

/* Linked list of functions we make available to Lua. */
struct LuaFunctionList
{
	LuaFunctionList( CString name, lua_CFunction func );
	CString name;
	lua_CFunction func;
	LuaFunctionList *next;
};
extern LuaFunctionList *g_LuaFunctionList;
#define RegisterLuaFunction( func ) static LuaFunctionList g_##func( #func, LuaFunc_##func )

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

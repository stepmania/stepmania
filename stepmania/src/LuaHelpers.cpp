#include "global.h"
#include "LuaHelpers.h"
#include "LuaFunctions.h"
#include "RageUtil.h"
#include "RageLog.h"

#include <csetjmp>
#include <cassert>

LuaFunctionList *g_LuaFunctionList = NULL;

#if defined(_WINDOWS)
	#pragma comment(lib, "lua-5.0/lib/LibLua.lib")
	#pragma comment(lib, "lua-5.0/lib/LibLuaLib.lib")
#endif
#if defined(_XBOX)
	#pragma comment(lib, "lua-5.0/lib/LibLuaXbox.lib")
	#pragma comment(lib, "lua-5.0/lib/LibLuaLibXbox.lib")
#endif
#if defined(_WINDOWS) || defined (_XBOX)
	/* "interaction between '_setjmp' and C++ object destruction is non-portable"
	 * We don't care; we'll throw a fatal exception immediately anyway. */
	#pragma warning (disable : 4611)
#endif
#if defined (DARWIN)
	extern void NORETURN longjmp(jmp_buf env, int val);
#endif



struct ChunkReaderData
{
	const CString *buf;
	bool done;
	ChunkReaderData() { buf = NULL; done = false; }
};

const char *ChunkReaderString( lua_State *L, void *ptr, size_t *size )
{
	ChunkReaderData *data = (ChunkReaderData *) ptr;
	if( data->done )
		return NULL;

	data->done = true;

	*size = data->buf->size();
	const char *ret = data->buf->data();
	
	return ret;
}

void Lua::PushStack( lua_State *L, int out )
{
	/* XXX: stack bounds */
	lua_pushnumber( L, out );
}

void Lua::PushStack( lua_State *L, bool out )
{
	/* XXX: stack bounds */
	lua_pushboolean( L, out );
}


void Lua::PushStack( lua_State *L, void *out )
{
	if( out )
		lua_pushlightuserdata( L, out );
	else
		lua_pushnil( L );
}

void Lua::PushStack( lua_State *L, const CString &out )
{
	lua_pushstring( L, out );
}

void Lua::PopStack( lua_State *L, CString &out )
{
	/* There must be at least one entry on the stack. */
	ASSERT( lua_gettop(L) > 0 );
	ASSERT( lua_isstring(L, -1) );

	out = lua_tostring( L, -1 );
	lua_pop( L, -1 );
}

bool Lua::GetStack( lua_State *L, int pos, int &out )
{
	if( pos < 0 )
		pos = lua_gettop(L) - pos - 1;
	if( pos < 1 )
		return false;
	
	out = (int) lua_tonumber( L, pos );
	return true;
}

void LoadFromString( lua_State *L, const CString &str )
{
	ChunkReaderData data;
	data.buf = &str;
	int ret = lua_load( L, ChunkReaderString, &data, "in" );

	if( ret )
	{
		CString err;
		Lua::PopStack( L, err );
		RageException::Throw( "Error loading script \"%s\": %s", str.c_str(), err.c_str() );
	}
}





static jmp_buf jbuf;
static CString jbuf_error;

static int LuaPanic( lua_State *L )
{
	CString err;
	Lua::PopStack( L, err );

	/* Grr.  We can't throw an exception from here: it'll explode going through the
	 * Lua stack for some reason.  Get it off the stack with a longjmp and throw
	 * an exception from there. */
	jbuf_error = err;
	longjmp( jbuf, 1 );
}

bool Lua::RunExpression( const CString &str )
{
try {
	if( setjmp(jbuf) )
		RageException::Throw("%s", jbuf_error.c_str());
	lua_State *L = lua_open();
	ASSERT( L );

	lua_atpanic( L, LuaPanic );
	
	luaopen_base( L );
	luaopen_math( L );
	luaopen_string( L );
	lua_settop(L, 0); // luaopen_* pushes stuff onto the stack that we don't need

	Lua::RegisterFunctions( L );

	LoadFromString( L, "return " + str );
	ASSERT_M( lua_gettop(L) == 1, ssprintf("%i", lua_gettop(L)) );

	int ret = lua_pcall(L, 0, 1, 0);
	if( ret )
	{
		CString err;
		Lua::PopStack( L, err );
		RageException::Throw( "Runtime error running \"%s\": %s", str.c_str(), err.c_str() );
	}

	ASSERT_M( lua_gettop(L) == 1, ssprintf("%i", lua_gettop(L)) );

	/* Don't accept a function as a return value; if you really want to use a function
	 * as a boolean, convert it before returning. */
	if( lua_isfunction( L, -1 ) )
		throw CString( "result is a function; did you forget \"()\"?" );

	bool result = !!lua_toboolean( L, -1 );
	lua_pop( L, -1 );

	lua_close( L );

	return result;
} catch( const CString &err ) {
	RageException::Throw( "Error running \"%s\": %s", str.c_str(), err.c_str() );
}
}

void Lua::Fail( lua_State *L, const CString &err )
{
	lua_pushstring( L, err );
	lua_error( L );
}

void Lua::RegisterFunctions( lua_State *L )
{
	for( const LuaFunctionList *p = g_LuaFunctionList; p; p=p->next )
		lua_register( L, p->name, p->func );
}

LuaFunctionList::LuaFunctionList( CString name_, lua_CFunction func_ )
{
	name = name_;
	func = func_;
	next = g_LuaFunctionList;
	g_LuaFunctionList = this;
}


LuaFunction_NoArgs( MonthOfYear, GetLocalTime().tm_mon+1 );
LuaFunction_NoArgs( DayOfMonth, GetLocalTime().tm_mday );
LuaFunction_NoArgs( Hour, GetLocalTime().tm_hour );
LuaFunction_NoArgs( Minute, GetLocalTime().tm_min );
LuaFunction_NoArgs( Second, GetLocalTime().tm_sec );
LuaFunction_NoArgs( Year, GetLocalTime().tm_year+1900 );
LuaFunction_NoArgs( Weekday, GetLocalTime().tm_wday );
LuaFunction_NoArgs( DayOfYear, GetLocalTime().tm_yday );

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

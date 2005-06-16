#include "global.h"
#include "LuaManager.h"
#include "LuaFunctions.h"
#include "LuaReference.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageFile.h"
#include "RageThreads.h"
#include "arch/Dialog/Dialog.h"
#include "Foreach.h"

#include <csetjmp>
#include <cassert>

LuaManager *LUA = NULL;
static LuaFunctionList *g_LuaFunctions = NULL;

#if defined(_MSC_VER) && !defined(_XBOX)
	#pragma comment(lib, "lua-5.0/lib/LibLua.lib")
	#pragma comment(lib, "lua-5.0/lib/LibLuaLib.lib")
#elif defined(_XBOX)
	#pragma comment(lib, "lua-5.0/lib/LibLuaXbox.lib")
	#pragma comment(lib, "lua-5.0/lib/LibLuaLibXbox.lib")
#endif
#if defined(_MSC_VER) || defined (_XBOX)
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

void LuaManager::SetGlobal( const CString &sName, int val )
{
	LuaHelpers::PushStack( val, L );
	lua_setglobal( L, sName );
}

void LuaManager::SetGlobal( const CString &sName, bool val )
{
	LuaHelpers::PushStack( val, L );
	lua_setglobal( L, sName );
}

void LuaManager::SetGlobal( const CString &sName, const CString &val )
{
	LuaHelpers::PushStack( val, L );
	lua_setglobal( L, sName );
}

void LuaManager::UnsetGlobal( const CString &sName )
{
	lua_pushnil( L );
	lua_setglobal( L, sName );
}


void LuaHelpers::Push( const bool &Object, lua_State *L ) { lua_pushboolean( L, Object ); }
void LuaHelpers::Push( const float &Object, lua_State *L ) { lua_pushnumber( L, Object ); }
void LuaHelpers::Push( const int &Object, lua_State *L ) { lua_pushnumber( L, Object ); }
void LuaHelpers::Push( const CString &Object, lua_State *L ) { lua_pushstring( L, Object ); }

bool LuaHelpers::FromStack( bool &Object, int iOffset, lua_State *L ) { Object = !!lua_toboolean( L, iOffset ); return true; }
bool LuaHelpers::FromStack( float &Object, int iOffset, lua_State *L ) { Object = (float)lua_tonumber( L, iOffset ); return true; }
bool LuaHelpers::FromStack( int &Object, int iOffset, lua_State *L ) { Object = (int) lua_tonumber( L, iOffset ); return true; }
bool LuaHelpers::FromStack( CString &Object, int iOffset, lua_State *L )
{
	const char *pStr = lua_tostring( L, iOffset );
	if( pStr != NULL )
		Object = pStr;

	return pStr != NULL;
}

void LuaHelpers::CreateTableFromArrayB( Lua *L, const vector<bool> &aIn )
{
	lua_newtable( L );
	for( unsigned i = 0; i < aIn.size(); ++i )
	{
		lua_pushboolean( L, aIn[i] );
		lua_rawseti( L, -2, i+1 );
	}
}

void LuaHelpers::ReadArrayFromTableB( Lua *L, vector<bool> &aOut )
{
	luaL_checktype( L, -1, LUA_TTABLE );

	for( unsigned i = 0; i < aOut.size(); ++i )
	{
		lua_rawgeti( L, -1, i+1 );
		bool bOn = !!lua_toboolean( L, -1 );
		aOut[i] = bOn;
		lua_pop( L, 1 );
	}
}


static int LuaPanic( lua_State *L )
{
	CString sErr;
	LuaHelpers::PopStack( sErr, L );

	RageException::Throw( "%s", sErr.c_str() );
}



// Actor registration
static vector<RegisterWithLuaFn>	*g_vRegisterActorTypes = NULL;

void LuaManager::Register( RegisterWithLuaFn pfn )
{
	if( g_vRegisterActorTypes == NULL )
		g_vRegisterActorTypes = new vector<RegisterWithLuaFn>;

	g_vRegisterActorTypes->push_back( pfn );
}





LuaManager::LuaManager()
{
	LUA = this;	// so that LUA is available when we call the Register functions

	L = NULL;
	m_pLock = new RageMutex( "Lua" );

	ResetState();
}

LuaManager::~LuaManager()
{
	lua_close( L );
	delete m_pLock;
}

Lua *LuaManager::Get()
{
	m_pLock->Lock();
	return L;
}

void LuaManager::Release( Lua *&p )
{
	ASSERT( p == L );
	m_pLock->Unlock();
	p = NULL;
}


void LuaManager::RegisterTypes()
{
	for( const LuaFunctionList *p = g_LuaFunctions; p; p=p->next )
		lua_register( L, p->name, p->func );
	
	if( g_vRegisterActorTypes )
	{
		for( unsigned i=0; i<g_vRegisterActorTypes->size(); i++ )
		{
			RegisterWithLuaFn fn = (*g_vRegisterActorTypes)[i];
			fn( L );
		}
	}
}

void LuaManager::ResetState()
{
	if( L != NULL )
	{
		LuaReference::BeforeResetAll();

		lua_close( L );
	}

	L = lua_open();
	ASSERT( L );

	lua_atpanic( L, LuaPanic );
	
	luaopen_base( L );
	luaopen_math( L );
	luaopen_string( L );
	luaopen_table( L );
	lua_settop(L, 0); // luaopen_* pushes stuff onto the stack that we don't need

	RegisterTypes();

	LuaReference::AfterResetAll();
}

void LuaManager::PrepareExpression( CString &sInOut )
{
	// HACK: Many metrics have "//" comments that Lua fails to parse.
	// Replace them with Lua-style comments.
	sInOut.Replace( "//", "--" );
	
	// comment out HTML style color values
	sInOut.Replace( "#", "--" );
	
	// Remove leading +, eg. "+50"; Lua doesn't handle that.
	if( sInOut.size() >= 1 && sInOut[0] == '+' )
		sInOut.erase( 0, 1 );
}

bool LuaHelpers::RunScriptFile( const CString &sFile )
{
	RageFile f;
	if( !f.Open( sFile ) )
	{
		CString sError = ssprintf( "Couldn't open Lua script \"%s\": %s", sFile.c_str(), f.GetError().c_str() );
		Dialog::OK( sError, "LUA_ERROR" );
		return false;
	}

	CString sScript;
	if( f.Read( sScript ) == -1 )
	{
		CString sError = ssprintf( "Error reading Lua script \"%s\": %s", sFile.c_str(), f.GetError().c_str() );
		Dialog::OK( sError, "LUA_ERROR" );
		return false;
	}

	CString sError;
	if( !LuaHelpers::RunScript( LUA->L, sScript, sFile, sError, 0 ) )
	{
		sError = ssprintf( "Lua runtime error: %s", sError.c_str() );
		Dialog::OK( sError, "LUA_ERROR" );
		return false;
	}
	
	return true;
}

bool LuaHelpers::RunScript( Lua *L, const CString &sScript, const CString &sName, CString &sError, int iReturnValues )
{
	// load string
	{
		ChunkReaderData data;
		data.buf = &sScript;
		int ret = lua_load( L, ChunkReaderString, &data, sName );

		if( ret )
		{
			LuaHelpers::PopStack( sError, L );
			return false;
		}
	}

	// evaluate
	{
		int ret = lua_pcall( L, 0, iReturnValues, 0 );
		if( ret )
		{
			LuaHelpers::PopStack( sError, L );
			return false;
		}
	}

	return true;
}


bool LuaHelpers::RunScript( Lua *L, const CString &sExpression, const CString &sName, int iReturnValues )
{
	CString sError;
	if( !LuaHelpers::RunScript( L, sExpression, sName.size()? sName:CString("in"), sError, iReturnValues ) )
	{
		sError = ssprintf( "Lua runtime error parsing \"%s\": %s", sName.size()? sName.c_str():sExpression.c_str(), sError.c_str() );
		Dialog::OK( sError, "LUA_ERROR" );
		return false;
	}

	return true;
}

bool LuaHelpers::RunExpressionB( const CString &str )
{
	Lua *L = LUA->Get();

	if( !LuaHelpers::RunScript(L, "return " + str, "", 1) )
	{
		LUA->Release(L);
		return false;
	}

	/* Don't accept a function as a return value. */
	if( lua_isfunction( L, -1 ) )
		RageException::Throw( "result is a function; did you forget \"()\"?" );

	bool result = !!lua_toboolean( L, -1 );
	lua_pop( L, 1 );
	LUA->Release(L);

	return result;
}

float LuaHelpers::RunExpressionF( const CString &str )
{
	Lua *L = LUA->Get();
	if( !LuaHelpers::RunScript(L, "return " + str, "", 1) )
	{
		LUA->Release(L);
		return 0;
	}

	/* Don't accept a function as a return value. */
	if( lua_isfunction( L, -1 ) )
		RageException::Throw( "result is a function; did you forget \"()\"?" );

	float result = (float) lua_tonumber( L, -1 );
	lua_pop( L, 1 );

	LUA->Release(L);
	return result;
}

int LuaHelpers::RunExpressionI( const CString &str )
{
	return (int) LuaHelpers::RunExpressionF(str);
}

bool LuaHelpers::RunExpressionS( const CString &str, CString &sOut )
{
	Lua *L = LUA->Get();
	if( !LuaHelpers::RunScript(L, "return " + str, "", 1) )
	{
		LUA->Release(L);
		return false;
	}

	/* Don't accept a function as a return value. */
	if( lua_isfunction( L, -1 ) )
		RageException::Throw( "result is a function; did you forget \"()\"?" );

	sOut = lua_tostring( L, -1 );
	lua_pop( L, 1 );

	LUA->Release(L);
	return true;
}

bool LuaManager::RunAtExpressionS( CString &sStr )
{
	if( sStr.size() == 0 || sStr[0] != '@' )
		return false;

	/* Erase "@". */
	sStr.erase( 0, 1 );

	CString sOut;
	LuaHelpers::RunExpressionS( sStr, sOut );
	sStr = sOut;
	return true;
}

/* Like luaL_typerror, but without the special case for argument 1 being "self"
 * in method calls, so we give a correct error message after we remove self. */
int LuaHelpers::TypeError( Lua *L, int iArgNo, const char *szName )
{
	lua_Debug debug;
	lua_getstack( L, 0, &debug );
	lua_getinfo( L, "n", &debug );
	return luaL_error( L, "bad argument #%d to \"%s\" (%s expected, got %s)",
		iArgNo, debug.name? debug.name:"(unknown)", szName, lua_typename(L, lua_type(L, iArgNo)) );
}


LuaFunctionList::LuaFunctionList( CString name_, lua_CFunction func_ )
{
	name = name_;
	func = func_;
	next = g_LuaFunctions;
	g_LuaFunctions = this;
}


static bool Trace( const CString &sString )
{
	LOG->Trace( "%s", sString.c_str() );
	return true;
}

LuaFunction( Trace, Trace(SArg(1)) );

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

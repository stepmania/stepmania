#ifndef LUA_MANAGER_H
#define LUA_MANAGER_H

struct lua_State;
typedef lua_State Lua;
typedef void (*RegisterWithLuaFn)(lua_State*);
class RageMutex;
class XNode;
class LuaReference;

extern "C"
{
#include "lua-5.1/src/lua.h"
#include "lua-5.1/src/lualib.h"
#include "lua-5.1/src/lauxlib.h"
}

class LuaManager;
extern LuaManager *LUA;

class LuaManager
{
public:
	// Every Actor should register its class at program initialization.
	static void Register( RegisterWithLuaFn pfn );

	LuaManager();
	~LuaManager();

	Lua *Get();
	void Release( Lua *&p );

	/* Explicitly lock and unlock Lua access.  This is done automatically by Get() and
	 * Release(). */
	void YieldLua();
	void UnyieldLua();

	/* Register all subscribing types.  There's no harm in registering when already registered. */
	void RegisterTypes();

	void SetGlobal( const RString &sName, int val );
	void SetGlobal( const RString &sName, const RString &val );
	void UnsetGlobal( const RString &sName );

private:
	lua_State *m_pLuaMain;
};


namespace LuaHelpers
{
	/* Run a script with the given name.  The given number of return values are left on
	 * the Lua stack.  On error, nils are left on the stack, sError is set and 
	 * false is returned. */
	bool RunScript( Lua *L, const RString &sScript, const RString &sName, RString &sError, int iArgs = 0, int iReturnValues = 0 );

	/* Run the given expression, returning a single value, and leave the return value on the
	 * stack.  On error, push nil. */
	bool RunExpression( Lua *L, const RString &sExpression, const RString &sName = "" );

	bool RunScriptFile( const RString &sFile );

	/* Create a Lua array (a table with indices starting at 1) of the given vector,
	 * and push it on the stack. */
	void CreateTableFromArrayB( Lua *L, const vector<bool> &aIn );
	
	/* Recursively copy elements from the table at stack element -2 into the table
	 * at stack -1.  Pop both elements from the stack. */
	void DeepCopy( lua_State *L );

	/* Read the table at the top of the stack back into a vector. */
	void ReadArrayFromTableB( Lua *L, vector<bool> &aOut );

	void ParseCommandList( lua_State *L, const RString &sCommands, const RString &sName );

	XNode *GetLuaInformation();

	/* Pops the last iArgs arguments from the stack, and return a function that returns
	 * those values. */
	void PushValueFunc( lua_State *L, int iArgs );

	template<class T>
	void Push( lua_State *L, const T &Object );

	template<class T>
	bool FromStack( lua_State *L, T &Object, int iOffset );

	template<class T>
	bool Pop( lua_State *L, T &val )
	{
		bool bRet = LuaHelpers::FromStack( L, val, -1 );
		lua_pop( L, 1 );
		return bRet;
	}
	
	template<class T>
	void ReadArrayFromTable( vector<T> &aOut, lua_State *L )
	{
		luaL_checktype( L, -1, LUA_TTABLE );

		int i = 0;
		while( lua_rawgeti(L, -1, ++i), !lua_isnil(L, -1) )
		{
			T value = T();
			LuaHelpers::Pop( L, value );
			aOut.push_back( value );
		}
		lua_pop( L, 1 ); // pop nil
	}
	template<class T>
	void CreateTableFromArray( const vector<T> &aIn, lua_State *L )
	{
		lua_newtable( L );
		for( unsigned i = 0; i < aIn.size(); ++i )
		{
			LuaHelpers::Push( L, aIn[i] );
			lua_rawseti( L, -2, i+1 );
		}
	}

	int TypeError( Lua *L, int narg, const char *tname );
	inline int AbsIndex( Lua *L, int i ) { if( i > 0 || i <= LUA_REGISTRYINDEX ) return i; return lua_gettop( L ) + i + 1; }
}

class LuaThreadVariable
{
public:
	LuaThreadVariable( const RString &sName, const RString &sValue );
	LuaThreadVariable( const RString &sName, const LuaReference &Value );
	LuaThreadVariable( lua_State *L ); // name and value are on stack
	~LuaThreadVariable();
	static void GetThreadVariable( lua_State *L );

private:
	LuaThreadVariable( const LuaThreadVariable &cpy ); // not defined

	void SetFromStack( lua_State *L );
	int AdjustCount( lua_State *L, int iAdd );
	static bool PushThreadTable( lua_State *L, bool bCreate );
	static RString GetCurrentThreadIDString();

	LuaReference *m_Name;
	LuaReference *m_pOldValue;
};

/* Iterate over all elements in the table. For safety reasons, the key is pushed onto
 * the stack and can be read (safely) as a string and popped or altered in any way.
 * Stack management is handled automatically. That is, you need not remove all stack
 * elements above the key. Once the loop exits normally, the top of the stack will be
 * where it was before. If you break out of the loop early, you need to handle that
 * explicitly. */
#define FOREACH_LUATABLE(L,index) \
for( const int UNIQUE_NAME(tab) = LuaHelpers::AbsIndex(L,index), \
     UNIQUE_NAME(top) = (lua_pushnil(L),lua_gettop(L)); \
     lua_next(L, UNIQUE_NAME(tab)) && (lua_pushvalue(L,-2),true); \
     lua_settop(L,UNIQUE_NAME(top)) )


struct RegisterLuaFunction { RegisterLuaFunction( RegisterWithLuaFn pfn ) { LuaManager::Register( pfn ); } };
#define REGISTER_WITH_LUA_FUNCTION( Fn ) \
	static RegisterLuaFunction register##Fn( Fn );

inline bool MyLua_checkboolean (lua_State *L, int numArg)
{
	luaL_checktype( L, numArg, LUA_TBOOLEAN );
	return !!lua_toboolean( L, numArg );
}

/* BIArg is like BArg, except 1 is accepted as a true value and (as a special case)
 * 0 is accepted as a false value.  This is to help transitions where "cmd,0" is
 * used to mean "cmd,false". */
inline bool MyLua_checkintboolean( lua_State *L, int iArg )
{
	int iType = lua_type( L, iArg );
	if( iType == LUA_TNUMBER )
	{
		int iValue = lua_tointeger( L, iArg );
		return iValue != 0;
	}

	return MyLua_checkboolean( L, iArg );
}

#define SArg(n) (luaL_checkstring(L,(n)))
#define BIArg(n) (MyLua_checkintboolean(L,(n)))
#define IArg(n) (luaL_checkint(L,(n)))
#define BArg(n) (MyLua_checkboolean(L,(n)))
#define FArg(n) ((float) luaL_checknumber(L,(n)))

#define LuaFunction( func, expr ) \
int LuaFunc_##func( lua_State *L ) { \
	LuaHelpers::Push( L, expr ); return 1; \
} \
void LuaFunc_Register_##func( lua_State *L ) { lua_register( L, #func, LuaFunc_##func ); } \
REGISTER_WITH_LUA_FUNCTION( LuaFunc_Register_##func );

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

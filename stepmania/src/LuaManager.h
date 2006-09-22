#ifndef LUA_MANAGER_H
#define LUA_MANAGER_H

struct lua_State;
typedef lua_State Lua;
typedef void (*RegisterWithLuaFn)(lua_State*);
class RageMutex;
class XNode;

extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
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

	/* Register all subscribing types.  There's no harm in registering when already registered. */
	void RegisterTypes();

	void SetGlobal( const RString &sName, int val );
	void SetGlobal( const RString &sName, float val );
	void SetGlobal( const RString &sName, bool val );
	void SetGlobal( const RString &sName, const RString &val );
	void UnsetGlobal( const RString &sName );
	XNode *GetLuaInformation() const;

private:
	lua_State *L;

	RageMutex *m_pLock;
};


namespace LuaHelpers
{
	/* Run a script with the given name.  Return values are left on the Lua stack.
	 * Returns false on error, with sError set. */
	bool RunScript( Lua *L, const RString &sScript, const RString &sName, RString &sError, int iReturnValues = 0 );

	/* Convenience: run a script with one return value, displaying an error on failure.
	 * The return value is left on the Lua stack. */
	bool RunScript( Lua *L, const RString &sExpression, const RString &sName = "", int iReturnValues = 0 );

	/* Run the given expression, returning a single value, and leave the return value on the
	 * stack.  On error, push nil. */
	bool RunExpression( Lua *L, const RString &sExpression );

	bool RunScriptFile( const RString &sFile );

	/* Strip "//" comments and "+". */
	void PrepareExpression( RString &sInOut );

	/* Create a Lua array (a table with indices starting at 1) of the given vector,
	 * and push it on the stack. */
	void CreateTableFromArrayB( Lua *L, const vector<bool> &aIn );

	/* Read the table at the top of the stack back into a vector. */
	void ReadArrayFromTableB( Lua *L, vector<bool> &aOut );

	/* Run an expression in the global environment, returning the given type. */
	bool RunExpressionB( const RString &str );
	float RunExpressionF( const RString &str );
	int RunExpressionI( const RString &str );
	bool RunExpressionS( const RString &str, RString &sOut );

	/* If sStr begins with @, evaluate the rest as an expression and store the result over sStr. */
	bool RunAtExpressionS( RString &sStr );

	template<class T>
	void Push( T *pObject, Lua *L );

	void Push( const bool &Object, Lua *L );
	void Push( const float &Object, Lua *L );
	void Push( const int &Object, Lua *L );
	void Push( const RString &Object, Lua *L );

	bool FromStack( Lua *L, bool &Object, int iOffset );
	bool FromStack( Lua *L, float &Object, int iOffset );
	bool FromStack( Lua *L, int &Object, int iOffset );
	bool FromStack( Lua *L, RString &Object, int iOffset );

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

		unsigned iCount = luaL_getn( L, -1 );

		for( unsigned i = 0; i < iCount; ++i )
		{
			lua_rawgeti( L, -1, i+1 );
			T value = T();
			LuaHelpers::Pop( L, value );
			aOut.push_back( value );
		}
	}
	template<class T>
	void CreateTableFromArray( const vector<T> &aIn, lua_State *L )
	{
		lua_newtable( L );
		for( unsigned i = 0; i < aIn.size(); ++i )
		{
			LuaHelpers::Push( aIn[i], L );
			lua_rawseti( L, -2, i+1 );
		}
	}

	int TypeError( Lua *L, int narg, const char *tname );
}


#define REGISTER_WITH_LUA_FUNCTION( Fn ) \
	class Register##Fn { public: Register##Fn() { LuaManager::Register( Fn ); } }; \
	static Register##Fn register##Fn;

inline bool MyLua_checkboolean (lua_State *L, int numArg)
{
	luaL_checktype( L, numArg, LUA_TBOOLEAN );
	return !!lua_toboolean( L, numArg );
}

#define SArg(n) (luaL_checkstring(L,n))
#define IArg(n) (luaL_checkint(L,n))
#define BArg(n) (MyLua_checkboolean(L,n))
#define FArg(n) ((float) luaL_checknumber(L,n))

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

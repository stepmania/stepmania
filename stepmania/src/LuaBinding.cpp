#include "global.h"
#include "LuaBinding.h"
#include "RageUtil.h"

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
		lua_pushstring( L, "type" );
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

bool GetGlobalTable( Lua *L, bool bCreate )
{
	lua_pushstring( L, "userdatas" );
	lua_rawget( L, LUA_REGISTRYINDEX );
	if( !lua_isnil(L, -1) )
		return true;

	lua_pop( L, 1 );
	if( !bCreate )
		return false;

	/* Save it. */
	lua_newtable( L );
	lua_pushstring( L, "userdatas" );
	lua_pushvalue( L, -2 );
	lua_rawset( L, LUA_REGISTRYINDEX );
	return true;
}

/* The object is on the stack.  It's either a table or a userdata.
 * If needed, associate the metatable; if a table, also add it to
 * the userdata table. */
void ApplyDerivedType( Lua *L, const CString &sClassName, void *pSelf )
{
	int iTable = lua_gettop( L );

	int iType = lua_type( L, iTable );
	ASSERT_M( iType == LUA_TTABLE || iType == LUA_TUSERDATA,
		ssprintf("%i", iType) );

	if( iType == LUA_TTABLE )
	{
		GetGlobalTable( L, true );
		int iGlobalTable = lua_gettop( L );

		/* If the table is already in the userdata table, then everything
		 * is already set up. */
		lua_pushvalue( L, iTable );
		lua_rawget( L, iGlobalTable );
		if( !lua_isnil(L, -1) )
		{
			void *pData = lua_touserdata( L, -1 );
			ASSERT( pSelf == pData );
			ASSERT( pSelf == lua_touserdata(L, -1) );

			lua_settop( L, iTable );
			return;
		}

		/* Create the userdata, and add it to the global table. */
		lua_pushvalue( L, iTable );
		lua_pushlightuserdata( L, pSelf );
		lua_rawset( L, iGlobalTable );

		/* Pop everything except the table. */
		lua_settop( L, iTable );
	}

	luaL_getmetatable( L, sClassName );
	lua_setmetatable( L, iTable );
}

#include "RageUtil_AutoPtr.h"
REGISTER_CLASS_TRAITS( LuaClass, new LuaClass(*pCopy) )

LuaClass::LuaClass()
{
	m_pSelf = NULL;
}

void *GetUserdataFromGlobalTable( Lua *L, const char *szType, int iArg )
{
	if( !GetGlobalTable(L, false) )
		luaL_error( L, "stale %s referenced (object used but no longer exists)", szType );

	lua_pushvalue( L, iArg );
	lua_rawget( L, -2 );
	if( lua_isnil(L, -1) )
		luaL_error( L, "stale %s referenced (object used but no longer exists)", szType );

	void *pRet = lua_touserdata( L, -1 );
	lua_pop( L, 2 );

	return pRet;
}

/* Tricky: when an instance table is copied, we want to do a deep
 * copy, not a reference copy.  Otherwise, the new higher-level object
 * will share a table with the original.  Aside from being confusing,
 * this breaks the global table, which assumes that we have a one-to-
 * one mapping between tables and objects. */
LuaClass::LuaClass( const LuaClass &cpy ):
	LuaTable(cpy)
{
	if( !IsSet() )
		return;

	CString sData = Serialize();
	LoadFromString( sData );
}

LuaClass &LuaClass::operator=( const LuaClass &cpy )
{
	LuaTable::operator=(cpy);

	if( !IsSet() )
		return *this;

	CString sData = Serialize();
	LoadFromString( sData );

	return *this;
}


LuaClass::~LuaClass()
{
	if( LUA == NULL )
		return;

	Lua *L = LUA->Get();

	int iTop = lua_gettop( L );

	/* If we're registered in the global table, unregister. */
	if( GetGlobalTable(L, false) )
	{
		this->PushSelf( L );
		lua_pushnil( L );
		lua_rawset( L, -3 );
	}

	lua_settop( L, iTop );

	LUA->Release( L );
}

void LuaClass::BeforeReset()
{
	LuaTable::BeforeReset();

	/* Read pSelf the name of the class, so we can use them to restore in Register(). */
	Lua *L = LUA->Get();

	if( !GetGlobalTable(L, false) )
	{
		LUA->Release( L );
		return;
	}

	this->PushSelf( L );
	lua_rawget( L, -2 );
	if( lua_isnil(L, -1) )
	{
		/* This table hasn't been pushed yet. */
		lua_pop( L, 2 );
		LUA->Release( L );
		return;
	}

	m_pSelf = lua_touserdata( L, -1 );
	lua_pop( L, 2 );

	this->PushSelf( L );
	lua_getmetatable( L, -1 );
	lua_rawget( L, LUA_REGISTRYINDEX );
	ASSERT( !lua_isnil(L, -1) );
	m_sClassName = lua_tostring( L, -1 );
	lua_pop( L, 1 );

	LUA->Release( L );
}

void LuaClass::Register()
{
	LuaTable::Register();

	if( m_pSelf != NULL )
	{
		Lua *L = LUA->Get();
		this->PushSelf(L);
		ApplyDerivedType( L, m_sClassName, m_pSelf );
		LUA->Release( L );

		/* To conserve memory, clear the class name.  We only need it while restoring. */
		m_sClassName = CString();
		m_pSelf = NULL;
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

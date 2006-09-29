#include "global.h"
#include "LuaBinding.h"
#include "LuaReference.h"
#include "RageUtil.h"

#include "SubscriptionManager.h"
static SubscriptionManager<LuaBinding> m_Subscribers;

namespace
{
	void RegisterTypes( lua_State *L )
	{
		if( m_Subscribers.m_pSubscribers == NULL )
			return;

		/* Register base classes first. */
		map<RString, LuaBinding *> mapToRegister;
		FOREACHS( LuaBinding*, *m_Subscribers.m_pSubscribers, p )
			mapToRegister[(*p)->GetClassName()] = (*p);

		set<RString> setRegisteredAlready;

		while( !mapToRegister.empty() )
		{
			/* Look at the first class.  If it has a base class that needs to be registered,
			 * go there first. */
			LuaBinding *pBinding = mapToRegister.begin()->second;
			while(1)
			{
				if( !pBinding->IsDerivedClass() )
					break;

				RString sBase = pBinding->GetBaseClassName();
				map<RString, LuaBinding *>::const_iterator it = mapToRegister.find(sBase);
				if( it != mapToRegister.end() )
				{
					pBinding = it->second;
					continue;
				}

				/* If the base class wasn't found, and hasn't been registered already, then
				 * a base class registration is missing. */
				if( setRegisteredAlready.find(sBase) != setRegisteredAlready.end() )
					break;

				FAIL_M( ssprintf("Base class of \"%s\" not registered: \"%s\"",
					pBinding->GetClassName().c_str(),
					sBase.c_str()) );
			}

			pBinding->Register( L );
			setRegisteredAlready.insert( pBinding->GetClassName() );
			mapToRegister.erase( pBinding->GetClassName() );
		}
	}
};
REGISTER_WITH_LUA_FUNCTION( RegisterTypes );

LuaBinding::LuaBinding()
{
	m_Subscribers.Subscribe( this );
}

LuaBinding::~LuaBinding()
{
	m_Subscribers.Unsubscribe( this );
}

void LuaBinding::Register( lua_State *L )
{
	/* Create the methods table, if it doesn't already exist. */
	LuaBinding::CreateMethodsTable( L, GetClassName() );
	int methods = lua_gettop( L );
	
	/* Create a metatable for the userdata objects. */
	luaL_newmetatable( L, GetClassName() );
	int metatable = lua_gettop( L );
	
	// We use the metatable to determine the type of the table, so don't
	// allow it to be changed.
	lua_pushstring( L, "(hidden)" );
	lua_setfield( L, metatable, "__metatable" );
	
	lua_pushvalue( L, methods );
	lua_setfield( L, metatable, "__index" );
		
	lua_pushcfunction( L, PushEqual );
	lua_setfield( L, metatable, "__eq" );

	/* Create a metatable for the methods table. */
	lua_newtable( L );
	int methods_metatable = lua_gettop( L );

	// Hide the metatable.
	lua_pushstring( L, "(hidden)" );
	lua_setfield( L, methods_metatable, "__metatable" );

	// If this type has a base class, set the __index of this type
	// to the base class.
	if( IsDerivedClass() )
	{
		lua_getfield( L, LUA_GLOBALSINDEX, GetBaseClassName() );
		lua_setfield( L, methods_metatable, "__index" );
	}

	lua_pushstring( L, GetClassName() );
	lua_setfield( L, methods_metatable, "type" );

	/* Set and pop the methods metatable. */
	lua_setmetatable( L, methods );

	/* Allow the derived class to populate the method table, and set any other
	 * metatable fields. */
	Register( L, methods, metatable );

	lua_pop( L, 2 );  // drop metatable and method table
}

// If defined, type checks for functions will be skipped.  These add
// up and can become expensive (performed for every function dispatch),
// but without them it's possible to call functions on incompatible
// types (eg. "Actor.x(GAMESTATE, 10)"), which will crash or cause corruption.
// #define FAST_LUA

void LuaBinding::CreateMethodsTable( lua_State *L, const RString &sName )
{
	lua_getfield( L, LUA_GLOBALSINDEX, sName );
	if( !lua_isnil(L, -1) )
		return;

	lua_pop( L, 1 );
	lua_newtable( L );
	lua_pushvalue( L, -1 );
	lua_setfield( L, LUA_GLOBALSINDEX, sName );
}

int LuaBinding::PushEqual( lua_State *L )
{
	lua_pushboolean( L, Equal(L) );
	return 1;
}

bool LuaBinding::Equal( lua_State *L )
{
	int iArg1 = lua_gettop( L ) - 1;
	int iArg2 = lua_gettop( L );

	int iType = lua_type( L, iArg1 );
	if( iType != lua_type(L, iArg2) )
		return false;

	if( iType != LUA_TTABLE && iType != LUA_TUSERDATA )
		return false;

	/* Use the regular method for tables.  If an object's table is
	 * kept around after the actual object has been destroyed, the
	 * table is still valid, and the pointer no longer exists. */
	if( iType == LUA_TTABLE )
		return !!lua_rawequal( L, iArg1, iArg2 );

	// This checks that they're the same type.  it does not check
	// that it's actually a LuaBinding type.  If iArg1 is a non-LuaBinding
	// type, this function should not be called and the return value is
	// undefined, but the lua_objlen check below will prevent us from crashing.
	if( !lua_getmetatable(L, iArg1) )
		return false;
	if( !lua_getmetatable(L, iArg2) )
	{
		lua_pop( L, 1 );
		return false;
	}

	bool bSameType = !!lua_rawequal( L, -1, -2 );
	lua_pop( L, 2 );
	if( !bSameType )
		return false;

	if( lua_objlen(L, iArg1) != sizeof(void *) )
		return false;
	if( lua_objlen(L, iArg2) != sizeof(void *) )
		return false;

	void **pData1 = (void **) lua_touserdata( L, iArg1 );
	void **pData2 = (void **) lua_touserdata( L, iArg2 );
	return *pData1 == *pData2;
}

/*
 * Get a userdata, and check that it's either szType or a type
 * derived from szType, by walking the __index chain.
 */
bool LuaBinding::CheckLuaObjectType( lua_State *L, int narg, const char *szType, bool bOptional )
{
#if defined(FAST_LUA)
	if( bOptional )
		return true;
#endif

	ASSERT_M( narg > 0, ssprintf("%i", narg) ); // negative offsets not supported

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

static void GetGlobalTable( Lua *L )
{
	static LuaReference UserDataTable;
	if( !UserDataTable.IsSet() )
	{
		lua_newtable( L );
		UserDataTable.SetFromStack( L );
	}

	UserDataTable.PushSelf( L );
}

/* The object is on the stack.  It's either a table or a userdata.
 * If needed, associate the metatable; if a table, also add it to
 * the userdata table. */
void LuaBinding::ApplyDerivedType( Lua *L, const RString &sClassName, void *pSelf )
{
	int iTable = lua_gettop( L );

	int iType = lua_type( L, iTable );
	ASSERT_M( iType == LUA_TTABLE || iType == LUA_TUSERDATA,
		ssprintf("%i", iType) );

	if( iType == LUA_TTABLE )
	{
		GetGlobalTable( L );
		int iGlobalTable = lua_gettop( L );

		/* If the table is already in the userdata table, then everything
		 * is already set up. */
		lua_pushvalue( L, iTable );
		lua_rawget( L, iGlobalTable );
		if( !lua_isnil(L, -1) )
		{
			void *pData = lua_touserdata( L, -1 );
			ASSERT( pSelf == pData );

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

void *LuaBinding::GetPointerFromStack( Lua *L, const RString &sType, int iArg )
{
	iArg = LuaHelpers::AbsIndex( L, iArg );

	/* The stack has a userdata or a table.  If it's a table, look up the associated userdata. */
	if( lua_istable(L, iArg) )
	{
		GetGlobalTable( L );

		lua_pushvalue( L, iArg );
		lua_rawget( L, -2 );
		if( lua_isnil(L, -1) )
			luaL_error( L, "stale %s referenced (object used but no longer exists)", sType.c_str() );

		void *pRet = lua_touserdata( L, -1 );
		lua_pop( L, 2 );

		return pRet;
	}
	else if( lua_isuserdata(L, iArg) )
	{
		void **pData = (void **) lua_touserdata( L, iArg );
		return *pData;
	}
	else
		return NULL;
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

	DeepCopy();
}

LuaClass &LuaClass::operator=( const LuaClass &cpy )
{
	LuaTable::operator=(cpy);

	if( !IsSet() )
		return *this;

	DeepCopy();

	return *this;
}


LuaClass::~LuaClass()
{
	if( LUA == NULL )
		return;

	Lua *L = LUA->Get();

	int iTop = lua_gettop( L );

	/* If we're registered in the global table, unregister. */
	GetGlobalTable( L );
	this->PushSelf( L );
	lua_pushnil( L );
	lua_rawset( L, -3 );

	lua_settop( L, iTop );

	LUA->Release( L );
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

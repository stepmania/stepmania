/* LuaBinding - helpers to expose Lua bindings for C++ classes. */

#ifndef LuaBinding_H
#define LuaBinding_H

#include "LuaManager.h"

void CreateGlobalTable( lua_State *L, const CString &szName );
bool CheckType( lua_State *L, int narg, const char *szType );

template <typename Type>
class Luna 
{
protected:
	typedef Type T;

	struct RegType
	{
		const char *szName; 
		int (*mfunc)(T *p, lua_State *L);
	};

	typedef struct { T *pT; } userdataType;

public:
	static void Register( Lua *L )
	{
		/* Create the methods table, if it doesn't already exist. */
		CreateGlobalTable( L, m_sClassName );

		int methods = lua_gettop( L );
		
		/* Create a metatable for the userdata objects. */
		luaL_newmetatable( L, m_sClassName );
		int metatable = lua_gettop( L );
		
		lua_pushliteral( L, "__metatable" );
		lua_pushvalue( L, methods );
		lua_settable( L, metatable );  // hide metatable from Lua getmetatable()
		
		lua_pushliteral( L, "__index" );
		lua_pushvalue( L, methods );
		lua_settable( L, metatable );
		
		lua_pushliteral( L, "__tostring" );
		lua_pushcfunction( L, tostring_T );
		lua_settable( L, metatable );
		
		lua_pushliteral( L, "__eq" );
		lua_pushcfunction( L, equal );
		lua_settable( L, metatable );

		// fill method table with methods from class T
		for( unsigned i=0; s_pvMethods && i < s_pvMethods->size(); i++ )
		{
			const RegType *l = &(*s_pvMethods)[i];
			lua_pushstring( L, l->szName );
			lua_pushlightuserdata( L, (void*)l );
			lua_pushcclosure( L, thunk, 1 );
			lua_settable( L, methods );
		}

		/* Create a metatable for the methods table. */
		lua_newtable( L );
		int methods_metatable = lua_gettop( L );

		// Hide the metatable.
		lua_pushliteral( L, "__metatable" );
		lua_pushstring( L, "(hidden)" );
		lua_settable( L, methods_metatable );

		if( strcmp(m_sBaseClassName, "none") )
		{
			// If this type has a base class, set the __index of this type
			// to the base class.  If the base class doesn't exist, we probably
			// were called before the Register() of the base class; we'll fill
			// it in when we get to it.
			lua_pushliteral( L, "__index" );
			CreateGlobalTable( L, m_sBaseClassName );
			lua_settable( L, methods_metatable );
		}

		lua_pushliteral( L, "__type" );
		lua_pushstring( L, m_sClassName );
		lua_settable( L, methods_metatable );

		/* Set and pop the methods metatable. */
		lua_setmetatable( L, methods );

		lua_pop( L, 2 );  // drop metatable and method table
	}

	// get userdata from Lua stack and return pointer to T object
	static T *check( lua_State *L, int narg, bool bIsSelf = false )
	{
		if( !CheckType(L, narg, m_sClassName) )
		{
			if( bIsSelf )
				luaL_typerror( L, narg, m_sClassName );
			else
				LuaHelpers::TypeError( L, narg, m_sClassName );
		}

		userdataType *pUserdata = static_cast<userdataType*>( lua_touserdata(L, narg) );
		return pUserdata->pT;  // pointer to T object
	}
	
private:
	// member function dispatcher
	static int thunk( Lua *L )
	{
		// stack has userdata, followed by method args
		T *obj = check( L, 1, true );  // get self
		lua_remove(L, 1);  // remove self so member function args start at index 1
		// get member function from upvalue
		RegType *l = static_cast<RegType*>(lua_touserdata(L, lua_upvalueindex(1)));
		return (*(l->mfunc))(obj,L);  // call member function
	}
	
	/* Two objects are equal if the underlying object is the same. */
	static int equal( lua_State *L )
	{
		userdataType *obj1 = static_cast<userdataType*>( lua_touserdata(L, 1) );
		userdataType *obj2 = static_cast<userdataType*>( lua_touserdata(L, 2) );
		lua_pushboolean( L, obj1->pT == obj2->pT );
		return 1;
	}

public:
	// push a userdata containing a pointer to T object
	static int Push( Lua *L, T* p )
	{
		userdataType *ud = static_cast<userdataType*>( lua_newuserdata(L, sizeof(userdataType)) );
		ud->pT = p;  // store pointer to object in userdata
		luaL_getmetatable( L, m_sClassName );  // lookup metatable in Lua registry
		lua_setmetatable( L, -2 );
		return 1;  // userdata containing pointer to T object
	}
	
	static void AddMethod( const char *szName, int (*pFunc)(T *p, lua_State *L) )
	{
		if( s_pvMethods == NULL )
			s_pvMethods = new RegTypeVector;

		RegType r = { szName, pFunc };
		s_pvMethods->push_back(r);
	}

private:
	typedef vector<RegType> RegTypeVector;
	static RegTypeVector *s_pvMethods;

	static const char *m_sClassName;
	static const char *m_sBaseClassName;
	
	static int tostring_T( lua_State *L )
	{
		char buff[32];
		userdataType *ud = static_cast<userdataType*>( lua_touserdata(L, 1) );
		T *obj = ud->pT;
		sprintf( buff, "%p", obj) ;
		lua_pushfstring( L, "%s (%s)", m_sClassName, buff );
		return 1;
	}
};

#define LUA_REGISTER_CLASS( T ) \
	LUA_REGISTER_DERIVED_CLASS( T, none )

#define LUA_REGISTER_DERIVED_CLASS( T, B ) \
	template<> const char *Luna<T>::m_sClassName = #T; \
	template<> const char *Luna<T>::m_sBaseClassName = #B; \
	Luna<T>::RegTypeVector* Luna<T>::s_pvMethods = NULL; \
	static Luna##T registera; \
void T::PushSelf( lua_State *L ) { Luna##T::Push( L, this ); } \
/* Call PushSelf, so we always call the derived Luna<T>::Push. */ \
namespace LuaHelpers { template<> void Push( T *pObject, lua_State *L ) { pObject->PushSelf( L ); } }

#define ADD_METHOD( method_name ) AddMethod( #method_name, method_name );


#endif

/*
 * (c) 2001-2005 Peter Shook, Chris Danford, Glenn Maynard
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

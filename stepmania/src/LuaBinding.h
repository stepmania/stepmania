/* LuaBinding - helpers to expose Lua bindings for C++ classes. */

#ifndef LuaBinding_H
#define LuaBinding_H

#include "LuaManager.h"
class LuaReference;

namespace LuaBinding
{
	void CreateMethodsTable( lua_State *L, const RString &szName );
	bool CheckLuaObjectType( lua_State *L, int narg, const char *szType, bool bOptional=false );
	void *GetUserdataFromGlobalTable( Lua *L, const char *szType, int iArg );
	void ApplyDerivedType( Lua *L, const RString &sClassname, void *pSelf );
};

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

public:
	static void Register( Lua *L )
	{
		/* Create the methods table, if it doesn't already exist. */
		LuaBinding::CreateMethodsTable( L, m_sClassName );

		int methods = lua_gettop( L );
		
		/* Create a metatable for the userdata objects. */
		luaL_newmetatable( L, m_sClassName );
		int metatable = lua_gettop( L );
		
		// We use the metatable to determine the type of the table, so don't
		// allow it to be changed.
		lua_pushstring( L, "(hidden)" );
		lua_setfield( L, metatable, "__metatable" );
		
		lua_pushvalue( L, methods );
		lua_setfield( L, metatable, "__index" );
		
		lua_pushcfunction( L, tostring_T );
		lua_setfield( L, metatable, "__tostring" );
		
		lua_pushcfunction( L, equal );
		lua_setfield( L, metatable, "__eq" );

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
		lua_pushstring( L, "(hidden)" );
		lua_setfield( L, methods_metatable, "__metatable" );

		if( strcmp(m_sBaseClassName, "none") )
		{
			// If this type has a base class, set the __index of this type
			// to the base class.  If the base class doesn't exist, we probably
			// were called before the Register() of the base class; we'll fill
			// it in when we get to it.
			LuaBinding::CreateMethodsTable( L, m_sBaseClassName );
			lua_setfield( L, methods_metatable, "__index" );
		}

		lua_pushstring( L, m_sClassName );
		lua_setfield( L, methods_metatable, "type" );

		/* Set and pop the methods metatable. */
		lua_setmetatable( L, methods );

		lua_pop( L, 2 );  // drop metatable and method table
	}

	// Get userdata from the Lua stack and return a pointer to T object.
	static T *check( lua_State *L, int narg, bool bIsSelf = false )
	{
		if( !LuaBinding::CheckLuaObjectType(L, narg, m_sClassName, true) )
		{
			if( bIsSelf )
				luaL_typerror( L, narg, m_sClassName );
			else
				LuaHelpers::TypeError( L, narg, m_sClassName );
		}

		return get( L, narg );
	}
	
	static T *get( lua_State *L, int narg )
	{
		/* The stack has a userdata or a table.  If it's a table, look up the associated userdata. */
		if( lua_istable(L, narg) )
		{
			return (T *) LuaBinding::GetUserdataFromGlobalTable( L, m_sClassName, narg );
		}
		else if( lua_isuserdata(L, narg) )
		{
			void **pData = (void **) lua_touserdata( L, narg );
			return (T *) *pData;
		}
		else
			return NULL;
	}
	
	/* Push a table or userdata for the given object.  This is called on the
	 * base class, so we pick up the instance of the base class, if any. */ 
	static void PushObject( Lua *L, T* p );

	static void AddMethod( const char *szName, int (*pFunc)(T *p, lua_State *L) )
	{
		if( s_pvMethods == NULL )
			s_pvMethods = new RegTypeVector;

		RegType r = { szName, pFunc };
		s_pvMethods->push_back(r);
	}


private:
	// member function dispatcher
	static int thunk( Lua *L )
	{
		// stack has userdata, followed by method args
		T *obj = check( L, 1, true );  // get self
		lua_remove(L, 1);  // remove self so member function args start at index 1
		// get member function from upvalue
		RegType *l = (RegType *) lua_touserdata( L, lua_upvalueindex(1) );
		return (*(l->mfunc))(obj,L);  // call member function
	}
	
	/* Two objects are equal if the underlying object is the same. */
	static int equal( lua_State *L )
	{
		int iType = lua_type( L, 1 );
		if( lua_type(L, 2) != iType )
		{
			lua_pushboolean( L, false );
			return 1;
		}

		/* Use the regular method for tables.  If an object's table is
		 * kept around after the actual object has been destroyed, the
		 * table is still valid, and the pointer no longer exists. */
		if( iType == LUA_TTABLE )
		{
			int iEqual = lua_rawequal( L, 1, 2 );
			lua_pushboolean( L, iEqual );
			return 1;
		}

		if( !LuaBinding::CheckLuaObjectType(L, 1, m_sClassName) ||
			!LuaBinding::CheckLuaObjectType(L, 2, m_sClassName) )
		{
			lua_pushboolean( L, false );
		}
		else
		{
			const void *pData1 = get( L, 1 );
			const void *pData2 = get( L, 2 );
			lua_pushboolean( L, pData1 != NULL && pData1 == pData2 );
		}

		return 1;
	}

	typedef vector<RegType> RegTypeVector;
	static RegTypeVector *s_pvMethods;

	static const char *m_sClassName;
	static const char *m_sBaseClassName;
	
	static int tostring_T( lua_State *L )
	{
		char buff[32];
		const void *pData = check( L, 1 );
		sprintf( buff, "%p", pData );
		lua_pushfstring( L, "%s (%s)", m_sClassName, buff );
		return 1;
	}
};

/*
 * Instanced classes have an associated table, which is used as "self"
 * instead of a raw userdata.  This should be as lightweight as possible.
 */
#include "LuaReference.h"
class LuaClass: public LuaTable
{
public:
	LuaClass() { }
	LuaClass( const LuaClass &cpy );
	virtual ~LuaClass();
	LuaClass &operator=( const LuaClass &cpy );
};

/* Only a base class has to indicate that it's instanced (has a per-object
 * Lua table).  Derived classes simply call the base class's Push function,
 * specifying a different class name, so they don't need to know about it. */
#define LUA_REGISTER_INSTANCED_BASE_CLASS( T ) \
	LUA_REGISTER_CLASS_BASIC( T, none ) \
	template<> void Luna<T>::PushObject( Lua *L, T* p ) { p->m_pLuaInstance->PushSelf( L ); } \
	void T::PushSelf( lua_State *L ) { Luna<T>::PushObject( L, this ); LuaBinding::ApplyDerivedType( L, #T, this ); }

#define LUA_REGISTER_CLASS( T ) \
	LUA_REGISTER_CLASS_BASIC( T, none ) \
	template<> void Luna<T>::PushObject( Lua *L, T* p ) { void **pData = (void **) lua_newuserdata( L, sizeof(void *) ); *pData = p; } \
	void T::PushSelf( lua_State *L ) { Luna<T>::PushObject( L, this ); LuaBinding::ApplyDerivedType( L, #T, this ); }

#define LUA_REGISTER_DERIVED_CLASS( T, B ) \
	LUA_REGISTER_CLASS_BASIC( T, B ) \
	template<> void Luna<T>::PushObject( Lua *L, T* p ) { Luna<B>::PushObject( L, p ); } \
	void T::PushSelf( lua_State *L ) { Luna<B>::PushObject( L, this ); LuaBinding::ApplyDerivedType( L, #T, this ); }

#define LUA_REGISTER_CLASS_BASIC( T, B ) \
	template<> const char *Luna<T>::m_sClassName = #T; \
	template<> const char *Luna<T>::m_sBaseClassName = #B; \
	template<> Luna<T>::RegTypeVector* Luna<T>::s_pvMethods = NULL; \
	static Luna##T registera##T; \
	/* Call PushSelf, so we always call the derived Luna<T>::Push. */ \
	namespace LuaHelpers { template<> void Push( lua_State *L, T *pObject ) { pObject->PushSelf( L ); } }

#define DEFINE_METHOD( method_name, expr ) \
	static int method_name( T* p, lua_State *L ) { LuaHelpers::Push( L, p->expr ); return 1; }

#define ADD_METHOD( method_name ) \
	AddMethod( #method_name, method_name )


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

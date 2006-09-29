/* LuaBinding - helpers to expose Lua bindings for C++ classes. */

#ifndef LuaBinding_H
#define LuaBinding_H

#include "LuaManager.h"
class LuaReference;

class LuaBinding
{
public:
	LuaBinding();
	~LuaBinding();
	void Register( lua_State *L );
	
	static void RegisterTypes( lua_State *L );

	bool IsDerivedClass() const { return GetClassName() != GetBaseClassName(); }
	virtual const RString &GetClassName() const = 0;
	virtual const RString &GetBaseClassName() const = 0;

	static bool CheckLuaObjectType( lua_State *L, int narg, const char *szType, bool bOptional=false );
	static void ApplyDerivedType( Lua *L, const RString &sClassname, void *pSelf );

protected:
	virtual void Register( Lua *L, int iMethods, int iMetatable ) = 0;

	static void CreateMethodsTable( lua_State *L, const RString &szName );
	static void *GetPointerFromStack( Lua *L, const RString &sType, int iArg );

	static bool Equal( lua_State *L );
	static int PushEqual( lua_State *L );
};

template <typename Type>
class Luna: public LuaBinding
{
protected:
	typedef Type T;
	typedef int (binding_t)(T *p, lua_State *L);

	struct RegType
	{
		const char *szName; 
		binding_t *mfunc;
	};

	void Register( Lua *L, int iMethods, int iMetatable )
	{
		lua_pushcfunction( L, tostring_T );
		lua_setfield( L, iMetatable, "__tostring" );

		// fill method table with methods from class T
		for( unsigned i=0; i < m_aMethods.size(); i++ )
		{
			const RegType *l = &m_aMethods[i];
			lua_pushstring( L, l->szName );
			lua_pushlightuserdata( L, (void*) l->mfunc );
			lua_pushcclosure( L, thunk, 1 );
			lua_settable( L, iMethods );
		}
	}

public:
	virtual const RString &GetClassName() const { return m_sClassName; }
	virtual const RString &GetBaseClassName() const { return m_sBaseClassName; }
	static RString m_sClassName;
	static RString m_sBaseClassName;

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
		return (T *) GetPointerFromStack( L, m_sClassName, narg );
	}
	
	/* Push a table or userdata for the given object.  This is called on the
	 * base class, so we pick up the instance of the base class, if any. */ 
	static void PushObject( Lua *L, const RString &sDerivedClassName, T* p );

protected:
	void AddMethod( const char *szName, int (*pFunc)(T *p, lua_State *L) )
	{
		RegType r = { szName, pFunc };
		m_aMethods.push_back(r);
	}

private:
	// member function dispatcher
	static int thunk( Lua *L )
	{
		// stack has userdata, followed by method args
		T *obj = check( L, 1, true );  // get self
		lua_remove(L, 1);  // remove self so member function args start at index 1
		// get member function from upvalue
		binding_t *pFunc = (binding_t *) lua_touserdata( L, lua_upvalueindex(1) );
		return pFunc( obj, L );  // call member function
	}

	vector<RegType> m_aMethods;

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
	LUA_REGISTER_CLASS_BASIC( T, T ) \
	template<> void Luna<T>::PushObject( Lua *L, const RString &sDerivedClassName, T* p ) { p->m_pLuaInstance->PushSelf( L ); LuaBinding::ApplyDerivedType( L, sDerivedClassName, p ); }

#define LUA_REGISTER_CLASS( T ) \
	LUA_REGISTER_CLASS_BASIC( T, T ) \
	template<> void Luna<T>::PushObject( Lua *L, const RString &sDerivedClassName, T* p ) { void **pData = (void **) lua_newuserdata( L, sizeof(void *) ); *pData = p; LuaBinding::ApplyDerivedType( L, sDerivedClassName, p ); }

#define LUA_REGISTER_DERIVED_CLASS( T, B ) \
	LUA_REGISTER_CLASS_BASIC( T, B ) \
	template<> void Luna<T>::PushObject( Lua *L, const RString &sDerivedClassName, T* p ) { Luna<B>::PushObject( L, sDerivedClassName, p ); }

#define LUA_REGISTER_CLASS_BASIC( T, B ) \
	template<> RString Luna<T>::m_sClassName = #T; \
	template<> RString Luna<T>::m_sBaseClassName = #B; \
	void T::PushSelf( lua_State *L ) { Luna<B>::PushObject( L, Luna<T>::m_sClassName, this ); } \
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

/* LuaBinding - helpers to expose Lua bindings for C++ classes. */

#ifndef LuaBinding_H
#define LuaBinding_H

#include "LuaManager.h"
class LuaReference;

class LuaBinding
{
public:
	LuaBinding();
	virtual ~LuaBinding();
	void Register( lua_State *L );

	static void RegisterTypes( lua_State *L );

	bool IsDerivedClass() const { return GetClassName() != GetBaseClassName(); }
	virtual const std::string &GetClassName() const = 0;
	virtual const std::string &GetBaseClassName() const = 0;

	static void ApplyDerivedType( Lua *L, const std::string &sClassname, void *pSelf );
	static bool CheckLuaObjectType( lua_State *L, int narg, std::string const &szType );

protected:
	virtual void Register( Lua *L, int iMethods, int iMetatable ) = 0;

	static void CreateMethodsTable( lua_State *L, const std::string &szName );
	static void *GetPointerFromStack( Lua *L, const std::string &sType, int iArg );

	static bool Equal( lua_State *L );
	static int PushEqual( lua_State *L );
};

/** @brief Allow the binding of Lua to various classes. */
template <typename Type>
class Luna: public LuaBinding
{
protected:
	typedef Type T;
	typedef int (binding_t)(T *p, lua_State *L);

	struct RegType
	{
		std::string regName;
		binding_t *mfunc;
	};

	void Register( Lua *L, int iMethods, int iMetatable )
	{
		lua_pushcfunction( L, tostring_T );
		lua_setfield( L, iMetatable, "__tostring" );

		// fill method table with methods from class T
		for (auto const m: m_aMethods)
		{
			lua_pushlightuserdata( L, (void*) m.mfunc );
			lua_pushcclosure( L, thunk, 1 );
			lua_setfield( L, iMethods, m.regName.c_str() );
		}
	}

public:
	virtual const std::string &GetClassName() const { return m_sClassName; }
	virtual const std::string &GetBaseClassName() const { return m_sBaseClassName; }
	static std::string m_sClassName;
	static std::string m_sBaseClassName;

	// Get userdata from the Lua stack and return a pointer to T object.
	static T *check( lua_State *L, int narg, bool bIsSelf = false )
	{
		if( !LuaBinding::CheckLuaObjectType(L, narg, m_sClassName) )
		{
			if( bIsSelf )
			{
				luaL_typerror( L, narg, m_sClassName.c_str());
			}
			else
			{
				LuaHelpers::TypeError( L, narg, m_sClassName );
			}
		}
		return get( L, narg );
	}

	static T *get( lua_State *L, int narg )
	{
		return (T *) GetPointerFromStack( L, m_sClassName, narg );
	}

	/* Push a table or userdata for the given object.  This is called on the
	 * base class, so we pick up the instance of the base class, if any. */
	static void PushObject( Lua *L, const std::string &sDerivedClassName, T* p );

protected:
	void AddMethod( std::string const &regName, int (*pFunc)(T *p, lua_State *L) )
	{
		RegType r = { regName, pFunc };
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

	std::vector<RegType> m_aMethods;

	static int tostring_T( lua_State *L )
	{
		char buff[32];
		const void *pData = check( L, 1 );
		snprintf( buff, sizeof(buff), "%p", pData );
		lua_pushfstring( L, "%s (%s)", m_sClassName.c_str(), buff );
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
	template<> void Luna<T>::PushObject( Lua *L, const std::string &sDerivedClassName, T* p ) { p->m_pLuaInstance->PushSelf( L ); LuaBinding::ApplyDerivedType( L, sDerivedClassName, p ); } \
	LUA_REGISTER_CLASS_BASIC( T, T )

#define LUA_REGISTER_CLASS( T ) \
	template<> void Luna<T>::PushObject( Lua *L, const std::string &sDerivedClassName, T* p ) { void **pData = (void **) lua_newuserdata( L, sizeof(void *) ); *pData = p; LuaBinding::ApplyDerivedType( L, sDerivedClassName, p ); } \
	LUA_REGISTER_CLASS_BASIC( T, T )

#define LUA_REGISTER_DERIVED_CLASS( T, B ) \
	template<> void Luna<T>::PushObject( Lua *L, const std::string &sDerivedClassName, T* p ) { Luna<B>::PushObject( L, sDerivedClassName, p ); } \
	LUA_REGISTER_CLASS_BASIC( T, B )

#define LUA_REGISTER_CLASS_BASIC( T, B ) \
	template<> std::string Luna<T>::m_sClassName = #T; \
	template<> std::string Luna<T>::m_sBaseClassName = #B; \
	void T::PushSelf( lua_State *L ) { Luna<B>::PushObject( L, Luna<T>::m_sClassName, this ); } \
	static Luna##T registera##T; \
	/* Call PushSelf, so we always call the derived Luna<T>::Push. */ \
	namespace LuaHelpers { template<> void Push<T*>( lua_State *L, T *const &pObject ) { if( pObject == nullptr ) lua_pushnil(L); else pObject->PushSelf( L ); } }

#define DEFINE_METHOD( method_name, expr ) \
	static int method_name( T* p, lua_State *L ) { LuaHelpers::Push( L, p->expr ); return 1; }

#define COMMON_RETURN_SELF p->PushSelf(L); return 1;

#define GET_SET_BOOL_METHOD(method_name, bool_name) \
static int get_##method_name(T* p, lua_State* L) \
{ \
	lua_pushboolean(L, p->bool_name); \
	return 1; \
} \
static int set_##method_name(T* p, lua_State* L) \
{ \
	p->bool_name= lua_toboolean(L, 1) != 0; \
	COMMON_RETURN_SELF; \
}

#define GETTER_SETTER_BOOL_METHOD(bool_name) \
static int get_##bool_name(T* p, lua_State* L) \
{ \
	lua_pushboolean(L, p->get_##bool_name()); \
	return 1; \
} \
static int set_##bool_name(T* p, lua_State* L) \
{ \
	p->set_##bool_name(lua_toboolean(L, 1) != 0); \
	COMMON_RETURN_SELF; \
}

#define GET_SET_FLOAT_METHOD(method_name, float_name) \
static int get_##method_name(T* p, lua_State* L) \
{ \
	lua_pushnumber(L, p->float_name); \
	return 1; \
} \
static int set_##method_name(T* p, lua_State* L) \
{ \
	p->float_name= FArg(1); \
	COMMON_RETURN_SELF; \
}

#define GET_SET_INT_METHOD(method_name, int_name) \
static int get_##method_name(T* p, lua_State* L) \
{ \
	lua_pushinteger(L, p->int_name); \
	return 1; \
} \
static int set_##method_name(T* p, lua_State* L) \
{ \
	p->int_name= IArg(1); \
	COMMON_RETURN_SELF; \
}

#define GETTER_SETTER_FLOAT_METHOD(float_name) \
static int get_##float_name(T* p, lua_State* L) \
{ \
	lua_pushnumber(L, p->get_##float_name()); \
	return 1; \
} \
static int set_##float_name(T* p, lua_State* L) \
{ \
	p->set_##float_name(FArg(1)); \
	COMMON_RETURN_SELF; \
}

#define GET_SET_ENUM_METHOD(method_name, enum_name, val_name) \
static int get_##method_name(T* p, lua_State* L) \
{ \
	Enum::Push(L, p->val_name); \
	return 1; \
} \
static int set_##method_name(T* p, lua_State* L) \
{ \
	p->val_name= Enum::Check<enum_name>(L, 1); \
	COMMON_RETURN_SELF; \
}

#define GETTER_SETTER_ENUM_METHOD(enum_name, val_name) \
static int get_##val_name(T* p, lua_State* L) \
{ \
	Enum::Push(L, p->get_##val_name()); \
	return 1; \
} \
static int set_##val_name(T* p, lua_State* L) \
{ \
	p->set_##val_name(Enum::Check<enum_name>(L, 1)); \
	COMMON_RETURN_SELF; \
}

#define ADD_METHOD( method_name ) \
	AddMethod( #method_name, method_name )
#define ADD_GET_SET_METHODS(method_name) \
	ADD_METHOD(get_##method_name); ADD_METHOD(set_##method_name);
#define LUA_SET_MEMBER(member, arg_conv) \
static int set_##member(T* p, lua_State* L) \
{ \
	p->m_##member= arg_conv(1); \
	COMMON_RETURN_SELF; \
}
#define GET_SET_MEMBER(member, arg_conv) \
DEFINE_METHOD(get_##member, m_##member); \
LUA_SET_MEMBER(member, arg_conv);

#define LUA_REGISTER_NAMESPACE( T ) \
	static void Register##T( lua_State *L ) { luaL_register( L, #T, T##Table ); lua_pop( L, 1 ); } \
	REGISTER_WITH_LUA_FUNCTION( Register##T )
#define LIST_METHOD( method_name ) \
	{ #method_name, method_name }

// Explicitly separates the stack into args and return values.
// This way, the stack can safely be used to store the previous values.
void DefaultNilArgs(lua_State* L, int n);
float FArgGTEZero(lua_State* L, int index);

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

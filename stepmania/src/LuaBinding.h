/* Based on Lua Wiki example: http://lua-users.org/wiki/SimplerCppBinding
 * Modified by Chris.
 */

#include "lua.h"
#include "lauxlib.h"
#include "LuaManager.h"


#define LUA_REGISTER_CLASS( T, base ) \
class Lua##T { \
public: \
	Lua##T() { LUA->Register( Register ); } \
	static const char className[]; \
	static Luna<T,Lua##T>::RegType methods[]; \
	static void Register( lua_State* L ) { \
		Luna<T,Lua##T>::Register(L); \
		if( stricmp(#base,"null") != 0 ) \
			LUA->RunScript( "getmetatable("#T").__index = "#base ); \
	} \
	static T* MakeNew( lua_State* L ) { \
		return new T; \
	} \
	LUA_##T##_METHODS( T ) \
}; \
static Lua##T registera; \
const char Lua##T::className[] = #T; \
Luna<T,Lua##T>::RegType Lua##T::methods[] = { \
	LUA_##T##_METHODS_MAP( T ) \
	{0,0} \
};
#define LUA_METHOD_MAP( T, Method ) 	{ #Method, Lua##T::##Method },


template <typename T, typename TInfo> class Luna {
  typedef struct { T *pT; } userdataType;
public:
  typedef int (*mfp)(T *p, lua_State *L);
  typedef struct { const char *name; mfp mfunc; } RegType;

  static void Register(lua_State *L) {
    lua_newtable(L);
    int methods = lua_gettop(L);

    luaL_newmetatable(L, TInfo::className);
    int metatable = lua_gettop(L);

    // store method table in globals so that
    // scripts can add functions written in Lua.
    lua_pushstring(L, TInfo::className);
    lua_pushvalue(L, methods);
    lua_settable(L, LUA_GLOBALSINDEX);

    lua_pushliteral(L, "__metatable");
    lua_pushvalue(L, methods);
    lua_settable(L, metatable);  // hide metatable from Lua getmetatable()

    lua_pushliteral(L, "__index");
    lua_pushvalue(L, methods);
    lua_settable(L, metatable);

    lua_pushliteral(L, "__tostring");
    lua_pushcfunction(L, tostring_T);
    lua_settable(L, metatable);

    lua_pushliteral(L, "__gc");
    lua_pushcfunction(L, gc_T);
    lua_settable(L, metatable);

    lua_newtable(L);                // mt for method table
    int mt = lua_gettop(L);
    lua_pushliteral(L, "__call");
    lua_pushcfunction(L, new_T);
    lua_pushliteral(L, "new");
    lua_pushvalue(L, -2);           // dup new_T function
    lua_settable(L, methods);       // add new_T to method table
    lua_settable(L, mt);            // mt.__call = new_T
    lua_setmetatable(L, methods);

    // fill method table with methods from class T
    for (RegType *l = TInfo::methods; l->name; l++) {
      lua_pushstring(L, l->name);
      lua_pushlightuserdata(L, (void*)l);
      lua_pushcclosure(L, thunk, 1);
      lua_settable(L, methods);
    }

    lua_pop(L, 2);  // drop metatable and method table
  }

  // get userdata from Lua stack and return pointer to T object
  static T *check(lua_State *L, int narg) {
    userdataType *ud =
      static_cast<userdataType*>(luaL_checkudata(L, narg, TInfo::className));
    if(!ud) luaL_typerror(L, narg, TInfo::className);
    return ud->pT;  // pointer to T object
  }

private:
  Luna();  // hide default constructor

  // member function dispatcher
  static int thunk(lua_State *L) {
    // stack has userdata, followed by method args
    T *obj = check(L, 1);  // get 'self', or if you prefer, 'this'
    lua_remove(L, 1);  // remove self so member function args start at index 1
    // get member function from upvalue
    RegType *l = static_cast<RegType*>(lua_touserdata(L, lua_upvalueindex(1)));
    return (*(l->mfunc))(obj,L);  // call member function
  }

  // create a new T object and
  // push onto the Lua stack a userdata containing a pointer to T object
  static int new_T(lua_State *L) {
    lua_remove(L, 1);   // use classname:new(), instead of classname.new()
    T *obj = TInfo::MakeNew(L);  // call constructor for T objects
    userdataType *ud =
      static_cast<userdataType*>(lua_newuserdata(L, sizeof(userdataType)));
    ud->pT = obj;  // store pointer to object in userdata
    luaL_getmetatable(L, TInfo::className);  // lookup metatable in Lua registry
    lua_setmetatable(L, -2);
    return 1;  // userdata containing pointer to T object
  }

  // garbage collection metamethod
  static int gc_T(lua_State *L) {
    userdataType *ud = static_cast<userdataType*>(lua_touserdata(L, 1));
    T *obj = ud->pT;
    delete obj;  // call destructor for T objects
    return 0;
  }

  static int tostring_T (lua_State *L) {
    char buff[32];
    userdataType *ud = static_cast<userdataType*>(lua_touserdata(L, 1));
    T *obj = ud->pT;
    sprintf(buff, "%p", obj);
    lua_pushfstring(L, "%s (%s)", TInfo::className, buff);
    return 1;
  }
};

/*
 * (c) 2001-2005 lua-users.org, Chris Danford
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

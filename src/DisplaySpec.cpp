#include "global.h"
#include "DisplaySpec.h"
#include "LuaBinding.h"
#include "RageLog.h"

#include <vector>


class LunaDisplayMode: public Luna<DisplayMode>
{
public:
	DEFINE_METHOD( GetWidth, width );
	DEFINE_METHOD( GetHeight, height );
	DEFINE_METHOD( GetRefreshRate, refreshRate );
	LunaDisplayMode()
	{
		ADD_METHOD( GetWidth );
		ADD_METHOD( GetHeight );
		ADD_METHOD( GetRefreshRate );
	}
};

LUA_REGISTER_CLASS( DisplayMode )

class LunaDisplaySpec: public Luna<DisplaySpec>
{
public:
	DEFINE_METHOD( GetId, id() );
	DEFINE_METHOD( GetName, name() );
	DEFINE_METHOD( IsVirtual, isVirtual() );
	static int GetSupportedModes( T* p, lua_State *L)
	{
		std::vector<DisplayMode*> v;
		for (auto const &m:  p->supportedModes())
		{
			v.push_back( const_cast<DisplayMode*>(&m));
		}
		LuaHelpers::CreateTableFromArray( v, L );
		return 1;
	}
	static int GetCurrentMode( T *p, lua_State *L )
	{
		if (p->currentMode() != nullptr) {
			DisplayMode *m = const_cast<DisplayMode *>( p->currentMode() );
			m->PushSelf( L );
		} else {
			lua_pushnil( L );
		}
		return 1;
	}

	LunaDisplaySpec()
	{
		ADD_METHOD( GetId );
		ADD_METHOD( GetName );
		ADD_METHOD( GetSupportedModes );
		ADD_METHOD( IsVirtual );
		ADD_METHOD( GetCurrentMode );
	}
};

LUA_REGISTER_CLASS( DisplaySpec )

namespace
{
	const char *DISPLAYSPECS = "DisplaySpecs";

	DisplaySpecs *check_DisplaySpecs(lua_State *L)
	{
		void *ud = luaL_checkudata( L, 1, DISPLAYSPECS );
		luaL_argcheck( L, ud != nullptr, 1, "`DisplaySpecs` expected" );
		return static_cast<DisplaySpecs *>(ud);
	}

	int DisplaySpecs_gc(lua_State *L)
	{
		DisplaySpecs *specs = static_cast<DisplaySpecs *> (lua_touserdata( L, 1 ));
		if (specs)
		{
			specs->~DisplaySpecs();
		}

		return 0;
	}

	int DisplaySpecs_len(lua_State *L)
	{
		DisplaySpecs *specs = check_DisplaySpecs( L );
		if (specs)
		{
			lua_pushinteger( L, specs->size() );
		} else
		{
			lua_pushinteger( L, 0 );
		}
		return 1;
	}

	int DisplaySpecs_tostring(lua_State *L)
	{
		DisplaySpecs *specs = check_DisplaySpecs( L );
		lua_pushfstring( L, "DisplaySpecs: %p", specs );
		return 1;
	}

	int DisplaySpecs_get(lua_State *L)
	{
		DisplaySpecs *specs = check_DisplaySpecs( L );
		if (specs)
		{
			int index = luaL_checkint( L, 2 );
			luaL_argcheck( L, 1 <= index && static_cast<unsigned int> (index) <= specs->size(), 2,
						   "index out of range" );
			DisplaySpecs::iterator it = specs->begin();
			std::advance( it, index - 1 );
			DisplaySpec *s = const_cast<DisplaySpec *>(&(*it));
			s->PushSelf( L );
		}
		else
		{
			lua_pushnil( L );
		}
		return 1;
	}

	const luaL_Reg DisplaySpecs_meta[] =
	{
		{"__gc",       DisplaySpecs_gc},
		{"__index",    DisplaySpecs_get},
		{"__len",      DisplaySpecs_len},
		{"__tostring", DisplaySpecs_tostring},
		{nullptr, nullptr}
	};


	void register_DisplaySpecs(lua_State *L)
	{
		luaL_newmetatable( L, DISPLAYSPECS );
		luaL_openlib( L, 0, DisplaySpecs_meta, 0 );
		lua_pop( L, 1 );
	}
}
REGISTER_WITH_LUA_FUNCTION(register_DisplaySpecs);

DisplaySpecs *pushDisplaySpecs(lua_State *L, const DisplaySpecs &specs)
{
	void *vpSpecs = lua_newuserdata( L, sizeof( DisplaySpecs ));
	DisplaySpecs *pspecs = new( vpSpecs ) DisplaySpecs( specs );
	luaL_getmetatable( L, DISPLAYSPECS );
	lua_setmetatable( L, -2 );
	return pspecs;
}

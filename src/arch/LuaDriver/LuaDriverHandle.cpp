#include "global.h"
#include "LuaManager.h"
#include "LuaDriverHandle.h"

map<RString,MakeHandleFn> g_pMapRegistrees = NULL;

void LuaDriverHandle::RegisterAPI( const RString &sName, MakeHandleFn pfn )
{
	if( g_pMapRegistrees == NULL )
		g_pMapRegistrees = new map<RString,MakeHandleFn>;

	if( g_pMapRegistrees->find(sName) != g_pMapRegistrees->end() )
		FAIL_M( ssprintf("LuaDriver API %s registered more than once", sName.c_str()) );

	(*g_pMapRegistrees)[sName] = pfn;
}

/* Pushes a new handle of the given type, or nil if the type doesn't exist */
void LuaDriverHandle::PushAPIHandle( Lua *L, const RString &sName )
{
	map<RString,MakeHandleFn>::iterator it = g_pMapRegistrees->find( sName );

	if( it == g_pMapRegistrees->end() )
	{
		lua_pushnil( L );
		return;
	}

	const MakeHandleFn &pfn = it->second;
	LuaDriverHandle *pHandle = pfn();
	pHandle->PushSelf( L );
}

LunaLuaDriverHandle : public Luna<LuaDriverHandle>
{
public:
	static int Destroy( T* p, lua_State *L )
	{
		SAFE_DELETE( p );
		return 0;
	}

	static int IsOpen( T *p, lua_State *L )
	{
		lua_pushboolean( L, p->IsOpen() );
		return 1;
	}

	static int Close( T* p, lua_State *L )
	{
		p->Close();
		return 0;
	}

	static int GetRevision( T* p, lua_State *L )
	{
		lua_pushnumber( L, p->GetRevisionMajor() );
		lua_pushnumber( L, p->GetRevisionMinor() );
		return 2;
	}

	LunaLuaDriverHandle()
	{
		ADD_METHOD( Destroy );
		ADD_METHOD( IsOpen );
		ADD_METHOD( Close );
		ADD_METHOD( GetRevision );
	}
};

LUA_REGISTER_INSTANCED_BASE_CLASS( LuaDriverHandle );

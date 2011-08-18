#include "global.h"
#include "RageUtil.h"
#include "LuaManager.h"
#include "LuaDriverHandle.h"
#include "LuaDriverHandle_USB.h"

#include <map>

map<RString,MakeHandleFn> *g_pMapRegistrees = NULL;

LuaDriverHandle::LuaDriverHandle()
{
}

LuaDriverHandle::~LuaDriverHandle()
{
}

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

	/* allocates a new handle; call Destroy() from Lua to deallocate */
	LuaDriverHandle *pHandle = pfn();
//	LuaDriverHandle_USB *pHandle2 = dynamic_cast<LuaDriverHandle_USB*>( pHandle );
	pHandle->PushSelf( L );
}

#include "LuaBinding.h"

/* For the sake of brevity, we're stealing "LuaDriver" for API utils */
namespace
{
	int CreateAPIHandle( lua_State *L )
	{
		LuaDriverHandle::PushAPIHandle( L, SArg(1) );
		return 1;
	}

	const luaL_Reg LuaDriverTable[] =
	{
		LIST_METHOD( CreateAPIHandle ),
		{ NULL, NULL }
	};
};

LUA_REGISTER_NAMESPACE( LuaDriver );


class LunaLuaDriverHandle : public Luna<LuaDriverHandle>
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

	static int GetError( T* p, lua_State *L )
	{
		lua_pushnumber( L, p->GetError() );
		return 1;
	}

	static int GetErrorStr( T* p, lua_State *L )
	{
		int error = p->GetError();

		if( lua_isnumber(L, 1) )
			error = lua_tointeger( L, 1 );

		lua_pushstring( L, p->GetErrorStr(error) );
		return 1;
	}

	LunaLuaDriverHandle()
	{
		ADD_METHOD( Destroy );
		ADD_METHOD( IsOpen );
		ADD_METHOD( Close );
		ADD_METHOD( GetRevision );
		ADD_METHOD( GetError );
		ADD_METHOD( GetErrorStr );
	}
};

LUA_REGISTER_CLASS( LuaDriverHandle );

/*
 * (c) 2011 Mark Cannon
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

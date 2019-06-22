#include "global.h"
#include "ArchHooks.h"
#include "LuaReference.h"
#include "RageLog.h"
#include "RageThreads.h"
#include "arch/arch_default.h"

bool ArchHooks::g_bQuitting = false;
bool ArchHooks::g_bToggleWindowed = false;
// Keep from pulling RageThreads.h into ArchHooks.h
static RageMutex g_Mutex( "ArchHooks" );
ArchHooks *HOOKS = nullptr; // global and accessible from anywhere in our program

ArchHooks::ArchHooks(): m_bHasFocus(true), m_bFocusChanged(false)
{
	
}

bool ArchHooks::GetAndClearToggleWindowed()
{
	LockMut( g_Mutex );
	bool bToggle = g_bToggleWindowed;
	
	g_bToggleWindowed = false;
	return bToggle;
}

void ArchHooks::SetToggleWindowed()
{
	LockMut( g_Mutex );
	g_bToggleWindowed = true;
}

void ArchHooks::SetHasFocus( bool bHasFocus )
{
	if( bHasFocus == m_bHasFocus )
		return;
	m_bHasFocus = bHasFocus;

	LOG->Trace( "App %s focus", bHasFocus? "has":"doesn't have" );
	LockMut( g_Mutex );
	m_bFocusChanged = true;
}

bool ArchHooks::AppFocusChanged()
{
	LockMut( g_Mutex );
	bool bFocusChanged = m_bFocusChanged;
	
	m_bFocusChanged = false;
	return bFocusChanged;
}

bool ArchHooks::GoToURL( RString sUrl )
{
	return false;
}

ArchHooks *ArchHooks::Create()
{
	return new ARCH_HOOKS;
}

RString ArchHooks::GetClipboard()
{
	LOG->Warn("ArchHooks: GetClipboard() NOT IMPLEMENTED");
	return "";
}

/* XXX: Most singletons register with lua in their constructor.  ArchHooks is
 * instantiated before Lua, so we encounter a dependency problem when
 * trying to register HOOKS. Work around it by registering HOOKS function
 * which sm_main will call after instantiating Lua. */
void ArchHooks::RegisterWithLua()
{
	Lua *L = LUA->Get();
	lua_pushstring( L, "HOOKS" );
	HOOKS->PushSelf( L );
	lua_settable( L, LUA_GLOBALSINDEX );
	LUA->Release( L );
}

// lua start
#include "LuaBinding.h"
#include "LuaReference.h"

class LunaArchHooks: public Luna<ArchHooks>
{
public:
	DEFINE_METHOD( AppHasFocus, AppHasFocus() );
	DEFINE_METHOD( GetArchName, GetArchName() );
	
	LunaArchHooks()
	{
		ADD_METHOD( AppHasFocus );
		ADD_METHOD( GetArchName );
	}
};
LUA_REGISTER_CLASS( ArchHooks );

/*
 * (c) 2003-2004 Glenn Maynard, Chris Danford
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

#include "global.h"
#include "LuaReference.h"
#include "LuaManager.h"
#include "LuaBinding.h"
#include "Foreach.h"
#include "SubscriptionManager.h"

template<>
set<LuaReference*>* SubscriptionManager<LuaReference>::s_pSubscribers = NULL;

LuaReference::LuaReference()
{
	m_iReference = LUA_NOREF;
	SubscriptionManager<LuaReference>::Subscribe( this );
}

LuaReference::~LuaReference()
{
	Unregister();
	SubscriptionManager<LuaReference>::Unsubscribe( this );
}

LuaReference::LuaReference( const LuaReference &cpy )
{
	SubscriptionManager<LuaReference>::Subscribe( this );

	/* Make a new reference. */
	lua_rawgeti( LUA->L, LUA_REGISTRYINDEX, cpy.m_iReference );
	m_iReference = luaL_ref( LUA->L, LUA_REGISTRYINDEX );
}

LuaReference &LuaReference::operator=( const LuaReference &cpy )
{
	if( this == &cpy )
		return *this;

	Unregister();

	/* Make a new reference. */
	lua_rawgeti( LUA->L, LUA_REGISTRYINDEX, cpy.m_iReference );
	m_iReference = luaL_ref( LUA->L, LUA_REGISTRYINDEX );

	return *this;
}

void LuaReference::SetFromStack()
{
	Unregister();

	m_iReference = luaL_ref( LUA->L, LUA_REGISTRYINDEX );
}

void LuaReference::SetFromNil()
{
	Unregister();
	m_iReference = LUA_REFNIL;
}

void LuaReference::PushSelf( lua_State *L ) const
{
	lua_rawgeti( LUA->L, LUA_REGISTRYINDEX, m_iReference );
}

bool LuaReference::IsSet() const
{
	return m_iReference != LUA_NOREF;
}

int LuaReference::GetLuaType() const
{
	this->PushSelf( LUA->L );
	int iRet = lua_type( LUA->L, -1 );
	lua_pop( LUA->L, 1 );
	return iRet;
}

void LuaReference::Register()
{
}

void LuaReference::Unregister()
{
	if( LUA == NULL )
		return; // nothing to do

	luaL_unref( LUA->L, LUA_REGISTRYINDEX, m_iReference );
	m_iReference = LUA_NOREF;
}

void LuaReference::ReRegisterAll()
{
	if( SubscriptionManager<LuaReference>::s_pSubscribers == NULL )
		return;
	FOREACHS( LuaReference*, *SubscriptionManager<LuaReference>::s_pSubscribers, p )
		(*p)->ReRegister();
}

void LuaReference::ReRegister()
{
	/* When this is called, the Lua state has been wiped.  Don't try to unregister our
	 * old function reference, since it's already gone (and the number may point
	 * somewhere else). */
	m_iReference = LUA_NOREF;

	Register();
}


void LuaExpression::SetFromExpression( const CString &sExpression )
{
	m_sExpression = sExpression;
	Register();
}

void LuaExpression::Register()
{
	LUA->RunExpression( m_sExpression );

	/* Store the result. */
	this->SetFromStack();
}

/*
 * (c) 2005 Glenn Maynard, Chris Danford
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

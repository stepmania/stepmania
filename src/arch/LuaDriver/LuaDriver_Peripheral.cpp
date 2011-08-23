#include "global.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "LuaManager.h"
#include "LuaReference.h"
#include "LuaDriver_Peripheral.h"

LuaDriver_Peripheral::~LuaDriver_Peripheral()
{
	map<RString,LuaReference*>::iterator it = m_mMessageFunctions.begin();
	for( it; it != m_mMessageFunctions.end(); ++it )
		delete it->second;

	m_mMessageFunctions.clear();
}

bool LuaDriver_Peripheral::LoadDerivedFromTable( lua_State *L, LuaReference *pTable )
{
	ASSERT( lua_gettop(L) == 0 );

	/* iterate over the key/value pairs in this table; if we find a key
	 * suffixed with 'Message', we subscribe to that message and allocate
	 * a LuaReference pointing to the function to be called. */

	pTable->PushSelf( L ); // position 1 in the stack

	lua_pushnil( L ); // starting key
	while( lua_next(L, 1) )
	{
		// ignore keys that aren't strings
		if( lua_type(L, -2) != LUA_TSTRING )
		{
			lua_pop( L, 1 );
			continue;
		}

		const RString sEnding = "Message";
		RString sKey( lua_tostring(L, -2) );

		if( !EndsWith(sKey, sEnding) )
		{
			lua_pop( L, 1 );
			continue;
		}

		// extract the message name from the string
		RString sMessage( sKey.begin(), sKey.end() - sEnding.size() );

		if( lua_type(L, -1) != LUA_TFUNCTION )
		{
			LOG->Warn( "[LuaDriver_Peripheral] Got key %s, but "
				"value is of type %s (should be function)",
				sKey.c_str(), lua_typename(L, lua_type(L,-1)) );

			lua_pop( L, -1 );
			continue;
		}

		// subscribe to this message
		this->SubscribeToMessage( sMessage );

		// pop the function value into a new reference
		LuaReference *pRef = new LuaReference;
		pRef->SetFromStack( L );

		// add it to the message functions map
		m_mMessageFunctions.insert( pair<RString,LuaReference*>(sMessage, pRef) );
	}

	lua_pop( L, 2 ); // pop last key and table
	ASSERT( lua_gettop(L) == 0);

	return true;
}

void LuaDriver_Peripheral::ModuleUpdate( lua_State *L, float fDeltaTime )
{
	m_pUpdate->PushSelf( L ); // function
	m_pDriver->PushSelf( L ); // argument 1
	LuaHelpers::Push( L, fDeltaTime ); // argument 2

	RString sError;

	LuaHelpers::RunScriptOnStack(L, sError, 2, 0);

}

void LuaDriver_Peripheral::HandleMessage( const Message &msg )
{
	map<RString,LuaReference*>::iterator it = m_mMessageFunctions.find( msg.GetName() );

	if( it == m_mMessageFunctions.end() )
	{
		LOG->Warn( "Subscribed to message \"%s\", but can't find handler", msg.GetName().c_str() );
		return;
	}

	LuaReference *pFunc = it->second;

	// obtain a Lua context and call the message function
	Lua *L = LUA->Get();

	pFunc->PushSelf( L ); // function
	m_pDriver->PushSelf( L ); // argument 1

	const LuaReference *params  = msg.GetParamTable();

	// argument 2
	if( params )
		params->PushSelf( L );
	else
		lua_pushnil( L );

	RString sError;
	LuaHelpers::RunScriptOnStack(L, sError, 2, 0); // 2 args, 0 results

	LUA->Release( L );

	if( !sError.empty() )
		LOG->Warn( "[LuaDriver_Peripheral::HandleMessage(%s)] %s",
			msg.GetName().c_str(), sError.c_str() );
}

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


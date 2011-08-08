#include "global.h"
#include "RageLog.h"
#include "RageThreads.h"
#include "LuaDriver_LightsModule.h"

#include "InputMapper.h" // for FOREACHGameButtonInScheme, GetInputScheme
#include "GameInput.h"

/*
 * LuaDriver_LightsModule functions
 */

LuaDriver_LightsModule::LuaDriver_LightsModule( const RString &sName ) :
	LuaDriver(sName), LightsDriver()
{
	// woop woop
	m_pEvent = NULL;
	m_bThreaded = false;
	ZERO( m_LightsState );
}

LuaDriver_LightsModule::~LuaDriver_LightsModule()
{
	if( m_bRunning )
	{
		Lua *L = LUA->Get();
		ModuleExit( L );
		LUA->Release( L );
	}

	SAFE_DELETE( m_pEvent );
}

/* This function is always called from the game loop, so drivers that cannot
 * do their updates very, very quickly have a thread that actually updates;
 * we just copy the LightsState and pulse the waiting thread here. */
void LuaDriver_LightsModule::Set( const LightsState *ls )
{
	/* If the thread is still updating, ignore this call */
	if( m_pEvent && !m_pEvent->TryLock() )
		return;

	m_LightsState = *ls;

	/* we have the lock: pulse the loading thread and return */
	if( m_pEvent )
	{
		m_pEvent->Signal();
		m_pEvent->Unlock();
		return;
	}
	/* not threaded: call Update directly */
	else
	{
		Lua *L = LUA->Get();
		this->ModuleUpdate( L );
		LUA->Release( L );
	}
}

void LuaDriver_LightsModule::ModuleThread()
{
	m_pEvent->Lock();

	while( m_bRunning )
	{
		// TODO: events can wake up spuriously: guard against it.
		m_pEvent->Wait();

		Lua *L = LUA->Get();
		this->ModuleUpdate( L );
		LUA->Release( L );
	}

	m_pEvent->Unlock();
}

bool LuaDriver_LightsModule::ModuleInit( Lua *L )
{
	bool ret = LuaDriver::ModuleInit( L );

	if( m_bThreaded )
	{
		RString sEventName = ssprintf( "%s event", m_sName.c_str() );
		m_pEvent = new RageEvent( sEventName );
	}

	return ret;
}

void LuaDriver_LightsModule::ModuleExit( Lua *L )
{
	/* zero lights on exit. */
	ZERO( m_LightsState );
	Set( &m_LightsState );

	/* stop the loop, signal our event, and wait on the thread; the
	 * thread itself will be cleaned up by LuaDriver::ModuleExit(). */
	LUA->YieldLua();

	if( m_pEvent )
	{
		m_bRunning = false;
		m_pThread->Wait();
	}

	LUA->UnyieldLua();

	LuaDriver::ModuleExit( L );
}

void LuaDriver_LightsModule::ModuleUpdate( Lua *L )
{
	RString sError;

	m_pUpdate->PushSelf( L ); // push Update function
	m_pDriver->PushSelf( L ); // push our table (self)
	PushLightsState( L, &m_LightsState );

	// TODO: error checking
	LuaHelpers::RunScriptOnStack( L, sError, 2, 0 ); // 2 args, 0 results

	if( !sError.empty() )
		LOG->Warn( "LuaDriver_LightsModule::ModuleUpdate(%s): %s", m_sName.c_str(), sError.c_str() );
}

/* Pushes a table describing the current LightsState to the stack */
void LuaDriver_LightsModule::PushLightsState( Lua *L, const LightsState *ls )
{
	lua_newtable( L );
	FOREACH_CabinetLight( cl )
	{
		Enum::Push( L, cl );
		lua_pushboolean( L, ls->m_bCabinetLights[cl] );
		lua_rawset( L, -3 );
	}

	if( INPUTMAPPER == NULL )
		return;

	const InputScheme *pScheme = INPUTMAPPER->GetInputScheme();

	FOREACH_ENUM( GameController, gc )
	{
		Enum::Push( L, gc );
		lua_newtable( L );

		FOREACH_GameButtonInScheme( pScheme, gb )
		{
			Enum::Push( L, gb );
			lua_pushboolean( L, ls->m_bGameButtonLights[gc][gb] );
			lua_rawset( L, -3 );
		}

		/* push this subtable into the main table */
		lua_rawset( L, -3 );
	}

	/* leave the new table on the top of the stack */
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

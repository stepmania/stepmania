#ifndef LUADRIVER_LIGHTSMODULE_H
#define LUADRIVER_LIGHTSMODULE_H

#include "LuaDriver.h"
#include "arch/Lights/LightsDriver.h"

class RageEvent;

class LuaDriver_LightsModule : public LuaDriver, public LightsDriver
{
public: 
	LuaDriver_LightsModule( const RString &sName );
	virtual ~LuaDriver_LightsModule();

	bool ModuleInit( lua_State *L );
	void ModuleUpdate( lua_State *L );

	/* we need to manually wait here, or LuaDriver hang on Wait() */
	void ModuleExit( lua_State *L );

	void Set( const LightsState *ls );

protected:
	/* on Set(), threaded drivers copy the state here for update threads */
	LightsState m_LightsState;

	/* In case the Set() takes longer than time between calls, we use
	 * this flag to ignore any subsequent Set()s until the first is done. */
	bool m_bUpdating;

	void ModuleThread();

	virtual bool LoadDerivedFromTable( Lua *L, LuaReference *pTable ) { return true; }

	/* Pushes a table representing the given LightsState. */
	static void PushLightsState( Lua *L, const LightsState *ls );

	/* LightsDrivers currently receive LightsState through Set(), so
	 * for threads, we use a RageEvent to signal threaded updates. */
	RageEvent *m_pEvent;
};

#endif // LUADRIVER_LIGHTSMODULE_H

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


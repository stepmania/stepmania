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

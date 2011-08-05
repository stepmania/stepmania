/* LuaDriver - I/O modules in Lua space.

TODO:
- Lazy load modules (is this feasible?)
*/

#ifndef LUA_DRIVER_H
#define LUA_DRIVER_H

/* a bunch of forward declarations */
struct lua_State;
class LuaReference;

class RageThread;
class InputHandler;
class LightsDriver;


#include <map>

typedef void (*PushAPIHandleFn)(lua_State*);

class LuaDriver;

/* convenience typedef */
typedef map<RString,LuaDriver*> LuaDriverMap;

class LuaDriver
{
public:
	/* Registers an API
	/* Loads an input or lights driver from the given script and, if
	 * successful, places it in the map of available module drivers. */
	static bool Load( const RString &sScript );

	/* Adds any handlers matching names in sDrivers to AddTo. */
	static void AddInputModules( const RString &sDrivers, vector<InputHandler*> &AddTo );
	static void AddLightsModules( const RString &sDrivers, vector<LightsDriver*> &AddTo );

	enum DriverType {
		DriverType_Input,
		DriverType_Lights,
		DriverType_Invalid,
		NUM_DriverType,
	};

	static const char* DriverTypeNames[NUM_DriverType];

protected:
	static LuaDriverMap s_mpInputModules, s_mpLightsModules;

public:
	LuaDriver( const RString &sName );
	~LuaDriver();

	bool IsRunning() const { return m_bRunning; }
	bool IsThreaded() const { return m_bThreaded; }
	RString GetName() const { return m_sName; }

	/* This must be set in InitModule or it will have no effect. */
	void SetThreaded( bool b ) { m_bThreaded = b; }

	/* Executes the appropriate function in the Lua driver script. */
	/* XXX: these are public for AddDrivers, but they shouldn't be public. */
	virtual bool ModuleInit( lua_State *L );
	virtual void ModuleExit( lua_State *L );
	virtual void ModuleUpdate( lua_State *L );

protected:
	bool LoadFromTable( lua_State *L, LuaReference *pTable );
	virtual bool LoadDerivedFromTable( lua_State *L, LuaReference *pTable ) { return true; }

	static int ModuleThread_Start( void *p ) { ((LuaDriver*)p)->ModuleThread(); return 0; }
	virtual void ModuleThread() = 0;

	bool m_bRunning, m_bThreaded;

	/* Used for logging, name matching, and GetDevicesAndDescriptions */
	RString m_sName, m_sDesc;

	/* m_pThread handles Update() if this driver is threaded */
	RageThread *m_pThread;

	/* Reference to the Lua table describing this driver */
	LuaReference *m_pDriver;

	/* References to the Lua functions that the driver requires */
	LuaReference *m_pInit, *m_pExit, *m_pUpdate;
};

#endif // LUA_DRIVER_H

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

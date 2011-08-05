#ifndef LUADRIVER_INPUTMODULE_H
#define LUADRIVER_INPUTMODULE_H

#include "LuaDriver.h"
#include "arch/InputHandler/InputHandler.h"
#include "RageInputDevice.h" // for InputDevice

class LuaDriver_InputModule : public LuaDriver, public InputHandler
{
public:
	LuaDriver_InputModule( const RString &sName );
	virtual ~LuaDriver_InputModule();

	bool ModuleInit( lua_State *L );

	/* InputHandler entry point, for non-threaded drivers */
	void Update();
	void GetDevicesAndDescriptions( vector<InputDeviceInfo> &vDevicesOut );

	/* C callback: we wrap InputHandler::ButtonPressed around this */
	void ButtonPressed( DeviceButton db, float level );

protected:
	virtual bool LoadDerivedFromTable( Lua *L, LuaReference *pTable ) { return true; }
	void ModuleThread();

	InputDevice m_InputDevice;
};

#endif // LUADRIVER_INPUTMODULE_H

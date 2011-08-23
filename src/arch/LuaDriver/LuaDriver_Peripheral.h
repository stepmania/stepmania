#ifndef LUA_DRIVER_PERIPHERAL_H
#define LUA_DRIVER_PERIPHERAL_H

#include "LuaDriver.h"
#include "MessageManager.h"

class LuaDriver_Peripheral : public LuaDriver, MessageSubscriber
{
public:
	void ModuleUpdate( lua_State *L, float fDeltaTime );
	void HandleMessage( const Message &msg );

protected:
	map<RString,LuaReference*> m_mMessageFunctions;
};

#endif // LUA_DRIVER_PERIPHERAL_H

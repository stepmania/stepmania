#ifndef LUA_DRIVER_PERIPHERAL_H
#define LUA_DRIVER_PERIPHERAL_H

#include "LuaDriver.h"
#include "MessageManager.h"

struct lua_State;

class LuaDriver_Peripheral : public LuaDriver, MessageSubscriber
{
public:
	~LuaDriver_Peripheral();

	void ModuleUpdate( lua_State *L, float fDeltaTime );
	void HandleMessage( const Message &msg );

protected:
	bool LoadDerivedFromTable( lua_State *L, LuaReference *pTable );

	map<RString,LuaReference*> m_mMessageFunctions;
};

#endif // LUA_DRIVER_PERIPHERAL_H

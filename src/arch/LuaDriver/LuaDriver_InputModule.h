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
	virtual bool LoadDerivedFromTable( Lua *L, LuaReference *pTable );

	void ModuleThread();

	InputDevice m_InputDevice;
};

#endif // LUADRIVER_INPUTMODULE_H

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

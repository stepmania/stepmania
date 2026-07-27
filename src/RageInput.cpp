#include "global.h"
#include "RageInput.h"
#include "RageLog.h"
#include "arch/InputHandler/InputHandler.h"
#include "Preference.h"
#include "LuaManager.h"
#include "LocalizedString.h"

#include <vector>


RageInput* INPUTMAN = nullptr; // global and accessible from anywhere in our program

Preference<RString> g_sInputDrivers( "InputDrivers", "" ); // "" == DEFAULT_INPUT_DRIVER_LIST
Preference<RString> g_sInputDeviceOrder( "InputDeviceOrder", "" ); // "" == DEFAULT_LINUX_INPUT_DEVICE_ORDER_LIST

namespace
{
	struct LoadedInputHandler
	{
		InputHandler *m_pDevice;
	};
	std::vector<LoadedInputHandler> m_InputHandlers;
	std::map<InputDevice, InputHandler*> g_mapDeviceToHandler;
}

RageInput::RageInput()
{
	LOG->Trace( "RageInput::RageInput()" );

	// Register with Lua.
	{
		Lua *L = LUA->Get();
		lua_pushstring( L, "INPUTMAN" );
		this->PushSelf( L );
		lua_settable( L, LUA_GLOBALSINDEX );
		LUA->Release( L );
	}

	LoadDrivers();
}

RageInput::~RageInput()
{
	// Delete optional devices.
	for( unsigned i = 0; i < m_InputHandlers.size(); ++i )
		delete m_InputHandlers[i].m_pDevice;
	m_InputHandlers.clear();
	g_mapDeviceToHandler.clear();

	// Unregister with Lua.
	LUA->UnsetGlobal( "INPUTMAN" );
}

static LocalizedString NO_INPUT_DEVICES_LOADED ( "RageInput", "No input devices were loaded." );
void RageInput::LoadDrivers()
{
	for( unsigned i = 0; i < m_InputHandlers.size(); ++i )
		delete m_InputHandlers[i].m_pDevice;
	m_InputHandlers.clear();
	g_mapDeviceToHandler.clear();

	// Init optional devices.
	std::vector<InputHandler *> apDevices;

	InputHandler::Create( g_sInputDrivers, apDevices );
	for( unsigned i = 0; i < apDevices.size(); ++i )
		AddHandler( apDevices[i] );

	// If no input devices are loaded, the user won't be able to input anything.
	if( apDevices.size() == 0 )
		LOG->Warn( "%s", NO_INPUT_DEVICES_LOADED.GetValue().c_str() );
}

void RageInput::Update()
{
	// Update optional devices.
	for( unsigned i = 0; i < m_InputHandlers.size(); ++i )
		m_InputHandlers[i].m_pDevice->Update();
}

bool RageInput::DevicesChanged()
{
	// Update optional devices.
	for( unsigned i = 0; i < m_InputHandlers.size(); ++i )
	{
		if( m_InputHandlers[i].m_pDevice->DevicesChanged() )
			return true;
	}
	return false;
}

void RageInput::GetDevicesAndDescriptions( std::vector<InputDeviceInfo>& vDevicesOut ) const
{
	for( unsigned i = 0; i < m_InputHandlers.size(); ++i )
		m_InputHandlers[i].m_pDevice->GetDevicesAndDescriptions( vDevicesOut );
}

void RageInput::WindowReset()
{
	for( unsigned i = 0; i < m_InputHandlers.size(); ++i )
		m_InputHandlers[i].m_pDevice->WindowReset();
}

void RageInput::AddHandler( InputHandler *pHandler )
{
	ASSERT( pHandler != nullptr );

	LoadedInputHandler hand;
	hand.m_pDevice = pHandler;
	m_InputHandlers.push_back(hand);

	std::vector<InputDeviceInfo> aDeviceInfo;
	hand.m_pDevice->GetDevicesAndDescriptions( aDeviceInfo );
	for (InputDeviceInfo const &idi : aDeviceInfo)
		g_mapDeviceToHandler[idi.id] = pHandler;
}

/** @brief Return the first InputDriver for the requested InputDevice. */
InputHandler *RageInput::GetHandlerForDevice( const InputDevice id )
{
	std::map<InputDevice, InputHandler*>::iterator it = g_mapDeviceToHandler.find(id);
	if( it == g_mapDeviceToHandler.end() )
		return nullptr;
	return it->second;
}

RString RageInput::GetDeviceSpecificInputString( const DeviceInput &di )
{
	InputHandler *pDriver = GetHandlerForDevice( di.device );
	if( pDriver != nullptr )
		return pDriver->GetDeviceSpecificInputString(di);
	else
		return di.ToString();
}

RString RageInput::GetLocalizedInputString( const DeviceInput &di )
{
	InputHandler *pDriver = GetHandlerForDevice( di.device );
	if( pDriver != nullptr )
		return pDriver->GetLocalizedInputString(di);
	else
		return Capitalize( DeviceButtonToString(di.button) );
}

wchar_t RageInput::DeviceInputToChar( DeviceInput di, bool bUseCurrentKeyModifiers )
{
	InputHandler *pDriver = GetHandlerForDevice( di.device );
	if( pDriver != nullptr )
		return pDriver->DeviceButtonToChar(di.button, bUseCurrentKeyModifiers);
	else
		return '\0';
}

InputDeviceState RageInput::GetInputDeviceState( InputDevice id )
{
	InputHandler *pDriver = GetHandlerForDevice( id );
	if( pDriver != nullptr )
		return pDriver->GetInputDeviceState(id);
	else
		return InputDeviceState_NoInputHandler;
}

RString RageInput::GetDisplayDevicesString() const
{
	std::vector<InputDeviceInfo> vDevices;
	GetDevicesAndDescriptions( vDevices );

	std::vector<RString> vs;
	for( unsigned i=0; i<vDevices.size(); ++i )
	{
		const RString &sDescription = vDevices[i].sDesc;
		InputDevice id = vDevices[i].id;
		if( sDescription == "MonkeyKeyboard" )
			continue;	// hide this
		vs.push_back( ssprintf("%s (%s)", sDescription.c_str(), InputDeviceToString(id).c_str()) );
	}
	return join("\n",vs);
}

// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to RageInput. */
class LunaRageInput: public Luna<RageInput>
{
public:
	static int GetDescriptions( T* p, lua_State *L )
	{
		std::vector<InputDeviceInfo> vDevices;
		p->GetDevicesAndDescriptions( vDevices );
		std::vector<RString> vsDescriptions;
		for (InputDeviceInfo const &idi : vDevices)
			vsDescriptions.push_back( idi.sDesc );
		LuaHelpers::CreateTableFromArray( vsDescriptions, L );
		return 1;
	}

	LunaRageInput()
	{
		ADD_METHOD( GetDescriptions );
	}
};

LUA_REGISTER_CLASS( RageInput )
// lua end


/*
 * Copyright (c) 2001-2004 Chris Danford, Glenn Maynard
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

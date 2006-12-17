#include "global.h"
#include "RageInput.h"
#include "RageLog.h"
#include "arch/InputHandler/InputHandler.h"
#include "Foreach.h"
#include "Preference.h"
#include "LuaManager.h"

RageInput*		INPUTMAN	= NULL;		// globally accessable input device

static Preference<RString> g_sInputDrivers( "InputDrivers", "" ); // "" == DEFAULT_INPUT_DRIVER_LIST

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
	/* Delete optional devices. */
	for( unsigned i = 0; i < m_pDevices.size(); ++i )
		delete m_pDevices[i];

	// Unregister with Lua.
	LUA->UnsetGlobal( "INPUTMAN" );
}

void RageInput::LoadDrivers()
{
	for( unsigned i = 0; i < m_pDevices.size(); ++i )
		delete m_pDevices[i];
	m_pDevices.clear();

	/* Init optional devices. */
	InputHandler::Create( g_sInputDrivers, m_pDevices );

	/* If no input devices are loaded, the user won't be able to input anything. */
	if( m_pDevices.size() == 0 )
		LOG->Warn( "No input devices were loaded." );
}

void RageInput::Update()
{
	/* Update optional devices. */
	for( unsigned i = 0; i < m_pDevices.size(); ++i )
		m_pDevices[i]->Update();
}

bool RageInput::DevicesChanged()
{
	/* Update optional devices. */
	for( unsigned i = 0; i < m_pDevices.size(); ++i )
	{
		if( m_pDevices[i]->DevicesChanged() )
			return true;
	}
	return false;
}

void RageInput::GetDevicesAndDescriptions( vector<InputDeviceInfo>& vDevicesOut ) const
{
	for( unsigned i = 0; i < m_pDevices.size(); ++i )
		m_pDevices[i]->GetDevicesAndDescriptions( vDevicesOut );	
}

void RageInput::WindowReset()
{
	for( unsigned i = 0; i < m_pDevices.size(); ++i )
		m_pDevices[i]->WindowReset();
}

void RageInput::AddHandler( InputHandler *pHandler )
{
	ASSERT( pHandler != NULL );
	m_pDevices.push_back( pHandler );
}

RString RageInput::GetDeviceSpecificInputString( const DeviceInput &di )
{
	FOREACH( InputHandler*, m_pDevices, i )
	{
		vector<InputDeviceInfo> vDevices;
		(*i)->GetDevicesAndDescriptions( vDevices );

		FOREACH_CONST( InputDeviceInfo, vDevices, idi )
		{
			if( idi->id == di.device )
				return (*i)->GetDeviceSpecificInputString(di);
		}
	}

	return di.ToString();
}

RString RageInput::GetLocalizedInputString( const DeviceInput &di )
{
	FOREACH( InputHandler*, m_pDevices, i )
	{
		vector<InputDeviceInfo> vDevices;
		(*i)->GetDevicesAndDescriptions( vDevices );

		FOREACH_CONST( InputDeviceInfo, vDevices, idi )
		{
			if( idi->id == di.device )
				return (*i)->GetLocalizedInputString(di);
		}
	}

	return Capitalize( DeviceButtonToString(di.button) );
}

wchar_t RageInput::DeviceInputToChar( DeviceInput di, bool bUseCurrentKeyModifiers )
{
	FOREACH( InputHandler*, m_pDevices, i )
	{
		vector<InputDeviceInfo> vDevices;
		(*i)->GetDevicesAndDescriptions( vDevices );

		FOREACH_CONST( InputDeviceInfo, vDevices, idi )
		{
			if( idi->id == di.device )
				return (*i)->DeviceButtonToChar(di.button, bUseCurrentKeyModifiers);
		}
	}

	return '\0';
}

InputDeviceState RageInput::GetInputDeviceState( InputDevice id )
{
	FOREACH( InputHandler*, m_pDevices, i )
	{
		vector<InputDeviceInfo> vDevices;
		(*i)->GetDevicesAndDescriptions( vDevices );

		FOREACH_CONST( InputDeviceInfo, vDevices, idi )
		{
			if( idi->id == id )
				return (*i)->GetInputDeviceState(id);
		}
	}

	return InputDeviceState_Invalid;
}

RString RageInput::GetDisplayDevicesString() const
{
	vector<InputDeviceInfo> vDevices;
	GetDevicesAndDescriptions( vDevices );

	vector<RString> vs;
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

class LunaRageInput: public Luna<RageInput>
{
public:
	static int GetDescriptions( T* p, lua_State *L )
	{
		vector<InputDeviceInfo> vDevices;
		p->GetDevicesAndDescriptions( vDevices );
		vector<RString> vsDescriptions;
		FOREACH_CONST( InputDeviceInfo, vDevices, idi )
			vsDescriptions.push_back( idi->sDesc );
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

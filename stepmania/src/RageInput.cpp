#include "global.h"
#include "RageInput.h"
#include "RageLog.h"
#include "RageException.h"
#include "arch/InputHandler/InputHandler.h"

#include "arch/arch.h"

RageInput*		INPUTMAN	= NULL;		// globally accessable input device

RageInput::RageInput( CString sDriverList )
{
	LOG->Trace( "RageInput::RageInput()" );

	/* Init optional devices. */
	MakeInputHandlers( sDriverList, m_pDevices );

	/* If no input devices are loaded, the user won't be able to input anything. */
	if( m_pDevices.size() == 0 )
		LOG->Warn( "No input devices were loaded." );
}

RageInput::~RageInput()
{
	/* Delete optional devices. */
	for( unsigned i = 0; i < m_pDevices.size(); ++i )
		delete m_pDevices[i];
}

void RageInput::Update( float fDeltaTime )
{
	/* Update optional devices. */
	for( unsigned i = 0; i < m_pDevices.size(); ++i )
		m_pDevices[i]->Update( fDeltaTime );
}

void RageInput::GetDevicesAndDescriptions( vector<InputDevice>& vDevicesOut, vector<CString>& vDescriptionsOut )
{
	for( unsigned i = 0; i < m_pDevices.size(); ++i )
		m_pDevices[i]->GetDevicesAndDescriptions( vDevicesOut, vDescriptionsOut );	
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


// lua start
#include "LuaBinding.h"

template<class T>
class LunaRageInput : public Luna<T>
{
public:
	LunaRageInput() { LUA->Register( Register ); }

	static int GetDescriptions( T* p, lua_State *L )
	{
		vector<InputDevice> vDevices;
		vector<CString> vsDescriptions;
		p->GetDevicesAndDescriptions( vDevices, vsDescriptions );
		LuaHelpers::CreateTableFromArray( vsDescriptions, L );
		return 1;
	}

	static void Register(lua_State *L)
	{
		ADD_METHOD( GetDescriptions )
		Luna<T>::Register( L );

		// Add global singleton if constructed already.  If it's not constructed yet,
		// then we'll register it later when we reinit Lua just before 
		// initializing the display.
		if( INPUTMAN )
		{
			lua_pushstring(L, "INPUTMAN");
			INPUTMAN->PushSelf( LUA->L );
			lua_settable(L, LUA_GLOBALSINDEX);
		}
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

#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: RageInput.h

 Desc: Wrapper for DirectInput.  Generates InputEvents.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "RageInput.h"
#include "SDL_utils.h"
#include "RageLog.h"
#include "InputFilter.h"

RageInput*		INPUTMAN	= NULL;		// globally accessable input device



RageInput::RageInput()
{
	LOG->Trace( "RageInput::RageInput()" );

	SDL_InitSubSystem( SDL_INIT_JOYSTICK );


	//
	// Init keyboard
	//
	SDL_EnableKeyRepeat( 0, 0 );

	/* If we use key events, we can do this to get Unicode values
	 * in the key struct, which (with a little more work) will make
	 * us work on international keyboards: */
	// SDL_EnableUNICODE( 1 );

	//
	// Init joysticks
	//
	int iNumJoySticks = min( SDL_NumJoysticks(), NUM_JOYSTICKS );
	LOG->Info( "Found %d joysticks", iNumJoySticks );
	for( int i=0; i<iNumJoySticks; i++ )
	{
		SDL_Joystick *pJoystick = SDL_JoystickOpen( i );
		LOG->Info( "   %d: '%s' axes: %d, hats: %d, buttons: %d",
			i,
			SDL_JoystickName(i),
			SDL_JoystickNumAxes(pJoystick),
			SDL_JoystickNumHats(pJoystick),
			SDL_JoystickNumButtons(pJoystick) );
		SDL_JoystickClose(pJoystick);
	}
	SDL_JoystickEventState( SDL_ENABLE );

	//
	// Init pump
	//
	m_Pump = new PumpPadDevice;
}

RageInput::~RageInput()
{
	//
	// De-init pump
	//
	delete m_Pump;

	SDL_QuitSubSystem( SDL_INIT_JOYSTICK );
}

void RageInput::Update( float fDeltaTime )
{
	//
	// Update Pump
	//
	m_Pump->Update();
}

bool RageInput::FeedSDLEvent(const SDL_Event &event)
{
	switch(event.type)
	{
	case SDL_KEYDOWN:
	case SDL_KEYUP:
	{
		DeviceInput di(DEVICE_KEYBOARD, event.key.keysym.sym);
		INPUTFILTER->ButtonPressed(di, event.key.state == SDL_PRESSED);
		return true;
	}

	case SDL_JOYBUTTONDOWN:
	case SDL_JOYBUTTONUP:
	{
		InputDevice i = InputDevice(DEVICE_JOY1 + event.jbutton.which);
		JoystickButton Button = JoystickButton(JOY_1 + event.jbutton.button);
		if(Button >= NUM_JOYSTICK_BUTTONS)
		{
			LOG->Warn("Ignored joystick event (button too high)");
			return true;
		}
		DeviceInput di(i, Button);
		INPUTFILTER->ButtonPressed(di, event.jbutton.state == SDL_PRESSED);
		return true;
	}
	
	case SDL_JOYAXISMOTION:
	{
		InputDevice i = InputDevice(DEVICE_JOY1 + event.jaxis.which);
		JoystickButton neg = (JoystickButton)(JOY_LEFT+2*event.jaxis.axis);
		JoystickButton pos = (JoystickButton)(JOY_RIGHT+2*event.jaxis.axis);
		INPUTFILTER->ButtonPressed(DeviceInput(i, neg), event.jaxis.value < -16000);
		INPUTFILTER->ButtonPressed(DeviceInput(i, pos), event.jaxis.value > +16000);
		return true;
	}
	
	case SDL_JOYHATMOTION:
	{
		InputDevice i = InputDevice(DEVICE_JOY1 + event.jhat.which);
		INPUTFILTER->ButtonPressed(DeviceInput(i, JOY_HAT_UP), !!(event.jhat.value & SDL_HAT_UP));
		INPUTFILTER->ButtonPressed(DeviceInput(i, JOY_HAT_DOWN), !!(event.jhat.value & JOY_HAT_DOWN));
		INPUTFILTER->ButtonPressed(DeviceInput(i, JOY_HAT_LEFT), !!(event.jhat.value & JOY_HAT_LEFT));
		INPUTFILTER->ButtonPressed(DeviceInput(i, JOY_HAT_RIGHT), !!(event.jhat.value & JOY_HAT_RIGHT));
		return true;
	}
	}
	return false;
}


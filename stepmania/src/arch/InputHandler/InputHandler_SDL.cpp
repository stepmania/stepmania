#include "global.h"

/* This input handler checks for SDL device input messages and sends them off to
 * InputFilter.  If we add mouse support, it should go here, too.
 *
 * Note that the SDL video event systems are interlinked--you can't use events
 * until video is initialized.  So, if we aren't using SDL for video, we can't
 * use SDL events at all, and so we lose input support, too.  In that case, you'll
 * need to write other input handlers for those cases and load them instead of this.
 * (Part of this is probably because, in Windows, you need a window to get input.)
 */
#include "InputHandler_SDL.h"
#include "SDL_utils.h"
#include "InputFilter.h"
#include "RageLog.h"

static const Sint8 Handled_SDL_Events[] = {
	SDL_KEYDOWN, SDL_KEYUP, SDL_JOYBUTTONDOWN, SDL_JOYBUTTONUP,
	SDL_JOYAXISMOTION, SDL_JOYHATMOTION, -1
};
static int SDL_EventMask;

InputHandler_SDL::InputHandler_SDL()
{
	SDL_InitSubSystem( SDL_INIT_JOYSTICK );

	SDL_EnableKeyRepeat( 0, 0 );

	/* We can do this to get Unicode values in the key struct, which (with
	 * a little more work) will make us work better on international keyboards.
	 * Not all archs support this well. */
	// SDL_EnableUNICODE( 1 );

	//
	// Init joysticks
	//
	int iNumJoySticks = min( SDL_NumJoysticks(), NUM_JOYSTICKS );
	LOG->Info( "Found %d joysticks", iNumJoySticks );
	int i;
	for( i=0; i<iNumJoySticks; i++ )
	{
		SDL_Joystick *pJoystick = SDL_JoystickOpen( i );

		if(pJoystick == NULL) {
			LOG->Info("   %d: '%s' Error opening: %s",
				i, SDL_JoystickName(i), SDL_GetError());
			continue;
		}

		LOG->Info( "   %d: '%s' axes: %d, hats: %d, buttons: %d",
			i,
			SDL_JoystickName(i),
			SDL_JoystickNumAxes(pJoystick),
			SDL_JoystickNumHats(pJoystick),
			SDL_JoystickNumButtons(pJoystick) );

		/* For some weird reason, we won't get any joystick events at all
		 * if we don't keep the joystick open.  (Why?  The joystick event
		 * API is completely separate from the SDL_Joystick polling API ...) */
		Joysticks.push_back(pJoystick);
	}

	for(i = 0; Handled_SDL_Events[i] != -1; ++i)
	{
		SDL_EventState(Handled_SDL_Events[i], SDL_ENABLE);
		SDL_EventMask |= SDL_EVENTMASK(Handled_SDL_Events[i]);
	}
}

InputHandler_SDL::~InputHandler_SDL()
{
	unsigned i;
	for(i = 0; i < Joysticks.size(); ++i)
		SDL_JoystickClose(Joysticks[i]);

	SDL_QuitSubSystem( SDL_INIT_JOYSTICK );

	for(i = 0; Handled_SDL_Events[i] != -1; ++i)
		SDL_EventState(Handled_SDL_Events[i], SDL_IGNORE);
}

void InputHandler_SDL::Update(float fDeltaTime)
{
	SDL_Event event;
	while(SDL_GetEvent(event, SDL_EventMask))
	{
		switch(event.type)
		{
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			{
			DeviceInput di(DEVICE_KEYBOARD, event.key.keysym.sym);
			INPUTFILTER->ButtonPressed(di, event.key.state == SDL_PRESSED);
			}
			continue;

		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
		{
			InputDevice i = InputDevice(DEVICE_JOY1 + event.jbutton.which);
			JoystickButton Button = JoystickButton(JOY_1 + event.jbutton.button);
			if(Button >= NUM_JOYSTICK_BUTTONS)
			{
				LOG->Warn("Ignored joystick event (button too high)");
				continue;
			}
			DeviceInput di(i, Button);
			INPUTFILTER->ButtonPressed(di, event.jbutton.state == SDL_PRESSED);
			continue;
		}
		
		case SDL_JOYAXISMOTION:
		{
			InputDevice i = InputDevice(DEVICE_JOY1 + event.jaxis.which);
			JoystickButton neg = (JoystickButton)(JOY_LEFT+2*event.jaxis.axis);
			JoystickButton pos = (JoystickButton)(JOY_RIGHT+2*event.jaxis.axis);
			INPUTFILTER->ButtonPressed(DeviceInput(i, neg), event.jaxis.value < -16000);
			INPUTFILTER->ButtonPressed(DeviceInput(i, pos), event.jaxis.value > +16000);
			continue;
		}
		
		case SDL_JOYHATMOTION:
		{
			InputDevice i = InputDevice(DEVICE_JOY1 + event.jhat.which);
			INPUTFILTER->ButtonPressed(DeviceInput(i, JOY_HAT_UP), !!(event.jhat.value & SDL_HAT_UP));
			INPUTFILTER->ButtonPressed(DeviceInput(i, JOY_HAT_DOWN), !!(event.jhat.value & SDL_HAT_DOWN));
			INPUTFILTER->ButtonPressed(DeviceInput(i, JOY_HAT_LEFT), !!(event.jhat.value & SDL_HAT_LEFT));
			INPUTFILTER->ButtonPressed(DeviceInput(i, JOY_HAT_RIGHT), !!(event.jhat.value & SDL_HAT_RIGHT));
			continue;
		}
		}
	}
}

/*
-----------------------------------------------------------------------------
 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Glenn Maynard
-----------------------------------------------------------------------------
*/

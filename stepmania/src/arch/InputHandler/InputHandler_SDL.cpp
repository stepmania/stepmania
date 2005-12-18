/*
 * This input handler checks for SDL device input messages and sends them off to
 * InputFilter.  If we add mouse support, it should go here, too.
 *
 * Note that the SDL video event systems are interlinked--you can't use events
 * until video is initialized.  So, if we aren't using SDL for video, we can't
 * use SDL events at all, and so we lose input support, too.  In that case, you'll
 * need to write other input handlers for those cases and load them instead of this.
 * (Part of this is probably because, in Windows, you need a window to get input.)
 */
#include "global.h"
#include "InputHandler_SDL.h"
#include "SDL_utils.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageDisplay.h"


static RageKeySym SDLSymToKeySym( SDLKey key )
{
#define KEY_INV KEY_INVALID
	const RageKeySym ASCIIKeySyms[] =
	{
		KEY_INV,	KEY_INV,	KEY_INV,	KEY_INV,	/* 0-3 */
		KEY_INV,	KEY_INV,	KEY_INV,	KEY_INV,	/* 4-7 */
		KEY_BACK,	KEY_TAB,	KEY_INV,	KEY_INV,	/* 8-11 */
		KEY_INV,	KEY_ENTER,	KEY_INV,	KEY_INV,	/* 12-15 */
		KEY_INV,	KEY_INV,	KEY_INV,	KEY_PAUSE,	/* 16-19 */
		KEY_INV,	KEY_INV,	KEY_INV,	KEY_INV,	/* 20-23 */
		KEY_INV,	KEY_INV,	KEY_INV,	KEY_ESC,	/* 24-27 */
		KEY_INV,	KEY_INV,	KEY_INV,	KEY_INV,	/* 28-31 */
		KEY_SPACE,	KEY_EXCL,	KEY_QUOTE,	KEY_HASH,	/* 32-35 */
		KEY_DOLLAR,	KEY_PERCENT,KEY_AMPER,	KEY_SQUOTE,	/* 36-39 */
		KEY_LPAREN,	KEY_RPAREN,	KEY_ASTERISK,KEY_PLUS,	/* 40-43 */
		KEY_COMMA,	KEY_HYPHEN,	KEY_PERIOD,	KEY_SLASH,	/* 44-47 */
		KEY_C0,		KEY_C1,		KEY_C2,		KEY_C3,		/* 48-51 */
		KEY_C4,		KEY_C5,		KEY_C6,		KEY_C7,		/* 52-55 */
		KEY_C8,		KEY_C9,		KEY_COLON,	KEY_SEMICOLON,	/* 56-59 */
		KEY_LANGLE,	KEY_EQUAL,	KEY_RANGLE,	KEY_QUESTION,	/* 60-63 */
		KEY_AT, KEY_CA, KEY_CB, KEY_CC, KEY_CD, KEY_CE, KEY_CF, KEY_CG, KEY_CH, /* 64-72 */
		KEY_CI, KEY_CJ, KEY_CK, KEY_CL, KEY_CM, KEY_CN, KEY_CO, KEY_CP, KEY_CQ, /* 73-81 */
		KEY_CR, KEY_CS, KEY_CT, KEY_CU, KEY_CV, KEY_CW, KEY_CX, KEY_CY, KEY_CZ, /* 82-90 */
		KEY_LBRACKET, KEY_BACKSLASH, KEY_RBRACKET, KEY_CARAT, /* 91-94 */
		KEY_UNDERSCORE, KEY_ACCENT, KEY_Ca, KEY_Cb,		/* 95-98 */
		KEY_Cc, KEY_Cd, KEY_Ce, KEY_Cf, KEY_Cg, KEY_Ch, KEY_Ci, KEY_Cj, KEY_Ck, /* 99-107 */
		KEY_Cl, KEY_Cm, KEY_Cn, KEY_Co, KEY_Cp, KEY_Cq, KEY_Cr, KEY_Cs, KEY_Ct, /* 108-116 */
		KEY_Cu, KEY_Cv, KEY_Cw, KEY_Cx, KEY_Cy, KEY_Cz, /* 117-122 */
		KEY_LBRACE, KEY_PIPE, KEY_RBRACE, KEY_INV, KEY_DEL /* 123-127 */
	};

	/* 0...127: */
	if( key < int(ARRAYSIZE(ASCIIKeySyms)) )
		return ASCIIKeySyms[key];

	/* SDLK_WORLD_0 ... SDLK_WORLD_95 to KEY_OTHER_0 ... KEY_OTHER_0 + 95 */
	if( key >= SDLK_WORLD_0 && key <= SDLK_WORLD_95 )
		return (RageKeySym) (key - SDLK_WORLD_0 + KEY_OTHER_0);

	/* SDLK_KP0 ... SDLK_KP9 to KEY_KP_C0 ... KEY_KP_C9 */
	if( key >= SDLK_KP0 && key <= SDLK_KP9 )
		return (RageKeySym) (key - SDLK_KP0 + KEY_KP_C0);
	
	switch( key )
	{
	case SDLK_KP_PERIOD:	return KEY_KP_PERIOD;
	case SDLK_KP_DIVIDE:	return KEY_KP_SLASH;
	case SDLK_KP_MULTIPLY:	return KEY_KP_ASTERISK;
	case SDLK_KP_MINUS:		return KEY_KP_HYPHEN;
	case SDLK_KP_PLUS:		return KEY_KP_PLUS;
	case SDLK_KP_EQUALS:	return KEY_KP_EQUAL;
	case SDLK_KP_ENTER:		return KEY_KP_ENTER;
	case SDLK_UP:			return KEY_UP;
	case SDLK_DOWN:			return KEY_DOWN;
	case SDLK_RIGHT:		return KEY_RIGHT;
	case SDLK_LEFT:			return KEY_LEFT;
	case SDLK_INSERT:		return KEY_INSERT;
	case SDLK_HOME:			return KEY_HOME;
	case SDLK_END:			return KEY_END;
	case SDLK_PAGEUP:		return KEY_PGUP;
	case SDLK_PAGEDOWN:		return KEY_PGDN;
	case SDLK_F1:			return KEY_F1;
	case SDLK_F2:			return KEY_F2;
	case SDLK_F3:			return KEY_F3;
	case SDLK_F4:			return KEY_F4;
	case SDLK_F5:			return KEY_F5;
	case SDLK_F6:			return KEY_F6;
	case SDLK_F7:			return KEY_F7;
	case SDLK_F8:			return KEY_F8;
	case SDLK_F9:			return KEY_F9;
	case SDLK_F10:			return KEY_F10;
	case SDLK_F11:			return KEY_F11;
	case SDLK_F12:			return KEY_F12;
	case SDLK_F13:			return KEY_F13;
	case SDLK_F14:			return KEY_F14;
	case SDLK_F15:			return KEY_F15;

	case SDLK_NUMLOCK:		return KEY_NUMLOCK;
	case SDLK_CAPSLOCK:		return KEY_CAPSLOCK;
	case SDLK_SCROLLOCK:	return KEY_SCRLLOCK;
	case SDLK_SYSREQ:		return KEY_PRTSC;
	case SDLK_PRINT:		return KEY_PRTSC;
	case SDLK_RSHIFT:		return KEY_RSHIFT;
	case SDLK_LSHIFT:		return KEY_LSHIFT;
	case SDLK_RCTRL:		return KEY_RCTRL;
	case SDLK_LCTRL:		return KEY_LCTRL;
	case SDLK_RALT:			return KEY_RALT;
	case SDLK_LALT:			return KEY_LALT;
	case SDLK_RMETA:		return KEY_RMETA;
	case SDLK_LMETA:		return KEY_LMETA;
	case SDLK_LSUPER:		return KEY_LSUPER;
	case SDLK_RSUPER:		return KEY_RSUPER;
	case SDLK_MENU:			return KEY_MENU;
	}

	/* Map anything else to key+KEY_OTHER_0+95. */
	if( key+KEY_OTHER_0+95 < KEY_LAST_OTHER )
		return (RageKeySym) (key+KEY_OTHER_0+95);

	return KEY_INVALID;
}

static const Sint8 Handled_SDL_Events[] = {
	SDL_KEYDOWN, SDL_KEYUP, SDL_JOYBUTTONDOWN, SDL_JOYBUTTONUP,
	SDL_JOYAXISMOTION, SDL_JOYHATMOTION, -1
};
static int SDL_EventMask;

InputHandler_SDL::InputHandler_SDL()
{
	SDL_InitSubSystem( SDL_INIT_JOYSTICK );

#ifdef _XBOX
	//strange hardware timing issue with 3rd party controllers
	Sleep(750);
#endif

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
		mySDL_EventState(Handled_SDL_Events[i], SDL_ENABLE);
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
		mySDL_EventState(Handled_SDL_Events[i], SDL_IGNORE);
}

void InputHandler_SDL::Update()
{
	SDL_Event event;
	while(SDL_GetEvent(event, SDL_EventMask))
	{
		switch(event.type)
		{
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			{
			LOG->Trace("key: sym %i, key %i, state %i",
				event.key.keysym.sym, SDLSymToKeySym(event.key.keysym.sym), event.key.state );
			DeviceInput di( DEVICE_KEYBOARD, SDLSymToKeySym(event.key.keysym.sym) );
			ButtonPressed(di, event.key.state == SDL_PRESSED);
			}
			continue;

		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
		{
			InputDevice i = InputDevice(DEVICE_JOY1 + event.jbutton.which);
			JoystickButton Button = JoystickButton(JOY_BUTTON_1 + event.jbutton.button);
			if(Button >= NUM_JOYSTICK_BUTTONS)
			{
				LOG->Warn("Ignored joystick event (button too high)");
				continue;
			}
			DeviceInput di(i, Button);
			ButtonPressed(di, event.jbutton.state == SDL_PRESSED);
			continue;
		}
		
		case SDL_JOYAXISMOTION:
		{
			InputDevice i = InputDevice(DEVICE_JOY1 + event.jaxis.which);
			JoystickButton neg = (JoystickButton)(JOY_LEFT+2*event.jaxis.axis);
			JoystickButton pos = (JoystickButton)(JOY_RIGHT+2*event.jaxis.axis);
			float l = SCALE( event.jaxis.value, 0.0f, 32768.0f, 0.0f, 1.0f );
			ButtonPressed(DeviceInput(i, neg,max(-l,0),RageZeroTimer), event.jaxis.value < -16000);
			ButtonPressed(DeviceInput(i, pos,max(+l,0),RageZeroTimer), event.jaxis.value > +16000);
			continue;
		}
		
		case SDL_JOYHATMOTION:
		{
			InputDevice i = InputDevice(DEVICE_JOY1 + event.jhat.which);
			ButtonPressed(DeviceInput(i, JOY_HAT_UP), !!(event.jhat.value & SDL_HAT_UP));
			ButtonPressed(DeviceInput(i, JOY_HAT_DOWN), !!(event.jhat.value & SDL_HAT_DOWN));
			ButtonPressed(DeviceInput(i, JOY_HAT_LEFT), !!(event.jhat.value & SDL_HAT_LEFT));
			ButtonPressed(DeviceInput(i, JOY_HAT_RIGHT), !!(event.jhat.value & SDL_HAT_RIGHT));
			continue;
		}
		}
	}

	InputHandler::UpdateTimer();
}


void InputHandler_SDL::GetDevicesAndDescriptions(vector<InputDevice>& vDevicesOut, vector<CString>& vDescriptionsOut)
{
	vDevicesOut.push_back( DEVICE_KEYBOARD );
	vDescriptionsOut.push_back( "Keyboard" );

	for( int i=0; i<SDL_NumJoysticks(); i++ )
	{
		if( SDL_JoystickOpened(i) )
		{
			vDevicesOut.push_back( InputDevice(DEVICE_JOY1+i) );
			vDescriptionsOut.push_back( SDL_JoystickName(i) );
		}
	}
}

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
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

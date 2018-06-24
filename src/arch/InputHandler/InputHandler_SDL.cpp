#include "global.h"
#include "InputHandler_SDL.h"

#include "RageUtil.h"
#include "RageLog.h"
#include "RageDisplay.h"
#include "InputFilter.h"

#include "arch/ArchHooks/ArchHooks.h"

#include "SDL.h"
#include "SDL_joystick.h"

bool has_sdl_input = false;

REGISTER_INPUT_HANDLER_CLASS2(SDL, SDL);

InputHandler_SDL::InputHandler_SDL()
{
	if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) != 0)
	{
		RageException::Throw("SDL Error in %s: %s", __FUNCTION__, SDL_GetError());
	}

	int joystickCount = SDL_NumJoysticks();
	LOG->Info("Found %d joysticks", joystickCount);
	for (int i=0; i < joystickCount; i++)
	{
		RegisterJoystick(i);
	}
	has_sdl_input = true;
}

InputHandler_SDL::~InputHandler_SDL()
{
	has_sdl_input = false;
	for (unsigned i = 0; i < m_joysticks.size(); ++i)
	{
		SDL_JoystickClose(m_joysticks[i]);
	}

	SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
}

bool InputHandler_SDL::RegisterJoystick(int i) {
	SDL_Joystick* joystick = SDL_JoystickOpen(i);

	if (joystick == NULL)
	{
		LOG->Info("   %d: error opening: %s", i, SDL_GetError());
		return false;
	}

	if (std::find(m_joysticks.begin(), m_joysticks.end(), joystick) != m_joysticks.end())
	{
		SDL_JoystickClose(joystick);
		return false;
	}

	LOG->Info("   %d: '%s' axes: %d, hats: %d, buttons: %d",
		i,
		SDL_JoystickName(joystick),
		SDL_JoystickNumAxes(joystick),
		SDL_JoystickNumHats(joystick),
		SDL_JoystickNumButtons(joystick) );

	m_joysticks.push_back(joystick);
}

static DeviceButton SDLKeycodeToDeviceButton(SDL_Keycode key)
{
#define KEY_INV DeviceButton_Invalid
	const DeviceButton ASCIIKeySyms[] =
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
		KEY_DOLLAR,	KEY_PERCENT,	KEY_AMPER,	KEY_SQUOTE,	/* 36-39 */
		KEY_LPAREN,	KEY_RPAREN,	KEY_ASTERISK,	KEY_PLUS,	/* 40-43 */
		KEY_COMMA,	KEY_HYPHEN,	KEY_PERIOD,	KEY_SLASH,	/* 44-47 */
		KEY_C0,		KEY_C1,		KEY_C2,		KEY_C3,		/* 48-51 */
		KEY_C4,		KEY_C5,		KEY_C6,		KEY_C7,		/* 52-55 */
		KEY_C8,		KEY_C9,		KEY_COLON,	KEY_SEMICOLON,	/* 56-59 */
		KEY_LANGLE,	KEY_EQUAL,	KEY_RANGLE,	KEY_QUESTION,	/* 60-63 */
		KEY_AT, KEY_CA, KEY_CB, KEY_CC, KEY_CD, KEY_CE, KEY_CF, KEY_CG, KEY_CH, /* 64-72 */
		KEY_CI, KEY_CJ, KEY_CK, KEY_CL, KEY_CM, KEY_CN, KEY_CO, KEY_CP, KEY_CQ, /* 73-81 */
		KEY_CR, KEY_CS, KEY_CT, KEY_CU, KEY_CV, KEY_CW, KEY_CX, KEY_CY, KEY_CZ, /* 82-90 */
		KEY_LBRACKET, 	KEY_BACKSLASH, KEY_RBRACKET, KEY_CARAT, /* 91-94 */
		KEY_UNDERSCORE, KEY_ACCENT, KEY_Ca, KEY_Cb,		/* 95-98 */
		KEY_Cc, KEY_Cd, KEY_Ce, KEY_Cf, KEY_Cg, KEY_Ch, KEY_Ci, KEY_Cj, KEY_Ck, /* 99-107 */
		KEY_Cl, KEY_Cm, KEY_Cn, KEY_Co, KEY_Cp, KEY_Cq, KEY_Cr, KEY_Cs, KEY_Ct, /* 108-116 */
		KEY_Cu, KEY_Cv, KEY_Cw, KEY_Cx, KEY_Cy, KEY_Cz, /* 117-122 */
		KEY_LBRACE, KEY_PIPE, KEY_RBRACE, KEY_INV, KEY_DEL /* 123-127 */
	};

	/* 0...127: */
	if (key < int(ARRAYLEN(ASCIIKeySyms)))
	{
		return ASCIIKeySyms[key];
	}

	/* SDLK_KP0 ... SDLK_KP9 to KEY_KP_C0 ... KEY_KP_C9 */
	if (key >= SDLK_KP_0 && key <= SDLK_KP_9)
	{
		return enum_add2(KEY_KP_C0, key - SDLK_KP_0);
	}

	switch (key)
	{
	case SDLK_KP_PERIOD:		return KEY_KP_PERIOD;
	case SDLK_KP_DIVIDE:		return KEY_KP_SLASH;
	case SDLK_KP_MULTIPLY:		return KEY_KP_ASTERISK;
	case SDLK_KP_MINUS:		return KEY_KP_HYPHEN;
	case SDLK_KP_PLUS:		return KEY_KP_PLUS;
	case SDLK_KP_EQUALS:		return KEY_KP_EQUAL;
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

	case SDLK_NUMLOCKCLEAR:		return KEY_NUMLOCK;
	case SDLK_CAPSLOCK:		return KEY_CAPSLOCK;
	case SDLK_SCROLLLOCK:		return KEY_SCRLLOCK;
	case SDLK_SYSREQ:		return KEY_PRTSC;
	case SDLK_PRINTSCREEN:		return KEY_PRTSC;
	case SDLK_RSHIFT:		return KEY_RSHIFT;
	case SDLK_LSHIFT:		return KEY_LSHIFT;
	case SDLK_RCTRL:		return KEY_RCTRL;
	case SDLK_LCTRL:		return KEY_LCTRL;
	case SDLK_RALT:			return KEY_RALT;
	case SDLK_LALT:			return KEY_LALT;
	case SDLK_MENU:			return KEY_MENU;
	}

	/* Map anything else to key+KEY_OTHER_0+95. */
	if (key+KEY_OTHER_0+95 < KEY_LAST_OTHER)
	{
		return enum_add2(KEY_OTHER_0, key+95);
	}

	return DeviceButton_Invalid;
}

static DeviceButton SDLMouseButtonToDeviceButton(int button)
{
	switch (button)
	{
		case SDL_BUTTON_LEFT: return MOUSE_LEFT;
		case SDL_BUTTON_MIDDLE: return MOUSE_MIDDLE;
		case SDL_BUTTON_RIGHT: return MOUSE_RIGHT;
	}
	return DeviceButton_Invalid;
}

// on linux SDL appears to restart SDL_JoystickIDs from 0 when the subsystem is shutdown.
// some other platforms, however, appear to continue counting. Be safe and use the index in our array
static InputDevice SDLJoystickIDToInputDevice(std::vector<SDL_Joystick *> joysticks, SDL_JoystickID joystickId)
{
	SDL_Joystick* joystick = SDL_JoystickFromInstanceID(joystickId);
	if (joystick == NULL)
	{
		return InputDevice_Invalid;
	}

	auto it = std::find(joysticks.begin(), joysticks.end(), joystick);
	if (it == joysticks.end())
	{
		return InputDevice_Invalid;
	}

	return enum_add2(DEVICE_JOY1, it - joysticks.begin());
}

void InputHandler_SDL::Update()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
		{
			LOG->Trace("SDL_QUIT: shutting down");
			ArchHooks::SetUserQuit();
			continue;
		}

		case SDL_KEYDOWN:
		case SDL_KEYUP:
		{
			if (event.key.repeat) continue;
			DeviceButton db = SDLKeycodeToDeviceButton(event.key.keysym.sym);
			DeviceInput di(DEVICE_KEYBOARD, db, event.key.state == SDL_PRESSED);
			ButtonPressed(di);
			continue;
		}

		case SDL_MOUSEMOTION:
		{
			INPUTFILTER->UpdateCursorLocation(event.motion.x, event.motion.y);
			continue;
		}

		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
		{
			InputDevice device = InputDevice(DEVICE_MOUSE);
			DeviceButton db = SDLMouseButtonToDeviceButton(event.button.button);
			ButtonPressed(DeviceInput(device, db, event.button.state == SDL_PRESSED));
			continue;
		}

		case SDL_MOUSEWHEEL:
		{
			INPUTFILTER->UpdateMouseWheel(INPUTFILTER->GetMouseWheel() + event.wheel.y);
			if (event.wheel.y != 0) {
				DeviceButton db = event.wheel.y > 0 ? MOUSE_WHEELUP : MOUSE_WHEELDOWN;
				ButtonPressed(DeviceInput(DEVICE_MOUSE, db, 1));
				ButtonPressed(DeviceInput(DEVICE_MOUSE, db, 0));
			}
			continue;
		}

		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
		{
			InputDevice device = SDLJoystickIDToInputDevice(m_joysticks, event.jbutton.which);
			DeviceButton button = enum_add2(JOY_BUTTON_1, event.jbutton.button);
			if (button > JOY_BUTTON_32)
			{
				LOG->Warn("Ignored joystick event (button too high)");
				continue;
			}
			DeviceInput di(device, button, event.jbutton.state == SDL_PRESSED);
			ButtonPressed(di);
			continue;
		}

		case SDL_JOYDEVICEADDED:
		{
			LOG->Info("Joy device added");
			// this event can happen when we (re)init the subsystem, so only report changed
			// if the joystick is not already opened to avoid looping endlessly
			if (RegisterJoystick(event.jdevice.which))
			{
				m_devices_changed = true;
			}
			break;
		}

		case SDL_JOYAXISMOTION:
		{
			InputDevice device = SDLJoystickIDToInputDevice(m_joysticks, event.jbutton.which);
			DeviceButton neg = enum_add2(JOY_LEFT, 2*event.jaxis.axis);
			DeviceButton pos = enum_add2(JOY_RIGHT, 2*event.jaxis.axis);
			auto l = Rage::scale((int)event.jaxis.value, 0, 32768, 0, 1);
			ButtonPressed(DeviceInput(device, neg, std::max(-l, 0)));
			ButtonPressed(DeviceInput(device, pos, std::max(+l, 0)));
			continue;
		}

		case SDL_JOYHATMOTION:
		{
			InputDevice device = SDLJoystickIDToInputDevice(m_joysticks, event.jbutton.which);
			ButtonPressed(DeviceInput(device, JOY_HAT_UP, !!(event.jhat.value & SDL_HAT_UP)));
			ButtonPressed(DeviceInput(device, JOY_HAT_DOWN, !!(event.jhat.value & SDL_HAT_DOWN)));
			ButtonPressed(DeviceInput(device, JOY_HAT_LEFT, !!(event.jhat.value & SDL_HAT_LEFT)));
			ButtonPressed(DeviceInput(device, JOY_HAT_RIGHT, !!(event.jhat.value & SDL_HAT_RIGHT)));
			continue;
		}

		case SDL_WINDOWEVENT_FOCUS_LOST:
		{
			INPUTFILTER->Reset();
			continue;
		}

		default:
			LOG->Trace("Unhandled sdl event %lu", (unsigned long)event.type);
			continue;
		}
	}

	InputHandler::UpdateTimer();
}


void InputHandler_SDL::GetDevicesAndDescriptions(std::vector<InputDeviceInfo>& vDevicesOut)
{
	vDevicesOut.push_back(InputDeviceInfo(DEVICE_KEYBOARD, "Keyboard"));
	vDevicesOut.push_back(InputDeviceInfo(DEVICE_MOUSE, "Mouse"));

	for (int i = 0; i < m_joysticks.size(); i++)
	{
		SDL_Joystick* joystick = m_joysticks.at(i);
		auto name = SDL_JoystickName(joystick);
		vDevicesOut.push_back(InputDeviceInfo(InputDevice(DEVICE_JOY1 + i), name != NULL ? name : ""));
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

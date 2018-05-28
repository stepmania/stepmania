/* RageInputDevice - User input types. */
#ifndef RAGE_INPUT_DEVICE_H
#define RAGE_INPUT_DEVICE_H

#include "RageTimer.h"
#include "EnumHelper.h"

const int NUM_JOYSTICKS = 32;
const int NUM_PUMPS = 2;

enum InputDevice
{
	DEVICE_KEYBOARD = 0,
	DEVICE_JOY1,
	DEVICE_JOY2,
	DEVICE_JOY3,
	DEVICE_JOY4,
	DEVICE_JOY5,
	DEVICE_JOY6,
	DEVICE_JOY7,
	DEVICE_JOY8,
	DEVICE_JOY9,
	DEVICE_JOY10,
	DEVICE_JOY11,
	DEVICE_JOY12,
	DEVICE_JOY13,
	DEVICE_JOY14,
	DEVICE_JOY15,
	DEVICE_JOY16,
	DEVICE_JOY17,
	DEVICE_JOY18,
	DEVICE_JOY19,
	DEVICE_JOY20,
	DEVICE_JOY21,
	DEVICE_JOY22,
	DEVICE_JOY23,
	DEVICE_JOY24,
	DEVICE_JOY25,
	DEVICE_JOY26,
	DEVICE_JOY27,
	DEVICE_JOY28,
	DEVICE_JOY29,
	DEVICE_JOY30,
	DEVICE_JOY31,
	DEVICE_JOY32,
	DEVICE_PUMP1,
	DEVICE_PUMP2,
	DEVICE_MIDI,
	DEVICE_MOUSE,
	DEVICE_PIUIO,
	NUM_InputDevice,		// leave this at the end
	InputDevice_Invalid		// means this is NULL
};
/** @brief A special foreach loop for each input device. */
#define FOREACH_InputDevice( i ) FOREACH_ENUM( InputDevice, i )
const RString& InputDeviceToString( InputDevice i );
InputDevice StringToInputDevice( const RString& s );
inline bool IsJoystick( InputDevice id ) { return DEVICE_JOY1 <= id && id < DEVICE_JOY1+NUM_JOYSTICKS; }
inline bool IsPump( InputDevice id ) { return DEVICE_PUMP1 <= id && id < DEVICE_PUMP1+NUM_PUMPS; }
inline bool IsMouse( InputDevice id ) { return id == DEVICE_MOUSE; }

struct InputDeviceInfo
{
	InputDeviceInfo( InputDevice id_, RString sDesc_ ):
		id(id_), sDesc(sDesc_) {}
	
	InputDevice id;
	RString sDesc;
};

inline bool operator==(InputDeviceInfo const &lhs, InputDeviceInfo const &rhs)
{
	return lhs.id == rhs.id && lhs.sDesc == rhs.sDesc;
}

inline bool operator!=(InputDeviceInfo const &lhs, InputDeviceInfo const &rhs)
{
	return !operator==(lhs, rhs);
}

enum InputDeviceState
{
	InputDeviceState_Connected,	// has an InputHandler and controller is plugged in
	InputDeviceState_Unplugged,	// has an InputHandler but controller is unplugged or lost wireless link
	InputDeviceState_NeedsMultitap,	// has an InputHandler but needs a multitap to function
	InputDeviceState_NoInputHandler,	// there is no InputHandler that implements this InputDevice
	NUM_InputDeviceState,
	InputDeviceState_Invalid
};
const RString& InputDeviceStateToString( InputDeviceState ids );

/* Only raw, unshifted keys go in this table; this doesn't include
 * internationalized keyboards, only keys that we might actually want to test
 * for programmatically. Any other keys are mapped to DB_KEY_OTHER_0 and up. (If we
 * want to support real international input, stick a wchar_t in DeviceInput.)  */
 
enum DeviceButton
{
	DB_KEY_SPACE	= 32,
	DB_KEY_EXCL	= 33,
	DB_KEY_QUOTE	= 34,
	DB_KEY_HASH	= 35,
	DB_KEY_DOLLAR	= 36,
	DB_KEY_PERCENT = 37,
	DB_KEY_AMPER	= 38,
	DB_KEY_SQUOTE	= 39,
	DB_KEY_LPAREN	= 40,
	DB_KEY_RPAREN	= 41,
	DB_KEY_ASTERISK= 42,
	DB_KEY_PLUS	= 43,
	DB_KEY_COMMA	= 44,
	DB_KEY_HYPHEN	= 45,
	DB_KEY_PERIOD	= 46,
	DB_KEY_SLASH	= 47,
	DB_KEY_C0		= 48,
	DB_KEY_C1		= 49,
	DB_KEY_C2		= 50,
	DB_KEY_C3		= 51,
	DB_KEY_C4		= 52,
	DB_KEY_C5		= 53,
	DB_KEY_C6		= 54,
	DB_KEY_C7		= 55,
	DB_KEY_C8		= 56,
	DB_KEY_C9		= 57,
	DB_KEY_COLON	= 58,
	DB_KEY_SEMICOLON = 59,
	DB_KEY_LANGLE	= 60,
	DB_KEY_EQUAL	= 61,
	DB_KEY_RANGLE	= 62,
	DB_KEY_QUESTION= 63,
	DB_KEY_AT		= 64,
	DB_KEY_CA		= 65,
	DB_KEY_CB		= 66,
	DB_KEY_CC		= 67,
	DB_KEY_CD		= 68,
	DB_KEY_CE		= 69,
	DB_KEY_CF		= 70,
	DB_KEY_CG		= 71,
	DB_KEY_CH		= 72,
	DB_KEY_CI		= 73,
	DB_KEY_CJ		= 74,
	DB_KEY_CK		= 75,
	DB_KEY_CL		= 76,
	DB_KEY_CM		= 77,
	DB_KEY_CN		= 78,
	DB_KEY_CO		= 79,
	DB_KEY_CP		= 80,
	DB_KEY_CQ		= 81,
	DB_KEY_CR		= 82,
	DB_KEY_CS		= 83,
	DB_KEY_CT		= 84,
	DB_KEY_CU		= 85,
	DB_KEY_CV		= 86,
	DB_KEY_CW		= 87,
	DB_KEY_CX		= 88,
	DB_KEY_CY		= 89,
	DB_KEY_CZ		= 90,
	DB_KEY_LBRACKET= 91,
	DB_KEY_BACKSLASH= 92,
	DB_KEY_RBRACKET= 93,
	DB_KEY_CARAT	= 94,
	DB_KEY_UNDERSCORE= 95,
	DB_KEY_ACCENT	= 96,
	DB_KEY_Ca		= 97,
	DB_KEY_Cb		= 98,
	DB_KEY_Cc		= 99,
	DB_KEY_Cd		= 100,
	DB_KEY_Ce		= 101,
	DB_KEY_Cf		= 102,
	DB_KEY_Cg		= 103,
	DB_KEY_Ch		= 104,
	DB_KEY_Ci		= 105,
	DB_KEY_Cj		= 106,
	DB_KEY_Ck		= 107,
	DB_KEY_Cl		= 108,
	DB_KEY_Cm		= 109,
	DB_KEY_Cn		= 110,
	DB_KEY_Co		= 111,
	DB_KEY_Cp		= 112,
	DB_KEY_Cq		= 113,
	DB_KEY_Cr		= 114,
	DB_KEY_Cs		= 115,
	DB_KEY_Ct		= 116,
	DB_KEY_Cu		= 117,
	DB_KEY_Cv		= 118,
	DB_KEY_Cw		= 119,
	DB_KEY_Cx		= 120,
	DB_KEY_Cy		= 121,
	DB_KEY_Cz		= 122,
	DB_KEY_LBRACE	= 123,
	DB_KEY_PIPE	= 124,
	DB_KEY_RBRACE	= 125,
	DB_KEY_DEL		= 127,

	DB_KEY_BACK,
	DB_KEY_TAB,
	DB_KEY_ENTER,
	DB_KEY_PAUSE,
	DB_KEY_ESC,

	DB_KEY_F1,
	DB_KEY_F2,
	DB_KEY_F3,
	DB_KEY_F4,
	DB_KEY_F5,
	DB_KEY_F6,
	DB_KEY_F7,
	DB_KEY_F8,
	DB_KEY_F9,
	DB_KEY_F10,
	DB_KEY_F11,
	DB_KEY_F12,
	DB_KEY_F13,
	DB_KEY_F14,
	DB_KEY_F15,
	DB_KEY_F16,

	DB_KEY_LCTRL,
	DB_KEY_RCTRL,
	DB_KEY_LSHIFT,
	DB_KEY_RSHIFT,
	DB_KEY_LALT,
	DB_KEY_RALT,
	DB_KEY_LMETA,
	DB_KEY_RMETA,
	DB_KEY_LSUPER,
	DB_KEY_RSUPER,
	DB_KEY_MENU,
	
	DB_KEY_FN, // Laptop function keys.

	DB_KEY_NUMLOCK,
	DB_KEY_SCRLLOCK,
	DB_KEY_CAPSLOCK,
	DB_KEY_PRTSC,

	DB_KEY_UP,
	DB_KEY_DOWN,
	DB_KEY_LEFT,
	DB_KEY_RIGHT,

	DB_KEY_INSERT,
	DB_KEY_HOME,
	DB_KEY_END,
	DB_KEY_PGUP,
	DB_KEY_PGDN,

	DB_KEY_KP_C0,
	DB_KEY_KP_C1,
	DB_KEY_KP_C2,
	DB_KEY_KP_C3,
	DB_KEY_KP_C4,
	DB_KEY_KP_C5,
	DB_KEY_KP_C6,
	DB_KEY_KP_C7,
	DB_KEY_KP_C8,
	DB_KEY_KP_C9,
	DB_KEY_KP_SLASH,
	DB_KEY_KP_ASTERISK,
	DB_KEY_KP_HYPHEN,
	DB_KEY_KP_PLUS,
	DB_KEY_KP_PERIOD,
	DB_KEY_KP_EQUAL,
	DB_KEY_KP_ENTER,

	DB_KEY_OTHER_0,
	// ...
	DB_KEY_LAST_OTHER=511,

	/* Joystick inputs. We try to have enough input names so any input on a
	 * reasonable joystick has an obvious mapping, but keep it generic and don't
	 * try to handle odd special cases. For example, many controllers have two
	 * sticks, so the JOY_LEFT_2, etc. pairs are useful for many types of sticks. */
	// Standard axis:
	JOY_LEFT, JOY_RIGHT, JOY_UP, JOY_DOWN,

	// Secondary sticks:
	JOY_LEFT_2, JOY_RIGHT_2, JOY_UP_2, JOY_DOWN_2,

	JOY_Z_UP, JOY_Z_DOWN,
	JOY_ROT_UP, JOY_ROT_DOWN, JOY_ROT_LEFT, JOY_ROT_RIGHT, JOY_ROT_Z_UP, JOY_ROT_Z_DOWN,
	JOY_HAT_LEFT, JOY_HAT_RIGHT, JOY_HAT_UP, JOY_HAT_DOWN, 
	JOY_AUX_1, JOY_AUX_2, JOY_AUX_3, JOY_AUX_4,

	// Buttons:
	JOY_BUTTON_1,	JOY_BUTTON_2,	JOY_BUTTON_3,	JOY_BUTTON_4,	JOY_BUTTON_5,
	JOY_BUTTON_6,	JOY_BUTTON_7,	JOY_BUTTON_8,	JOY_BUTTON_9,	JOY_BUTTON_10,
	JOY_BUTTON_11,	JOY_BUTTON_12,	JOY_BUTTON_13,	JOY_BUTTON_14,	JOY_BUTTON_15,
	JOY_BUTTON_16,	JOY_BUTTON_17,	JOY_BUTTON_18,	JOY_BUTTON_19,	JOY_BUTTON_20,
	JOY_BUTTON_21,	JOY_BUTTON_22,	JOY_BUTTON_23,	JOY_BUTTON_24,	JOY_BUTTON_25,
	JOY_BUTTON_26,	JOY_BUTTON_27,	JOY_BUTTON_28,	JOY_BUTTON_29,	JOY_BUTTON_30,
	JOY_BUTTON_31,	JOY_BUTTON_32,

	MIDI_FIRST = 600,
	MIDI_LAST = 699,

	// Mouse buttons
	MOUSE_LEFT = 700,
	MOUSE_RIGHT,
	MOUSE_MIDDLE,
	// todo: button4/5
	// axis
	MOUSE_X_LEFT, MOUSE_X_RIGHT,
	MOUSE_Y_UP, MOUSE_Y_DOWN,
	// Mouse wheel (z axis)
	MOUSE_WHEELUP, MOUSE_WHEELDOWN,

	NUM_DeviceButton,
	DeviceButton_Invalid
};

RString DeviceButtonToString( DeviceButton i );
DeviceButton StringToDeviceButton( const RString& s );

struct DeviceInput
{
public:
	InputDevice device;
	DeviceButton button;

	/* This is usually 0 or 1. Analog joystick inputs can set this to a percentage
	 * (0..1). This should be 0 for analog axes within the dead zone. */
	float level;

	// Mouse coordinates
	//unsigned x;
	//unsigned y;
	int z; // mousewheel

	/* Whether this button is pressed. This is level with a threshold and
	 * debouncing applied. */
	bool bDown;

	RageTimer ts;

	DeviceInput(): device(InputDevice_Invalid), button(DeviceButton_Invalid), level(0), z(0), bDown(false), ts(RageZeroTimer) { }
	DeviceInput( InputDevice d, DeviceButton b, float l=0 ): device(d), button(b), level(l), z(0), bDown(l > 0.5f), ts(RageZeroTimer) { }
	DeviceInput( InputDevice d, DeviceButton b, float l, const RageTimer &t ):
		device(d), button(b), level(l), z(0), bDown(level > 0.5f), ts(t) { }
	DeviceInput( InputDevice d, DeviceButton b, const RageTimer &t, int zVal=0 ):
		device(d), button(b), level(0), z(zVal), bDown(false), ts(t) { }

	RString ToString() const;
	bool FromString( const RString &s );

	bool IsValid() const { return device != InputDevice_Invalid; };
	void MakeInvalid() { device = InputDevice_Invalid; };

	bool IsJoystick() const { return ::IsJoystick(device); }
	bool IsMouse() const { return ::IsMouse(device); }
};

inline bool operator==(DeviceInput const &lhs, DeviceInput const &rhs)
{
	/* Return true if we represent the same button on the same device.
	 * Don't compare level or ts. */
	return lhs.device == rhs.device &&
		lhs.button == rhs.button;
}
inline bool operator!=(DeviceInput const &lhs, DeviceInput const &rhs)
{
	return !operator==(lhs, rhs);
}

inline bool operator<(DeviceInput const &lhs, DeviceInput const &rhs)
{
	/* Only the devices and buttons matter here. */
	if ( lhs.device != rhs.device)
	{
		return lhs.device < rhs.device;
	}
	return lhs.button < rhs.button;
}
inline bool operator>(DeviceInput const &lhs, DeviceInput const &rhs)
{
	return operator<(rhs, lhs);
}
inline bool operator<=(DeviceInput const &lhs, DeviceInput const &rhs)
{
	return !operator<(rhs, lhs);
}
inline bool operator>=(DeviceInput const &lhs, DeviceInput const &rhs)
{
	return !operator<(lhs, rhs);
}

typedef vector<DeviceInput> DeviceInputList;

#endif
/*
 * Copyright (c) 2001-2002 Chris Danford
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

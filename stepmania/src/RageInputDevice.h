#ifndef RAGEINPUTDEVICE_H
#define RAGEINPUTDEVICE_H 1

// #include "SDL_keysym.h"
#include "RageTimer.h"

/* Only raw, unshifted keys go in this table; this doesn't include internationalized
 * keyboards, only keys that we might actually want to test for programmatically.  Any
 * other keys are mapped to KEY_OTHER_0 and up.  (If we want to support real international
 * input, stick a wchar_t in DeviceInput.)  */
 
enum RageKeySym
{
	KEY_SPACE	= 32,
	KEY_EXCL	= 33,
	KEY_QUOTE	= 34,
	KEY_HASH	= 35,
	KEY_DOLLAR	= 36,
	KEY_PERCENT = 37,
	KEY_AMPER	= 38,
	KEY_SQUOTE	= 39,
	KEY_LPAREN	= 40,
	KEY_RPAREN	= 41,
	KEY_ASTERISK= 42,
	KEY_PLUS	= 43,
	KEY_COMMA	= 44,
	KEY_HYPHEN	= 45,
	KEY_PERIOD	= 46,
	KEY_SLASH	= 47,
	KEY_C0		= 48,
	KEY_C1		= 49,
	KEY_C2		= 50,
	KEY_C3		= 51,
	KEY_C4		= 52,
	KEY_C5		= 53,
	KEY_C6		= 54,
	KEY_C7		= 55,
	KEY_C8		= 56,
	KEY_C9		= 57,
	KEY_COLON	= 58,
	KEY_SEMICOLON = 59,
	KEY_LANGLE	= 60,
	KEY_EQUAL	= 61,
	KEY_RANGLE	= 62,
	KEY_QUESTION= 63,
	KEY_AT		= 64,
	KEY_CA		= 65,
	KEY_CB		= 66,
	KEY_CC		= 67,
	KEY_CD		= 68,
	KEY_CE		= 69,
	KEY_CF		= 70,
	KEY_CG		= 71,
	KEY_CH		= 72,
	KEY_CI		= 73,
	KEY_CJ		= 74,
	KEY_CK		= 75,
	KEY_CL		= 76,
	KEY_CM		= 77,
	KEY_CN		= 78,
	KEY_CO		= 79,
	KEY_CP		= 80,
	KEY_CQ		= 81,
	KEY_CR		= 82,
	KEY_CS		= 83,
	KEY_CT		= 84,
	KEY_CU		= 85,
	KEY_CV		= 86,
	KEY_CW		= 87,
	KEY_CX		= 88,
	KEY_CY		= 89,
	KEY_CZ		= 90,
	KEY_LBRACKET= 91,
	KEY_BACKSLASH= 92,
	KEY_RBRACKET= 93,
	KEY_CARAT	= 94,
	KEY_UNDERSCORE= 95,
	KEY_ACCENT	= 96,
	KEY_Ca		= 97,
	KEY_Cb		= 98,
	KEY_Cc		= 99,
	KEY_Cd		= 100,
	KEY_Ce		= 101,
	KEY_Cf		= 102,
	KEY_Cg		= 103,
	KEY_Ch		= 104,
	KEY_Ci		= 105,
	KEY_Cj		= 106,
	KEY_Ck		= 107,
	KEY_Cl		= 108,
	KEY_Cm		= 109,
	KEY_Cn		= 110,
	KEY_Co		= 111,
	KEY_Cp		= 112,
	KEY_Cq		= 113,
	KEY_Cr		= 114,
	KEY_Cs		= 115,
	KEY_Ct		= 116,
	KEY_Cu		= 117,
	KEY_Cv		= 118,
	KEY_Cw		= 119,
	KEY_Cx		= 120,
	KEY_Cy		= 121,
	KEY_Cz		= 122,
	KEY_LBRACE	= 123,
	KEY_PIPE	= 124,
	KEY_RBRACE	= 125,
	KEY_DEL		= 127,

	KEY_BACK,
	KEY_TAB,
	KEY_ENTER,
	KEY_PAUSE,
	KEY_ESC,

	KEY_F1,
	KEY_F2,
	KEY_F3,
	KEY_F4,
	KEY_F5,
	KEY_F6,
	KEY_F7,
	KEY_F8,
	KEY_F9,
	KEY_F10,
	KEY_F11,
	KEY_F12,
	KEY_F13,
	KEY_F14,
	KEY_F15,
	KEY_F16,

	KEY_LCTRL,
	KEY_RCTRL,
	KEY_LSHIFT,
	KEY_RSHIFT,
	KEY_LALT,
	KEY_RALT,
	KEY_LMETA,
	KEY_RMETA,
	KEY_LSUPER,
	KEY_RSUPER,
	KEY_MENU,

	KEY_NUMLOCK,
	KEY_SCRLLOCK,
	KEY_CAPSLOCK,
	KEY_PRTSC,

	KEY_UP,
	KEY_DOWN,
	KEY_LEFT,
	KEY_RIGHT,

	KEY_INSERT,
	KEY_HOME,
	KEY_END,
	KEY_PGUP,
	KEY_PGDN,

	KEY_KP_C0,
	KEY_KP_C1,
	KEY_KP_C2,
	KEY_KP_C3,
	KEY_KP_C4,
	KEY_KP_C5,
	KEY_KP_C6,
	KEY_KP_C7,
	KEY_KP_C8,
	KEY_KP_C9,
	KEY_KP_SLASH,
	KEY_KP_ASTERISK,
	KEY_KP_HYPHEN,
	KEY_KP_PLUS,
	KEY_KP_PERIOD,
	KEY_KP_ENTER,

	KEY_OTHER_0,
	/* ... */
	KEY_LAST_OTHER=512,

	NUM_KEYS,
	KEY_INVALID
};

#ifdef COMPAT_KEYSYMS
/* XXX: remove */
#define SDLK_FIRST 0
#define SDLK_LAST NUM_KEYS
#define SDLKey RageKeySym

#define SDLK_BACKSPACE	KEY_BACK
#define SDLK_TAB		KEY_TAB
#define SDLK_RETURN		KEY_ENTER
#define SDLK_PAUSE		KEY_PAUSE
#define SDLK_ESCAPE		KEY_ESC
#define SDLK_SPACE		KEY_SPACE
#define SDLK_EXCLAIM	KEY_EXCL
#define SDLK_QUOTEDBL	KEY_QUOTE
#define SDLK_HASH		KEY_HASH
#define SDLK_DOLLAR		KEY_DOLLAR
#define SDLK_AMPERSAND	KEY_AMPER
#define SDLK_QUOTE		KEY_SQUOTE
#define SDLK_LEFTPAREN	KEY_LPAREN
#define SDLK_RIGHTPAREN	KEY_RPAREN
#define SDLK_ASTERISK	KEY_ASTERISK
#define SDLK_PLUS		KEY_PLUS
#define SDLK_COMMA		KEY_COMMA
#define SDLK_MINUS		KEY_HYPHEN
#define SDLK_PERIOD		KEY_PERIOD
#define SDLK_SLASH		KEY_SLASH
#define SDLK_0			KEY_C0
#define SDLK_1			KEY_C1
#define SDLK_2			KEY_C2
#define SDLK_3			KEY_C3
#define SDLK_4			KEY_C4
#define SDLK_5			KEY_C5
#define SDLK_6			KEY_C6
#define SDLK_7			KEY_C7
#define SDLK_8			KEY_C8
#define SDLK_9			KEY_C9
#define SDLK_COLON		KEY_COLON
#define SDLK_SEMICOLON	KEY_SEMICOLON
#define SDLK_LESS		KEY_LANGLE
#define SDLK_EQUALS		KEY_EQUAL
#define SDLK_GREATER	KEY_RANGLE
#define SDLK_QUESTION	KEY_QUESTION
#define SDLK_AT			KEY_AT
/*#define KEY_CA
#define KEY_CB
#define KEY_CC
#define KEY_CD
#define KEY_CE
#define KEY_CF
#define KEY_CG
#define KEY_CH
#define KEY_CI
#define KEY_CJ
#define KEY_CK
#define KEY_CL
#define KEY_CM
#define KEY_CN
#define KEY_CO
#define KEY_CP
#define KEY_CQ
#define KEY_CR
#define KEY_CS
#define KEY_CT
#define KEY_CU
#define KEY_CV
#define KEY_CW
#define KEY_CX
#define KEY_CY
#define KEY_CZ*/
#define SDLK_LEFTBRACKET	KEY_LBRACKET
#define SDLK_BACKSLASH	KEY_BACKSLASH
#define SDLK_RIGHTBRACKET	KEY_RBRACKET
#define SDLK_CARET		KEY_CARAT
#define SDLK_UNDERSCORE	KEY_UNDERSCORE
#define SDLK_BACKQUOTE	KEY_ACCENT
#define SDLK_a			KEY_Ca
#define SDLK_b			KEY_Cb
#define SDLK_c			KEY_Cc
#define SDLK_d			KEY_Cd
#define SDLK_e			KEY_Ce
#define SDLK_f			KEY_Cf
#define SDLK_g			KEY_Cg
#define SDLK_h			KEY_Ch
#define SDLK_i			KEY_Ci
#define SDLK_j			KEY_Cj
#define SDLK_k			KEY_Ck
#define SDLK_l			KEY_Cl
#define SDLK_m			KEY_Cm
#define SDLK_n			KEY_Cn
#define SDLK_o			KEY_Co
#define SDLK_p			KEY_Cp
#define SDLK_q			KEY_Cq
#define SDLK_r			KEY_Cr
#define SDLK_s			KEY_Cs
#define SDLK_t			KEY_Ct
#define SDLK_u			KEY_Cu
#define SDLK_v			KEY_Cv
#define SDLK_w			KEY_Cw
#define SDLK_x			KEY_Cx
#define SDLK_y			KEY_Cy
#define SDLK_z			KEY_Cz
#define KEY_PIPE
#define SDLK_DELETE		KEY_DEL

#define SDLK_F1			KEY_F1
#define SDLK_F2			KEY_F2
#define SDLK_F3			KEY_F3
#define SDLK_F4			KEY_F4
#define SDLK_F5			KEY_F5
#define SDLK_F6			KEY_F6
#define SDLK_F7			KEY_F7
#define SDLK_F8			KEY_F8
#define SDLK_F9			KEY_F9
#define SDLK_F10		KEY_F10
#define SDLK_F11		KEY_F11
#define SDLK_F12		KEY_F12
#define SDLK_F13		KEY_F13
#define SDLK_F14		KEY_F14
#define SDLK_F15		KEY_F15

#define SDLK_LCTRL		KEY_LCTRL
#define SDLK_RCTRL		KEY_RCTRL
#define SDLK_LSHIFT		KEY_LSHIFT
#define SDLK_RSHIFT		KEY_RSHIFT
#define SDLK_LALT		KEY_LALT
#define SDLK_RALT		KEY_RALT
#define SDLK_LMETA		KEY_LMETA
#define SDLK_RMETA		KEY_RMETA
#define SDLK_LSUPER		KEY_LSUPER
#define SDLK_RSUPER		KEY_RSUPER
#define SDLK_MENU		KEY_MENU

#define SDLK_NUMLOCK	KEY_NUMLOCK
#define SDLK_SCROLLOCK	KEY_SCRLLOCK
#define SDLK_CAPSLOCK	KEY_CAPSLOCK
#define SDLK_PRINT		KEY_PRTSC

#define SDLK_UP			KEY_UP
#define SDLK_DOWN		KEY_DOWN
#define SDLK_LEFT		KEY_LEFT
#define SDLK_RIGHT		KEY_RIGHT

#define SDLK_INSERT		KEY_INSERT
#define SDLK_HOME		KEY_HOME
#define SDLK_END		KEY_END
#define SDLK_PAGEUP		KEY_PGUP
#define SDLK_PAGEDOWN	KEY_PGDN

#define SDLK_KP0		KEY_KP_C0
#define SDLK_KP1		KEY_KP_C1
#define SDLK_KP2		KEY_KP_C2
#define SDLK_KP3		KEY_KP_C3
#define SDLK_KP4		KEY_KP_C4
#define SDLK_KP5		KEY_KP_C5
#define SDLK_KP6		KEY_KP_C6
#define SDLK_KP7		KEY_KP_C7
#define SDLK_KP8		KEY_KP_C8
#define SDLK_KP9		KEY_KP_C9
#define SDLK_KP_DIVIDE	KEY_KP_SLASH
#define SDLK_KP_MULTIPLY KEY_KP_ASTERISK
#define SDLK_KP_MINUS	KEY_KP_HYPHEN
#define SDLK_KP_PLUS	KEY_KP_PLUS
#define SDLK_KP_PERIOD	KEY_KP_PERIOD
#define SDLK_KP_ENTER	KEY_KP_ENTER

#endif

const int NUM_KEYBOARD_BUTTONS = NUM_KEYS;
const int NUM_JOYSTICKS = 6;
const int NUM_JOYSTICK_HATS = 1;
const int NUM_PUMPS = 2;
const int NUM_PARAS = 2;

enum InputDevice {
	DEVICE_KEYBOARD = 0,
	DEVICE_JOY1,
	DEVICE_JOY2,
	DEVICE_JOY3,
	DEVICE_JOY4,
	DEVICE_JOY5,
	DEVICE_JOY6,
	DEVICE_PUMP1,
	DEVICE_PUMP2,
	DEVICE_PARA,
	NUM_INPUT_DEVICES,	// leave this at the end
	DEVICE_NONE			// means this is NULL
};

// button byte codes for directional pad
enum JoystickButton {
	JOY_LEFT = 0, JOY_RIGHT, 
	JOY_UP, JOY_DOWN,
	JOY_Z_UP, JOY_Z_DOWN,
	JOY_ROT_UP, JOY_ROT_DOWN, JOY_ROT_LEFT, JOY_ROT_RIGHT, JOY_ROT_Z_UP, JOY_ROT_Z_DOWN,
	JOY_HAT_LEFT, JOY_HAT_RIGHT, JOY_HAT_UP, JOY_HAT_DOWN, 
	JOY_AUX_1, JOY_AUX_2, JOY_AUX_3, JOY_AUX_4,
	JOY_1,	JOY_2,	JOY_3,	JOY_4,	JOY_5,	JOY_6,	JOY_7,	JOY_8,	JOY_9,	JOY_10,
	JOY_11,	JOY_12,	JOY_13,	JOY_14,	JOY_15,	JOY_16,	JOY_17,	JOY_18,	JOY_19,	JOY_20,
	JOY_21,	JOY_22,	JOY_23,	JOY_24, JOY_25, JOY_26, JOY_27, JOY_28, JOY_29, JOY_30,
	JOY_31,	JOY_32,
	NUM_JOYSTICK_BUTTONS	// leave this at the end
};

enum PumpButton {
	PUMP_UL,
	PUMP_UR,
	PUMP_MID,
	PUMP_DL,
	PUMP_DR,
	PUMP_ESCAPE,

	/* These buttons are for slave pads, attached to the first pad; they don't have
	 * their own USB device, and they have no escape button. */
	PUMP_2P_UL,
	PUMP_2P_UR,
	PUMP_2P_MID,
	PUMP_2P_DL,
	PUMP_2P_DR,
	NUM_PUMP_PAD_BUTTONS	// leave this at the end
};

enum ParaButton {
	PARA_L,
	PARA_UL,
	PARA_U,
	PARA_UR,
	PARA_R,
	PARA_SELECT,
	PARA_START,
	PARA_MENU_LEFT,
	PARA_MENU_RIGHT,
	NUM_PARA_PAD_BUTTONS	// leave this at the end
};


const int NUM_DEVICE_BUTTONS[NUM_INPUT_DEVICES] =
{
	NUM_KEYBOARD_BUTTONS, // DEVICE_KEYBOARD
	NUM_JOYSTICK_BUTTONS, // DEVICE_JOY1
	NUM_JOYSTICK_BUTTONS, // DEVICE_JOY2
	NUM_JOYSTICK_BUTTONS, // DEVICE_JOY3
	NUM_JOYSTICK_BUTTONS, // DEVICE_JOY4
	NUM_JOYSTICK_BUTTONS, // DEVICE_JOY5
	NUM_JOYSTICK_BUTTONS, // DEVICE_JOY6
	NUM_PUMP_PAD_BUTTONS, // DEVICE_PUMP1
	NUM_PUMP_PAD_BUTTONS, // DEVICE_PUMP2
	NUM_PARA_PAD_BUTTONS, // DEVICE_PARA
};

const int MAX_DEVICE_BUTTONS = NUM_KEYBOARD_BUTTONS;

struct DeviceInput
{
public:
	InputDevice device;
	int button;
	RageTimer ts;

	DeviceInput(): device(DEVICE_NONE), button(-1), ts(RageZeroTimer) { }
	DeviceInput( InputDevice d, int b ): device(d), button(b), ts(RageZeroTimer) { }
	DeviceInput( InputDevice d, int b, const RageTimer &t ):
		device(d), button(b), ts(t) { }

	bool operator==( const DeviceInput &other ) 
	{ 
		return device == other.device  &&  button == other.button;
	};

	CString GetDescription();
	
	CString toString();
	bool fromString( const CString &s );

	bool IsValid() const { return device != DEVICE_NONE; };
	void MakeInvalid() { device = DEVICE_NONE; };

	char ToChar() const;

	bool IsJoystick() const { return DEVICE_JOY1 <= device && device < DEVICE_JOY1+NUM_JOYSTICKS; }

	static int NumButtons(InputDevice device);
};

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

#ifndef RAGEINPUTDEVICE_H
#define RAGEINPUTDEVICE_H 1

#include "SDL_keysym.h"

const int NUM_KEYBOARD_BUTTONS = SDLK_LAST;
const int NUM_JOYSTICKS = 4;
const int NUM_JOYSTICK_AXES = 4;	// X, Y, Z, rudder
const int NUM_JOYSTICK_HATS = 1;
const int NUM_PUMPS = 2;

const int NUM_DEVICE_BUTTONS = NUM_KEYBOARD_BUTTONS;

enum InputDevice {
	DEVICE_KEYBOARD = 0,
	DEVICE_JOY1,
	DEVICE_JOY2,
	DEVICE_JOY3,
	DEVICE_JOY4,
	DEVICE_PUMP1,
	DEVICE_PUMP2,
	NUM_INPUT_DEVICES,	// leave this at the end
	DEVICE_NONE			// means this is NULL
};

// button byte codes for directional pad
enum JoystickButton {
	JOY_LEFT = 0, JOY_RIGHT, 
	JOY_UP, JOY_DOWN,
	JOY_Z_UP, JOY_Z_DOWN,
	JOY_Z_ROT_UP, JOY_Z_ROT_DOWN,
	JOY_HAT_LEFT, JOY_HAT_RIGHT, JOY_HAT_UP, JOY_HAT_DOWN, 
	JOY_1,	JOY_2,	JOY_3,	JOY_4,	JOY_5,	JOY_6,	JOY_7,	JOY_8,	JOY_9,	JOY_10,
	JOY_11,	JOY_12,	JOY_13,	JOY_14,	JOY_15,	JOY_16,	JOY_17,	JOY_18,	JOY_19,	JOY_20,
	JOY_21,	JOY_22,	JOY_23,	JOY_24,
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

struct DeviceInput
{
public:
	InputDevice device;
	int button;

	DeviceInput() { device=DEVICE_NONE; };
	DeviceInput( InputDevice d, int b ) { device=d; button=b; };

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

	static int NumButtons(InputDevice device);
};

#endif
/*
 * Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
 *	Chris Danford
 */

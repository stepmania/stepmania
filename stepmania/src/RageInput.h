#ifndef RAGEINPUT_H
#define RAGEINPUT_H
/*
-----------------------------------------------------------------------------
 File: RageInput.h

 Desc: Wrapper for SDL's input routines.  Generates InputEvents.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

struct _SDL_Joystick;
typedef struct _SDL_Joystick SDL_Joystick;

#include "SDL_keysym.h"

const int NUM_KEYBOARD_BUTTONS = SDLK_LAST;
const int NUM_JOYSTICKS = 4;
const int NUM_JOYSTICK_AXES = 4;	// X, Y, Z, rudder
const int NUM_JOYSTICK_HATS = 1;
const int NUM_PUMPS = 2;
const int NUM_PUMP_PAD_BUTTONS = 11;

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


class RageInput
{
	SDL_Joystick*		m_pJoystick[NUM_JOYSTICKS];

	enum State { CURRENT = 0, LAST, NUM_STATES };

	struct {
		// Keyboard state data
		bool m_keys[NUM_KEYBOARD_BUTTONS];
		
		// Joystick state data
		struct {
			bool button[NUM_JOYSTICK_BUTTONS];
		} m_joyState[NUM_JOYSTICKS];

		// Pump state data
		struct {
			bool button[NUM_PUMP_PAD_BUTTONS];
		} m_pumpState[NUM_PUMPS];
	} state[NUM_STATES];

	/* Structure for reading Pump pads: */
	struct pump_t;
	pump_t *m_Pumps;

public:
	RageInput();
	~RageInput();

	void Update( float fDeltaTime );
	bool BeingPressed( DeviceInput di, bool bPrevState );
	bool IsBeingPressed( DeviceInput di ) { return BeingPressed(di, false); }
	bool WasBeingPressed( DeviceInput di ) { return BeingPressed(di, true); }
};

namespace USB {
	char *GetUSBDevicePath (int num);
	HANDLE OpenUSB (int VID, int PID, int num);
};

extern RageInput*			INPUTMAN;	// global and accessable from anywhere in our program


#endif

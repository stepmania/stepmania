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

#include "RageInputDevice.h"

struct _SDL_Joystick;
typedef struct _SDL_Joystick SDL_Joystick;

class RageInput
{
	SDL_Joystick*		m_pJoystick[NUM_JOYSTICKS];

	enum State { CURRENT = 0, LAST, NUM_STATES };

	struct state_t {
		struct device_t {
			bool button[NUM_DEVICE_BUTTONS];
		} m_Devices[NUM_INPUT_DEVICES];
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
/*
 * Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
 *	Chris Danford
 *  Glenn Maynard
 */

#ifndef RAGEINPUT_H
#define RAGEINPUT_H
/*
-----------------------------------------------------------------------------
 File: RageInput.h

 Desc: Wrapper for SDL's input routines.  Generates InputEvents.
-----------------------------------------------------------------------------
*/

#include "RageInputDevice.h"
#include "SDL_utils.h"
#include "arch/arch.h"

struct _SDL_Joystick;
typedef struct _SDL_Joystick SDL_Joystick;

class RageInput
{
	vector<InputHandler *> Devices;

	vector<SDL_Joystick *> Joysticks;

public:
	RageInput();
	~RageInput();

	void Update( float fDeltaTime );
	bool FeedSDLEvent(const SDL_Event &event);
};

extern RageInput*			INPUTMAN;	// global and accessable from anywhere in our program

#endif
/*
 * Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
 *	Chris Danford
 *  Glenn Maynard
 */

#ifndef INPUT_HANDLER_SDL_KEYBOARD_H
#define INPUT_HANDLER_SDL_KEYBOARD_H

#include "InputHandler.h"

struct _SDL_Joystick;
typedef struct _SDL_Joystick SDL_Joystick;

class InputHandler_SDL: public InputHandler
{
	vector<SDL_Joystick *> Joysticks;

public:
	void Update(float fDeltaTime);
	InputHandler_SDL();
	~InputHandler_SDL();
	void GetDevicesAndDescriptions(vector<InputDevice>& vDevicesOut, vector<CString>& vDescriptionsOut);
};


#endif
/*
-----------------------------------------------------------------------------
 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Glenn Maynard
-----------------------------------------------------------------------------
*/

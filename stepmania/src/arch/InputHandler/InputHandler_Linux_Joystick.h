#ifndef INPUT_HANDLER_LINUX_JOYSTICK_H
#define INPUT_HANDLER_LINUX_JOYSTICK_H 1

#include "InputHandler.h"


class InputHandler_Linux_Joystick: public InputHandler
{
public:
	enum { NUM_JOYSTICKS = 2 };
	void Update(float fDeltaTime);
	InputHandler_Linux_Joystick();
	~InputHandler_Linux_Joystick();
	void GetDevicesAndDescriptions(vector<InputDevice>& vDevicesOut, vector<CString>& vDescriptionsOut);

private:
	int fds[NUM_JOYSTICKS];
};

#endif
/*
-----------------------------------------------------------------------------
 Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
	Glenn Maynard
-----------------------------------------------------------------------------
*/

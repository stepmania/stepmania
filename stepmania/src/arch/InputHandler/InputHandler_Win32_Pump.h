#ifndef INPUT_HANDLER_WIN32_PUMP_H
#define INPUT_HANDLER_WIN32_PUMP_H 1

#include "InputHandler.h"

class USBDevice;
class InputHandler_Win32_Pump: public InputHandler
{
	USBDevice *dev;

public:
	void Update(float fDeltaTime);
	InputHandler_Win32_Pump();
	~InputHandler_Win32_Pump();
	void GetDevicesAndDescriptions(vector<InputDevice>& vDevicesOut, vector<CString>& vDescriptionsOut);
};

#endif
/*
-----------------------------------------------------------------------------
 Copyright (c) 2002-2003 by the person(s) listed below.  All rights reserved.
	Glenn Maynard
-----------------------------------------------------------------------------
*/

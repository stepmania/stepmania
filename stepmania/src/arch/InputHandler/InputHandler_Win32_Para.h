#ifndef INPUT_HANDLER_WIN32_PARA_H
#define INPUT_HANDLER_WIN32_PARA_H 1

#include "InputHandler.h"
#include "RageThreads.h"

class USBDevice;
class InputHandler_Win32_Para: public InputHandler
{
	USBDevice *dev;
	RageThread InputThread;
	bool shutdown;

	static int InputThread_Start( void *p );
	void InputThreadMain();
	void HandleInput( int devno, int event );


public:
	void Update(float fDeltaTime);
	InputHandler_Win32_Para();
	~InputHandler_Win32_Para();
	void GetDevicesAndDescriptions(vector<InputDevice>& vDevicesOut, vector<CString>& vDescriptionsOut);
};

#endif
/*
-----------------------------------------------------------------------------
 Copyright (c) 2002-2003 by the person(s) listed below.  All rights reserved.
	Glenn Maynard
-----------------------------------------------------------------------------
*/

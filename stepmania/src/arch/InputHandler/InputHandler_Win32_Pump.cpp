#include "global.h"
#include "InputHandler_Win32_Pump.h"

#include "RageLog.h"
#include "RageInputDevice.h"
#include "InputFilter.h"
#include "archutils/Win32/USB.h"

InputHandler_Win32_Pump::InputHandler_Win32_Pump()
{
	const int pump_usb_vid = 0x0d2f, pump_usb_pid = 0x0001;

	dev = new USBDevice[NUM_PUMPS];
	
	for(int i = 0; i < NUM_PUMPS; ++i)
	{
		if(dev[i].Open(pump_usb_vid, pump_usb_pid, i))
			LOG->Info("Found Pump pad %i", i);
	}
}

InputHandler_Win32_Pump::~InputHandler_Win32_Pump()
{
	delete[] dev;
}

void InputHandler_Win32_Pump::Update(float fDeltaTime)
{
	static const int bits[] = {
	/* P1 */	(1<<9), (1<<12), (1<<13), (1<<11), (1<<10),
	/* ESC */	(1<<16),
	/* P1 */	(1<<17), (1<<20), (1<<21), (1<<19), (1<<18),
	};

	for(int i = 0; i < NUM_PUMPS; ++i)
	{
		int ret = dev[i].GetPadEvent();

		if(ret == -1) 
			continue; /* no event */

		InputDevice id = InputDevice(DEVICE_PUMP1 + i);
	
		for (int butno = 0 ; butno < NUM_PUMP_PAD_BUTTONS ; butno++)
			INPUTFILTER->ButtonPressed(DeviceInput(id, butno), !(ret & bits[butno]));
	}
}

/*
-----------------------------------------------------------------------------
 Copyright (c) 2002-2003 by the person(s) listed below.  All rights reserved.
	Glenn Maynard
-----------------------------------------------------------------------------
*/

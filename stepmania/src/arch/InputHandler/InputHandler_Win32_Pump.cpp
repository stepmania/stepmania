#include "global.h"
#include "InputHandler_Win32_Pump.h"

#include "PrefsManager.h"
#include "RageLog.h"
#include "RageInputDevice.h"
#include "archutils/Win32/USB.h"

InputHandler_Win32_Pump::InputHandler_Win32_Pump()
{
	shutdown = false;
	const int pump_usb_vid = 0x0d2f, pump_usb_pid = 0x0001;

	dev = new USBDevice[NUM_PUMPS];
	
	for(int i = 0; i < NUM_PUMPS; ++i)
	{
		if(dev[i].Open(pump_usb_vid, pump_usb_pid, sizeof(long), i))
			LOG->Info("Found Pump pad %i", i);
	}

	if( PREFSMAN->m_bThreadedInput )
		InputThreadPtr = SDL_CreateThread(InputThread_Start, this);
	else
		InputThreadPtr = NULL;
}

InputHandler_Win32_Pump::~InputHandler_Win32_Pump()
{
	if( InputThreadPtr )
	{
		shutdown = true;
		LOG->Trace("Shutting down Pump thread ...");
		SDL_WaitThread(InputThreadPtr, NULL);
		LOG->Trace("Pump thread shut down.");
	}

	delete[] dev;
}

void InputHandler_Win32_Pump::HandleInput( int devno, int event )
{
	static const int bits[NUM_PUMP_PAD_BUTTONS] = {
	/* P1 */	(1<<9), (1<<12), (1<<13), (1<<11), (1<<10),
	/* ESC */	(1<<16),
	/* P1 */	(1<<17), (1<<20), (1<<21), (1<<19), (1<<18),
	};

	InputDevice id = InputDevice(DEVICE_PUMP1 + devno);

	for (int butno = 0 ; butno < NUM_PUMP_PAD_BUTTONS ; butno++)
	{
		DeviceInput dev(id, butno);
		
		/* If we're in a thread, our timestamp is accurate. */
		if( InputThreadPtr )
			dev.ts.Touch();

		ButtonPressed(dev, !(event & bits[butno]));
	}
}

void InputHandler_Win32_Pump::GetDevicesAndDescriptions(vector<InputDevice>& vDevicesOut, vector<CString>& vDescriptionsOut)
{
	for(int i = 0; i < NUM_PUMPS; ++i)
	{
		if( dev[i].IsOpen() )
		{
			vDevicesOut.push_back( InputDevice(DEVICE_PUMP1+i) );
			vDescriptionsOut.push_back( "Pump USB" );
		}
	}
}

int InputHandler_Win32_Pump::InputThread_Start( void *p )
{
	((InputHandler_Win32_Pump *) p)->InputThread();
	return 0;
}

void InputHandler_Win32_Pump::InputThread()
{
	vector<WindowsFileIO *> sources;
	int i;
	for(i = 0; i < NUM_PUMPS; ++i)
	{
		if( dev[i].io.IsOpen() )
			sources.push_back( &dev[i].io );
	}

	while(!shutdown)
	{
		int actual = 0, val = 0;
		int ret = WindowsFileIO::read_several(sources, &val, actual, 0.100f);

		if(ret <= 0) 
			continue; /* no event */

		HandleInput( actual, val );
		InputHandler::UpdateTimer();
	}
}

void InputHandler_Win32_Pump::Update(float fDeltaTime)
{
	if( InputThreadPtr == NULL )
	{
		for(int i = 0; i < NUM_PUMPS; ++i)
		{
			int ret = dev[i].GetPadEvent();

			if(ret == -1) 
				continue; /* no event */

			HandleInput( i, ret );
		}
		InputHandler::UpdateTimer();
	}
}

/*
-----------------------------------------------------------------------------
 Copyright (c) 2002-2003 by the person(s) listed below.  All rights reserved.
	Glenn Maynard
-----------------------------------------------------------------------------
*/

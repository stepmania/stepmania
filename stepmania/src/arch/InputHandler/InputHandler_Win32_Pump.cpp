#include "global.h"
#include "InputHandler_Win32_Pump.h"

#include "PrefsManager.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "RageInputDevice.h"
#include "archutils/Win32/USB.h"

InputHandler_Win32_Pump::InputHandler_Win32_Pump()
{
	shutdown = false;
	const int pump_usb_vid = 0x0d2f, pump_usb_pid = 0x0001;

	dev = new USBDevice[NUM_PUMPS];

	bool FoundOnePad = false;
	for(int i = 0; i < NUM_PUMPS; ++i)
	{
		if(dev[i].Open(pump_usb_vid, pump_usb_pid, sizeof(long), i))
		{
			FoundOnePad = true;
			LOG->Info("Found Pump pad %i", i);
		}
	}

	/* Don't start a thread if we have no pads. */
	if( FoundOnePad && PREFSMAN->m_bThreadedInput )
	{
		InputThread.SetName("Pump thread");
		InputThread.Create( InputThread_Start, this );
	}
}

InputHandler_Win32_Pump::~InputHandler_Win32_Pump()
{
	if( InputThread.IsCreated() )
	{
		shutdown = true;
		LOG->Trace("Shutting down Pump thread ...");
		InputThread.Wait();
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
		DeviceInput di(id, butno);
		
		/* If we're in a thread, our timestamp is accurate. */
		if( InputThread.IsCreated() )
			di.ts.Touch();

		ButtonPressed(di, !(event & bits[butno]));
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
	((InputHandler_Win32_Pump *) p)->InputThreadMain();
	return 0;
}

void InputHandler_Win32_Pump::InputThreadMain()
{
	if(!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST))
		LOG->Warn(werr_ssprintf(GetLastError(), "Failed to set Pump thread priority"));

	/* Enable priority boosting. */
	SetThreadPriorityBoost( GetCurrentThread(), FALSE );

	vector<WindowsFileIO *> sources;
	int i;
	for(i = 0; i < NUM_PUMPS; ++i)
	{
		if( dev[i].io.IsOpen() )
			sources.push_back( &dev[i].io );
	}

	while(!shutdown)
	{
		CHECKPOINT;
		int actual = 0, val = 0;
		int ret = WindowsFileIO::read_several(sources, &val, actual, 0.100f);

		CHECKPOINT;
		if(ret <= 0) 
			continue; /* no event */

		HandleInput( actual, val );
		InputHandler::UpdateTimer();
	}
	CHECKPOINT;
}

void InputHandler_Win32_Pump::Update(float fDeltaTime)
{
	if( !InputThread.IsCreated() )
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
 * (c) 2002-2004 Glenn Maynard
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */


#include "global.h"
#include "InputHandler_Win32_Para.h"

#include "PrefsManager.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "RageInputDevice.h"
#include "archutils/Win32/USB.h"

InputHandler_Win32_Para::InputHandler_Win32_Para()
{
	shutdown = false;
	const int para_usb_vid = 0x0507, para_usb_pid = 0x0409;

	dev = new USBDevice;

	bool FoundOnePad = false;
	if( dev->Open(para_usb_vid, para_usb_pid, sizeof(long), 0) )
	{
		FoundOnePad = true;
		LOG->Info("Found Para controller");
	}

	/* Don't start a thread if we have no pads. */
	if( FoundOnePad && PREFSMAN->m_bThreadedInput )
	{
		InputThread.SetName("Para thread");
		InputThread.Create( InputThread_Start, this );
	}
}

InputHandler_Win32_Para::~InputHandler_Win32_Para()
{
	if( InputThread.IsCreated() )
	{
		shutdown = true;
		LOG->Trace("Shutting down Para thread ...");
		InputThread.Wait();
		LOG->Trace("Para thread shut down.");
	}

	delete dev;
}

void InputHandler_Win32_Para::HandleInput( int devno, int event )
{
	static const int bits[NUM_PARA_PAD_BUTTONS] = {
	/* sensors */	(1<<9), (1<<12), (1<<13), (1<<11), (1<<10),
	/* buttons */	(1<<16),(1<<17), (1<<20), (1<<21),
	};

	InputDevice id = DEVICE_PARA;

	for (int butno = 0 ; butno < NUM_PARA_PAD_BUTTONS ; butno++)
	{
		DeviceInput di(id, butno);
		
		/* If we're in a thread, our timestamp is accurate. */
		if( InputThread.IsCreated() )
			di.ts.Touch();

		ButtonPressed(di, !(event & bits[butno]));
	}
}

void InputHandler_Win32_Para::GetDevicesAndDescriptions(vector<InputDevice>& vDevicesOut, vector<CString>& vDescriptionsOut)
{
	if( dev->IsOpen() )
	{
		vDevicesOut.push_back( DEVICE_PARA );
		vDescriptionsOut.push_back( "Para USB" );
	}
}

int InputHandler_Win32_Para::InputThread_Start( void *p )
{
	((InputHandler_Win32_Para *) p)->InputThreadMain();
	return 0;
}

void InputHandler_Win32_Para::InputThreadMain()
{
	if(!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST))
		LOG->Warn(werr_ssprintf(GetLastError(), "Failed to set Para thread priority"));

	vector<WindowsFileIO *> sources;
	if( dev->io.IsOpen() )
		sources.push_back( &dev->io );

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

void InputHandler_Win32_Para::Update(float fDeltaTime)
{
	if( !InputThread.IsCreated() )
	{
		int ret = dev->GetPadEvent();

		if(ret == -1) 
			return;

		HandleInput( 0, ret );
		InputHandler::UpdateTimer();
	}
}

/*
 * (c) 2003-2004 Glenn Maynard
 * (c) 2002-2004 Chris Danford, Glenn Maynard
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

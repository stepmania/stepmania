#include "global.h"
#include "RageUtil.h"
#include "InputHandler_Win32_MIDI.h"
#include "RageLog.h"

#include <windows.h>
#include <mmsystem.h>

#include "ScreenManager.h"

#pragma comment (lib,"winmm.lib")

HMIDIIN g_device;
void CALLBACK midiCallback(HMIDIIN g_device, UINT status, DWORD instancePtr, DWORD data, DWORD timestamp);


InputHandler_Win32_MIDI::InputHandler_Win32_MIDI()
{
	int device_id = 0;

	MMRESULT result;
	g_device = NULL;

	if (device_id >= midiInGetNumDevs()) {
		m_bFoundDevice = false;
		return;
    } else
		m_bFoundDevice = true;

	result = midiInOpen(&g_device, device_id, (DWORD)&(midiCallback), (DWORD)this, CALLBACK_FUNCTION);
    if (result != MMSYSERR_NOERROR) {
		char strError[256];
		midiOutGetErrorText( result, strError, 256 );
		LOG->Warn( "Error with MIDI Opening: %s", strError );
        return;
    }

    result = midiInStart(g_device);
    if (result != MMSYSERR_NOERROR) {
		char strError[256];
		midiOutGetErrorText( result, strError, 256 );
		LOG->Warn( "Error with MIDI Starting: %s", strError );
        return;
    }
}

InputHandler_Win32_MIDI::~InputHandler_Win32_MIDI()
{
	MMRESULT result;

    result = midiInReset(g_device);
    if (result != MMSYSERR_NOERROR) {
		char strError[256];
		midiOutGetErrorText( result, strError, 256 );
		LOG->Warn( "Error with MIDI Reset: %s", strError );
        return;
    }

    result = midiInClose(g_device);
    if (result != MMSYSERR_NOERROR) {
		char strError[256];
		midiOutGetErrorText( result, strError, 256 );
		LOG->Warn( "Error with MIDI Close: %s", strError );
        return;
    }

}

void InputHandler_Win32_MIDI::GetDevicesAndDescriptions(vector<InputDevice>& vDevicesOut, vector<CString>& vDescriptionsOut)
{
	if ( m_bFoundDevice )
	{
		vDevicesOut.push_back( InputDevice(DEVICE_MIDI) );
		vDescriptionsOut.push_back( "Win32_MIDI" );
	}
}

void CALLBACK midiCallback(HMIDIIN g_device, UINT status, DWORD instancePtr, DWORD data, DWORD timestamp)
{
    if (status == MIM_DATA) {
		int type = data & 0x00ff;
		int channel = (data & 0xff00) >> 8;
		int value = ( data & 0x00ff0000) >> 16;

		//Channel 0 in midi is a special channel that generally will get triggerd when too many channels are pressed
		if ( channel == 0 ) return;

		if ( type == 144 )
		{
			DeviceInput di = DeviceInput(DEVICE_MIDI,channel);
			di.ts.Touch();
			if ( value > 0 )
				((InputHandler_Win32_MIDI *)instancePtr)->SetDev( di, true );
			else
				((InputHandler_Win32_MIDI *)instancePtr)->SetDev( di, false );
		}
    }
}

void InputHandler_Win32_MIDI::Update( float fDeltaTime )
{

}

/*
 * Copyright (c) 2005 Charles Lohr
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

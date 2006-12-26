#include "global.h"
#include "RageUtil.h"
#include "InputHandler_Win32_MIDI.h"
#include "RageLog.h"

#include <windows.h>
#include <mmsystem.h>

#pragma comment (lib,"winmm.lib")

REGISTER_INPUT_HANDLER_CLASS2( MIDI, Win32_MIDI );

static HMIDIIN g_device;
static void CALLBACK midiCallback(HMIDIIN g_device, UINT status, DWORD instancePtr, DWORD data, DWORD timestamp);

static RString GetMidiError( MMRESULT result )
{
	char szError[256];
	midiOutGetErrorText( result, szError, 256 );
	return szError;
}

InputHandler_Win32_MIDI::InputHandler_Win32_MIDI()
{
	int device_id = 0;

	g_device = NULL;

	if( device_id >= (int) midiInGetNumDevs() )
	{
		m_bFoundDevice = false;
		return;
	}
	m_bFoundDevice = true;

	MMRESULT result = midiInOpen( &g_device, device_id, (DWORD) &midiCallback, (DWORD) this, CALLBACK_FUNCTION );
	if( result != MMSYSERR_NOERROR )
	{
		LOG->Warn( "Error opening MIDI device: %s", GetMidiError(result).c_str() );
		return;
	}

	result = midiInStart(g_device);
	if( result != MMSYSERR_NOERROR )
	{
		LOG->Warn( "Error starting MIDI device: %s", GetMidiError(result).c_str() );
		return;
	}
}

InputHandler_Win32_MIDI::~InputHandler_Win32_MIDI()
{
	MMRESULT result;

	result = midiInReset( g_device );
	if( result != MMSYSERR_NOERROR )
	{
		LOG->Warn( "Error resetting MIDI device: %s", GetMidiError(result).c_str() );
		return;
	}

	result = midiInClose( g_device );
	if( result != MMSYSERR_NOERROR )
	{
		LOG->Warn( "Error closing MIDI device: %s", GetMidiError(result).c_str() );
		return;
	}
}

void InputHandler_Win32_MIDI::GetDevicesAndDescriptions( vector<InputDeviceInfo>& vDevicesOut )
{
	if( m_bFoundDevice )
	{
		vDevicesOut.push_back( InputDeviceInfo(DEVICE_MIDI,"Win32_MIDI") );
	}
}

static void CALLBACK midiCallback( HMIDIIN device, UINT status, DWORD instancePtr, DWORD data, DWORD timestamp )
{
	if( status == MIM_DATA )
	{
		int iType = data & 0xff;
		int iChannel = (data & 0xff00) >> 8;
		int iValue = (data & 0xff0000) >> 16;

		// Channel 0 in midi is a special channel that generally will get triggered when too many channels are pressed.
		if( iChannel == 0 )
			return;

		if( iType == 144 )
		{
			DeviceInput di = DeviceInput( DEVICE_MIDI, enum_add2(MIDI_FIRST, iChannel), iValue > 0 );
			di.ts.Touch();
			((InputHandler_Win32_MIDI *)instancePtr)->SetDev( di );
		}
	}
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

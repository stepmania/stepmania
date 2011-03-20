#include "global.h"
#include "InputHandler_Win32_Para.h"

#include "RageLog.h"
#include "RageUtil.h"
#include "RageInputDevice.h"
#include "archutils/Win32/USB.h"

// TODO: Abstract this windows-specific stuff into USBDevice.
extern "C" {
#include "archutils/Win32/ddk/setupapi.h"
/* Quiet header warning: */
#include "archutils/Win32/ddk/hidsdi.h"
}

REGISTER_INPUT_HANDLER_CLASS2( Para, Win32_Para );

static void InitHack( HANDLE h )
{
	UCHAR hack[] = {0, 1};

	if( HidD_SetFeature(h, (PVOID) hack, 2) == TRUE )
		LOG->Info( "Para controller powered on successfully" );
	else
		LOG->Warn( "Para controller power-on failed" );
}

InputHandler_Win32_Para::InputHandler_Win32_Para()
{
	const int para_usb_vid = 0x0507;
	const int para_usb_pid = 0x0011;

	USBDevice *dev = new USBDevice;

	if( dev->Open(para_usb_vid, para_usb_pid, sizeof(long), 0, InitHack) )
	{
		LOG->Info("Para controller initialized");
	}
	SAFE_DELETE( dev );
}

void InputHandler_Win32_Para::GetDevicesAndDescriptions(vector<InputDeviceInfo>& vDevicesOut )
{
	// The device appears as a HID joystick
}

/*
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

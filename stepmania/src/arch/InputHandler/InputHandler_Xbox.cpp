#include "global.h"
#include "InputHandler_Xbox.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageDisplay.h"


#include <xtl.h>

struct DEVICE_STATE {
    XPP_DEVICE_TYPE *pxdt;
    DWORD dwState;
};

byte buttonMasks[] = { XINPUT_GAMEPAD_DPAD_LEFT, 
						XINPUT_GAMEPAD_DPAD_RIGHT, 
						XINPUT_GAMEPAD_DPAD_UP, 
						XINPUT_GAMEPAD_DPAD_DOWN,
						XINPUT_GAMEPAD_START,
						XINPUT_GAMEPAD_BACK,
						XINPUT_GAMEPAD_LEFT_THUMB,
						XINPUT_GAMEPAD_RIGHT_THUMB};


/**
 * XBOX controller maps to the following RageInputDevice constants:
 * DPAD -> JOY_HAT_...
 * START -> JOY_AUX_1
 * BACK -> JOY_AUX_2
 * Left thumb button -> JOY_AUX_3
 * Right thumb button -> JOY_AUX_4
 * Following buttons are JOY_(index):
 * A, B, X, Y, BLACK, WHITE, Left trigger, right trigger
 */

InputHandler_Xbox::InputHandler_Xbox()
{
	//
	// Init joysticks
	//
	XDEVICE_PREALLOC_TYPE xdpt[] = {{XDEVICE_TYPE_GAMEPAD, 4}};
	XInitDevices( sizeof(xdpt) / sizeof(XDEVICE_PREALLOC_TYPE), xdpt );
	ZeroMemory( joysticks, sizeof(joysticks) );

	getHandles();
}

InputHandler_Xbox::~InputHandler_Xbox()
{
	for(unsigned i = 0; i < NUM_JOYSTICKS; i++)
	{
		if(joysticks[i] != 0)
			XInputClose(joysticks[i]);
	}
}

void InputHandler_Xbox::Update()
{
	// check insertions and removals
	DWORD dwInsert, dwRemove;
	DEVICE_STATE devices = {XDEVICE_TYPE_GAMEPAD, 0};
    
	bool changes = false;
    // Check each device type to see if any changes have occurred.
    if( XGetDeviceChanges( devices.pxdt, &dwInsert, &dwRemove ) )
    {
		for(int j = 0; j < 4; j++)
		{
			if(1 << j & dwRemove)
			{
				changes = true;
				LOG->Trace("A joystick was removed");
			}
			if(1 << j & dwInsert)
			{
				changes = true;
				LOG->Trace("A joystick was inserted");
			}
		}
    }
    
	if(changes)
	{
		getHandles();
		return;
	}

	for(unsigned i = 0; i < NUM_JOYSTICKS; i++)
	{
		if(joysticks[i] == 0)
			continue;
		
		InputDevice inputDevice = InputDevice(DEVICE_JOY1 + i);
		XINPUT_STATE xis;
		// Query latest state.
		XInputGetState( joysticks[i], &xis );
			
		// check buttons
		for(int j = 0; j < ARRAYLEN(buttonMasks); j++)
		{
			DWORD nowPressed = xis.Gamepad.wButtons & buttonMasks[j];
			DWORD wasPressed = lastState[i].wButtons & buttonMasks[j];

			if(nowPressed != wasPressed)
			{
				DeviceButton Button = DeviceButton(JOY_HAT_LEFT + j);
				if(Button >= JOY_BUTTON_32)
				{
					LOG->Warn("Ignored joystick event (button too high)");
					continue;
				}
				DeviceInput di(inputDevice, Button);
				ButtonPressed(di, nowPressed != 0);
				continue;
			}
		}
		
		// check analog buttons
		for(int j = 0; j < ARRAYLEN(xis.Gamepad.bAnalogButtons); j++)
		{
			bool nowPressed = xis.Gamepad.bAnalogButtons[j] > XINPUT_GAMEPAD_MAX_CROSSTALK;
			bool wasPressed = lastState[i].bAnalogButtons[j] > XINPUT_GAMEPAD_MAX_CROSSTALK;

			if(nowPressed != wasPressed)
			{
				DeviceButton Button = DeviceButton(JOY_BUTTON_1 + j);
				if(Button >= JOY_BUTTON_32)
				{
					LOG->Warn("Ignored joystick event (button too high)");
					continue;
				}
				DeviceInput di(inputDevice, Button);
				ButtonPressed(di, nowPressed);
				continue;
			}
		}

		// check thumbsticks
		SHORT axes[] = { xis.Gamepad.sThumbLX, xis.Gamepad.sThumbLY, xis.Gamepad.sThumbRX, xis.Gamepad.sThumbRY};

		for(int j = 0; j < ARRAYLEN(axes); j++)
		{
			if(axes[j] != 0)
			{
				// Reverse y axis (negative values are down, not up)
				if(j == 1 || j == 3)
					axes[j] = -axes[j];

				DeviceButton neg = (DeviceButton)(JOY_LEFT + (2 * j));
				DeviceButton pos = (DeviceButton)(JOY_RIGHT + (2 * j));
				float l = SCALE( axes[j], 0.0f, 32768.0f, 0.0f, 1.0f );
				ButtonPressed(DeviceInput(inputDevice, neg,max(-l,0),RageZeroTimer), axes[j] > -16000);
				ButtonPressed(DeviceInput(inputDevice, pos,max(+l,0),RageZeroTimer), axes[j] < +16000);
				continue;
			}
		}

		memcpy(&lastState[i], &xis.Gamepad, sizeof(XINPUT_GAMEPAD));
	}

	InputHandler::UpdateTimer();
}

void InputHandler_Xbox::GetDevicesAndDescriptions( vector<InputDeviceInfo>& vDevicesOut )
{
	for( int i=0; i<NUM_JOYSTICKS; i++ )
	{
		if( joysticks[i] != 0 )
		{
			vDevicesOut.push_back( InputDeviceInfo(InputDevice(DEVICE_JOY1+i),"XboxGameHardware") );
		}
	}
}

void InputHandler_Xbox::getHandles()
{
	for(unsigned i = 0; i < NUM_JOYSTICKS; i++)
	{
		if(joysticks[i] != 0)
			XInputClose(joysticks[i]);
	}

	ZeroMemory( joysticks, sizeof(joysticks) );
	ZeroMemory( lastState, sizeof(lastState) );

	// Work out joystick handles
	DEVICE_STATE devices = {XDEVICE_TYPE_GAMEPAD, 0};
	devices.dwState = XGetDevices( devices.pxdt );

	// Check the global gamepad state for a connected device.
	unsigned playersAllocated = 0;
	unsigned joysFound = 0;
	
	for( unsigned i = 0; i < NUM_PORTS; i++ )
    {
        if( devices.dwState & 1 << i)
		{
			if(playersAllocated < NUM_JOYSTICKS)
			{
				XINPUT_POLLING_PARAMETERS pollingParameters = {TRUE, TRUE, 0, 8, 8, 0,};
				joysticks[playersAllocated] = XInputOpen(XDEVICE_TYPE_GAMEPAD, (DWORD)i, XDEVICE_NO_SLOT, &pollingParameters);
				playersAllocated++;
			}
			joysFound++;
		}
	}

	LOG->Info( "Found %d connected joysticks for %d players", joysFound, playersAllocated );
}

/*
 * (c) 2004 Ryan Dortmans
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

#include "global.h"
#include "InputHandler_DirectInput.h"

#include "StepMania.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "archutils/Win32/AppInstance.h"
#include "archutils/Win32/GraphicsWindow.h"
#include "archutils/Win32/RegistryAccess.h"
#include "InputFilter.h"
#include "PrefsManager.h"

#include "InputHandler_DirectInputHelper.h"

static vector<DIDevice> Devices;

/* Number of joysticks found: */
static int g_iNumJoysticks;

static BOOL CALLBACK EnumDevices( const DIDEVICEINSTANCE *pdidInstance, void *pContext )
{
	DIDevice device;

	switch( pdidInstance->dwDevType & 0xFF )
	{
	case DIDEVTYPE_KEYBOARD: device.type = device.KEYBOARD; break;
	case DIDEVTYPE_JOYSTICK: device.type = device.JOYSTICK; break;
	default: return DIENUM_CONTINUE;
	}

	device.JoystickInst = *pdidInstance;

	switch( device.type )
	{
	case device.JOYSTICK:
		if( g_iNumJoysticks == NUM_JOYSTICKS )
			return DIENUM_CONTINUE;

		device.dev = enum_add2( DEVICE_JOY1, g_iNumJoysticks );
		g_iNumJoysticks++;
		break;

	case device.KEYBOARD:
		device.dev = DEVICE_KEYBOARD;
		break;
	}

	Devices.push_back(device);

	return DIENUM_CONTINUE;
}

static void CheckForDirectInputDebugMode()
{
	int iVal;
	if( RegistryAccess::GetRegValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\DirectInput", "emulation", iVal) )
	{
		if( iVal & 0x8 )
			LOG->Warn("DirectInput keyboard debug mode appears to be enabled.  This reduces\n"
			          "input timing accuracy significantly.  Disabling this is strongly recommended." );
	}
}

static int GetNumUsbHidDevices()
{
	// The "Enum" key doesn't exist if no hid devices are attached, so it's expected that GetRegValue will sometimes fail.
	// TODO: Does this work in Win98 and Win2K?
	int i = 0;	
	RegistryAccess::GetRegValue( "HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\HidUsb\\Enum", "Count", i );
	return i;
}

InputHandler_DInput::InputHandler_DInput()
{
	LOG->Trace( "InputHandler_DInput::InputHandler_DInput()" );

	CheckForDirectInputDebugMode();
	
	m_bShutdown = false;
	m_iLastSeenNumUsbHid = GetNumUsbHidDevices();
	g_iNumJoysticks = 0;

	AppInstance inst;	
	HRESULT hr = DirectInputCreate(inst.Get(), DIRECTINPUT_VERSION, &dinput, NULL);
	if( hr != DI_OK )
		RageException::Throw( hr_ssprintf(hr, "InputHandler_DInput: DirectInputCreate") );

	LOG->Trace( "InputHandler_DInput: IDirectInput::EnumDevices(DIDEVTYPE_KEYBOARD)" );
	hr = dinput->EnumDevices( DIDEVTYPE_KEYBOARD, EnumDevices, NULL, DIEDFL_ATTACHEDONLY );
	if( hr != DI_OK )
		RageException::Throw( hr_ssprintf(hr, "InputHandler_DInput: IDirectInput::EnumDevices") );

	LOG->Trace( "InputHandler_DInput: IDirectInput::EnumDevices(DIDEVTYPE_JOYSTICK)" );
	hr = dinput->EnumDevices( DIDEVTYPE_JOYSTICK, EnumDevices, NULL, DIEDFL_ATTACHEDONLY );
	if( hr != DI_OK )
		RageException::Throw( hr_ssprintf(hr, "InputHandler_DInput: IDirectInput::EnumDevices") );

	for( unsigned i = 0; i < Devices.size(); ++i )
	{
		if( Devices[i].Open() )
			continue;

		Devices.erase( Devices.begin() + i );
		i--;
		continue;
	}

	LOG->Info( "Found %u DirectInput devices:", Devices.size() );
	for( unsigned i = 0; i < Devices.size(); ++i )
	{
		LOG->Info( "   %d: '%s' axes: %d, hats: %d, buttons: %d (%s)",
			i,
			Devices[i].JoystickInst.tszProductName,
			Devices[i].axes,
			Devices[i].hats,
			Devices[i].buttons,
			Devices[i].buffered? "buffered": "unbuffered" );
	}

	StartThread();
}

void InputHandler_DInput::StartThread()
{
	ASSERT( !m_Thread.IsCreated() );
	if( PREFSMAN->m_bThreadedInput )
	{
		m_Thread.SetName( "DirectInput thread" );
		m_Thread.Create( InputThread_Start, this );
	}
}

void InputHandler_DInput::ShutdownThread()
{
	if( !m_Thread.IsCreated() )
		return;

	m_bShutdown = true;
	LOG->Trace( "Shutting down DirectInput thread ..." );
	m_Thread.Wait();
	LOG->Trace( "DirectInput thread shut down." );
	m_bShutdown = false;
}

InputHandler_DInput::~InputHandler_DInput()
{
	ShutdownThread();

	for( unsigned i = 0; i < Devices.size(); ++i )
		Devices[i].Close();

	Devices.clear();
	dinput->Release();	
	dinput = NULL;
}

void InputHandler_DInput::WindowReset()
{
	/* We need to reopen keyboards. */
	ShutdownThread();

	for( unsigned i = 0; i < Devices.size(); ++i )
	{
		if( Devices[i].type != Devices[i].KEYBOARD )
			continue;

		Devices[i].Close();

		/* We lose buffered inputs here, so we need to clear all pressed keys. */
		INPUTFILTER->ResetDevice( Devices[i].dev );

		bool ret = Devices[i].Open();

		/* Reopening it should succeed. */
		ASSERT( ret );
	}

	StartThread();
}

#define HAT_UP_MASK 1
#define HAT_DOWN_MASK 2
#define HAT_LEFT_MASK 4
#define HAT_RIGHT_MASK 8

static int TranslatePOV(DWORD value)
{
	const int HAT_VALS[] =
	{
	    HAT_UP_MASK,
	    HAT_UP_MASK   | HAT_RIGHT_MASK,
	    HAT_RIGHT_MASK,
	    HAT_DOWN_MASK | HAT_RIGHT_MASK,
	    HAT_DOWN_MASK,
	    HAT_DOWN_MASK | HAT_LEFT_MASK,
	    HAT_LEFT_MASK,
	    HAT_UP_MASK   | HAT_LEFT_MASK
	};

	if( LOWORD(value) == 0xFFFF )
	    return 0;

	/* Round the value up: */
	value += 4500 / 2;
	value %= 36000;
	value /= 4500;

	if( value >= 8 )
	    return 0; /* shouldn't happen */
	
	return HAT_VALS[value];
}

static HRESULT GetDeviceState( LPDIRECTINPUTDEVICE2 dev, int size, void *ptr )
{
	HRESULT hr = dev->GetDeviceState( size, ptr );
	if( hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED )
	{
		hr = dev->Acquire();
		if( hr != DI_OK )
		{
			LOG->Trace( hr_ssprintf(hr, "?") );
			return hr;
		}

		hr = dev->GetDeviceState( size, ptr );
	}

	return hr;
}

/* This doesn't take a timestamp; instead, we let InputHandler::ButtonPressed figure
 * it out.  Be sure to call InputHandler::Update() between each poll. */
void InputHandler_DInput::UpdatePolled( DIDevice &device, const RageTimer &tm )
{
	switch( device.type )
	{
	default:
		ASSERT(0);
	case device.KEYBOARD:
		{
			unsigned char keys[256];

			HRESULT hr = GetDeviceState( device.Device, 256, keys );
			if( hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED )
				return;

			if( hr != DI_OK )
			{
				LOG->MapLog( "UpdatePolled", hr_ssprintf(hr, "Failures on polled keyboard update") );
				return;
			}

			for( int k = 0; k < 256; ++k )
			{
				const DeviceButton key = (DeviceButton) device.Inputs[k].num;
				ButtonPressed(DeviceInput(device.dev, key), !!(keys[k] & 0x80));
			}
		}
		break;
	case device.JOYSTICK:
		{
			DIJOYSTATE state;

			HRESULT hr = GetDeviceState(device.Device, sizeof(state), &state);
			if( hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED )
				return;

			/* Set each known axis, button and POV. */
			for( unsigned i = 0; i < device.Inputs.size(); ++i )
			{
				const input_t &in = device.Inputs[i];
				const InputDevice dev = device.dev;

				switch(in.type)
				{
				case in.BUTTON:
				{
					DeviceInput di( dev, enum_add2(JOY_BUTTON_1, in.num), -1, tm );
					ButtonPressed( di, !!state.rgbButtons[in.ofs - DIJOFS_BUTTON0] );
					break;
				}

				case in.AXIS:
				{
					DeviceButton neg = DeviceButton_Invalid, pos = DeviceButton_Invalid;
					int val = 0;
					switch( in.ofs )
					{
					case DIJOFS_X:  neg = JOY_LEFT; pos = JOY_RIGHT;
									val = state.lX;
									break;
					case DIJOFS_Y:  neg = JOY_UP; pos = JOY_DOWN;
									val = state.lY;
									break;
					case DIJOFS_Z:  neg = JOY_Z_UP; pos = JOY_Z_DOWN;
									val = state.lZ;
									break;
					case DIJOFS_RX: neg = JOY_ROT_LEFT; pos = JOY_ROT_RIGHT;
									val = state.lRx;
									break;
					case DIJOFS_RY: neg = JOY_ROT_UP; pos = JOY_ROT_DOWN;
									val = state.lRy;
									break;
					case DIJOFS_RZ: neg = JOY_ROT_Z_UP; pos = JOY_ROT_Z_DOWN;
									val = state.lRz;
									break;
					case DIJOFS_SLIDER(0):
									neg = JOY_AUX_1; pos = JOY_AUX_2;
									val = state.rglSlider[0];
									break;
					case DIJOFS_SLIDER(1):
									neg = JOY_AUX_3; pos = JOY_AUX_4;
									val = state.rglSlider[1];
									break;
					default: LOG->MapLog( "unknown input", 
									"Controller '%s' is returning an unknown joystick offset, %i",
									device.JoystickInst.tszProductName, in.ofs );
							 continue;
					}
					if( neg != DeviceButton_Invalid )
					{
						float l = SCALE( int(val), 0.0f, 100.0f, 0.0f, 1.0f );
						ButtonPressed(DeviceInput(dev, neg, max(-l,0), tm), val < -50);
						ButtonPressed(DeviceInput(dev, pos, max(+l,0), tm), val > 50);
					}

					break;
				}

				case in.HAT:
					if( in.num == 0 )
					{
						const int pos = TranslatePOV( state.rgdwPOV[in.ofs - DIJOFS_POV(0)] );
						ButtonPressed( DeviceInput(dev, JOY_HAT_UP, -1, tm), !!(pos & HAT_UP_MASK) );
						ButtonPressed( DeviceInput(dev, JOY_HAT_DOWN, -1, tm), !!(pos & HAT_DOWN_MASK) );
						ButtonPressed( DeviceInput(dev, JOY_HAT_LEFT, -1, tm), !!(pos & HAT_LEFT_MASK) );
						ButtonPressed( DeviceInput(dev, JOY_HAT_RIGHT, -1, tm), !!(pos & HAT_RIGHT_MASK) );
					}

					break;
				}
			}
		}
		break;
	}
}

void InputHandler_DInput::UpdateBuffered( DIDevice &device, const RageTimer &tm )
{
	DWORD numevents;
	DIDEVICEOBJECTDATA evtbuf[INPUT_QSIZE];

	numevents = INPUT_QSIZE;
	HRESULT hr = device.Device->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), evtbuf, &numevents, 0 );
	if( hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED )
		return;

	/* Handle the events */
	if( hr != DI_OK )
	{
		LOG->Trace( hr_ssprintf(hr, "UpdateBuffered: IDirectInputDevice2_GetDeviceData") );
		return;
	}

	/* XXX: We should check GetConsoleWindow(), to allow input while the console window
	 * is focused. */
	if( GetForegroundWindow() != GraphicsWindow::GetHwnd() )
		return;

	for( int i = 0; i < (int) numevents; ++i )
	{
		for(unsigned j = 0; j < device.Inputs.size(); ++j)
		{
			const input_t &in = device.Inputs[j];
			const InputDevice dev = device.dev;

			if( evtbuf[i].dwOfs != in.ofs )
				continue;
		
			switch( in.type )
			{
			case in.KEY:
				ButtonPressed( DeviceInput(dev, (DeviceButton) in.num, -1, tm), !!(evtbuf[i].dwData & 0x80)) ;
				break;

			case in.BUTTON:
				ButtonPressed( DeviceInput(dev, enum_add2(JOY_BUTTON_1, in.num), -1, tm), !!evtbuf[i].dwData );
				break;

			case in.AXIS:
			{
				DeviceButton up = DeviceButton_Invalid, down = DeviceButton_Invalid;
				switch(in.ofs)
				{
				case DIJOFS_X:  up = JOY_LEFT; down = JOY_RIGHT; break;
				case DIJOFS_Y:  up = JOY_UP; down = JOY_DOWN; break;
				case DIJOFS_Z: up = JOY_Z_UP; down = JOY_Z_DOWN; break;
				case DIJOFS_RX: up = JOY_ROT_UP; down = JOY_ROT_DOWN; break;
				case DIJOFS_RY: up = JOY_ROT_LEFT; down = JOY_ROT_RIGHT; break;
				case DIJOFS_RZ: up = JOY_ROT_Z_UP; down = JOY_ROT_Z_DOWN; break;
				case DIJOFS_SLIDER(0): up = JOY_AUX_1; down = JOY_AUX_2; break;
				case DIJOFS_SLIDER(1): up = JOY_AUX_3; down = JOY_AUX_4; break;
				default: LOG->MapLog( "unknown input", 
							 "Controller '%s' is returning an unknown joystick offset, %i",
							 device.JoystickInst.tszProductName, in.ofs );
					continue;
				}

				float l = SCALE( int(evtbuf[i].dwData), 0.0f, 100.0f, 0.0f, 1.0f );
				ButtonPressed( DeviceInput(dev, up, max(-l,0), tm), int(evtbuf[i].dwData) < -50 );
				ButtonPressed( DeviceInput(dev, down, max(+l,0), tm), int(evtbuf[i].dwData) > 50 );
				break;
			}
			case in.HAT:
			{
				const int pos = TranslatePOV( evtbuf[i].dwData );
				ButtonPressed( DeviceInput(dev, JOY_HAT_UP, -1, tm), !!(pos & HAT_UP_MASK) );
				ButtonPressed( DeviceInput(dev, JOY_HAT_DOWN, -1, tm), !!(pos & HAT_DOWN_MASK) );
				ButtonPressed( DeviceInput(dev, JOY_HAT_LEFT, -1, tm), !!(pos & HAT_LEFT_MASK) );
				ButtonPressed( DeviceInput(dev, JOY_HAT_RIGHT, -1, tm), !!(pos & HAT_RIGHT_MASK) );
			}
			}
		}
	}
}


void InputHandler_DInput::PollAndAcquireDevices( bool bBuffered )
{
	for( unsigned i = 0; i < Devices.size(); ++i )
	{
		if( Devices[i].buffered != bBuffered )
			continue;

		HRESULT hr = IDirectInputDevice2_Poll( Devices[i].Device );
		if( hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED )
		{
			/* This will fail with "access denied" on the keyboard if we don't
			 * have focus. */
			hr = Devices[i].Device->Acquire();
			if( hr != DI_OK )
				continue;

			Devices[i].Device->Poll();
		}
	}
}

void InputHandler_DInput::Update()
{
	RageTimer zero;
	zero.SetZero();

	/* Handle polled devices.  Handle buffered, too, if there's no input thread to do it. */
	PollAndAcquireDevices( false );
	if( !m_Thread.IsCreated() )
		PollAndAcquireDevices( true );

	for( unsigned i = 0; i < Devices.size(); ++i )
	{
		if( !Devices[i].buffered )
		{
			UpdatePolled( Devices[i], zero );
		}
		else if( !m_Thread.IsCreated() )
		{
			/* If we have an input thread, it'll handle buffered devices. */
			UpdateBuffered( Devices[i], zero );
		}
	}

	InputHandler::UpdateTimer();
}

bool InputHandler_DInput::DevicesChanged()
{
	int iOldNumUsbHid = m_iLastSeenNumUsbHid;
	m_iLastSeenNumUsbHid = GetNumUsbHidDevices();
	return iOldNumUsbHid != m_iLastSeenNumUsbHid;
}

void InputHandler_DInput::InputThreadMain()
{
	if(!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST))
		LOG->Warn(werr_ssprintf(GetLastError(), "Failed to set DirectInput thread priority"));

	/* Enable priority boosting. */
	SetThreadPriorityBoost( GetCurrentThread(), FALSE );

	vector<DIDevice*> BufferedDevices;
	HANDLE Handle = CreateEvent( NULL, FALSE, FALSE, NULL );
	for( unsigned i = 0; i < Devices.size(); ++i )
	{
		if( !Devices[i].buffered )
			continue;
        
		BufferedDevices.push_back( &Devices[i] );

		Devices[i].Device->Unacquire();
		HRESULT hr = Devices[i].Device->SetEventNotification( Handle );
		if( FAILED(hr) )
			LOG->Warn( "IDirectInputDevice2_SetEventNotification failed on %i", i );
		Devices[i].Device->Acquire();
	}

	while( !m_bShutdown )
	{
		CHECKPOINT;
		if( BufferedDevices.size() )
		{
			/* Update buffered devices. */
			PollAndAcquireDevices( true );

			int ret = WaitForSingleObjectEx( Handle, 50, true );
			if( ret == -1 )
			{
				LOG->Trace( werr_ssprintf(GetLastError(), "WaitForSingleObjectEx failed") );
				continue;
			}

			if( ret == WAIT_OBJECT_0 )
			{
				RageTimer now;
				for( unsigned i = 0; i < BufferedDevices.size(); ++i )
					UpdateBuffered( *BufferedDevices[i], now );
			}
		}
		CHECKPOINT;

		/* If we have no buffered devices, we didn't delay at WaitForMultipleObjectsEx. */
		if( BufferedDevices.size() == 0 )
			usleep( 50000 );
		CHECKPOINT;
	}
	CHECKPOINT;

	for( unsigned i = 0; i < Devices.size(); ++i )
	{
		if( !Devices[i].buffered )
			continue;

		Devices[i].Device->IDirectInputDevice2_Unacquire();
		Devices[i].Device->SetEventNotification( NULL );
	}

	CloseHandle(Handle);
}

void InputHandler_DInput::GetDevicesAndDescriptions( vector<InputDevice>& vDevicesOut, vector<CString>& vDescriptionsOut )
{
	for( unsigned i=0; i < Devices.size(); ++i )
	{
		vDevicesOut.push_back( Devices[i].dev );
		vDescriptionsOut.push_back( Devices[i].JoystickInst.tszProductName );
	}
}

/*
 * (c) 2003-2004 Glenn Maynard
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

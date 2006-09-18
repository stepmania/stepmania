#include "global.h"
#include "InputHandler_DirectInput.h"

#include "RageUtil.h"
#include "RageLog.h"
#include "archutils/Win32/AppInstance.h"
#include "archutils/Win32/DirectXHelpers.h"
#include "archutils/Win32/ErrorStrings.h"
#include "archutils/Win32/GraphicsWindow.h"
#include "archutils/Win32/RegistryAccess.h"
#include "InputFilter.h"
#include "PrefsManager.h"

#include "InputHandler_DirectInputHelper.h"

static vector<DIDevice> Devices;

/* Number of joysticks found: */
static int g_iNumJoysticks;

static BOOL CALLBACK EnumDevicesCallback( const DIDEVICEINSTANCE *pdidInstance, void *pContext )
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


static BOOL CALLBACK CountDevicesCallback( const DIDEVICEINSTANCE *pdidInstance, void *pContext )
{
	(*(int*)pContext)++;
	return DIENUM_CONTINUE;
}

static int GetNumHidDevices()
{
	int i = 0;	
	RegistryAccess::GetRegValue( "HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\HidUsb\\Enum", "Count", i, false );	// don't warn on error
	return i;
}

static int GetNumJoysticksSlow()
{
	int iCount = 0;
	HRESULT hr = g_dinput->EnumDevices( DIDEVTYPE_JOYSTICK, CountDevicesCallback, &iCount, DIEDFL_ATTACHEDONLY );
	if( hr != DI_OK )
	{
		LOG->Warn( hr_ssprintf(hr, "g_dinput->EnumDevices") );
	}
	return iCount;
}

InputHandler_DInput::InputHandler_DInput()
{
	LOG->Trace( "InputHandler_DInput::InputHandler_DInput()" );

	CheckForDirectInputDebugMode();
	
	m_bShutdown = false;
	g_iNumJoysticks = 0;

	AppInstance inst;	
	HRESULT hr = DirectInputCreate(inst.Get(), DIRECTINPUT_VERSION, &g_dinput, NULL);
	if( hr != DI_OK )
		RageException::Throw( hr_ssprintf(hr, "InputHandler_DInput: DirectInputCreate") );

	LOG->Trace( "InputHandler_DInput: IDirectInput::EnumDevices(DIDEVTYPE_KEYBOARD)" );
	hr = g_dinput->EnumDevices( DIDEVTYPE_KEYBOARD, EnumDevicesCallback, NULL, DIEDFL_ATTACHEDONLY );
	if( hr != DI_OK )
		RageException::Throw( hr_ssprintf(hr, "InputHandler_DInput: IDirectInput::EnumDevices") );

	LOG->Trace( "InputHandler_DInput: IDirectInput::EnumDevices(DIDEVTYPE_JOYSTICK)" );
	hr = g_dinput->EnumDevices( DIDEVTYPE_JOYSTICK, EnumDevicesCallback, NULL, DIEDFL_ATTACHEDONLY );
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
			Devices[i].m_sName.c_str(),
			Devices[i].axes,
			Devices[i].hats,
			Devices[i].buttons,
			Devices[i].buffered? "buffered": "unbuffered" );
	}

	m_iLastSeenNumHidDevices = GetNumHidDevices();
	m_iNumTimesLeftToPollForJoysticksChanged = 0;
	m_iLastSeenNumJoysticks = GetNumJoysticksSlow();

	StartThread();
}

void InputHandler_DInput::StartThread()
{
	ASSERT( !m_InputThread.IsCreated() );
	if( PREFSMAN->m_bThreadedInput )
	{
		m_InputThread.SetName( "DirectInput thread" );
		m_InputThread.Create( InputThread_Start, this );
	}
}

void InputHandler_DInput::ShutdownThread()
{
	m_bShutdown = true;
	if( m_InputThread.IsCreated() )
	{
		LOG->Trace( "Shutting down DirectInput thread ..." );
		m_InputThread.Wait();
		LOG->Trace( "DirectInput thread shut down." );
	}
	m_bShutdown = false;
}

InputHandler_DInput::~InputHandler_DInput()
{
	ShutdownThread();

	for( unsigned i = 0; i < Devices.size(); ++i )
		Devices[i].Close();

	Devices.clear();
	g_dinput->Release();	
	g_dinput = NULL;
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
				ButtonPressed( DeviceInput(device.dev, key), !!(keys[k] & 0x80) );
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
									device.m_sName.c_str(), in.ofs );
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
	{
		INPUTFILTER->ResetDevice( device.dev );
		return;
	}

	if( hr != DI_OK )
	{
		LOG->Trace( hr_ssprintf(hr, "UpdateBuffered: IDirectInputDevice2_GetDeviceData") );
		return;
	}

	if( GetForegroundWindow() != GraphicsWindow::GetHwnd() )
	{
		/* Discard input when not focused, and release all keys. */
		INPUTFILTER->ResetDevice( device.dev );
		return;
	}

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
				/*
				switch( in.num )
				{
					// "Joystick with Keyboard" hack
				case 115:	//s
					ButtonPressed( DeviceInput(DEVICE_JOY1, JOY_UP, -1, tm), !!(evtbuf[i].dwData & 0x80) );
					break;
				case 120:	//x
					ButtonPressed( DeviceInput(DEVICE_JOY1, JOY_DOWN, -1, tm), !!(evtbuf[i].dwData & 0x80) );
					break;
				case 122:	//z
					ButtonPressed( DeviceInput(DEVICE_JOY1, JOY_LEFT, -1, tm), !!(evtbuf[i].dwData & 0x80) );
					break;
				case 99:	//c
					ButtonPressed( DeviceInput(DEVICE_JOY1, JOY_RIGHT, -1, tm), !!(evtbuf[i].dwData & 0x80) );
					break;
				case 100:	//d
					ButtonPressed( DeviceInput(DEVICE_JOY1, JOY_BUTTON_1, -1, tm), !!(evtbuf[i].dwData & 0x80) );
					break;
				case 101:	//e
					ButtonPressed( DeviceInput(DEVICE_JOY1, JOY_BUTTON_2, -1, tm), !!(evtbuf[i].dwData & 0x80) );
					break;
				default:
					*/
					ButtonPressed( DeviceInput(dev, (DeviceButton) in.num, -1, tm), !!(evtbuf[i].dwData & 0x80)) ;
					/*
					break;
				}
				*/
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
							 device.m_sName.c_str(), in.ofs );
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
			INPUTFILTER->ResetDevice( Devices[i].dev );

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
	/* Handle polled devices.  Handle buffered, too, if there's no input thread to do it. */
	PollAndAcquireDevices( false );
	if( !m_InputThread.IsCreated() )
		PollAndAcquireDevices( true );

	for( unsigned i = 0; i < Devices.size(); ++i )
	{
		if( !Devices[i].buffered )
		{
			UpdatePolled( Devices[i], RageZeroTimer );
		}
		else if( !m_InputThread.IsCreated() )
		{
			/* If we have an input thread, it'll handle buffered devices. */
			UpdateBuffered( Devices[i], RageZeroTimer );
		}
	}

	InputHandler::UpdateTimer();
}

const float POLL_FOR_JOYSTICK_CHANGES_LENGTH_SECONDS = 15.0f;
const float POLL_FOR_JOYSTICK_CHANGES_EVERY_SECONDS = 0.25f;

bool InputHandler_DInput::DevicesChanged()
{
	//
	// GetNumJoysticksSlow() blocks DirectInput for a while even if called from a 
	// different thread, so we can't poll with it.
	// GetNumHidDevices() is fast, but sometimes the DirectInput joysticks haven't updated by 
	// the time the HID registry value changes.
	// So, poll using GetNumHidDevices().   When that changes, poll using GetNumJoysticksSlow() 
	// for a little while to give DirectInput time to catch up.  On this XP machine, it takes
	// 2-10 DirectInput polls (0.5-2.5 seconds) to catch a newly installed device after the 
	// registry value changes, and catches non-new plugged/unplugged devices on the first 
	// DirectInputPoll.
	// Note that this "poll for N seconds" method will not work if the Add New Hardware wizard
	// halts device installation to wait for a driver.  Most of the joysticks people would
	// want to use don't prompt for a driver though and the wizard adds them pretty quickly.
	//

	int iOldNumHidDevices = m_iLastSeenNumHidDevices;
	m_iLastSeenNumHidDevices = GetNumHidDevices();
	if( iOldNumHidDevices != m_iLastSeenNumHidDevices )
	{
		LOG->Warn( "HID devices changes" );
		m_iNumTimesLeftToPollForJoysticksChanged = (int)(POLL_FOR_JOYSTICK_CHANGES_LENGTH_SECONDS / POLL_FOR_JOYSTICK_CHANGES_EVERY_SECONDS);
	}

	if( m_iNumTimesLeftToPollForJoysticksChanged > 0 )
	{
		static RageTimer timerPollJoysticks;
		if( timerPollJoysticks.Ago() >= POLL_FOR_JOYSTICK_CHANGES_EVERY_SECONDS )
		{
			m_iNumTimesLeftToPollForJoysticksChanged--;
			timerPollJoysticks.Touch();
			LOG->Warn( "polling for joystick changes" );

			int iOldNumJoysticks = m_iLastSeenNumJoysticks;
			m_iLastSeenNumJoysticks = GetNumJoysticksSlow();
			if( iOldNumJoysticks != m_iLastSeenNumJoysticks )
			{
				LOG->Warn( "joysticks changed" );
				m_iNumTimesLeftToPollForJoysticksChanged = 0;
				return true;
			}
		}
	}

	return false;
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

			/* Update devices even if no event was triggered, since this also checks for focus
			 * loss. */
			RageTimer now;
			for( unsigned i = 0; i < BufferedDevices.size(); ++i )
				UpdateBuffered( *BufferedDevices[i], now );
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

		Devices[i].Device->Unacquire();
		Devices[i].Device->SetEventNotification( NULL );
	}

	CloseHandle(Handle);
}

void InputHandler_DInput::GetDevicesAndDescriptions( vector<InputDeviceInfo>& vDevicesOut )
{
	for( unsigned i=0; i < Devices.size(); ++i )
		vDevicesOut.push_back( InputDeviceInfo(Devices[i].dev, Devices[i].m_sName) );
}

static wchar_t ScancodeAndKeysToChar( DWORD scancode, unsigned char keys[256] )
{
	static HKL layout = GetKeyboardLayout(0);	// 0 == current thread
	UINT vk = MapVirtualKeyEx( scancode, 1, layout );
	
	unsigned short result[2];	// ToAscii writes a max of 2 chars
	ZERO( result );
	// TODO: Use ToUnicodeEx
	int iNum = ToAsciiEx( vk, scancode, keys, result, 0, layout );
	// iNum == 2 will happen only for dead keys.  See MSDN for ToAsciiEx.
	if( iNum == 1 )
	{
		RString s = RString()+(char)result[0];
		return ConvertCodepageToWString( s, CP_ACP )[0];
	}
	return '\0';
}

wchar_t InputHandler_DInput::DeviceButtonToChar( DeviceButton button, bool bUseCurrentKeyModifiers )
{
	// ToAsciiEx maps these keys to a character.  They shouldn't be mapped to any character.
	switch( button )
	{
	case KEY_ESC:
	case KEY_TAB:
	case KEY_ENTER:
	case KEY_BACK:
		return '\0';
	}

	FOREACH_CONST( DIDevice, Devices, d )
	{
		if( d->type != DIDevice::KEYBOARD )
			continue;

		FOREACH_CONST( input_t, d->Inputs, i )
		{
			if( button != i->num )
				continue;

			unsigned char keys[256];
			ZERO( keys );
			if( bUseCurrentKeyModifiers )
				GetKeyboardState(keys);
			wchar_t c = ScancodeAndKeysToChar( i->ofs, keys );
			if( c )
				return c;
		}
	}

	return InputHandler::DeviceButtonToChar( button, bUseCurrentKeyModifiers );
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

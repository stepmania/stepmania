/* Completely standard-issue DirectInput code.  Written by Glenn Maynard; in the
 * public domain. */

#include "global.h"
#include "InputHandler_DirectInput.h"

#include "RageUtil.h"
#include "RageLog.h"
#include "SDL_utils.h"
#include "archutils/Win32/AppInstance.h"
#include "InputFilter.h"
#include "PrefsManager.h"
#include "archutils/win32/tls.h"

#include "InputHandler_DirectInputHelper.h"

static vector<DIDevice> Devices;

/* Number of joysticks found: */
static int g_NumJoysticks;

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
		if( g_NumJoysticks == NUM_JOYSTICKS )
			return DIENUM_CONTINUE;

		device.dev = InputDevice( DEVICE_JOY1 + g_NumJoysticks );
		g_NumJoysticks++;
		break;

	case device.KEYBOARD:
		device.dev = InputDevice( DEVICE_KEYBOARD );
		break;
	}

	Devices.push_back(device);

	return DIENUM_CONTINUE;
}

static void CheckForDirectInputDebugMode()
{
	HKEY    hkey;
	if (RegOpenKey(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\DirectInput", &hkey) != ERROR_SUCCESS)
		return;

	long val;
	DWORD nSize = sizeof(val);
	if( RegQueryValueEx(hkey, "emulation", NULL, NULL, (LPBYTE)&val, &nSize) == ERROR_SUCCESS ) 
	{
		if( val&0x8 )
			LOG->Warn("DirectInput keyboard debug mode appears to be enabled.  This reduces\n"
			          "input timing accuracy significantly.  Disabling this is strongly recommended." );
	}
	RegCloseKey(hkey);
}

InputHandler_DInput::InputHandler_DInput()
{
	LOG->Trace( "InputHandler_DInput::InputHandler_DInput()" );

	CheckForDirectInputDebugMode();
	
	shutdown = false;
	g_NumJoysticks = 0;

	AppInstance inst;	
	HRESULT hr = DirectInputCreate(/* SDL_Instance */inst.Get(), DIRECTINPUT_VERSION, &dinput, NULL);
	if ( hr != DI_OK )
		RageException::Throw( hr_ssprintf(hr, "InputHandler_DInput: DirectInputCreate") );

	LOG->Trace( "InputHandler_DInput::IDirectInput_EnumDevices(DIDEVTYPE_KEYBOARD)" );
	hr = IDirectInput_EnumDevices(dinput, DIDEVTYPE_KEYBOARD, EnumDevices, NULL, DIEDFL_ATTACHEDONLY );
	if ( hr != DI_OK )
		RageException::Throw( hr_ssprintf(hr, "InputHandler_DInput: IDirectInput_EnumDevices") );

	LOG->Trace( "InputHandler_DInput::IDirectInput_EnumDevices(DIDEVTYPE_JOYSTICK)" );
	hr = IDirectInput_EnumDevices(dinput, DIDEVTYPE_JOYSTICK, EnumDevices, NULL, DIEDFL_ATTACHEDONLY );
	if ( hr != DI_OK )
		RageException::Throw( hr_ssprintf(hr, "InputHandler_DInput: IDirectInput_EnumDevices") );

	unsigned i;
	for( i = 0; i < Devices.size(); ++i )
	{
		if( Devices[i].Open() )
			continue;

		Devices.erase( Devices.begin() + i );
		i--;
		continue;
	}

	LOG->Info( "Found %u DirectInput devices:", Devices.size() );
	for( i = 0; i < Devices.size(); ++i )
	{
		LOG->Info( "   %d: '%s' axes: %d, hats: %d, buttons: %d (%s)",
			i,
			Devices[i].JoystickInst.tszProductName,
			Devices[i].axes,
			Devices[i].hats,
			Devices[i].buttons,
			Devices[i].buffered? "buffered": "unbuffered" );
	}

	InputThreadPtr = NULL;
	StartThread();
}

void InputHandler_DInput::StartThread()
{
	ASSERT( InputThreadPtr == NULL );
	if( PREFSMAN->m_bThreadedInput )
		InputThreadPtr = SDL_CreateThread(InputThread_Start, this);
	else
		InputThreadPtr = NULL;
}

void InputHandler_DInput::ShutdownThread()
{
	if( InputThreadPtr == NULL )
		return;

	shutdown = true;
	LOG->Trace("Shutting down DirectInput thread ...");
	SDL_WaitThread(InputThreadPtr, NULL);
	InputThreadPtr = NULL;
	LOG->Trace("DirectInput thread shut down.");
	shutdown = false;
}

InputHandler_DInput::~InputHandler_DInput()
{
	ShutdownThread();

	for( unsigned i = 0; i < Devices.size(); ++i )
		Devices[i].Close();

	Devices.clear();
	IDirectInput_Release(dinput);
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

static int TranslatePOV(DWORD value)
{
	const int HAT_VALS[] = {
	    SDL_HAT_UP,
	    SDL_HAT_UP   | SDL_HAT_RIGHT,
	    SDL_HAT_RIGHT,
	    SDL_HAT_DOWN | SDL_HAT_RIGHT,
	    SDL_HAT_DOWN,
	    SDL_HAT_DOWN | SDL_HAT_LEFT,
	    SDL_HAT_LEFT,
	    SDL_HAT_UP   | SDL_HAT_LEFT
	};

	if(LOWORD(value) == 0xFFFF)
	    return SDL_HAT_CENTERED;

	/* Round the value up: */
	value += 4500 / 2;
	value %= 36000;
	value /= 4500;

	if(value >= 8)
	    return SDL_HAT_CENTERED; /* shouldn't happen */
	
	return HAT_VALS[value];
}

HRESULT GetDeviceState(LPDIRECTINPUTDEVICE2 dev, int size, void *ptr)
{
	HRESULT hr = IDirectInputDevice2_GetDeviceState(dev, size, ptr);
	if ( hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED ) {
		hr = IDirectInputDevice2_Acquire(dev);
		if ( hr != DI_OK )
		{
			LOG->Trace( hr_ssprintf(hr, "?") );
			return hr;
		}

		hr = IDirectInputDevice2_GetDeviceState(dev, size, ptr);
	}

	return hr;
}

/* This doesn't take a timestamp; instead, we let InputHandler::ButtonPressed figure
 * it out.  Be sure to call InputHandler::Update() between each poll. */
void InputHandler_DInput::UpdatePolled(DIDevice &device, const RageTimer &tm)
{
	if( device.type == device.KEYBOARD )
	{
		unsigned char keys[256];

		HRESULT hr = GetDeviceState(device.Device, 256, keys);
		if ( hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED )
			return;

		if ( hr != DI_OK )
		{
			LOG->MapLog( "UpdatePolled", hr_ssprintf(hr, "Failures on polled keyboard update") );
			return;
		}

		for( int k = 0; k < 256; ++k )
		{
			const int key = device.Inputs[k].num;
			ButtonPressed(DeviceInput(device.dev, key), !!(keys[k] & 0x80));
		}
		return;
	}

	DIJOYSTATE state;

	HRESULT hr = GetDeviceState(device.Device, sizeof(state), &state);
	if ( hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED )
		return;

	/* Set each known axis, button and POV. */
	for(unsigned i = 0; i < device.Inputs.size(); ++i)
	{
		const input_t &in = device.Inputs[i];
		const InputDevice dev = device.dev;

		switch(in.type)
		{
		case in.BUTTON:
		{
			DeviceInput di(dev, JOY_1 + in.num, tm);
			ButtonPressed(di, !!state.rgbButtons[in.ofs - DIJOFS_BUTTON0]);
			break;
		}

		case in.AXIS:
		{
			switch(in.ofs)
			{
			case DIJOFS_X:  ButtonPressed(DeviceInput(dev, JOY_LEFT, tm), state.lX < -50);
							ButtonPressed(DeviceInput(dev, JOY_RIGHT, tm), state.lX > 50);
							break;
			case DIJOFS_Y:  ButtonPressed(DeviceInput(dev, JOY_UP, tm), state.lY < -50);
							ButtonPressed(DeviceInput(dev, JOY_DOWN, tm), state.lY > 50);
							break;
			case DIJOFS_Z:  ButtonPressed(DeviceInput(dev, JOY_Z_UP, tm), state.lZ < -50);
							ButtonPressed(DeviceInput(dev, JOY_Z_DOWN, tm), state.lZ > 50);
							break;
			case DIJOFS_RX: ButtonPressed(DeviceInput(dev, JOY_ROT_LEFT, tm), state.lRx < -50);
							ButtonPressed(DeviceInput(dev, JOY_ROT_RIGHT, tm), state.lRx > 50);
							break;
			case DIJOFS_RY: ButtonPressed(DeviceInput(dev, JOY_ROT_UP, tm), state.lRy < -50);
							ButtonPressed(DeviceInput(dev, JOY_ROT_DOWN, tm), state.lRy > 50);
							break;
			case DIJOFS_RZ: ButtonPressed(DeviceInput(dev, JOY_ROT_Z_UP, tm), state.lRz < -50);
							ButtonPressed(DeviceInput(dev, JOY_ROT_Z_DOWN, tm), state.lRz > 50);
							break;
			case DIJOFS_SLIDER(0):
							ButtonPressed(DeviceInput(dev, JOY_AUX_1, tm), state.rglSlider[0] < -50);
							ButtonPressed(DeviceInput(dev, JOY_AUX_2, tm), state.rglSlider[0] > 50);
							break;
			case DIJOFS_SLIDER(1):
							ButtonPressed(DeviceInput(dev, JOY_AUX_3, tm), state.rglSlider[1] < -50);
							ButtonPressed(DeviceInput(dev, JOY_AUX_4, tm), state.rglSlider[1] > 50);
							break;
			default: LOG->MapLog("unknown input", 
							"Controller '%s' is returning an unknown joystick offset, %i",
							device.JoystickInst.tszProductName, in.ofs );
					 continue;
			}

			break;
		}

		case in.HAT:
		{
			ASSERT( in.num == 0 ); // XXX

			const int pos = TranslatePOV(state.rgdwPOV[in.ofs - DIJOFS_POV(0)]);
			ButtonPressed(DeviceInput(dev, JOY_HAT_UP, tm), !!(pos & SDL_HAT_UP));
			ButtonPressed(DeviceInput(dev, JOY_HAT_DOWN, tm), !!(pos & SDL_HAT_DOWN));
			ButtonPressed(DeviceInput(dev, JOY_HAT_LEFT, tm), !!(pos & SDL_HAT_LEFT));
			ButtonPressed(DeviceInput(dev, JOY_HAT_RIGHT, tm), !!(pos & SDL_HAT_RIGHT));

			break;
		}
		}
	}
}

void InputHandler_DInput::UpdateBuffered(DIDevice &device, const RageTimer &tm)
{
	DWORD numevents;
	DIDEVICEOBJECTDATA evtbuf[INPUT_QSIZE];

	numevents = INPUT_QSIZE;
	HRESULT hr = IDirectInputDevice2_GetDeviceData( device.Device, sizeof(DIDEVICEOBJECTDATA), evtbuf, &numevents, 0);
	if ( hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED )
		return;

	/* Handle the events */
	if ( hr != DI_OK )
	{
		LOG->Trace( hr_ssprintf(hr, "UpdateBuffered: IDirectInputDevice2_GetDeviceData") );
		return;
	}

	for(int i = 0; i < (int) numevents; ++i)
	{
		for(unsigned j = 0; j < device.Inputs.size(); ++j)
		{
			const input_t &in = device.Inputs[j];
			const InputDevice dev = device.dev;

			if(evtbuf[i].dwOfs != in.ofs)
				continue;
		
			switch(in.type)
			{
			case in.KEY:
				ButtonPressed(DeviceInput(dev, in.num, tm), !!(evtbuf[i].dwData & 0x80));
				break;

			case in.BUTTON:
				ButtonPressed(DeviceInput(dev, JOY_1 + in.num, tm), !!evtbuf[i].dwData);
				break;

			case in.AXIS:
			{
				int up = 0, down = 0;
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
				default: LOG->MapLog("unknown input", 
							 "Controller '%s' is returning an unknown joystick offset, %i",
							 device.JoystickInst.tszProductName, in.ofs );
					continue;
				}

				ButtonPressed(DeviceInput(dev, up, tm), int(evtbuf[i].dwData) < -50);
				ButtonPressed(DeviceInput(dev, down, tm), int(evtbuf[i].dwData) > 50);
				break;
			}
			case in.HAT:
		    {
				const int pos = TranslatePOV(evtbuf[i].dwData);
				ButtonPressed(DeviceInput(dev, JOY_HAT_UP, tm), !!(pos & SDL_HAT_UP));
				ButtonPressed(DeviceInput(dev, JOY_HAT_DOWN, tm), !!(pos & SDL_HAT_DOWN));
				ButtonPressed(DeviceInput(dev, JOY_HAT_LEFT, tm), !!(pos & SDL_HAT_LEFT));
				ButtonPressed(DeviceInput(dev, JOY_HAT_RIGHT, tm), !!(pos & SDL_HAT_RIGHT));
		    }
			}
		}
	}
}


void InputHandler_DInput::PollAndAcquireDevices()
{
	for( unsigned i = 0; i < Devices.size(); ++i )
	{
		HRESULT hr = IDirectInputDevice2_Poll( Devices[i].Device );
		if ( hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED )
		{
			/* This will fail with "access denied" on the keyboard if we don't
			 * have focus. */
			hr = IDirectInputDevice2_Acquire( Devices[i].Device );
			if ( hr != DI_OK )
				continue;

			IDirectInputDevice2_Poll( Devices[i].Device );
		}
	}
}

void InputHandler_DInput::Update(float fDeltaTime)
{
	RageTimer zero;
	zero.SetZero();

	/* Handle polled devices. */
	PollAndAcquireDevices();
	for( unsigned i = 0; i < Devices.size(); ++i )
	{
		if( !Devices[i].buffered )
			UpdatePolled( Devices[i], zero );
		else if( InputThreadPtr == NULL )
		{
			/* If we have an input thread, it'll handle buffered devices. */
			UpdateBuffered( Devices[i], zero );
		}
	}

	InputHandler::UpdateTimer();
}


void InputHandler_DInput::InputThread()
{
	InitThreadData("DirectInput thread");
	VDCHECKPOINT;
	if(!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST))
		LOG->Warn(werr_ssprintf(GetLastError(), "Failed to set DirectInput thread priority"));

	unsigned i;
	vector<DIDevice*> BufferedDevices, UnbufferedDevices;
	HANDLE Handle = CreateEvent(NULL, FALSE, FALSE, NULL);
	for( i = 0; i < Devices.size(); ++i )
	{
		if( !Devices[i].buffered )
		{
			UnbufferedDevices.push_back( &Devices[i] );
			continue;
		}
        
		BufferedDevices.push_back( &Devices[i] );

		IDirectInputDevice2_Unacquire(Devices[i].Device);
		HRESULT hr = IDirectInputDevice2_SetEventNotification(Devices[i].Device, Handle);
		if( FAILED(hr) )
			LOG->Warn("IDirectInputDevice2_SetEventNotification failed on %i", i);
		IDirectInputDevice2_Acquire(Devices[i].Device);
	}

	while(!shutdown)
	{
		VDCHECKPOINT;
		if( BufferedDevices.size() )
		{
			/* Update buffered devices. */
			PollAndAcquireDevices();

			int ret = WaitForSingleObjectEx( Handle, 50, true );
			if( ret == -1 )
			{
				LOG->Trace( werr_ssprintf(GetLastError(), "WaitForSingleObjectEx failed") );
				continue;
			}

			if( ret == WAIT_OBJECT_0 )
			{
				RageTimer now;
				for( i = 0; i < BufferedDevices.size(); ++i )
					UpdateBuffered( *BufferedDevices[i], now );
			}
		}
		VDCHECKPOINT;

		/* If we have no buffered devices, we didn't delay at WaitForMultipleObjectsEx. */
		if( BufferedDevices.size() == 0 )
			SDL_Delay( 50 );
		VDCHECKPOINT;
	}
	VDCHECKPOINT;

	for( i = 0; i < Devices.size(); ++i )
	{
		if( !Devices[i].buffered )
			continue;

		IDirectInputDevice2_Unacquire(Devices[i].Device);
        IDirectInputDevice2_SetEventNotification( Devices[i].Device, NULL );
	}

	CloseHandle(Handle);
}

void InputHandler_DInput::GetDevicesAndDescriptions(vector<InputDevice>& vDevicesOut, vector<CString>& vDescriptionsOut)
{
	for( unsigned i=0; i < Devices.size(); i++ )
	{
		vDevicesOut.push_back( Devices[i].dev );
		vDescriptionsOut.push_back( Devices[i].JoystickInst.tszProductName );
	}
}

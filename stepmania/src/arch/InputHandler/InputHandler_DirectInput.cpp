/* Completely standard-issue DirectInput code.  Written by Glenn Maynard; in the
 * public domain. */

#include "global.h"
#include "InputHandler_DirectInput.h"

#include "RageUtil.h"
#include "RageLog.h"
#include "SDL_utils.h"
#include "archutils/Win32/AppInstance.h"
#include "InputFilter.h"

#include "InputHandler_DirectInputHelper.h"

/*extern "C" {
extern HINSTANCE __declspec(dllimport) SDL_Instance;
};*/

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

InputHandler_DInput::InputHandler_DInput()
{
	LOG->Trace( "InputHandler_DInput::InputHandler_DInput()" );
	g_NumJoysticks = 0;
	AppInstance inst;
	
	HRESULT hr = DirectInputCreate(/* SDL_Instance */inst.Get(), DIRECTINPUT_VERSION, &dinput, NULL);
	if ( hr != DI_OK )
		RageException::Throw( hr_ssprintf(hr, "InputHandler_DInput: DirectInputCreate") );

	LOG->Trace( "InputHandler_DInput::IDirectInput_EnumDevices()" );
	hr = IDirectInput_EnumDevices(dinput, 0 /*DIDEVTYPE_KEYBOARD*/, EnumDevices, NULL, DIEDFL_ATTACHEDONLY );
	if ( hr != DI_OK )
		RageException::Throw( hr_ssprintf(hr, "InputHandler_DInput: IDirectInput_EnumDevices") );

//	hr = IDirectInput_EnumDevices(dinput, DIDEVTYPE_JOYSTICK, EnumDevices, NULL, DIEDFL_ATTACHEDONLY );
//	if ( hr != DI_OK )
//		RageException::Throw( hr_ssprintf(hr, "InputHandler_DInput: IDirectInput_EnumDevices") );

	unsigned i;
	for( i = 0; i < Devices.size(); ++i )
	{
		if( OpenDevice( Devices[i] ) )
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
}


/* device.JoystickInst and device.dev must be filled in.  Opens the device and
 * fills in the remaining parameters. */
bool InputHandler_DInput::OpenDevice(DIDevice &device)
{
	return device.Open();
}


InputHandler_DInput::~InputHandler_DInput()
{
	for( unsigned i = 0; i < Devices.size(); ++i )
		Devices[i].Close();

	Devices.clear();
	IDirectInput_Release(dinput);
	dinput = NULL;
}

void InputHandler_DInput::WindowReset()
{
	/* We need to reopen keyboards. */
	for( unsigned i = 0; i < Devices.size(); ++i )
	{
		if( Devices[i].type != Devices[i].KEYBOARD )
			continue;

		Devices[i].Close();
		bool ret = Devices[i].Open();

		/* Reopening it should succeed. */
		ASSERT( ret );
	}
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

void UpdatePolled(DIDevice &device)
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
			INPUTFILTER->ButtonPressed(DeviceInput(device.dev, key), !!(keys[k] & 0x80));
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
			DeviceInput di(dev, JOY_1 + in.num);
			INPUTFILTER->ButtonPressed(di, !!state.rgbButtons[in.ofs - DIJOFS_BUTTON0]);
			break;
		}

		case in.AXIS:
		{
			switch(in.ofs)
			{
			case DIJOFS_X:  INPUTFILTER->ButtonPressed(DeviceInput(dev, JOY_LEFT), state.lX < -50);
							INPUTFILTER->ButtonPressed(DeviceInput(dev, JOY_RIGHT), state.lX > 50);
							break;
			case DIJOFS_Y:  INPUTFILTER->ButtonPressed(DeviceInput(dev, JOY_UP), state.lY < -50);
							INPUTFILTER->ButtonPressed(DeviceInput(dev, JOY_DOWN), state.lY > 50);
							break;
			case DIJOFS_Z: INPUTFILTER->ButtonPressed(DeviceInput(dev, JOY_Z_UP), state.lZ < -50);
							INPUTFILTER->ButtonPressed(DeviceInput(dev, JOY_Z_DOWN), state.lZ > 50);
							break;
			case DIJOFS_RX: INPUTFILTER->ButtonPressed(DeviceInput(dev, JOY_ROT_LEFT), state.lRx < -50);
							INPUTFILTER->ButtonPressed(DeviceInput(dev, JOY_ROT_RIGHT), state.lRx > 50);
							break;
			case DIJOFS_RY: INPUTFILTER->ButtonPressed(DeviceInput(dev, JOY_ROT_UP), state.lRy < -50);
							INPUTFILTER->ButtonPressed(DeviceInput(dev, JOY_ROT_DOWN), state.lRy > 50);
							break;
			case DIJOFS_RZ: INPUTFILTER->ButtonPressed(DeviceInput(dev, JOY_ROT_Z_UP), state.lRz < -50);
							INPUTFILTER->ButtonPressed(DeviceInput(dev, JOY_ROT_Z_DOWN), state.lRz > 50);
							break;
			}

			break;
		}

		case in.HAT:
		{
			ASSERT( in.num == 1 ); // XXX

			const int pos = TranslatePOV(state.rgdwPOV[in.ofs - DIJOFS_POV(0)]);
			INPUTFILTER->ButtonPressed(DeviceInput(dev, JOY_HAT_UP), !!(pos & SDL_HAT_UP));
			INPUTFILTER->ButtonPressed(DeviceInput(dev, JOY_HAT_DOWN), !!(pos & SDL_HAT_DOWN));
			INPUTFILTER->ButtonPressed(DeviceInput(dev, JOY_HAT_LEFT), !!(pos & SDL_HAT_LEFT));
			INPUTFILTER->ButtonPressed(DeviceInput(dev, JOY_HAT_RIGHT), !!(pos & SDL_HAT_RIGHT));

			break;
		}
		}
	}
}

void UpdateBuffered(DIDevice &device)
{
	DWORD numevents;
	DIDEVICEOBJECTDATA evtbuf[INPUT_QSIZE];

	numevents = INPUT_QSIZE;
	HRESULT hr = IDirectInputDevice2_GetDeviceData(
			device.Device, sizeof(DIDEVICEOBJECTDATA),
						evtbuf, &numevents, 0);
	if ( hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED )
	{
		hr = IDirectInputDevice2_Acquire(device.Device);
		hr = IDirectInputDevice2_GetDeviceData(
			device.Device, sizeof(DIDEVICEOBJECTDATA),
						evtbuf, &numevents, 0);
	}

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
				INPUTFILTER->ButtonPressed(DeviceInput(dev, in.num), !!(evtbuf[i].dwData & 0x80));
				break;

			case in.BUTTON:
				INPUTFILTER->ButtonPressed(DeviceInput(dev, JOY_1 + in.num), !!evtbuf[i].dwData);
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
				default: ASSERT(0);
				}

				INPUTFILTER->ButtonPressed(DeviceInput(dev, up), int(evtbuf[i].dwData) < -50);
				INPUTFILTER->ButtonPressed(DeviceInput(dev, down), int(evtbuf[i].dwData) > 50);
				break;
			}
			case in.HAT:
		    {
				const int pos = TranslatePOV(evtbuf[i].dwData);
				INPUTFILTER->ButtonPressed(DeviceInput(dev, JOY_HAT_UP), !!(pos & SDL_HAT_UP));
				INPUTFILTER->ButtonPressed(DeviceInput(dev, JOY_HAT_DOWN), !!(pos & SDL_HAT_DOWN));
				INPUTFILTER->ButtonPressed(DeviceInput(dev, JOY_HAT_LEFT), !!(pos & SDL_HAT_LEFT));
				INPUTFILTER->ButtonPressed(DeviceInput(dev, JOY_HAT_RIGHT), !!(pos & SDL_HAT_RIGHT));
		    }
			}
		}
	}
}


void InputHandler_DInput::Update(float fDeltaTime)
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

		if( Devices[i].buffered )
			UpdateBuffered( Devices[i] );
		else
			UpdatePolled( Devices[i] );
	}
}

void InputHandler_DInput::GetDevicesAndDescriptions(vector<InputDevice>& vDevicesOut, vector<CString>& vDescriptionsOut)
{
	for( unsigned i=0; i < Devices.size(); i++ )
	{
		vDevicesOut.push_back( Devices[i].dev );
		vDescriptionsOut.push_back( Devices[i].JoystickInst.tszProductName );
	}
}

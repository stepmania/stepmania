/* Completely standard-issue DirectInput code.  Written by Glenn Maynard; in the
 * public domain. */

#include "global.h"
#include "InputHandler_DirectInput.h"

#include "RageUtil.h"
#include "RageLog.h"
#include "SDL_utils.h"
#include "archutils/Win32/AppInstance.h"
#include "InputFilter.h"

#define DIRECTINPUT_VERSION 0x0500
#include <dinput.h>

#define INPUT_QSIZE	32		/* Buffer up to 32 input messages */

static LPDIRECTINPUT dinput = NULL;

#pragma comment(lib, "dinput.lib")
/*extern "C" {
extern HINSTANCE __declspec(dllimport) SDL_Instance;
};*/

typedef struct input_t
{
	/* DirectInput offset for this input type: */
	DWORD ofs;

	/* Button, axis or hat: */
	enum Type { KEY, BUTTON, AXIS, HAT } type;

	int num;
} input_t;

struct DIDevice
{
	DIDEVICEINSTANCE JoystickInst;
	LPDIRECTINPUTDEVICE2 Device;

	enum { KEYBOARD, JOYSTICK } type;

	bool buffered;
	int buttons, axes, hats;
	vector<input_t> Inputs;
	InputDevice dev;
	
	DIDevice()
	{
		buttons = axes = hats = 0;
		dev = DEVICE_NONE;
		buffered = true;
		memset(&JoystickInst, 0, sizeof(JoystickInst));
		Device = NULL;
	}
};

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
	g_NumJoysticks = 0;
	AppInstance inst;
	
	HRESULT hr = DirectInputCreate(/* SDL_Instance */inst.Get(), DIRECTINPUT_VERSION, &dinput, NULL);
	if ( hr != DI_OK )
		RageException::Throw( hr_ssprintf(hr, "InputHandler_DInput: DirectInputCreate") );

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

static HWND GetHwnd()
{
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	if( SDL_GetWMInfo(&info) < 0 ) 
		RageException::Throw( "SDL_GetWMInfo failed" );

	return info.window;
}

static BOOL CALLBACK DIJoystick_EnumDevObjectsProc(LPCDIDEVICEOBJECTINSTANCE dev,
						    LPVOID data)
{
	DIDevice *device = (DIDevice *) data;
	HRESULT hr;

	input_t in;
	const int SupportedMask = DIDFT_BUTTON | DIDFT_POV | DIDFT_AXIS;
	if(!(dev->dwType & SupportedMask))
	    return DIENUM_CONTINUE; /* unsupported */

	in.ofs = dev->dwOfs;

	if(dev->dwType & DIDFT_BUTTON) {
		if( device->buttons == 24 )
			return DIENUM_CONTINUE; /* too many buttons */

		in.type = in.BUTTON;
		in.num = device->buttons;
		device->buttons++;
	} else if(dev->dwType & DIDFT_POV) {
		in.type = in.HAT;
		in.num = device->hats;
		device->hats++;
	} else { /* dev->dwType & DIDFT_AXIS */
		DIPROPRANGE diprg;
		DIPROPDWORD dilong;
		
		in.type = in.AXIS;
		in.num = device->axes;
		
		diprg.diph.dwSize		= sizeof(diprg);
		diprg.diph.dwHeaderSize	= sizeof(diprg.diph);
		diprg.diph.dwObj		= dev->dwOfs;
		diprg.diph.dwHow		= DIPH_BYOFFSET;
		diprg.lMin				= -100;
		diprg.lMax				= 100;

		hr = IDirectInputDevice2_SetProperty(device->Device, DIPROP_RANGE, &diprg.diph);
		if ( hr != DI_OK )
			return DIENUM_CONTINUE; /* don't use this axis */
	
		/* Set dead zone to 0. */
		dilong.diph.dwSize		= sizeof(dilong);
		dilong.diph.dwHeaderSize	= sizeof(dilong.diph);
		dilong.diph.dwObj		= dev->dwOfs;
		dilong.diph.dwHow		= DIPH_BYOFFSET;
		dilong.dwData = 0;
		hr = IDirectInputDevice2_SetProperty(device->Device, DIPROP_DEADZONE, &dilong.diph);
		if ( hr != DI_OK )
			return DIENUM_CONTINUE; /* don't use this axis */

		device->axes++;
	}

	device->Inputs.push_back(in);

	return DIENUM_CONTINUE;
}

static int ConvertScancodeToKey( int scancode )
{
	switch(scancode)
	{
	case DIK_ESCAPE: return SDLK_ESCAPE;
	case DIK_1: return SDLK_1;
	case DIK_2: return SDLK_2;
	case DIK_3: return SDLK_3;
	case DIK_4: return SDLK_4;
	case DIK_5: return SDLK_5;
	case DIK_6: return SDLK_6;
	case DIK_7: return SDLK_7;
	case DIK_8: return SDLK_8;
	case DIK_9: return SDLK_9;
	case DIK_0: return SDLK_0;
	case DIK_MINUS: return SDLK_MINUS;
	case DIK_EQUALS: return SDLK_EQUALS;
	case DIK_BACK: return SDLK_BACKSPACE;
	case DIK_TAB: return SDLK_TAB;
	case DIK_Q: return SDLK_q;
	case DIK_W: return SDLK_w;
	case DIK_E: return SDLK_e;
	case DIK_R: return SDLK_r;
	case DIK_T: return SDLK_t;
	case DIK_Y: return SDLK_y;
	case DIK_U: return SDLK_u;
	case DIK_I: return SDLK_i;
	case DIK_O: return SDLK_o;
	case DIK_P: return SDLK_p;
	case DIK_LBRACKET: return SDLK_LEFTBRACKET;
	case DIK_RBRACKET: return SDLK_RIGHTBRACKET;
	case DIK_RETURN: return SDLK_RETURN;
	case DIK_LCONTROL: return SDLK_LCTRL;
	case DIK_A: return SDLK_a;
	case DIK_S: return SDLK_s;
	case DIK_D: return SDLK_d;
	case DIK_F: return SDLK_f;
	case DIK_G: return SDLK_g;
	case DIK_H: return SDLK_h;
	case DIK_J: return SDLK_j;
	case DIK_K: return SDLK_k;
	case DIK_L: return SDLK_l;
	case DIK_SEMICOLON: return SDLK_SEMICOLON;
	case DIK_APOSTROPHE: return SDLK_QUOTE;
	case DIK_GRAVE: return SDLK_BACKQUOTE;
	case DIK_LSHIFT: return SDLK_LSHIFT;
	case DIK_BACKSLASH: return SDLK_BACKSLASH;
	case DIK_OEM_102: return SDLK_BACKSLASH;
	case DIK_Z: return SDLK_z;
	case DIK_X: return SDLK_x;
	case DIK_C: return SDLK_c;
	case DIK_V: return SDLK_v;
	case DIK_B: return SDLK_b;
	case DIK_N: return SDLK_n;
	case DIK_M: return SDLK_m;
	case DIK_COMMA: return SDLK_COMMA;
	case DIK_PERIOD: return SDLK_PERIOD;
	case DIK_SLASH: return SDLK_SLASH;
	case DIK_RSHIFT: return SDLK_RSHIFT;
	case DIK_MULTIPLY: return SDLK_KP_MULTIPLY;
	case DIK_LMENU: return SDLK_LALT;
	case DIK_SPACE: return SDLK_SPACE;
	case DIK_CAPITAL: return SDLK_CAPSLOCK;
	case DIK_F1: return SDLK_F1;
	case DIK_F2: return SDLK_F2;
	case DIK_F3: return SDLK_F3;
	case DIK_F4: return SDLK_F4;
	case DIK_F5: return SDLK_F5;
	case DIK_F6: return SDLK_F6;
	case DIK_F7: return SDLK_F7;
	case DIK_F8: return SDLK_F8;
	case DIK_F9: return SDLK_F9;
	case DIK_F10: return SDLK_F10;
	case DIK_NUMLOCK: return SDLK_NUMLOCK;
	case DIK_SCROLL: return SDLK_SCROLLOCK;
	case DIK_NUMPAD7: return SDLK_KP7;
	case DIK_NUMPAD8: return SDLK_KP8;
	case DIK_NUMPAD9: return SDLK_KP9;
	case DIK_SUBTRACT: return SDLK_KP_MINUS;
	case DIK_NUMPAD4: return SDLK_KP4;
	case DIK_NUMPAD5: return SDLK_KP5;
	case DIK_NUMPAD6: return SDLK_KP6;
	case DIK_ADD: return SDLK_KP_PLUS;
	case DIK_NUMPAD1: return SDLK_KP1;
	case DIK_NUMPAD2: return SDLK_KP2;
	case DIK_NUMPAD3: return SDLK_KP3;
	case DIK_NUMPAD0: return SDLK_KP0;
	case DIK_DECIMAL: return SDLK_KP_PERIOD;
	case DIK_F11: return SDLK_F11;
	case DIK_F12: return SDLK_F12;

	case DIK_F13: return SDLK_F13;
	case DIK_F14: return SDLK_F14;
	case DIK_F15: return SDLK_F15;

	case DIK_NUMPADEQUALS: return SDLK_KP_EQUALS;
	case DIK_NUMPADENTER: return SDLK_KP_ENTER;
	case DIK_RCONTROL: return SDLK_RCTRL;
	case DIK_DIVIDE: return SDLK_KP_DIVIDE;
	case DIK_SYSRQ: return SDLK_SYSREQ;
	case DIK_RMENU: return SDLK_RALT;
	case DIK_PAUSE: return SDLK_PAUSE;
	case DIK_HOME: return SDLK_HOME;
	case DIK_UP: return SDLK_UP;
	case DIK_PRIOR: return SDLK_PAGEUP;
	case DIK_LEFT: return SDLK_LEFT;
	case DIK_RIGHT: return SDLK_RIGHT;
	case DIK_END: return SDLK_END;
	case DIK_DOWN: return SDLK_DOWN;
	case DIK_NEXT: return SDLK_PAGEDOWN;
	case DIK_INSERT: return SDLK_INSERT;
	case DIK_DELETE: return SDLK_DELETE;
	case DIK_LWIN: return SDLK_LMETA;
	case DIK_RWIN: return SDLK_RMETA;
	case DIK_APPS: return SDLK_MENU;
	default: return '?';
	};
}


bool InputHandler_DInput::OpenDevice(DIDevice &device)
{
	LOG->Trace( "Opening device '%s'", device.JoystickInst.tszProductName );
	device.buffered = true;
	
	LPDIRECTINPUTDEVICE tmpdevice;
	HRESULT hr = IDirectInput_CreateDevice(dinput, device.JoystickInst.guidInstance,
			    &tmpdevice, NULL);
	if ( hr != DI_OK )
	{
		LOG->Info( hr_ssprintf(hr, "OpenDevice: IDirectInput_CreateDevice") );
		return false;
	}

	hr = IDirectInputDevice_QueryInterface(tmpdevice,
		   	    IID_IDirectInputDevice2, (LPVOID *) &device.Device);
	IDirectInputDevice_Release(tmpdevice);
	if ( hr != DI_OK )
	{
		LOG->Info( hr_ssprintf(hr, "OpenDevice(%s): IDirectInputDevice_QueryInterface", device.JoystickInst.tszProductName) );
		return false;
	}

	int coop = DISCL_NONEXCLUSIVE | DISCL_BACKGROUND;
	if( device.type == device.KEYBOARD )
		coop = DISCL_FOREGROUND|DISCL_NONEXCLUSIVE;

	hr = IDirectInputDevice2_SetCooperativeLevel( device.Device, GetHwnd(), coop );
	if ( hr != DI_OK )
	{
		LOG->Info( hr_ssprintf(hr, "OpenDevice(%s): IDirectInputDevice2_SetCooperativeLevel", device.JoystickInst.tszProductName) );
		return false;
	}

	hr = IDirectInputDevice2_SetDataFormat(device.Device, 
		device.type == device.JOYSTICK? &c_dfDIJoystick: &c_dfDIKeyboard);
	if ( hr != DI_OK )
	{
		LOG->Info( hr_ssprintf(hr, "OpenDevice(%s): IDirectInputDevice2_SetDataFormat", device.JoystickInst.tszProductName) );
		return false;
	}

	switch( device.type )
	{
	case device.JOYSTICK:
		IDirectInputDevice2_EnumObjects(device.Device,
					DIJoystick_EnumDevObjectsProc,
					&device,
					DIDFT_BUTTON | DIDFT_AXIS | DIDFT_POV);
		break;
	case device.KEYBOARD:
		/* Always 256-button. */
		for( int b = 0; b < 256; ++b )
		{
			input_t in;
			in.type = in.KEY;

			in.num = ConvertScancodeToKey(b);
			in.ofs = b;
			device.buttons++;
			device.Inputs.push_back(in);
		}
		break;
	}

	{
		DIPROPDWORD dipdw;
		memset(&dipdw, 0, sizeof(dipdw));
		dipdw.diph.dwSize = sizeof(dipdw);
		dipdw.diph.dwHeaderSize = sizeof(dipdw.diph);
		dipdw.diph.dwObj = 0;
		dipdw.diph.dwHow = DIPH_DEVICE;
		dipdw.dwData = INPUT_QSIZE;
		hr = IDirectInputDevice2_SetProperty(device.Device,
						DIPROP_BUFFERSIZE, &dipdw.diph);
		/* XXX: Try to use event notification.  It might give better response in a thread. */
		if ( hr == DI_POLLEDDEVICE )
		{
			/* This device doesn't support buffering, so we're forced
			 * to use less reliable polling. */
			device.buffered = false;
		} else if ( hr != DI_OK ) {
			LOG->Info( hr_ssprintf(hr, "OpenDevice(%s): IDirectInputDevice2_SetProperty", device.JoystickInst.tszProductName) );
			return false;
		}
	}

	return true;
}

InputHandler_DInput::~InputHandler_DInput()
{
	for( unsigned i = 0; i < Devices.size(); ++i )
	{
		IDirectInputDevice2_Unacquire(Devices[i].Device);
		IDirectInputDevice2_Release(Devices[i].Device);
	}

	Devices.clear();
	IDirectInput_Release(dinput);
	dinput = NULL;
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
		IDirectInputDevice2_Acquire(dev);
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

		for( int k = 0; k < 256; ++k )
		{
			const int key = device.Inputs[k].num;
			INPUTFILTER->ButtonPressed(DeviceInput(device.dev, key), !!(keys[k] & 0x80));
		}
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
		}
			break;

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
			case DIJOFS_RZ: INPUTFILTER->ButtonPressed(DeviceInput(dev, JOY_Z_ROT_UP), state.lRz < -50);
							INPUTFILTER->ButtonPressed(DeviceInput(dev, JOY_Z_ROT_DOWN), state.lRz > 50);
							break;
			}
		}

			break;

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
				case DIJOFS_RZ: up = JOY_Z_ROT_UP; down = JOY_Z_ROT_DOWN; break;
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
			IDirectInputDevice2_Acquire( Devices[i].Device );
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

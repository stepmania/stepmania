#include "global.h"
#include "InputHandler_DirectInputHelper.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "SDL_utils.h"

#pragma comment(lib, "dinput.lib")
LPDIRECTINPUT dinput = NULL;

static int ConvertScancodeToKey( int scancode );
static BOOL CALLBACK DIJoystick_EnumDevObjectsProc(LPCDIDEVICEOBJECTINSTANCE dev, LPVOID data);
static HWND GetHwnd();

DIDevice::DIDevice()
{
	buttons = axes = hats = 0;
	dev = DEVICE_NONE;
	buffered = true;
	memset(&JoystickInst, 0, sizeof(JoystickInst));
	Device = NULL;
}

bool DIDevice::Open()
{
	LOG->Trace( "Opening device '%s'", JoystickInst.tszProductName );
	buffered = true;
	
	LPDIRECTINPUTDEVICE tmpdevice;
	HRESULT hr = IDirectInput_CreateDevice(dinput, JoystickInst.guidInstance,
			    &tmpdevice, NULL);
	if ( hr != DI_OK )
	{
		LOG->Info( hr_ssprintf(hr, "OpenDevice: IDirectInput_CreateDevice") );
		return false;
	}

	hr = IDirectInputDevice_QueryInterface(tmpdevice,
		   	    IID_IDirectInputDevice2, (LPVOID *) &Device);
	IDirectInputDevice_Release(tmpdevice);
	if ( hr != DI_OK )
	{
		LOG->Info( hr_ssprintf(hr, "OpenDevice(%s): IDirectInputDevice_QueryInterface", JoystickInst.tszProductName) );
		return false;
	}

	int coop = DISCL_NONEXCLUSIVE | DISCL_BACKGROUND;
	if( type == KEYBOARD )
		coop = DISCL_FOREGROUND|DISCL_NONEXCLUSIVE;

	hr = IDirectInputDevice2_SetCooperativeLevel( Device, GetHwnd(), coop );
	if ( hr != DI_OK )
	{
		LOG->Info( hr_ssprintf(hr, "OpenDevice(%s): IDirectInputDevice2_SetCooperativeLevel", JoystickInst.tszProductName) );
		return false;
	}

	hr = IDirectInputDevice2_SetDataFormat(Device, 
		type == JOYSTICK? &c_dfDIJoystick: &c_dfDIKeyboard);
	if ( hr != DI_OK )
	{
		LOG->Info( hr_ssprintf(hr, "OpenDevice(%s): IDirectInputDevice2_SetDataFormat", JoystickInst.tszProductName) );
		return false;
	}

	switch( type )
	{
	case JOYSTICK:
		IDirectInputDevice2_EnumObjects(Device, DIJoystick_EnumDevObjectsProc,
					this, DIDFT_BUTTON | DIDFT_AXIS | DIDFT_POV);
		break;
	case KEYBOARD:
		/* Always 256-button. */
		for( int b = 0; b < 256; ++b )
		{
			input_t in;
			in.type = in.KEY;

			in.num = ConvertScancodeToKey(b);
			in.ofs = b;
			buttons++;
			Inputs.push_back(in);
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
		hr = IDirectInputDevice2_SetProperty(Device,
						DIPROP_BUFFERSIZE, &dipdw.diph);
		/* XXX: Try to use event notification.  It might give better response in a thread. */
		if ( hr == DI_POLLEDDEVICE )
		{
			/* This device doesn't support buffering, so we're forced
			 * to use less reliable polling. */
			buffered = false;
		} else if ( hr != DI_OK ) {
			LOG->Info( hr_ssprintf(hr, "OpenDevice(%s): IDirectInputDevice2_SetProperty", JoystickInst.tszProductName) );
			return false;
		}
	}

	return true;
}

void DIDevice::Close()
{
	/* Don't try to close a device that isn't open. */
	ASSERT( Device != NULL );

	IDirectInputDevice2_Unacquire(Device);
	IDirectInputDevice2_Release(Device);

	Device = NULL;
	buttons = axes = hats = NULL;
	Inputs.clear();
}

static BOOL CALLBACK DIJoystick_EnumDevObjectsProc(LPCDIDEVICEOBJECTINSTANCE dev, LPVOID data)
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

static HWND GetHwnd()
{
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	if( SDL_GetWMInfo(&info) < 0 ) 
		RageException::Throw( "SDL_GetWMInfo failed" );

	return info.window;
}

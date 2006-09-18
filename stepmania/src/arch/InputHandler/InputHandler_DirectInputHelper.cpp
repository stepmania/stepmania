#include "global.h"
#include "InputHandler_DirectInputHelper.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "archutils/Win32/DirectXHelpers.h"
#include "archutils/Win32/GraphicsWindow.h"

#if defined(_MSC_VER)
#pragma comment(lib, "dinput.lib")
#if defined(_WINDOWS)
#pragma comment(lib, "dxguid.lib")
#endif
#endif
LPDIRECTINPUT g_dinput = NULL;

static int ConvertScancodeToKey( int scancode );
static BOOL CALLBACK DIJoystick_EnumDevObjectsProc(LPCDIDEVICEOBJECTINSTANCE dev, LPVOID data);

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
	m_sName = ConvertACPToUTF8( JoystickInst.tszProductName );

	LOG->Trace( "Opening device '%s'", m_sName.c_str() );
	buffered = true;
	
	LPDIRECTINPUTDEVICE tmpdevice;
	HRESULT hr = g_dinput->CreateDevice( JoystickInst.guidInstance, &tmpdevice, NULL );
	if ( hr != DI_OK )
	{
		LOG->Info( hr_ssprintf(hr, "OpenDevice: IDirectInput_CreateDevice") );
		return false;
	}

	hr = tmpdevice->QueryInterface( IID_IDirectInputDevice2, (LPVOID *) &Device );
	tmpdevice->Release();
	if ( hr != DI_OK )
	{
		LOG->Info( hr_ssprintf(hr, "OpenDevice(%s): IDirectInputDevice::QueryInterface", m_sName.c_str()) );
		return false;
	}

	int coop = DISCL_NONEXCLUSIVE | DISCL_BACKGROUND;
	if( type == KEYBOARD )
		coop = DISCL_NONEXCLUSIVE | DISCL_FOREGROUND;

	hr = Device->SetCooperativeLevel( GraphicsWindow::GetHwnd(), coop );
	if ( hr != DI_OK )
	{
		LOG->Info( hr_ssprintf(hr, "OpenDevice(%s): IDirectInputDevice2::SetCooperativeLevel", m_sName.c_str()) );
		return false;
	}

	hr = Device->SetDataFormat( type == JOYSTICK? &c_dfDIJoystick: &c_dfDIKeyboard );
	if ( hr != DI_OK )
	{
		LOG->Info( hr_ssprintf(hr, "OpenDevice(%s): IDirectInputDevice2::SetDataFormat", m_sName.c_str()) );
		return false;
	}

	switch( type )
	{
	case JOYSTICK:
		Device->EnumObjects( DIJoystick_EnumDevObjectsProc, this, DIDFT_BUTTON | DIDFT_AXIS | DIDFT_POV);
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
		hr = Device->SetProperty( DIPROP_BUFFERSIZE, &dipdw.diph );
		if ( hr == DI_POLLEDDEVICE )
		{
			/* This device doesn't support buffering, so we're forced
			 * to use less reliable polling. */
			buffered = false;
		}
		else if ( hr != DI_OK )
		{
			LOG->Info( hr_ssprintf(hr, "OpenDevice(%s): IDirectInputDevice2::SetProperty", m_sName.c_str()) );
			return false;
		}
	}

	return true;
}

void DIDevice::Close()
{
	/* Don't try to close a device that isn't open. */
	ASSERT( Device != NULL );

	Device->Unacquire();
	Device->Release();

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

		hr = device->Device->SetProperty( DIPROP_RANGE, &diprg.diph );
		if ( hr != DI_OK )
			return DIENUM_CONTINUE; /* don't use this axis */
	
		/* Set dead zone to 0. */
		dilong.diph.dwSize		= sizeof(dilong);
		dilong.diph.dwHeaderSize	= sizeof(dilong.diph);
		dilong.diph.dwObj		= dev->dwOfs;
		dilong.diph.dwHow		= DIPH_BYOFFSET;
		dilong.dwData = 0;
		hr = device->Device->SetProperty( DIPROP_DEADZONE, &dilong.diph );
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
	case DIK_ESCAPE: return KEY_ESC;
	case DIK_1: return KEY_C1;
	case DIK_2: return KEY_C2;
	case DIK_3: return KEY_C3;
	case DIK_4: return KEY_C4;
	case DIK_5: return KEY_C5;
	case DIK_6: return KEY_C6;
	case DIK_7: return KEY_C7;
	case DIK_8: return KEY_C8;
	case DIK_9: return KEY_C9;
	case DIK_0: return KEY_C0;
	case DIK_MINUS: return KEY_HYPHEN;
	case DIK_EQUALS: return KEY_EQUAL;
	case DIK_BACK: return KEY_BACK;
	case DIK_TAB: return KEY_TAB;
	case DIK_Q: return KEY_Cq;
	case DIK_W: return KEY_Cw;
	case DIK_E: return KEY_Ce;
	case DIK_R: return KEY_Cr;
	case DIK_T: return KEY_Ct;
	case DIK_Y: return KEY_Cy;
	case DIK_U: return KEY_Cu;
	case DIK_I: return KEY_Ci;
	case DIK_O: return KEY_Co;
	case DIK_P: return KEY_Cp;
	case DIK_LBRACKET: return KEY_LBRACKET;
	case DIK_RBRACKET: return KEY_RBRACKET;
	case DIK_RETURN: return KEY_ENTER;
	case DIK_LCONTROL: return KEY_LCTRL;
	case DIK_A: return KEY_Ca;
	case DIK_S: return KEY_Cs;
	case DIK_D: return KEY_Cd;
	case DIK_F: return KEY_Cf;
	case DIK_G: return KEY_Cg;
	case DIK_H: return KEY_Ch;
	case DIK_J: return KEY_Cj;
	case DIK_K: return KEY_Ck;
	case DIK_L: return KEY_Cl;
	case DIK_SEMICOLON: return KEY_SEMICOLON;
	case DIK_APOSTROPHE: return KEY_SQUOTE;
	case DIK_GRAVE: return KEY_ACCENT;
	case DIK_LSHIFT: return KEY_LSHIFT;
	case DIK_BACKSLASH: return KEY_BACKSLASH;
	case DIK_OEM_102: return KEY_BACKSLASH;
	case DIK_Z: return KEY_Cz;
	case DIK_X: return KEY_Cx;
	case DIK_C: return KEY_Cc;
	case DIK_V: return KEY_Cv;
	case DIK_B: return KEY_Cb;
	case DIK_N: return KEY_Cn;
	case DIK_M: return KEY_Cm;
	case DIK_COMMA: return KEY_COMMA;
	case DIK_PERIOD: return KEY_PERIOD;
	case DIK_SLASH: return KEY_SLASH;
	case DIK_RSHIFT: return KEY_RSHIFT;
	case DIK_MULTIPLY: return KEY_KP_ASTERISK;
	case DIK_LMENU: return KEY_LALT;
	case DIK_SPACE: return KEY_SPACE;
	case DIK_CAPITAL: return KEY_CAPSLOCK;
	case DIK_F1: return KEY_F1;
	case DIK_F2: return KEY_F2;
	case DIK_F3: return KEY_F3;
	case DIK_F4: return KEY_F4;
	case DIK_F5: return KEY_F5;
	case DIK_F6: return KEY_F6;
	case DIK_F7: return KEY_F7;
	case DIK_F8: return KEY_F8;
	case DIK_F9: return KEY_F9;
	case DIK_F10: return KEY_F10;
	case DIK_NUMLOCK: return KEY_NUMLOCK;
	case DIK_SCROLL: return KEY_SCRLLOCK;
	case DIK_NUMPAD7: return KEY_KP_C7;
	case DIK_NUMPAD8: return KEY_KP_C8;
	case DIK_NUMPAD9: return KEY_KP_C9;
	case DIK_SUBTRACT: return KEY_KP_HYPHEN;
	case DIK_NUMPAD4: return KEY_KP_C4;
	case DIK_NUMPAD5: return KEY_KP_C5;
	case DIK_NUMPAD6: return KEY_KP_C6;
	case DIK_ADD: return KEY_KP_PLUS;
	case DIK_NUMPAD1: return KEY_KP_C1;
	case DIK_NUMPAD2: return KEY_KP_C2;
	case DIK_NUMPAD3: return KEY_KP_C3;
	case DIK_NUMPAD0: return KEY_KP_C0;
	case DIK_DECIMAL: return KEY_KP_PERIOD;
	case DIK_F11: return KEY_F11;
	case DIK_F12: return KEY_F12;

	case DIK_F13: return KEY_F13;
	case DIK_F14: return KEY_F14;
	case DIK_F15: return KEY_F15;

	case DIK_NUMPADEQUALS: return KEY_KP_EQUAL;
	case DIK_NUMPADENTER: return KEY_KP_ENTER;
	case DIK_RCONTROL: return KEY_RCTRL;
	case DIK_DIVIDE: return KEY_KP_SLASH;
	case DIK_SYSRQ: return KEY_PRTSC;
	case DIK_RMENU: return KEY_RALT;
	case DIK_PAUSE: return KEY_PAUSE;
	case DIK_HOME: return KEY_HOME;
	case DIK_UP: return KEY_UP;
	case DIK_PRIOR: return KEY_PGUP;
	case DIK_LEFT: return KEY_LEFT;
	case DIK_RIGHT: return KEY_RIGHT;
	case DIK_END: return KEY_END;
	case DIK_DOWN: return KEY_DOWN;
	case DIK_NEXT: return KEY_PGDN;
	case DIK_INSERT: return KEY_INSERT;
	case DIK_DELETE: return KEY_DEL;
	case DIK_LWIN: return KEY_LMETA;
	case DIK_RWIN: return KEY_RMETA;
	case DIK_APPS: return KEY_MENU;
	default: return '?';
	};
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

#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: RageInput.h

 Desc: Wrapper for DirectInput.  Generates InputEvents.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

//-----------------------------------------------------------------------------
// In-line Links
//-----------------------------------------------------------------------------
#pragma comment(lib, "ddk/setupapi.lib") 
#pragma comment(lib, "ddk/hid.lib") 

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "RageInput.h"
#include "SDL.h"
#include "SDL_keyboard.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageException.h"


RageInput*		INPUTMAN	= NULL;		// globally accessable input device

static const char *PumpButtonNames[] = {
	"UL", "UR", "MID", "DL", "DR", "Esc",
	"P2 UL", "P2 UR", "P2 MID", "P2 DL", "P2 DR"
};

int DeviceInput::NumButtons(InputDevice device)
{
	switch( device )
	{
	case DEVICE_KEYBOARD:	
		return NUM_KEYBOARD_BUTTONS;	
	case DEVICE_JOY1:
	case DEVICE_JOY2:
	case DEVICE_JOY3:
	case DEVICE_JOY4:
		return NUM_JOYSTICK_BUTTONS;	
	case DEVICE_PUMP1:
	case DEVICE_PUMP2:
		return NUM_PUMP_PAD_BUTTONS;	
	default:
		ASSERT(0);	// invalid device
	}
	return -1; /* quiet compiler */
}

CString DeviceInput::GetDescription() 
{
	CString sReturn;

	switch( device )
	{
	case DEVICE_NONE:	// blank
		ASSERT( false );	// called GetDescription on a blank DeviceInput!
		break;

	case DEVICE_JOY1:	// joystick
	case DEVICE_JOY2:
	case DEVICE_JOY3:
	case DEVICE_JOY4:
		sReturn = ssprintf("Joy%d ", device - DEVICE_JOY1 + 1 );

		switch( button )
		{
		case JOY_LEFT:		sReturn += "Left";		break;
		case JOY_RIGHT:		sReturn += "Right";		break;
		case JOY_UP:		sReturn += "Up";		break;
		case JOY_DOWN:		sReturn += "Down";		break;
		case JOY_Z_UP:		sReturn += "Z-Up";		break;
		case JOY_Z_DOWN:	sReturn += "Z-Down";	break;
		case JOY_Z_ROT_UP:  sReturn += "ZR-Up";		break;
		case JOY_Z_ROT_DOWN:sReturn += "ZR-Down";	break;
		case JOY_HAT_LEFT:	sReturn += "H-Left";	break;
		case JOY_HAT_RIGHT:	sReturn += "H-Right";	break;
		case JOY_HAT_UP:	sReturn += "H-Up";		break;
		case JOY_HAT_DOWN:	sReturn += "H-Down";	break;
		case JOY_1:		sReturn += "1";		break;
		case JOY_2:		sReturn += "2";		break;
		case JOY_3:		sReturn += "3";		break;
		case JOY_4:		sReturn += "4";		break;
		case JOY_5:		sReturn += "5";		break;
		case JOY_6:		sReturn += "6";		break;
		case JOY_7:		sReturn += "7";		break;
		case JOY_8:		sReturn += "8";		break;
		case JOY_9:		sReturn += "9";		break;
		case JOY_10:	sReturn += "10";	break;
		case JOY_11:	sReturn += "11";	break;
		case JOY_12:	sReturn += "12";	break;
		case JOY_13:	sReturn += "13";	break;
		case JOY_14:	sReturn += "14";	break;
		case JOY_15:	sReturn += "15";	break;
		case JOY_16:	sReturn += "16";	break;
		case JOY_17:	sReturn += "17";	break;
		case JOY_18:	sReturn += "18";	break;
		case JOY_19:	sReturn += "19";	break;
		case JOY_20:	sReturn += "20";	break;
		case JOY_21:	sReturn += "21";	break;
		case JOY_22:	sReturn += "22";	break;
		case JOY_23:	sReturn += "23";	break;
		case JOY_24:	sReturn += "24";	break;
		}
		
		break;

	case DEVICE_PUMP1:
	case DEVICE_PUMP2:
		/* There is almost always only one Pump device, even if there are
		 * two pads, so only enumerate the second and higher. */
		if(device == DEVICE_PUMP1)
			sReturn = ssprintf("PIU ");
		else
			sReturn = ssprintf("PIU#%d ", device - DEVICE_PUMP1 + 1 );

		if(button < NUM_PUMP_PAD_BUTTONS)
			sReturn += PumpButtonNames[button];
		else 
			/* This can happen if the INI is corrupt and has an invalid
			 * button number.  Crashing with an assertion failure because
			 * we got something unexpected is no good. */
			sReturn += "???";

		break;

	case DEVICE_KEYBOARD:		// keyboard
		sReturn = ssprintf("Key %s", SDL_GetKeyName((SDLKey)button) );
/*
		switch( button )
		{
		case SDLK_ESCAPE:	sReturn += "Escape";	break;
		case SDLK_1:			sReturn += "1";			break;
		case SDLK_2:			sReturn += "2";			break;
		case SDLK_3:			sReturn += "3";			break;
		case SDLK_4:			sReturn += "4";			break;
		case SDLK_5:			sReturn += "5";			break;
		case SDLK_6:			sReturn += "6";			break;
		case SDLK_7:			sReturn += "7";			break;
		case SDLK_8:			sReturn += "8";			break;
		case SDLK_9:			sReturn += "9";			break;
		case SDLK_0:			sReturn += "0";			break;
		case SDLK_MINUS:		sReturn += "Minus";		break;
		case SDLK_EQUALS:	sReturn += "Equals";	break;
		case SDLK_BACKSPACE:		sReturn += "Backsp";	break;
		case SDLK_TAB:		sReturn += "Tab";		break;
		case SDLK_Q:			sReturn += "Q";			break;
		case SDLK_W:			sReturn += "W";			break;
		case SDLK_E:			sReturn += "E";			break;
		case SDLK_R:			sReturn += "R";			break;
		case SDLK_T:			sReturn += "T";			break;
		case SDLK_Y:			sReturn += "Y";			break;
		case SDLK_U:			sReturn += "U";			break;
		case SDLK_I:			sReturn += "I";			break;
		case SDLK_O:			sReturn += "O";			break;
		case SDLK_P:			sReturn += "P";			break;
		case SDLK_LBRACKET:	sReturn += "LBracket";	break;
		case SDLK_RBRACKET:	sReturn += "RBracket";	break;
		case SDLK_RETURN:	sReturn += "Return";	break;
		case SDLK_LCONTROL:	sReturn += "LControl";	break;
		case SDLK_A:			sReturn += "A";			break;
		case SDLK_S:			sReturn += "S";			break;
		case SDLK_D:			sReturn += "D";			break;
		case SDLK_F:			sReturn += "F";			break;
		case SDLK_G:			sReturn += "G";			break;
		case SDLK_H:			sReturn += "H";			break;
		case SDLK_J:			sReturn += "J";			break;
		case SDLK_K:			sReturn += "K";			break;
		case SDLK_L:			sReturn += "L";			break;
		case SDLK_SEMICOLON:	sReturn += "Semicln";	break;
		case SDLK_APOSTROPHE:sReturn += "Apostro";	break;
		case SDLK_GRAVE:		sReturn += "Grave";		break;
		case SDLK_LSHIFT:	sReturn += "LShift";	break;
		case SDLK_BACKSLASH:	sReturn += "Backslsh";	break;
		case SDLK_Z:			sReturn += "Z";			break;
		case SDLK_X:			sReturn += "X";			break;
		case SDLK_C:			sReturn += "C";			break;
		case SDLK_V:			sReturn += "V";			break;
		case SDLK_B:			sReturn += "B";			break;
		case SDLK_N:			sReturn += "N";			break;
		case SDLK_M:			sReturn += "M";			break;
		case SDLK_COMMA:		sReturn += "Comma";		break;
		case SDLK_PERIOD:	sReturn += "Period";	break;
		case SDLK_SLASH:		sReturn += "Slash";		break;
		case SDLK_RSHIFT:	sReturn += "RShift";	break;
		case SDLK_MULTIPLY:	sReturn += "Mult";		break;
		case SDLK_LMENU:		sReturn += "LMenu";		break;
		case SDLK_SPACE:		sReturn += "Space";		break;
		case SDLK_CAPITAL:	sReturn += "CapsLk";	break;
		case SDLK_F1:		sReturn += "F1";		break;
		case SDLK_F2:		sReturn += "F2";		break;
		case SDLK_F3:		sReturn += "F3";		break;
		case SDLK_F4:		sReturn += "F4";		break;
		case SDLK_F5:		sReturn += "F5";		break;
		case SDLK_F6:		sReturn += "F6";		break;
		case SDLK_F7:		sReturn += "F7";		break;
		case SDLK_F8:		sReturn += "F8";		break;
		case SDLK_F9:		sReturn += "F9";		break;
		case SDLK_F10:		sReturn += "F10";		break;
		case SDLK_NUMLOCK:	sReturn += "Numlock";	break;
		case SDLK_SCROLL:	sReturn += "Scroll";	break;
		case SDLK_NUMPAD7:	sReturn += "NumPad7";	break;
		case SDLK_NUMPAD8:	sReturn += "NumPad8";	break;
		case SDLK_NUMPAD9:	sReturn += "NumPad9";	break;
		case SDLK_SUBTRACT:	sReturn += "Subtract";	break;
		case SDLK_NUMPAD4:	sReturn += "NumPad4";	break;
		case SDLK_NUMPAD5:	sReturn += "NumPad5";	break;
		case SDLK_NUMPAD6:	sReturn += "NumPad6";	break;
		case SDLK_ADD:		sReturn += "Add";		break;
		case SDLK_NUMPAD1:	sReturn += "NumPad1";	break;
		case SDLK_NUMPAD2:	sReturn += "NumPad2";	break;
		case SDLK_NUMPAD3:	sReturn += "NumPad3";	break;
		case SDLK_NUMPAD0:	sReturn += "NumPad0";	break;
		case SDLK_DECIMAL:	sReturn += "Decimal";	break;
		case SDLK_RMENU:		sReturn += "RightAlt";	break;
		case SDLK_PAUSE:		sReturn += "Pause";		break;
		case SDLK_HOME:		sReturn += "Home";		break;
		case SDLK_UP:		sReturn += "Up";		break;
		case SDLK_PRIOR:		sReturn += "PageUp";	break;
		case SDLK_LEFT:		sReturn += "Left";		break;
		case SDLK_RIGHT:		sReturn += "Right";		break;
		case SDLK_END:		sReturn += "End";		break;
		case SDLK_DOWN:		sReturn += "Down";		break;
		case SDLK_NEXT:		sReturn += "PageDn";	break;
		case SDLK_INSERT:	sReturn += "Insert";	break;
		case SDLK_DELETE:	sReturn += "Delete";	break;
		case SDLK_LWIN:		sReturn += "LeftWin";	break;
		case SDLK_RWIN:		sReturn += "RightWin";	break;
		case SDLK_APPS:		sReturn += "AppMenu";	break;
		case SDLK_NUMPADENTER:	sReturn += "PadEnter";	break;
		case SDLK_DIVIDE:	sReturn += "PadSlash";	break;

		default:			sReturn += "Unknown Key"; break;
		}
*/
		break;
	default:
		ASSERT(0);	// what device is this?
	}

	return sReturn;
}

CString DeviceInput::toString() 
{
	if( device == DEVICE_NONE )
		return "";
	else
		return ssprintf("%d-%d", device, button );
}

bool DeviceInput::fromString( const CString &s )
{ 
	CStringArray a;
	split( s, "-", a);

	if( a.size() != 2 ) {
		device = DEVICE_NONE;
		return false;
	}

	device = (InputDevice)atoi( a[0] );
	button = atoi( a[1] );
	return true;
}

#if 0

/* FIXME: The main font doesn't have '`' (0x29), so that's disabled. */
static const char SDLK_charmap[] = {
	    /*   0   1   2   3   4   5   6   7   8    9    A   B   C   D   E  F */ 
/* 0x0x */    0,  0,'1','2','3','4','5','6','7', '8','9', '0','-','=',  0,  0,
/* 0x1x */	'q','w','e','r','t','y','u','i','o', 'p','[', ']',  0,  0,'a','s',
/* 0x2x */	'd','f','g','h','j','k','l',';','\'',  0,  0,'\\','z','x','c','v',
/* 0x3x */	'b','n','m',',','.','/',  0,'*',  0, ' ',  0,   0,  0,  0,  0,  0,
/* 0x4x */	  0,  0,  0,  0,  0,  0,  0,'7','8', '9','-', '4','5','6','+','1',
/* 0x5x */	'2','3','0','.',  0,  0,  0,  0,  0,   0,  0,   0,  0,  0,  0,  0,
/* 0x6x */	  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,   0,  0,  0,  0,  0,
/* 0x7x */	  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,   0,  0,  0,'.',  0,
/* 0x8x */	  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,   0,  0,'=',  0,  0,
/* 0x9x */	  0,'@',':','_',  0,  0,  0,  0,  0,   0,  0,   0,  0,  0,  0,  0,
/* 0xAx */	  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,   0,  0,  0,  0,  0,
/* 0xBx */	  0,  0,  0,',',  0,'/',  0,  0,  0,   0,  0,   0,  0,  0,  0,  0,
 };
#endif

char DeviceInput::ToChar() const
{
	switch( device )
	{
	case DEVICE_KEYBOARD:
		if( button < 128 )
			return (char) button;
		/* XXX: SDLK_WORLD_* are for international keyboards; we can handle those,
		 * now, by mapping it to Unicode.  However, I can't find any documentation
		 * on those keysyms. */
		return '\0';
	default:
		return '\0';
	}
}



RageInput::RageInput()
{
	LOG->Trace( "RageInput::RageInput()" );

	SDL_InitSubSystem( SDL_INIT_JOYSTICK );

	// init state info
	memset( m_keys, 0, sizeof(m_keys) );
	memset( m_oldKeys, 0, sizeof(m_oldKeys) );
	memset( m_joyState, 0, sizeof(m_joyState) );
	memset( m_oldJoyState, 0, sizeof(m_oldJoyState) );
	memset( m_pumpState, 0, sizeof(m_pumpState) );
	memset( m_oldPumpState, 0, sizeof(m_oldPumpState) );


	//
	// Init keyboard
	//
	SDL_EnableKeyRepeat( 0, 0 );


	//
	// Init joysticks
	//
	memset( m_pJoystick, 0, sizeof(m_pJoystick) );
	int iNumJoySticks = min( SDL_NumJoysticks(), NUM_JOYSTICKS );
	for( int i=0; i<iNumJoySticks; i++ )
	{
		m_pJoystick[i] = SDL_JoystickOpen( i );
		LOG->Trace( "Found joystick %d: %s", i, SDL_JoystickName(i) );
	}
	SDL_JoystickEventState( SDL_IGNORE );


	//
	// Init pumps
	//
	m_Pumps = new pump_t[NUM_PUMPS];

	for(int pumpNo = 0; pumpNo < NUM_PUMPS; ++pumpNo)
	{
		if(m_Pumps[pumpNo].init(pumpNo))
			LOG->Trace("Found Pump pad %i\n", pumpNo);
	}
}

RageInput::~RageInput()
{
	//
	// De-init keyboard
	//

	//
	// De-init joysticks
	//
	for( int i=0; i<NUM_JOYSTICKS; i++ )
	{
		if( m_pJoystick[i] )
		{
			SDL_JoystickClose(m_pJoystick[i]);
			m_pJoystick[i] = NULL;
		}
	}

	//
	// De-init pumps
	//
	delete[] m_Pumps;


	SDL_QuitSubSystem( SDL_INIT_JOYSTICK );
}

void RageInput::Update( float fDeltaTime )
{
	//
	// Move last current state to old state
	//
	memcpy( m_oldKeys, m_keys, sizeof(m_oldKeys) );
	memcpy( m_oldJoyState, m_joyState, sizeof(m_joyState) );
	memcpy( m_oldPumpState, m_pumpState, sizeof(m_pumpState) );


	//
	// Update keyboard
	//
	SDL_PumpEvents();
	Uint8* keystate = SDL_GetKeyState(NULL);
	memcpy( m_keys, keystate, sizeof(m_keys) );

	//
	// Update Joystick
	//
	SDL_JoystickUpdate();

	memset( m_joyState, 0, sizeof(m_joyState) );	// clear current state

	for( int joy=0; joy<NUM_JOYSTICKS; joy++ )	// foreach joystick
	{
		SDL_Joystick* pJoy = m_pJoystick[joy];
		if( !pJoy )
			continue;

		int iNumJoyAxes = min(NUM_JOYSTICK_AXES,SDL_JoystickNumAxes(pJoy));
		for( int axis=0; axis<iNumJoyAxes; axis++ )
		{
			Sint16 val = SDL_JoystickGetAxis(pJoy,axis);
//			LOG->Trace( "axis %d = %d", axis, val );
 			if( val < -16000 )
			{
				JoystickButton b = (JoystickButton)(JOY_LEFT+2*axis);
				m_joyState[joy].button[b] = true;
			}
			else if( val > +16000 )
			{
				JoystickButton b = (JoystickButton)(JOY_RIGHT+2*axis);
				m_joyState[joy].button[b] = true;
			}
		}

		int iNumJoyHats = min(NUM_JOYSTICK_HATS,SDL_JoystickNumHats(pJoy));
		for( int hat=0; axis<iNumJoyHats; hat++ )
		{
			Uint8 val = SDL_JoystickGetHat(pJoy,hat);
			m_joyState[joy].button[JOY_HAT_UP] = (val & SDL_HAT_UP) != 0;
			m_joyState[joy].button[JOY_HAT_RIGHT] = (val & SDL_HAT_RIGHT) != 0;
			m_joyState[joy].button[JOY_HAT_DOWN] = (val & SDL_HAT_DOWN) != 0;
			m_joyState[joy].button[JOY_HAT_LEFT] = (val & SDL_HAT_LEFT) != 0;
		}

		int iNumJoyButtons = MIN(NUM_JOYSTICK_BUTTONS,SDL_JoystickNumButtons(pJoy));
		for( int button=0; button<iNumJoyButtons; button++ )
		{
			JoystickButton b = (JoystickButton)(JOY_1 + button);
			m_joyState[joy].button[b] = SDL_JoystickGetButton(pJoy,button) != 0;
		}
	}


	//
	// Update Pump
	//
	for( int i=0; i<NUM_PUMPS; i++ )
	{
		if(m_Pumps[i].h == INVALID_HANDLE_VALUE)
			continue; /* no pad */

		int ret = m_Pumps[i].GetPadEvent();

		if(ret == -1) 
			continue; /* no event */

		/* Since we're checking for messages, and not polling,
		 * only zero this out when we actually *have* a new
		 * message. */
		ZeroMemory( &m_pumpState[i], sizeof(m_pumpState[i]) );
	    
		int bits[] = {
		/* P1 */	(1<<9), (1<<12), (1<<13), (1<<11), (1<<10),
		/* ESC */	(1<<16),
		/* P1 */	(1<<17), (1<<20), (1<<21), (1<<19), (1<<18),
		};

		for (int butno = 0 ; butno < NUM_PUMP_PAD_BUTTONS ; butno++)
		{
			if(!(ret & bits[butno]))
				m_pumpState[i].button[butno] = true;
		}
	}

}


bool RageInput::BeingPressed( DeviceInput di, bool bPrevState )
{
	switch( di.device )
	{
	case DEVICE_KEYBOARD:
		{
			ASSERT(di.button < NUM_KEYBOARD_BUTTONS);
			return (bPrevState?m_oldKeys:m_keys) [ di.button ];
		}
	case DEVICE_JOY1:
	case DEVICE_JOY2:
	case DEVICE_JOY3:
	case DEVICE_JOY4:
		{
			ASSERT(di.button < NUM_JOYSTICK_BUTTONS);
			return (bPrevState?m_oldJoyState:m_joyState) [di.device - DEVICE_JOY1].button[ di.button ];
		}
	case DEVICE_PUMP1:
	case DEVICE_PUMP2:
		{
			ASSERT(di.button < NUM_PUMP_PAD_BUTTONS);
			return (bPrevState?m_oldPumpState:m_pumpState) [di.device - DEVICE_PUMP1].button[di.button];
		}
	default:
		ASSERT( 0 ); // bad device
		return false;	// hush compiler
	}
}

extern "C" {
#include "ddk/setupapi.h"
/* Quiet header warning: */
#pragma warning( push )
#pragma warning (disable : 4201)
#include "ddk/hidsdi.h"
#pragma warning( pop )
}

char *USB::GetUSBDevicePath (int num)
{
    GUID guid;
    HidD_GetHidGuid(&guid);

    HDEVINFO DeviceInfo = SetupDiGetClassDevs (&guid,
                 NULL, NULL, (DIGCF_PRESENT | DIGCF_DEVICEINTERFACE));

    SP_DEVICE_INTERFACE_DATA DeviceInterface;
    DeviceInterface.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    char *ret = NULL;
    PSP_INTERFACE_DEVICE_DETAIL_DATA DeviceDetail = NULL;

    if (!SetupDiEnumDeviceInterfaces (DeviceInfo,
               NULL, &guid, num, &DeviceInterface))
		goto err;

    unsigned long size;
    SetupDiGetDeviceInterfaceDetail (DeviceInfo, &DeviceInterface, NULL, 0, &size, 0);

    DeviceDetail = (PSP_INTERFACE_DEVICE_DETAIL_DATA) malloc(size);
    DeviceDetail->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);

    if (SetupDiGetDeviceInterfaceDetail (DeviceInfo, &DeviceInterface,
		DeviceDetail, size, &size, NULL)) 
    {
        ret = strdup(DeviceDetail->DevicePath);
    }

err:
    SetupDiDestroyDeviceInfoList (DeviceInfo);
    free (DeviceDetail);
    return ret;
}

HANDLE USB::OpenUSB (int VID, int PID, int num)
{
    DWORD index = 0;

    char *path;
	HANDLE h = INVALID_HANDLE_VALUE;

    while ((path = GetUSBDevicePath (index++)) != NULL)
    {
		if(h != INVALID_HANDLE_VALUE)
			CloseHandle (h);

		h = CreateFile (path, GENERIC_READ,
			   FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

		free(path);

		if(h == INVALID_HANDLE_VALUE)
			continue;

		HIDD_ATTRIBUTES attr;
		if (!HidD_GetAttributes (h, &attr))
			continue;

        if ((VID != -1 && attr.VendorID != VID) &&
            (PID != -1 && attr.ProductID != PID))
			continue; /* This isn't it. */

		/* The VID and PID match. */
		if(num-- == 0)
            return h;
    }
	if(h != INVALID_HANDLE_VALUE)
		CloseHandle (h);

    return INVALID_HANDLE_VALUE;
}

RageInput::pump_t::pump_t()
{
	ZeroMemory( &ov, sizeof(ov) );
	pending=false;
	h = INVALID_HANDLE_VALUE;
}

RageInput::pump_t::~pump_t()
{
	if(h != INVALID_HANDLE_VALUE)
		CloseHandle(h);
}

bool RageInput::pump_t::init(int devno)
{
	const int pump_usb_vid = 0x0d2f, pump_usb_pid = 0x0001;
	h = USB::OpenUSB (pump_usb_vid, pump_usb_pid, devno);
	return h != INVALID_HANDLE_VALUE;
}

int RageInput::pump_t::GetPadEvent()
{
    int ret;

    if(!pending)
    {
		/* Request feedback from the device. */
	    unsigned long r;
    	ret = ReadFile(h, &buf, sizeof(buf), &r, &ov);
    	pending=true;
    }

	/* See if we have a response for our request (which we may
	 * have made on a previous cal): */
    if(WaitForSingleObjectEx(h, 0, TRUE) == WAIT_TIMEOUT)
		return -1;
    
	/* We do; get the result.  It'll go into the original &buf
	 * we supplied on the original call; that's why buf is a
	 * member instead of a local. */
    unsigned long cnt;
    ret = GetOverlappedResult(h, &ov, &cnt, FALSE);
    pending=false;

    if(ret == 0 && (GetLastError() == ERROR_IO_PENDING || GetLastError() == ERROR_IO_INCOMPLETE))
		return -1;

    if(ret == 0) {
		LOG->Trace(werr_ssprintf(GetLastError(), "Error reading Pump pad"));
	    return -1;
    }

    return buf;
}


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

struct RageInput::pump_t
{
	HANDLE h;
	OVERLAPPED ov;
	long buf;
	bool pending;

	pump_t();
	~pump_t();
	void Update();
	bool init(int devno);
	int GetPadEvent();

	bool current_state[NUM_PUMP_PAD_BUTTONS];
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
	memset( state, 0, sizeof(state) );


	//
	// Init keyboard
	//
	SDL_EnableKeyRepeat( 0, 0 );

	/* If we use key events, we can do this to get Unicode values
	 * in the key struct, which (with a little more work) will make
	 * us work on international keyboards: */
	// SDL_EnableUNICODE( 1 );

	//
	// Init joysticks
	//
	memset( m_pJoystick, 0, sizeof(m_pJoystick) );
	int iNumJoySticks = min( SDL_NumJoysticks(), NUM_JOYSTICKS );
	LOG->Info( "Found %d joysticks", iNumJoySticks );
	for( int i=0; i<iNumJoySticks; i++ )
	{
		m_pJoystick[i] = SDL_JoystickOpen( i );
		LOG->Info( "   %d: '%s' axes: %d, hats: %d, buttons: %d",
			i,
			SDL_JoystickName(i),
			SDL_JoystickNumAxes(m_pJoystick[i]),
			SDL_JoystickNumHats(m_pJoystick[i]),
			SDL_JoystickNumButtons(m_pJoystick[i]) );
	}
	SDL_JoystickEventState( SDL_IGNORE );


	//
	// Init pumps
	//
	m_Pumps = new pump_t[NUM_PUMPS];

	for(int pumpNo = 0; pumpNo < NUM_PUMPS; ++pumpNo)
	{
		if(m_Pumps[pumpNo].init(pumpNo))
			LOG->Info("Found Pump pad %i", pumpNo);
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
	memcpy( &state[LAST], &state[CURRENT], sizeof(state[LAST]) );


	//
	// Update keyboard
	//
	SDL_PumpEvents();
	Uint8* keystate = SDL_GetKeyState(NULL);
	memcpy( state[CURRENT].m_Devices[DEVICE_KEYBOARD].button, keystate, NUM_KEYBOARD_BUTTONS );

	//
	// Update Joystick
	//
	SDL_JoystickUpdate();


	for( int joy=0; joy<NUM_JOYSTICKS; joy++ )	// foreach joystick
	{
		state_t::device_t *dev = &state[CURRENT].m_Devices[DEVICE_JOY1+joy];
		memset( dev->button, 0, sizeof(dev->button) );	// clear current state
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
				dev->button[b] = true;
			}
			else if( val > +16000 )
			{
				JoystickButton b = (JoystickButton)(JOY_RIGHT+2*axis);
				dev->button[b] = true;
			}
		}

		int iNumJoyHats = min(NUM_JOYSTICK_HATS,SDL_JoystickNumHats(pJoy));
		for( int hat=0; hat<iNumJoyHats; hat++ )
		{
			Uint8 val = SDL_JoystickGetHat(pJoy,hat);
			dev->button[JOY_HAT_UP] = (val & SDL_HAT_UP) != 0;
			dev->button[JOY_HAT_RIGHT] = (val & SDL_HAT_RIGHT) != 0;
			dev->button[JOY_HAT_DOWN] = (val & SDL_HAT_DOWN) != 0;
			dev->button[JOY_HAT_LEFT] = (val & SDL_HAT_LEFT) != 0;
		}

		int iNumJoyButtons = MIN(NUM_JOYSTICK_BUTTONS,SDL_JoystickNumButtons(pJoy));
		for( int button=0; button<iNumJoyButtons; button++ )
		{
			JoystickButton b = (JoystickButton)(JOY_1 + button);
			dev->button[b] = SDL_JoystickGetButton(pJoy,button) != 0;
		}
	}


	//
	// Update Pump
	//
	for( int i=0; i<NUM_PUMPS; i++ )
		m_Pumps[i].Update();

	memcpy(state[CURRENT].m_Devices[DEVICE_PUMP1].button, m_Pumps[0].current_state, sizeof(m_Pumps[0].current_state));
	memcpy(state[CURRENT].m_Devices[DEVICE_PUMP2].button, m_Pumps[1].current_state, sizeof(m_Pumps[0].current_state));
}

bool RageInput::BeingPressed( DeviceInput di, bool bPrevState )
{
	ASSERT(di.button < NUM_DEVICE_BUTTONS);
	ASSERT(di.device < NUM_INPUT_DEVICES);

	State s = bPrevState? LAST:CURRENT;

	return state[s].m_Devices[di.device].button[ di.button ];
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
	memset(current_state, 0, sizeof(current_state));
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
		LOG->Warn(werr_ssprintf(GetLastError(), "Error reading Pump pad"));
	    return -1;
    }

    return buf;
}

void RageInput::pump_t::Update()
{
	if(h == INVALID_HANDLE_VALUE) return;

	int ret = GetPadEvent();

	if(ret == -1) 
		return; /* no event */

	/* Since we're checking for messages, and not polling,
	 * only zero this out when we actually *have* a new
	 * message. */
	
	memset( &current_state, 0, sizeof(current_state) );
	
	int bits[] = {
	/* P1 */	(1<<9), (1<<12), (1<<13), (1<<11), (1<<10),
	/* ESC */	(1<<16),
	/* P1 */	(1<<17), (1<<20), (1<<21), (1<<19), (1<<18),
	};

	for (int butno = 0 ; butno < NUM_PUMP_PAD_BUTTONS ; butno++)
	{
		if(!(ret & bits[butno]))
			current_state[butno] = true;
	}
}


#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: RageInput.h

 Desc: Wrapper for DirectInput.  Generates InputEvents.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/


//-----------------------------------------------------------------------------
// In-line Links
//-----------------------------------------------------------------------------
#pragma comment(lib, "dinput8.lib") 
#pragma comment(lib, "dxguid.lib") 

// #define HAVE_DDK
#ifdef HAVE_DDK
#pragma comment(lib, "setupapi.lib") 
#pragma comment(lib, "hid.lib") 
#endif

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <windows.h>
#include "RageInput.h"
#include <dinput.h>
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
		ASSERT( false );
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
		sReturn = "Key ";

		switch( button )
		{
		case DIK_ESCAPE:	sReturn += "Escape";	break;
		case DIK_1:			sReturn += "1";			break;
		case DIK_2:			sReturn += "2";			break;
		case DIK_3:			sReturn += "3";			break;
		case DIK_4:			sReturn += "4";			break;
		case DIK_5:			sReturn += "5";			break;
		case DIK_6:			sReturn += "6";			break;
		case DIK_7:			sReturn += "7";			break;
		case DIK_8:			sReturn += "8";			break;
		case DIK_9:			sReturn += "9";			break;
		case DIK_0:			sReturn += "0";			break;
		case DIK_MINUS:		sReturn += "Minus";		break;
		case DIK_EQUALS:	sReturn += "Equals";	break;
		case DIK_BACK:		sReturn += "Backsp";	break;
		case DIK_TAB:		sReturn += "Tab";		break;
		case DIK_Q:			sReturn += "Q";			break;
		case DIK_W:			sReturn += "W";			break;
		case DIK_E:			sReturn += "E";			break;
		case DIK_R:			sReturn += "R";			break;
		case DIK_T:			sReturn += "T";			break;
		case DIK_Y:			sReturn += "Y";			break;
		case DIK_U:			sReturn += "U";			break;
		case DIK_I:			sReturn += "I";			break;
		case DIK_O:			sReturn += "O";			break;
		case DIK_P:			sReturn += "P";			break;
		case DIK_LBRACKET:	sReturn += "LBracket";	break;
		case DIK_RBRACKET:	sReturn += "RBracket";	break;
		case DIK_RETURN:	sReturn += "Return";	break;
		case DIK_LCONTROL:	sReturn += "LControl";	break;
		case DIK_A:			sReturn += "A";			break;
		case DIK_S:			sReturn += "S";			break;
		case DIK_D:			sReturn += "D";			break;
		case DIK_F:			sReturn += "F";			break;
		case DIK_G:			sReturn += "G";			break;
		case DIK_H:			sReturn += "H";			break;
		case DIK_J:			sReturn += "J";			break;
		case DIK_K:			sReturn += "K";			break;
		case DIK_L:			sReturn += "L";			break;
		case DIK_SEMICOLON:	sReturn += "Semicln";	break;
		case DIK_APOSTROPHE:sReturn += "Apostro";	break;
		case DIK_GRAVE:		sReturn += "Grave";		break;
		case DIK_LSHIFT:	sReturn += "LShift";	break;
		case DIK_BACKSLASH:	sReturn += "Backslsh";	break;
		case DIK_Z:			sReturn += "Z";			break;
		case DIK_X:			sReturn += "X";			break;
		case DIK_C:			sReturn += "C";			break;
		case DIK_V:			sReturn += "V";			break;
		case DIK_B:			sReturn += "B";			break;
		case DIK_N:			sReturn += "N";			break;
		case DIK_M:			sReturn += "M";			break;
		case DIK_COMMA:		sReturn += "Comma";		break;
		case DIK_PERIOD:	sReturn += "Period";	break;
		case DIK_SLASH:		sReturn += "Slash";		break;
		case DIK_RSHIFT:	sReturn += "RShift";	break;
		case DIK_MULTIPLY:	sReturn += "Mult";		break;
		case DIK_LMENU:		sReturn += "LMenu";		break;
		case DIK_SPACE:		sReturn += "Space";		break;
		case DIK_CAPITAL:	sReturn += "CapsLk";	break;
		case DIK_F1:		sReturn += "F1";		break;
		case DIK_F2:		sReturn += "F2";		break;
		case DIK_F3:		sReturn += "F3";		break;
		case DIK_F4:		sReturn += "F4";		break;
		case DIK_F5:		sReturn += "F5";		break;
		case DIK_F6:		sReturn += "F6";		break;
		case DIK_F7:		sReturn += "F7";		break;
		case DIK_F8:		sReturn += "F8";		break;
		case DIK_F9:		sReturn += "F9";		break;
		case DIK_F10:		sReturn += "F10";		break;
		case DIK_NUMLOCK:	sReturn += "Numlock";	break;
		case DIK_SCROLL:	sReturn += "Scroll";	break;
		case DIK_NUMPAD7:	sReturn += "NumPad7";	break;
		case DIK_NUMPAD8:	sReturn += "NumPad8";	break;
		case DIK_NUMPAD9:	sReturn += "NumPad9";	break;
		case DIK_SUBTRACT:	sReturn += "Subtract";	break;
		case DIK_NUMPAD4:	sReturn += "NumPad4";	break;
		case DIK_NUMPAD5:	sReturn += "NumPad5";	break;
		case DIK_NUMPAD6:	sReturn += "NumPad6";	break;
		case DIK_ADD:		sReturn += "Add";		break;
		case DIK_NUMPAD1:	sReturn += "NumPad1";	break;
		case DIK_NUMPAD2:	sReturn += "NumPad2";	break;
		case DIK_NUMPAD3:	sReturn += "NumPad3";	break;
		case DIK_NUMPAD0:	sReturn += "NumPad0";	break;
		case DIK_DECIMAL:	sReturn += "Decimal";	break;
		case DIK_RMENU:		sReturn += "RightAlt";	break;
		case DIK_PAUSE:		sReturn += "Pause";		break;
		case DIK_HOME:		sReturn += "Home";		break;
		case DIK_UP:		sReturn += "Up";		break;
		case DIK_PRIOR:		sReturn += "PageUp";	break;
		case DIK_LEFT:		sReturn += "Left";		break;
		case DIK_RIGHT:		sReturn += "Right";		break;
		case DIK_END:		sReturn += "End";		break;
		case DIK_DOWN:		sReturn += "Down";		break;
		case DIK_NEXT:		sReturn += "PageDn";	break;
		case DIK_INSERT:	sReturn += "Insert";	break;
		case DIK_DELETE:	sReturn += "Delete";	break;
		case DIK_LWIN:		sReturn += "LeftWin";	break;
		case DIK_RWIN:		sReturn += "RightWin";	break;
		case DIK_APPS:		sReturn += "AppMenu";	break;
		case DIK_NUMPADENTER:	sReturn += "PadEnter";	break;
		case DIK_DIVIDE:	sReturn += "PadSlash";	break;

		default:			sReturn += "Unknown Key"; break;
		}
		break;
	default:
		ASSERT( false );	// what device is this?
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

	if( a.GetSize() != 2 ) {
		device = DEVICE_NONE;
		return false;
	}

	device = (InputDevice)atoi( a[0] );
	button = atoi( a[1] );
	return true;
}

/* FIXME: The main font doesn't have '`' (0x29), so that's disabled. */
static const char dik_charmap[] = {
	    /*   0   1   2   3   4   5   6   7   8    9    A   B   C   D   E  F */ 
/* 0x0x */    0,  0,'1','2','3','4','5','6','7', '8','9', '0','-','=',  0,  0,
/* 0x1x */	'q','w','e','r','t','y','u','i','u', 'p','[', ']',  0,  0,'a','s',
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

char DeviceInput::ToChar() const
{
	switch( device )
	{
	case DEVICE_KEYBOARD:
		if( button < sizeof(dik_charmap) )
			return dik_charmap[button];
		return '\0';
	default:
		return '\0';
	}
}


//-----------------------------------------------------------------------------
// Name: EnumJoysticksCallMenuBack( const PlayerNumber p )
// Desc: Called once for each enumerated joystick. If we find one, create a
//       device interface on it so we can play with it.
//-----------------------------------------------------------------------------
BOOL CALLBACK	EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance,
										       VOID* pContext )
{
	RageInput* pInput = (RageInput*)pContext;
	LPDIRECTINPUT8 pDI = pInput->GetDirectInput();


	static int i=0;		// ASSUMPTION:  This callback is only used on application start!

    // Obtain an interface to the enumerated joystick.
	if( i >= NUM_JOYSTICKS  )
	    return DIENUM_STOP;		// we only care about the first 4 

	HRESULT hr = pDI->CreateDevice( pdidInstance->guidInstance, 
									&pInput->m_pJoystick[i++], 
									NULL );
	if( FAILED( hr ) )
		throw RageException( hr, "Error in CreateDevice() for joystick %d.", i );

	return DIENUM_CONTINUE;
}




//-----------------------------------------------------------------------------
// Name: EnumAxesCallMenuBack( const PlayerNumber p )
// Desc: Callback function for enumerating the axes on a joystick
//-----------------------------------------------------------------------------
BOOL CALLBACK	EnumAxesCallback( const DIDEVICEOBJECTINSTANCE* pdidoi,
									      VOID* pContext )
{
    LPDIRECTINPUTDEVICE8 pJoystick = (LPDIRECTINPUTDEVICE8)pContext;

    DIPROPRANGE diprg; 
    diprg.diph.dwSize       = sizeof(DIPROPRANGE); 
    diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
    diprg.diph.dwHow        = DIPH_BYID; 
    diprg.diph.dwObj        = pdidoi->dwType; // Specify the enumerated axis
    diprg.lMin              = -1000; 
    diprg.lMax              = +1000; 
    
	// Set the range for the axis
	if( FAILED( pJoystick->SetProperty( DIPROP_RANGE, &diprg.diph ) ) )
		return DIENUM_STOP;


    return DIENUM_CONTINUE;
}




RageInput::RageInput( HWND hWnd )
{
	LOG->Trace( "RageInput::RageInput()" );

	int i;

	m_hWnd = hWnd;


	m_pDI = NULL;
	m_pKeyboard = NULL;
	m_pMouse = NULL;
	for( i=0; i<NUM_JOYSTICKS; i++ )
		m_pJoystick[i]=NULL;


	ZeroMemory( &m_keys, sizeof(m_keys) );
	ZeroMemory( &m_oldKeys, sizeof(m_oldKeys) );
	for( i=0; i<NUM_JOYSTICKS; i++ )
	{
		ZeroMemory( &m_joyState[i], sizeof(m_joyState[i]) );
		ZeroMemory( &m_oldKeys[i], sizeof(m_joyState[i]) );
	}

	ZeroMemory( &m_pumpState, sizeof(m_pumpState) );
	ZeroMemory( &m_oldPumpState, sizeof(m_oldPumpState) );
	m_Pumps = new pump_t[NUM_PUMPS];

	Initialize();
}

RageInput::~RageInput()
{
	Release();
}

HRESULT RageInput::Initialize()
{
	HRESULT hr;

	////////////////////////////////
	// Create the DirectInput object
	////////////////////////////////
    if( FAILED(hr = DirectInput8Create( GetModuleHandle(NULL), DIRECTINPUT_VERSION, 
                                         IID_IDirectInput8, (VOID**)&m_pDI, NULL ) ) )
		throw RageException( hr, "DirectInput8Create failed." );
	
	/////////////////////////////
	// Create the keyboard device
	/////////////////////////////

	// Create our DirectInput Object for the Keyboard
    if( FAILED( hr = m_pDI->CreateDevice( GUID_SysKeyboard, &m_pKeyboard, NULL ) ) )
		m_pKeyboard = NULL;

	if(m_pKeyboard) {
		// Set our Cooperation Level with each Device
		if( FAILED( hr = m_pKeyboard->SetCooperativeLevel(m_hWnd, DISCL_FOREGROUND | 
																  DISCL_NOWINKEY |
																  DISCL_NONEXCLUSIVE) ) )
			throw RageException( hr, "m_pKeyboard->SetCooperativeLevel failed." );

		// Set the Data Format of each device
		if( FAILED( hr = m_pKeyboard->SetDataFormat(&c_dfDIKeyboard) ) )
			throw RageException( hr, "m_pKeyboard->SetDataFormat failed." );

		// Acquire the Keyboard Device
		//if( FAILED( hr = m_pKeyboard->Acquire() ) )
		//	throw RageException( "m_pKeyboard->Acquire failed.", hr );
	}


	//////////////////////////
	// Create the mouse device
	//////////////////////////
	
	// Obtain an interface to the system mouse device.
	if( FAILED( hr = m_pDI->CreateDevice( GUID_SysMouse, &m_pMouse, NULL ) ) )
		m_pMouse = NULL;

	if(m_pMouse) {
		if( FAILED( hr = m_pMouse->SetCooperativeLevel( m_hWnd, DISCL_NONEXCLUSIVE|DISCL_FOREGROUND ) ) )
			throw RageException( hr, "m_pMouse->SetCooperativeLevel failed." );
    
		if( FAILED( hr = m_pMouse->SetDataFormat( &c_dfDIMouse2 ) ) )
			throw RageException( hr, "m_pMouse->SetDataFormat failed." );
/*
    DIPROPDWORD dipdw;
    dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
    dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    dipdw.diph.dwObj        = 0;
    dipdw.diph.dwHow        = DIPH_DEVICE;
    dipdw.dwData            = DIPROPAXISMODE_ABS;

    if( FAILED( m_pMouse->SetProperty( DIPROP_AXISMODE, &dipdw.diph ) ) )
        return E_FAIL;*/
	//if( FAILED( hr = m_pMouse->Acquire()))
	//	throw RageException( "m_pMouse->Acquire failed.", hr );

		m_RelPosition_x = m_RelPosition_y = 0;

		m_AbsPosition_x = 640/2;
		m_AbsPosition_y = 480/2;
	}	

	{
		/* Nasty hack to work around a bug in DirectInput8: Pump pads
		 * crash EnumDevices.  Prevent this by opening the pad with
		 * exclusive access while we enumerate, then closing it when
		 * we finish.  We don't do anything with this; we open it
		 * for real down below, since we don't *really* want to open
		 * the pad with exclusive access.  (I also don't want to introduce
		 * dependencies, such as "pump must be initialized before joysticks",
		 * so this doesn't bite us again down the road.) */
		pump_t m_TempPumps[NUM_PUMPS];
		for(int pumpNo = 0; pumpNo < NUM_PUMPS; ++pumpNo)
			m_TempPumps[pumpNo].init(pumpNo, false);

		//////////////////////////////
		// Create the joystick devices
		//////////////////////////////
		// Look for joysticks
		// TODO:  Why is this function so slow to return?  Is it just my machine?
		if( FAILED( hr = m_pDI->EnumDevices( DI8DEVCLASS_GAMECTRL, 
											 EnumJoysticksCallback,
											 (VOID*)this, 
											 DIEDFL_ATTACHEDONLY ) ) )
			throw RageException( hr, "m_pDI->EnumDevices failed." );
	}
//	for(int pumpNo = 0; pumpNo < NUM_PUMPS; ++pumpNo)
//		m_Pumps[pumpNo].init(pumpNo);

	for( int i=0; i<NUM_JOYSTICKS; i++ )
	{
		// Set the data format to "simple joystick" - a predefined data format 
		//
		// A data format specifies which controls on a device we are interested in,
		// and how they should be reported. This tells DInput that we will be
		// passing a DIJOYSTATE2 structure to IDirectInputDevice::GetDeviceState().
		if( m_pJoystick[i] )
			if( FAILED( hr = m_pJoystick[i]->SetDataFormat( &c_dfDIJoystick2 ) ) )
				throw RageException( hr, "m_pJoystick[i]->SetDataFormat failed." );


		// Set the cooperative level to let DInput know how this device should
		// interact with the system and with other DInput applications.
		if( m_pJoystick[i] )
			if( FAILED( hr = m_pJoystick[i]->SetCooperativeLevel( m_hWnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND ) ) )
				throw RageException( hr, "m_pJoystick[i]->SetCooperativeLevel failed." );


		/*
		// Determine how many axis the joystick has (so we don't error out setting
		// properties for unavailable axis)	
		if ( m_pJoystick[i] )	{
			g_diDevCaps.dwSize = sizeof(DIDEVCAPS);
			if ( FAILED( hr = m_pJoystick[i]->GetCapabilities(&g_diDevCaps) ) )
				return hr;
		}
		*/

		// Enumerate the axes of the joyctick and set the range of each axis. Note:
		// we could just use the defaults, but we're just trying to show an example
		// of enumerating device objects (axes, buttons, etc.).
		if( m_pJoystick[i] )
			if ( FAILED( hr = m_pJoystick[i]->EnumObjects( EnumAxesCallback, (VOID*)m_pJoystick[i], DIDFT_AXIS ) ) )
				throw RageException( hr, "m_pJoystick[i]->EnumObjects failed." );

		// Acquire the newly created devices
		if( m_pJoystick[i] )
			if( FAILED( hr = m_pJoystick[i]->Acquire() ) )
				throw RageException( hr, "m_pJoystick[i]->Acquire failed." );
	}

	for(int pumpNo = 0; pumpNo < NUM_PUMPS; ++pumpNo)
	{
		if(m_Pumps[pumpNo].init(pumpNo))
			LOG->Trace("Found Pump pad %i\n", pumpNo);
	}

	return S_OK;
}

void RageInput::Release()
{
	// Unacquire the keyboard device
	if (m_pKeyboard)
		m_pKeyboard->Unacquire();
	// Release the keyboard device
	SAFE_RELEASE(m_pKeyboard);
	// Unacquire the Mouse device
	if (m_pMouse)
		m_pMouse->Unacquire();
	// Release the Mouse device
	SAFE_RELEASE(m_pMouse);

	for( int i=0; i<NUM_JOYSTICKS; i++ )
	{
		if (m_pJoystick[i])
			m_pJoystick[i]->Unacquire();
		// Release the Mouse device
		SAFE_RELEASE(m_pJoystick[i]);
	}

	// Release the DirectInput object
	SAFE_RELEASE(m_pDI);
	delete[] m_Pumps;
}

HRESULT RageInput::UpdateMouse()
{
	if( NULL == m_pMouse ) 
		return E_FAIL;

	ZeroMemory( &m_mouseState, sizeof(m_mouseState) );

	// Get the input's device state, and put the state in DIMOUSESTATE2
    if (FAILED(m_pMouse->GetDeviceState( sizeof(DIMOUSESTATE2), &m_mouseState )))
    {
        // If input is lost then acquire and keep trying 
		HRESULT hr = m_pMouse->Acquire();
        while( hr == DIERR_INPUTLOST ) 
            hr = m_pMouse->Acquire();

		return E_FAIL;
    }


	m_RelPosition_x = m_mouseState.lX;
	m_RelPosition_y = m_mouseState.lY;

	m_AbsPosition_x += m_mouseState.lX;
	m_AbsPosition_y += m_mouseState.lY;

	/* Clamp the mouse to 0...640-1, 0...480-1. */
	m_AbsPosition_x = clamp(m_AbsPosition_x, 0, 640-1);
	m_AbsPosition_y = clamp(m_AbsPosition_y, 0, 480-1);

	return S_OK;
}

HRESULT RageInput::UpdateKeyboard()
{
	ZeroMemory( &m_keys, sizeof(m_keys) );

	if( NULL == m_pKeyboard ) 
		return E_FAIL;

	if( FAILED(m_pKeyboard->GetDeviceState( sizeof(m_keys),(LPVOID)&m_keys )) )
	{
		// DirectInput may be telling us that the input stream has been
		// interrupted.  We aren't tracking any state between polls, so
		// we don't have any special reset that needs to be done.
		// We just re-acquire and try again.
        
		// If input is lost then acquire and keep trying 
		HRESULT hr = m_pKeyboard->Acquire();
		while( hr == DIERR_INPUTLOST ) 
			hr = m_pKeyboard->Acquire();

		return E_FAIL;
	}

	return S_OK;
}

HRESULT RageInput::UpdatePump()
{
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
	return S_OK;
}

HRESULT RageInput::Update()
{
// macros for reading DI state structures
#define IS_PRESSED(b)	(b & 0x80) 
#define AXIS_THRESHOLD	250		// joystick axis threshold
#define IS_LEFT(a)		(a <= -AXIS_THRESHOLD)
#define IS_RIGHT(a)		(a >=  AXIS_THRESHOLD)
#define IS_UP(a)		IS_LEFT(a)
#define IS_DOWN(a)		IS_RIGHT(a)


	HRESULT hr;
	
	//////////////////////////////////////////////////////////////////
	// the current state last update becomes the old state this update
	//////////////////////////////////////////////////////////////////

	CopyMemory( &m_oldKeys, &m_keys, sizeof(m_keys) );
	CopyMemory( &m_oldJoyState, &m_joyState, sizeof(m_joyState) );
	CopyMemory( &m_oldPumpState, &m_pumpState, sizeof(m_pumpState) );

	ZeroMemory( &m_joyState, sizeof(m_joyState) );


	////////////////////
	// Read the keyboard
	////////////////////
	UpdateKeyboard();

	////////////////////
	// Read the mouse
	////////////////////
	UpdateMouse();

	/////////////////////
	// Read the joysticks
	/////////////////////

	// read joystick state
	for( BYTE i=0; i<4; i++ )	// foreach joystick
	{
		// read joystick states
		if ( m_pJoystick[i] )
		{
			// Poll the device to read the current state
			hr = m_pJoystick[i]->Poll(); 
			if( FAILED(hr) )  
			{
				// DInput is telling us that the input stream has been
				// interrupted. We aren't tracking any state between polls, so
				// we don't have any special reset that needs to be done. We
				// just re-acquire and try again.
				hr = m_pJoystick[i]->Acquire();
				while( hr == DIERR_INPUTLOST ) 
					hr = m_pJoystick[i]->Acquire();

				// hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
				// may occur when the app is minimized or in the process of 
				// switching, so just try again later 
				return E_FAIL; 
			}

			// Get the input's device state
			if( FAILED( hr = m_pJoystick[i]->GetDeviceState( sizeof(DIJOYSTATE2), &m_joyState[i] ) ) )
				return hr; // The device should have been acquired during the Poll()
		}
	}

	UpdatePump();

	return S_OK;
}


bool RageInput::IsBeingPressed( DeviceInput di )
{
	switch( di.device )
	{
	case DEVICE_KEYBOARD:
		return 0 != m_keys[ di.button ];
	case DEVICE_JOY1:
	case DEVICE_JOY2:
	case DEVICE_JOY3:
	case DEVICE_JOY4: {
		int joy_index;
		joy_index = di.device - DEVICE_JOY1;

		switch( di.button )
		{
		case JOY_LEFT:
			return IS_LEFT( m_joyState[joy_index].lX );
		case JOY_RIGHT:
			return IS_RIGHT( m_joyState[joy_index].lX );
		case JOY_UP:
			return IS_UP( m_joyState[joy_index].lY );
		case JOY_DOWN:
			return IS_DOWN( m_joyState[joy_index].lY );
		default:	// a joystick button
			int button_index = di.button - JOY_1;
			return 0 != IS_PRESSED( m_joyState[joy_index].rgbButtons[button_index] );
		}
	}
	case DEVICE_PUMP1:
	case DEVICE_PUMP2:
	{
		int pump_index;
		pump_index = di.device - DEVICE_PUMP1;
		if(m_pumpState[pump_index].button[di.button])
			return 1;
		ASSERT(di.button < NUM_PUMP_PAD_BUTTONS);
		return m_pumpState[pump_index].button[di.button];
	}
	default:
		ASSERT( false ); // bad device
	}

	return false;	// how did we get here?!?
}

bool RageInput::WasBeingPressed( DeviceInput di )
{
	switch( di.device )
	{
	case DEVICE_KEYBOARD:
		return 0 != m_oldKeys[ di.button ];
	case DEVICE_JOY1:
	case DEVICE_JOY2:
	case DEVICE_JOY3:
	case DEVICE_JOY4:
		int joy_index;
		joy_index = di.device - DEVICE_JOY1;

		switch( di.button )
		{
		case JOY_LEFT:
			return IS_LEFT( m_oldJoyState[joy_index].lX );
		case JOY_RIGHT:
			return IS_RIGHT( m_oldJoyState[joy_index].lX );
		case JOY_UP:
			return IS_UP( m_oldJoyState[joy_index].lY );
		case JOY_DOWN:
			return IS_DOWN( m_oldJoyState[joy_index].lY );
		default:	// a joystick button
			int button_index = di.button - JOY_1;
			return 0 != IS_PRESSED( m_oldJoyState[joy_index].rgbButtons[button_index] );
		}

	case DEVICE_PUMP1:
	case DEVICE_PUMP2: {
		int pump_index;
		pump_index = di.device - DEVICE_PUMP1;
		ASSERT(di.button < NUM_PUMP_PAD_BUTTONS);
		return m_oldPumpState[pump_index].button[di.button];
	}

	default:
		ASSERT( false ); // bad device
	}

	return false;	// how did we get here?!?
}

#ifdef HAVE_DDK
extern "C" {
#include <setupapi.h>
#include <hidsdi.h>
}
#endif

char *USB::GetUSBDevicePath (int num)
{
#ifndef HAVE_DDK
	LOG->Trace( "Can't get USB device #%i: DDK not available.", 
		num );
	return NULL;
#else
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
#endif
}

HANDLE USB::OpenUSB (int VID, int PID, int num, bool share)
{
#ifndef HAVE_DDK
	LOG->Trace( "Can't open USB device %-4.4x:%-4.4x#%i: DDK not available.", 
		VID, PID, num );
    return INVALID_HANDLE_VALUE;
#else
    DWORD index = 0;

    char *path;
	HANDLE h = INVALID_HANDLE_VALUE;

    while ((path = GetUSBDevicePath (index++)) != NULL)
    {
		if(h != INVALID_HANDLE_VALUE)
			CloseHandle (h);

		int ShareMode = 0;
		if(share) ShareMode |= FILE_SHARE_READ | FILE_SHARE_WRITE;
		h = CreateFile (path, GENERIC_READ,
			   ShareMode, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

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
#endif
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

bool RageInput::pump_t::init(int devno, bool share)
{
	const int pump_usb_vid = 0x0d2f, pump_usb_pid = 0x0001;
	h = USB::OpenUSB (pump_usb_vid, pump_usb_pid, devno, share);
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
		int err = GetLastError();
		char ebuf[1024];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
			0, err, 0, ebuf, sizeof(ebuf), NULL);
		LOG->Trace("Error reading Pump pad: %s\n", ebuf);
	    return -1;
    }

    return buf;
}

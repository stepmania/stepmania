#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: RageInput.h

 Desc: Wrapper for DirectInput.  Generates InputEvents.

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/


//-----------------------------------------------------------------------------
// In-line Links
//-----------------------------------------------------------------------------
#pragma comment(lib, "dinput8.lib") 
#pragma comment(lib, "dxguid.lib") 


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <windows.h>
#include "RageInput.h"
#include <dinput.h>
#include "RageUtil.h"
#include "RageHelper.h"
#include "ErrorCatcher/ErrorCatcher.h"


RageInput*		INPUTM	= NULL;		// globally accessable input device



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
		sReturn = ssprintf("Joystick %d: ", device - DEVICE_JOY1 + 1 );

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

	case DEVICE_KEYBOARD:		// keyboard
		sReturn = "Keyboard: ";

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
		case DIK_BACK:		sReturn += "Backspace";	break;
		case DIK_TAB:		sReturn += "Tab";		break;
// why?		case DIK_BACK:		sReturn += "Backspace";	break;
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
		case DIK_SEMICOLON:	sReturn += "Semicolon";	break;
		case DIK_APOSTROPHE:sReturn += "Apostroph";	break;
		case DIK_GRAVE:		sReturn += "Grave";		break;
		case DIK_LSHIFT:	sReturn += "LShift";	break;
		case DIK_BACKSLASH:	sReturn += "Backslash";	break;
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
		case DIK_RSHIFT:	sReturn += "R Shift";	break;
		case DIK_MULTIPLY:	sReturn += "Multiply";	break;
		case DIK_LMENU:		sReturn += "Left Menu";	break;
		case DIK_SPACE:		sReturn += "Space";		break;
		case DIK_CAPITAL:	sReturn += "Caps Lock";	break;
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
		case DIK_RMENU:		sReturn += "Right Alt";	break;
		case DIK_PAUSE:		sReturn += "Pause";		break;
		case DIK_HOME:		sReturn += "Home";		break;
		case DIK_UP:		sReturn += "Up";		break;
		case DIK_PRIOR:		sReturn += "Page Up";	break;
		case DIK_LEFT:		sReturn += "Left";		break;
		case DIK_RIGHT:		sReturn += "Right";		break;
		case DIK_END:		sReturn += "End";		break;
		case DIK_DOWN:		sReturn += "Down";		break;
		case DIK_NEXT:		sReturn += "Page Down";	break;
		case DIK_INSERT:	sReturn += "Insert";	break;
		case DIK_DELETE:	sReturn += "Delete";	break;
		case DIK_LWIN:		sReturn += "Left Win";	break;
		case DIK_RWIN:		sReturn += "Right Win";	break;
		case DIK_APPS:		sReturn += "App Menu";	break;

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
};




//-----------------------------------------------------------------------------
// Name: EnumJoysticksCallMenuBack( PlayerNumber p )
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
	else
		ASSERT( false );	// we should never get here since the app is only initialized once

	HRESULT hr = pDI->CreateDevice( pdidInstance->guidInstance, 
									&pInput->m_pJoystick[i++], 
									NULL );
		FatalErrorHr( hr, "Error in CreateDevice() for joystick %d.", i );

	return DIENUM_CONTINUE;
}




//-----------------------------------------------------------------------------
// Name: EnumAxesCallMenuBack( PlayerNumber p )
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
		FatalErrorHr( hr, "DirectInput8Create failed." );
	
	/////////////////////////////
	// Create the keyboard device
	/////////////////////////////

	// Create our DirectInput Object for the Keyboard
    if( FAILED( hr = m_pDI->CreateDevice( GUID_SysKeyboard, &m_pKeyboard, NULL ) ) )
		FatalErrorHr( hr, "CreateDevice keyboard failed." );

	// Set our Cooperation Level with each Device
	if( FAILED( hr = m_pKeyboard->SetCooperativeLevel(m_hWnd, DISCL_FOREGROUND | 
															  DISCL_NOWINKEY |
															  DISCL_NONEXCLUSIVE) ) )
		FatalErrorHr( hr, "m_pKeyboard->SetCooperativeLevel failed." );

	// Set the Data Format of each device
	if( FAILED( hr = m_pKeyboard->SetDataFormat(&c_dfDIKeyboard) ) )
		FatalErrorHr( hr, "m_pKeyboard->SetDataFormat failed." );

	// Acquire the Keyboard Device
	//if( FAILED( hr = m_pKeyboard->Acquire() ) )
	//	FatalErrorHr( "m_pKeyboard->Acquire failed.", hr );



	//////////////////////////
	// Create the mouse device
	//////////////////////////
	
	// Obtain an interface to the system mouse device.
	if( FAILED( hr = m_pDI->CreateDevice( GUID_SysMouse, &m_pMouse, NULL ) ) )
		FatalErrorHr( hr, "CreateDevice mouse failed." );

    if( FAILED( hr = m_pMouse->SetCooperativeLevel( m_hWnd, DISCL_NONEXCLUSIVE|DISCL_FOREGROUND ) ) )
		FatalErrorHr( hr, "m_pMouse->SetCooperativeLevel failed." );
    
	if( FAILED( hr = m_pMouse->SetDataFormat( &c_dfDIMouse2 ) ) )
		FatalErrorHr( hr, "m_pMouse->SetDataFormat failed." );

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
	//	FatalErrorHr( "m_pMouse->Acquire failed.", hr );

	m_RelPosition_x = 0;
	m_RelPosition_y = 0;

	m_AbsPosition_x = 640/2;
	m_AbsPosition_y = 480/2;
	
	//////////////////////////////
	// Create the joystick devices
	//////////////////////////////
	// Look for joysticks
	// TODO:  Why is this function so slow to return?  Is it just my machine?
    if( FAILED( hr = m_pDI->EnumDevices( DI8DEVCLASS_GAMECTRL, 
                                         EnumJoysticksCallback,
                                         (VOID*)this, 
										 DIEDFL_ATTACHEDONLY ) ) )
		FatalErrorHr( hr, "m_pDI->EnumDevices failed." );

	for( int i=0; i<NUM_JOYSTICKS; i++ )
	{
		// Set the data format to "simple joystick" - a predefined data format 
		//
		// A data format specifies which controls on a device we are interested in,
		// and how they should be reported. This tells DInput that we will be
		// passing a DIJOYSTATE2 structure to IDirectInputDevice::GetDeviceState().
		if( m_pJoystick[i] )
			if( FAILED( hr = m_pJoystick[i]->SetDataFormat( &c_dfDIJoystick2 ) ) )
				FatalErrorHr( hr, "m_pJoystick[i]->SetDataFormat failed." );


		// Set the cooperative level to let DInput know how this device should
		// interact with the system and with other DInput applications.
		if( m_pJoystick[i] )
			if( FAILED( hr = m_pJoystick[i]->SetCooperativeLevel( m_hWnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND ) ) )
				FatalErrorHr( hr, "m_pJoystick[i]->SetCooperativeLevel failed." );


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
				FatalErrorHr( hr, "m_pJoystick[i]->EnumObjects failed." );

		// Acquire the newly created devices
		if( m_pJoystick[i] )
			if( FAILED( hr = m_pJoystick[i]->Acquire() ) )
				FatalErrorHr( hr, "m_pJoystick[i]->Acquire failed." );
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
}



HRESULT RageInput::Update()
{
// macros for reading DI state structures
#define IS_PRESSED(b)	(b & 0x80) 
#define AXIS_THRESHOLD	50		// joystick axis threshold
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

	ZeroMemory( &m_keys, sizeof(m_keys) );
	ZeroMemory( &m_joyState, sizeof(m_joyState) );


	////////////////////
	// Read the keyboard
	////////////////////
	if( NULL == m_pKeyboard ) 
		return E_FAIL;

	if FAILED(m_pKeyboard->GetDeviceState( sizeof(m_keys),(LPVOID)&m_keys ))
	{
		// DirectInput may be telling us that the input stream has been
		// interrupted.  We aren't tracking any state between polls, so
		// we don't have any special reset that needs to be done.
		// We just re-acquire and try again.
        
		// If input is lost then acquire and keep trying 
		hr = m_pKeyboard->Acquire();
		while( hr == DIERR_INPUTLOST ) 
			hr = m_pKeyboard->Acquire();

		return E_FAIL;
	}


	////////////////////
	// Read the mouse
	////////////////////
	if( NULL == m_pMouse ) 
		return E_FAIL;

	ZeroMemory( &m_mouseState, sizeof(m_mouseState) );

	// Get the input's device state, and put the state in DIMOUSESTATE2
    if (FAILED(m_pMouse->GetDeviceState( sizeof(DIMOUSESTATE2), &m_mouseState )))
    {
        // If input is lost then acquire and keep trying 
        hr = m_pMouse->Acquire();
        while( hr == DIERR_INPUTLOST ) 
            hr = m_pMouse->Acquire();

		return E_FAIL;
    }


	m_RelPosition_x = m_mouseState.lX;
	m_RelPosition_y = m_mouseState.lY;


	m_AbsPosition_x += m_mouseState.lX;
	m_AbsPosition_y += m_mouseState.lY;

	if (m_AbsPosition_x >= 640)
		m_AbsPosition_x = 640-1;
	else
		if (m_AbsPosition_x < 0)
			m_AbsPosition_x = 0;

	// now the y boundaries
	if (m_AbsPosition_y >= 480)
		m_AbsPosition_y= 480-1;
	else
		if (m_AbsPosition_y < 0)
			m_AbsPosition_y = 0;


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
	case DEVICE_JOY4:
		int joy_index;
		joy_index = di.device - DEVICE_JOY1;

		switch( di.button )
		{
		case JOY_LEFT:
			return IS_LEFT( m_joyState[joy_index].lX );
		case JOY_RIGHT:
			return IS_RIGHT( m_joyState[joy_index].lX );
		case JOY_UP:
			return IS_UP( m_joyState[joy_index].lX );
		case JOY_DOWN:
			return IS_DOWN( m_joyState[joy_index].lX );
		default:	// a joystick button
			int button_index = di.button - JOY_1;
			return 0 != IS_PRESSED( m_joyState[joy_index].rgbButtons[button_index] );
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
			return IS_UP( m_oldJoyState[joy_index].lX );
		case JOY_DOWN:
			return IS_DOWN( m_oldJoyState[joy_index].lX );
		default:	// a joystick button
			int button_index = di.button - JOY_1;
			return 0 != IS_PRESSED( m_oldJoyState[joy_index].rgbButtons[button_index] );
		}
	default:
		ASSERT( false ); // bad device
	}

	return false;	// how did we get here?!?
}
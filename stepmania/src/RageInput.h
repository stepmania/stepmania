/*
-----------------------------------------------------------------------------
 File: RageInput.h

 Desc: Wrapper for DirectInput.  Generates InputEvents.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

#ifndef _RAGEINPUT_H_
#define _RAGEINPUT_H_


#include <dinput.h>
#include "RageUtil.h"


#define NUM_JOYSTICKS 4

// button byte codes for directional pad
#define JOY_LEFT	101
#define JOY_RIGHT	102
#define JOY_UP		103
#define JOY_DOWN	104



#define DEVICE_NONE		0
#define DEVICE_KEYBOARD	1
#define DEVICE_JOYSTICK	2


class RageRawInput
{
public:
	RageRawInput() { device = device_no = button = just_pressed = 0; };
	RageRawInput( BYTE device, 
				  BYTE device_no,
				  BYTE button,
				  BYTE just_pressed )
	{ 
		this->device = device; 
		this->device_no = device_no;
		this->button = button;
		this->just_pressed = just_pressed;
	};
	RageRawInput( CString sEncoding )
	{ 
		CStringArray a;
		split( sEncoding, "-", a);
		this->device    = atoi( a[0] );
		this->device_no = atoi( a[1] );
		this->button    = atoi( a[2] );
	};
	CString Encode() { return ssprintf("%u-%u-%u", device, device_no, button); };

	BOOL LookupChar( TCHAR &char_out ) const;
	CString GetDescription() const;

	BOOL IsBlank() { return device == DEVICE_NONE; };

	BYTE device;
	BYTE device_no;
	BYTE button;
	BYTE just_pressed;
};



typedef CList<RageRawInput, RageRawInput&> RageRawInputList;


class RageInput
{
	// Our Windows Handle
	HWND m_hWnd;

	// Main DirectInput Object
	LPDIRECTINPUT8			m_pDI;	
	// Keyboard Device
	LPDIRECTINPUTDEVICE8	m_pKeyboard;
	// Mouse Device
	LPDIRECTINPUTDEVICE8    m_pMouse;
public:
	// hack: make them public to allow the callbacks access to the pointers
	// Joystick Devices
	LPDIRECTINPUTDEVICE8	m_pJoystick[NUM_JOYSTICKS];

private:
	// Arrays for Keyboard Data
	byte m_keys[256];
	byte m_oldKeys[256];

	// DirectInput mouse state structure
	DIMOUSESTATE2 m_mouseState;

	// Joystick data for 4 controllers
    DIJOYSTATE2 m_joyState[4];
    DIJOYSTATE2 m_oldJoyState[4];

	INT m_AbsPosition_x;
	INT m_AbsPosition_y;

	INT m_RelPosition_x;
	INT m_RelPosition_y;

public:
	RageInput(HWND hWnd);
	~RageInput();

	// Initialize DirectInput Resources
	HRESULT Initialize();
	// Release all DirectInput Resources
	VOID Release();
	// Get our Devices State
	HRESULT GetRawInput( RageRawInputList &listRawInput );

	LPDIRECTINPUT8		 GetDirectInput() { return m_pDI; }
	LPDIRECTINPUTDEVICE8 GetMouseDevice() { return m_pMouse; }
	LPDIRECTINPUTDEVICE8 GetKeyboardDevice() { return m_pKeyboard; }
	LPDIRECTINPUTDEVICE8 GetJoystickDevice( int i ) { return m_pJoystick[i]; }

//	DIMOUSESTATE2		 GetMouseState() { return dimMouseState; }
	VOID GetAbsPosition( DWORD &x, DWORD &y ) { x = m_AbsPosition_x; y = m_AbsPosition_y; }

};


typedef RageInput* LPRageInput;

extern LPRageInput			INPUT;	// global and accessable from anywhere in our program


#endif
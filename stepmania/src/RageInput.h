#pragma once
/*
-----------------------------------------------------------------------------
 File: RageInput.h

 Desc: Wrapper for DirectInput.  Generates InputEvents.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#ifndef DIRECTINPUT_VERSION 
#define DIRECTINPUT_VERSION 0x0800
#endif

#include <dinput.h>
#include "RageUtil.h"

const int NUM_KEYBOARD_BUTTONS = 256;

const int NUM_JOYSTICKS = 4;
const int NUM_PUMPS = 2;
const int NUM_PUMP_PAD_BUTTONS = 6;

enum InputDevice {
	DEVICE_KEYBOARD = 0,
	DEVICE_JOY1,
	DEVICE_JOY2,
	DEVICE_JOY3,
	DEVICE_JOY4,
	DEVICE_PUMP1,
	DEVICE_PUMP2,
	NUM_INPUT_DEVICES,	// leave this at the end
	DEVICE_NONE			// means this is NULL
};

// button byte codes for directional pad
enum JoystickButton {
	JOY_LEFT = 0,
	JOY_RIGHT,
	JOY_UP,
	JOY_DOWN,
	JOY_1,
	JOY_2,
	JOY_3,
	JOY_4,
	JOY_5,
	JOY_6,
	JOY_7,
	JOY_8,
	JOY_9,
	JOY_10,
	JOY_11,
	JOY_12,
	JOY_13,
	JOY_14,
	JOY_15,
	JOY_16,
	JOY_17,
	JOY_18,
	JOY_19,
	JOY_20,
	JOY_21,
	JOY_22,
	JOY_23,
	JOY_24,
	NUM_JOYSTICK_BUTTONS	// leave this at the end
};

const int NUM_DEVICE_BUTTONS = MAX( MAX( NUM_KEYBOARD_BUTTONS, NUM_JOYSTICK_BUTTONS ), 
								NUM_PUMP_PAD_BUTTONS );

extern const char *PumpButtonNames[];

struct DeviceInput
{
public:
	InputDevice device;
	int button;

	DeviceInput() { device=DEVICE_NONE; };
	DeviceInput( InputDevice d, int b ) { device=d; button=b; };

	bool operator==( const DeviceInput &other ) 
	{ 
		return device == other.device  &&  button == other.button;
	};

	CString GetDescription();
	
	CString toString();
	bool fromString( const CString &s );

	bool IsValid() const { return device != DEVICE_NONE; };
	void MakeInvalid() { device = DEVICE_NONE; };

	static int NumButtons(InputDevice device);
};



class RageInput
{
	// Our Screens Handle
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
	byte m_keys[NUM_KEYBOARD_BUTTONS];
	byte m_oldKeys[NUM_KEYBOARD_BUTTONS];

	// DirectInput mouse state structure
	DIMOUSESTATE2 m_mouseState;

	// Joystick data for NUM_JOYSTICKS controllers
    DIJOYSTATE2 m_joyState[NUM_JOYSTICKS];
    DIJOYSTATE2 m_oldJoyState[NUM_JOYSTICKS];

	struct {
		bool button[NUM_PUMP_PAD_BUTTONS];
	} m_pumpState[NUM_PUMPS], m_oldPumpState[NUM_PUMPS];

	/* Structure for reading Pump pads: */
	struct pump_t {
		HANDLE h;
		OVERLAPPED ov;
		long buf;
		bool pending;

		pump_t();
		~pump_t();
		init(int devno);
		int GetPadEvent();
	} *m_Pumps;

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
	HRESULT Update();
	bool IsBeingPressed( DeviceInput di );
	bool WasBeingPressed( DeviceInput di );

	LPDIRECTINPUT8		 GetDirectInput() { return m_pDI; }
	LPDIRECTINPUTDEVICE8 GetMouseDevice() { return m_pMouse; }
	LPDIRECTINPUTDEVICE8 GetKeyboardDevice() { return m_pKeyboard; }
	LPDIRECTINPUTDEVICE8 GetJoystickDevice( int i ) { return m_pJoystick[i]; }

//	DIMOUSESTATE2		 GetMouseState() { return dimMouseState; }
	VOID GetAbsPosition( DWORD &x, DWORD &y ) { x = m_AbsPosition_x; y = m_AbsPosition_y; }

};

namespace USB {
	char *GetUSBDevicePath (int num);
	HANDLE OpenUSB (int VID, int PID, int num);
};

extern RageInput*			INPUTMAN;	// global and accessable from anywhere in our program

#pragma once
/*
-----------------------------------------------------------------------------
 Class: InputFilter

 Desc: Checks RageInput and generates a list of InputEvents, which can 
	represent when a button is first pressed, when a button is released,
	or when an repeating input fires.

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "RageInput.h"

const float TIME_BEFORE_SLOW_REPEATS = 0.7f;
const float TIME_BEFORE_FAST_REPEATS = 2.0f;

const float TIME_BETWEEN_SLOW_REPEATS = 0.25f;
const float TIME_BETWEEN_FAST_REPEATS = 0.125f;

enum InputEventType { IET_FIRST_PRESS, IET_SLOW_REPEAT, IET_FAST_REPEAT, IET_RELEASE };

class InputEvent : public DeviceInput
{
public:
	InputEvent() { type=IET_FIRST_PRESS; };
	InputEvent( InputDevice d, int b, InputEventType t ) { device=d; button=b; type=t; };
	InputEvent( DeviceInput di, InputEventType t ) { device=di.device; button=di.button; type=t; };

	InputEventType type;
};

typedef CArray<InputEvent, InputEvent> InputEventArray;

class InputFilter
{
public:
	InputFilter()
	{
		for( int i=0; i<NUM_INPUT_DEVICES; i++ )
		{
			for( int j=0; j<NUM_DEVICE_BUTTONS; j++ )
				m_fTimeHeld[i][j] = 0;
		}
	};

	void GetInputEvents( InputEventArray &array, float fDeltaTime );

	float m_fTimeHeld[NUM_INPUT_DEVICES][NUM_DEVICE_BUTTONS];
};


extern InputFilter*	FILTER;	// global and accessable from anywhere in our program


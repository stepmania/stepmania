#pragma once
/*
-----------------------------------------------------------------------------
 Class: InputFilter

 Desc: Checks RageInput and generates a list of InputEvents, which can 
	represent when a button is first pressed, when a button is released,
	or when an repeating input fires.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "RageInput.h"

const float TIME_BEFORE_SLOW_REPEATS = 0.6f;
const float TIME_BEFORE_FAST_REPEATS = 1.5f;

//const float TIME_BETWEEN_SLOW_REPEATS = 0.25f;
const float TIME_BETWEEN_SLOW_REPEATS = 0.125f;
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
	InputFilter();

	bool BeingPressed( DeviceInput di, bool Prev = false);
	bool WasBeingPressed( DeviceInput di );
	bool IsBeingPressed( DeviceInput di );
	
	void GetInputEvents( InputEventArray &array, float fDeltaTime );

	float m_fTimeHeld[NUM_INPUT_DEVICES][NUM_DEVICE_BUTTONS];
};


extern InputFilter*	INPUTFILTER;	// global and accessable from anywhere in our program



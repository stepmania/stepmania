/* InputFilter - Checks RageInput and generates a list of InputEvents, representing button presses, releases, and repeats. */

#ifndef INPUTFILTER_H
#define INPUTFILTER_H

#include "RageInputDevice.h"

enum InputEventType { IET_FIRST_PRESS, IET_SLOW_REPEAT, IET_FAST_REPEAT, IET_RELEASE };

struct InputEvent : public DeviceInput
{
	InputEvent() { type=IET_FIRST_PRESS; };
	InputEvent( InputDevice d, int b, InputEventType t ): DeviceInput(d, b) { type=t; };
	InputEvent( DeviceInput di, InputEventType t ): DeviceInput(di) { type=t; };

	InputEventType type;
};

typedef vector<InputEvent> InputEventArray;

class RageMutex;
class InputFilter
{
	bool m_BeingHeld[NUM_INPUT_DEVICES][MAX_DEVICE_BUTTONS];
	bool m_BeingForced[NUM_INPUT_DEVICES][MAX_DEVICE_BUTTONS];
	float m_fSecsHeld[NUM_INPUT_DEVICES][MAX_DEVICE_BUTTONS];

	/* If > 0, then when it reaches 0, stop forcing. */
	float m_fSecsToForce[NUM_INPUT_DEVICES][MAX_DEVICE_BUTTONS];
	InputEventArray queue;
	RageMutex *queuemutex;

	void ForceKey( DeviceInput di, float duration );
	void StopForcingKey( DeviceInput di );

public:
	void ButtonPressed( DeviceInput di, bool Down );
	void ResetDevice( InputDevice dev );

	InputFilter();
	~InputFilter();
	void Reset();
	void Update(float fDeltaTime);

	void SetRepeatRate( float fSlowDelay, float fSlowRate, float fFastDelay, float fFastRate );
	void ResetRepeatRate();
	void ResetKeyRepeat( DeviceInput di );

	bool IsBeingPressed( DeviceInput di );
	float GetSecsHeld( DeviceInput di );
	
	void GetInputEvents( InputEventArray &array );
};


extern InputFilter*	INPUTFILTER;	// global and accessable from anywhere in our program


#endif

/*
 * (c) 2001-2004 Chris Danford
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

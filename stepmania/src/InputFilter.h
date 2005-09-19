/* InputFilter - Checks RageInput and generates a list of InputEvents, representing button presses, releases, and repeats. */

#ifndef INPUTFILTER_H
#define INPUTFILTER_H

#include "RageInputDevice.h"

enum InputEventType
{
	/* The device was just pressed. */
	IET_FIRST_PRESS,

	/* The device is auto-repeating.  These events are guaranteed to be sent only between
	 * IET_FIRST_PRESS and IET_RELEASE pairs. */
	IET_SLOW_REPEAT,
	IET_FAST_REPEAT,

	/* The device is no longer pressed.  Exactly one IET_RELEASE event will be sent
	 * for each IET_FIRST_PRESS. */
	IET_RELEASE,

	/* The depression level of a button has changed.  Read di.level to find the new
	 * value.  This can be sent outside of a IET_FIRST_PRESS/IET_RELEASE pair, since
	 * a button/axis can have a non-zero level (eg. outside the axis dead zone) without
	 * being high enough to count as a press. */
	IET_LEVEL_CHANGED
};

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
public:
	void ButtonPressed( DeviceInput di, bool Down );
	void SetButtonComment( DeviceInput di, const CString &sComment = "" );
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
	CString GetButtonComment( DeviceInput di ) const;
	
	void GetInputEvents( InputEventArray &array );

private:
	struct ButtonState
	{
		ButtonState();
		bool m_BeingHeld; // actual current state
		bool m_bLastReportedHeld; // last state reported by Update()
		CString m_sComment;
		float m_fSecsHeld;
		float m_Level, m_LastLevel;

		// Timestamp of the last real event that came in: the time m_BeingHeld changed.
		RageTimer m_BeingHeldTime;

		// The time that we actually reported the last event (used for debouncing).
		RageTimer m_LastReportTime;
	};
	ButtonState m_ButtonState[NUM_INPUT_DEVICES][MAX_DEVICE_BUTTONS];

	InputEventArray queue;
	RageMutex *queuemutex;
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

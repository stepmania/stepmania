/* InputFilter - Checks RageInput and generates a list of InputEvents, representing button presses, releases, and repeats. */

#ifndef INPUT_FILTER_H
#define INPUT_FILTER_H

#include "RageInputDevice.h"

enum InputEventType
{
	/* The device was just pressed. */
	IET_FIRST_PRESS,

	/* The device is auto-repeating.  This event is guaranteed to be sent only between
	 * IET_FIRST_PRESS and IET_RELEASE pairs. */
	IET_REPEAT,

	/* The device is no longer pressed.  Exactly one IET_RELEASE event will be sent
	 * for each IET_FIRST_PRESS. */
	IET_RELEASE,
};

struct InputEvent
{
	InputEvent() { type=IET_FIRST_PRESS; };

	DeviceInput di;
	InputEventType type;

	/* A list of all buttons that were pressed at the time of this event: */
	DeviceInputList m_ButtonState;
};

class RageMutex;
struct ButtonState;
class InputFilter
{
public:
	void ButtonPressed( const DeviceInput &di, bool Down );
	void SetButtonComment( const DeviceInput &di, const RString &sComment = "" );
	void ResetDevice( InputDevice dev );

	InputFilter();
	~InputFilter();
	void Reset();
	void Update( float fDeltaTime );

	void SetRepeatRate( float fDelay, float fRepeatRate );
	void ResetRepeatRate();
	void ResetKeyRepeat( const DeviceInput &di );

	// If aButtonState is NULL, use the last reported state.
	bool IsBeingPressed( const DeviceInput &di, const DeviceInputList *pButtonState = NULL ) const;
	float GetSecsHeld( const DeviceInput &di, const DeviceInputList *pButtonState = NULL ) const;
	RString GetButtonComment( const DeviceInput &di ) const;
	
	void GetInputEvents( vector<InputEvent> &aEventOut );
	void GetPressedButtons( vector<DeviceInput> &array ) const;

private:
	void CheckButtonChange( ButtonState &bs, DeviceInput di, const RageTimer &now );
	void ReportButtonChange( const DeviceInput &di, InputEventType t );
	void MakeButtonStateList( vector<DeviceInput> &aInputOut ) const;

	vector<InputEvent> queue;
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

/* InputFilter - Checks RageInput and generates a list of InputEvents, representing button presses, releases, and repeats. */

#ifndef INPUT_FILTER_H
#define INPUT_FILTER_H

#include "RageInputDevice.h"

#include <vector>


enum InputEventType
{
	// The device was just pressed.
	IET_FIRST_PRESS,

	/* The device is auto-repeating. This event is guaranteed to be sent only
	 * between IET_FIRST_PRESS and IET_RELEASE pairs. */
	IET_REPEAT,

	/* The device is no longer pressed. Exactly one IET_RELEASE event will be
	 * sent for each IET_FIRST_PRESS. */
	IET_RELEASE,

	NUM_InputEventType,
	InputEventType_Invalid
};

const RString& InputEventTypeToString(InputEventType cat);
const RString& InputEventTypeToLocalizedString(InputEventType cat);
LuaDeclareType(InputEventType);

struct InputEvent
{
	InputEvent(): type(IET_FIRST_PRESS) {}

	DeviceInput di;
	InputEventType type;

	// A list of all buttons that were pressed at the time of this event:
	DeviceInputList m_ButtonState;
};

struct MouseCoordinates
{
	float fX;
	float fY;
	float fZ;
};

class RageMutex;
struct ButtonState;
class InputFilter
{
public:
	void ButtonPressed( const DeviceInput &di );
	void SetButtonComment( const DeviceInput &di, const RString &sComment = "" );
	void ResetDevice( InputDevice dev );

	InputFilter();
	~InputFilter();
	void Reset();
	void Update( float fDeltaTime );

	void SetRepeatRate( float fRepeatRate );
	void SetRepeatDelay( float fDelay );
	void ResetRepeatRate();
	void ResetKeyRepeat( const DeviceInput &di );
	void RepeatStopKey( const DeviceInput &di );

	// If aButtonState is nullptr, use the last reported state.
	bool IsBeingPressed( const DeviceInput &di, const DeviceInputList *pButtonState = nullptr ) const;
	float GetSecsHeld( const DeviceInput &di, const DeviceInputList *pButtonState = nullptr ) const;
	float GetLevel( const DeviceInput &di, const DeviceInputList *pButtonState = nullptr ) const;
	RString GetButtonComment( const DeviceInput &di ) const;

	void GetInputEvents( std::vector<InputEvent> &aEventOut );
	void GetPressedButtons( std::vector<DeviceInput> &array ) const;

	// cursor
	void UpdateCursorLocation(float _fX, float _fY);
	void UpdateMouseWheel(float _fZ);
	float GetCursorX(){ return m_MouseCoords.fX; }
	float GetCursorY(){ return m_MouseCoords.fY; }
	float GetMouseWheel(){ return m_MouseCoords.fZ; }

	// Lua
	void PushSelf( lua_State *L );

private:
	void CheckButtonChange( ButtonState &bs, DeviceInput di, const RageTimer &now );
	void ReportButtonChange( const DeviceInput &di, InputEventType t );
	void MakeButtonStateList( std::vector<DeviceInput> &aInputOut ) const;

	std::vector<InputEvent> queue;
	RageMutex *queuemutex;
	MouseCoordinates m_MouseCoords;

	InputFilter(const InputFilter& rhs);
	InputFilter& operator=(const InputFilter& rhs);
};

extern InputFilter*	INPUTFILTER;	// global and accessible from anywhere in our program

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

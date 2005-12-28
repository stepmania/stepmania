#include "global.h"
#include "InputFilter.h"
#include "RageLog.h"
#include "RageInput.h"
#include "RageUtil.h"
#include "RageThreads.h"
#include "Preference.h"
#include "Foreach.h"

#include <set>
struct ButtonState
{
	ButtonState();
	bool m_BeingHeld; // actual current state
	bool m_bLastReportedHeld; // last state reported by Update()
	CString m_sComment;
	float m_fSecsHeld;
	float m_Level, m_LastLevel;

	// Timestamp of m_BeingHeld changing.
	RageTimer m_BeingHeldTime;

	// The time that we actually reported the last event (used for debouncing).
	RageTimer m_LastReportTime;
};

namespace
{
	/* Maintain a set of all interesting buttons: buttons which are being held
	 * down, or which were held down and need a RELEASE event.  We use this to
	 * optimize InputFilter::Update, so we don't have to process every button
	 * we know about when most of them aren't in use.  This set is protected
	 * by queuemutex. */
	typedef map<DeviceInput, ButtonState> ButtonStateMap;
	ButtonStateMap g_ButtonStates;
	ButtonState &GetButtonState( InputDevice device, DeviceButton button )
	{
		return g_ButtonStates[ DeviceInput(device, button) ];
	}
}

/*
 * Some input devices require debouncing.  Do this on both press and release.  After
 * reporting a change in state, don't report another for the debounce period.  If a
 * button is reported pressed, report it.  If the button is immediately reported
 * released, wait a period before reporting it; if the button is repressed during
 * that time, the release is never reported.
 *
 * The detail is important: if a button is pressed for 1ms and released, we must
 * always report it, even if the debounce period is 10ms, since it might be a coin
 * counter with a very short signal.  The only time we discard events is if a button
 * is pressed, released and then pressed again quickly.
 *
 * This delay in events is ordinarily not noticable, because the we report initial
 * presses and releases immediately.  However, if a real press is ever delayed,
 * this won't cause timing problems, because the event timestamp is preserved.
 */
static Preference<float> g_fInputDebounceTime( "InputDebounceTime", 0 );

InputFilter*	INPUTFILTER = NULL;	// global and accessable from anywhere in our program

static const float TIME_BEFORE_SLOW_REPEATS = 0.375f;
static const float TIME_BEFORE_FAST_REPEATS = 1.5f;

static const float REPEATS_PER_SEC = 8;

static float g_fTimeBeforeSlow, g_fTimeBeforeFast, g_fTimeBetweenRepeats;


InputFilter::InputFilter()
{
	queuemutex = new RageMutex("InputFilter");

	Reset();
	ResetRepeatRate();
}

InputFilter::~InputFilter()
{
	delete queuemutex;
	g_ButtonStates.clear();
}

void InputFilter::Reset()
{
	for( int i=0; i<NUM_INPUT_DEVICES; i++ )
		ResetDevice( InputDevice(i) );
}

void InputFilter::SetRepeatRate( float fSlowDelay, float fFastDelay, float fRepeatRate )
{
	g_fTimeBeforeSlow = fSlowDelay;
	g_fTimeBeforeFast = fFastDelay;
	g_fTimeBetweenRepeats = 1/fRepeatRate;
}

void InputFilter::ResetRepeatRate()
{
	SetRepeatRate( TIME_BEFORE_SLOW_REPEATS, TIME_BEFORE_FAST_REPEATS, REPEATS_PER_SEC );
}

ButtonState::ButtonState():
	m_BeingHeldTime(RageZeroTimer),
	m_LastReportTime(RageZeroTimer)
{
	m_BeingHeld = false;
	m_bLastReportedHeld = false;
	m_fSecsHeld = m_Level = m_LastLevel = 0;
}

void InputFilter::ButtonPressed( const DeviceInput &di, bool Down )
{
	LockMut(*queuemutex);

	if( di.ts.IsZero() )
		LOG->Warn( "InputFilter::ButtonPressed: zero timestamp is invalid" );

	ASSERT_M( di.device < NUM_INPUT_DEVICES, ssprintf("%i", di.device) );
	ASSERT_M( di.button < NUM_DeviceButton, ssprintf("%i", di.button) );

	ButtonState &bs = GetButtonState( di.device, di.button );

	bs.m_Level = di.level;

	if( bs.m_BeingHeld != Down )
	{
		/* Flush any delayed input, like Update() (in case Update() isn't being called). */
		RageTimer now;
		CheckButtonChange( bs, di, now );

		bs.m_BeingHeld = Down;
		bs.m_BeingHeldTime = di.ts;

		/* Try to report presses immediately. */
		CheckButtonChange( bs, di, now );
	}
}

void InputFilter::SetButtonComment( const DeviceInput &di, const CString &sComment )
{
	LockMut(*queuemutex);
	ButtonState &bs = GetButtonState( di.device, di.button );
	bs.m_sComment = sComment;
}

/* Release all buttons on the given device. */
void InputFilter::ResetDevice( InputDevice device )
{
	LockMut(*queuemutex);
	RageTimer now;
	
	vector<DeviceInput> DeviceInputs;
	GetPressedButtons( DeviceInputs );
	
	FOREACH( DeviceInput, DeviceInputs, di )
	{
		if( di->device == device )
			ButtonPressed( DeviceInput(device, di->button, -1, now), false );
	}
}

/* Check for reportable presses. */
void InputFilter::CheckButtonChange( ButtonState &bs, DeviceInput di, const RageTimer &now )
{
	if( bs.m_BeingHeld == bs.m_bLastReportedHeld )
		return;

	/* If the last IET_FIRST_PRESS or IET_RELEASE event was sent too recently,
	 * wait a while before sending it. */
	if( now - bs.m_LastReportTime < g_fInputDebounceTime )
		return;

	bs.m_LastReportTime = now;
	bs.m_bLastReportedHeld = bs.m_BeingHeld;
	bs.m_fSecsHeld = 0;

	di.ts = bs.m_BeingHeldTime;
	queue.push_back( InputEvent(di,bs.m_bLastReportedHeld? IET_FIRST_PRESS:IET_RELEASE) );
}

void InputFilter::Update( float fDeltaTime )
{
	RageTimer now;

	INPUTMAN->Update();

	/* Make sure that nothing gets inserted while we do this, to prevent
	 * things like "key pressed, key release, key repeat". */
	LockMut(*queuemutex);

	DeviceInput di( (InputDevice)0,DeviceButton_Invalid,1.0f,now);

	vector<ButtonStateMap::iterator> ButtonsToErase;
	
	FOREACHM( DeviceInput, ButtonState, g_ButtonStates, b )
	{
		di.device = b->first.device;
		di.button = b->first.button;
		ButtonState &bs = b->second;
		di.level = bs.m_Level;

		/* Generate IET_FIRST_PRESS and IET_RELEASE events that were delayed. */
		CheckButtonChange( bs, di, now );

		/* Generate IET_LEVEL_CHANGED events. */
		if( bs.m_LastLevel != bs.m_Level && bs.m_Level != -1 )
		{
			queue.push_back( InputEvent(di,IET_LEVEL_CHANGED) );
			bs.m_LastLevel = bs.m_Level;
		}

		/* Generate IET_FAST_REPEAT and IET_SLOW_REPEAT events. */
		if( !bs.m_bLastReportedHeld )
		{
			// If the key isn't pressed, and hasn't been pressed for a while (so debouncing
			// isn't interested in it), purge the entry.
			if( now - bs.m_LastReportTime > g_fInputDebounceTime )
				ButtonsToErase.push_back( b );
			continue;
		}

		const float fOldHoldTime = bs.m_fSecsHeld;
		bs.m_fSecsHeld += fDeltaTime;
		const float fNewHoldTime = bs.m_fSecsHeld;

		float fTimeBeforeRepeats;
		InputEventType iet;
		if( fNewHoldTime > g_fTimeBeforeSlow )
		{
			if( fNewHoldTime > g_fTimeBeforeFast )
			{
				fTimeBeforeRepeats = g_fTimeBeforeFast;
				iet = IET_FAST_REPEAT;
			}
			else
			{
				fTimeBeforeRepeats = g_fTimeBeforeSlow;
				iet = IET_SLOW_REPEAT;
			}

			float fRepeatTime;
			if( fOldHoldTime < fTimeBeforeRepeats )
			{
				fRepeatTime = fTimeBeforeRepeats;
			}
			else
			{
				float fAdjustedOldHoldTime = fOldHoldTime - fTimeBeforeRepeats;
				float fAdjustedNewHoldTime = fNewHoldTime - fTimeBeforeRepeats;
				if( int(fAdjustedOldHoldTime/g_fTimeBetweenRepeats) == int(fAdjustedNewHoldTime/g_fTimeBetweenRepeats) )
					continue;
				fRepeatTime = ftruncf( fNewHoldTime, g_fTimeBetweenRepeats );
			}

			/* Set the timestamp to the exact time of the repeat.  This way,
			 * as long as tab/` aren't being used, the timestamp will always
			 * increase steadily during repeats. */
			di.ts = bs.m_BeingHeldTime + fRepeatTime;

			queue.push_back( InputEvent(di,iet) );
		}
	}

	FOREACH( ButtonStateMap::iterator, ButtonsToErase, it )
		g_ButtonStates.erase( *it );
}

bool InputFilter::IsBeingPressed( const DeviceInput &di )
{
	LockMut(*queuemutex);
	return GetButtonState(di.device, di.button).m_bLastReportedHeld;
}

float InputFilter::GetSecsHeld( const DeviceInput &di )
{
	LockMut(*queuemutex);
	return GetButtonState(di.device, di.button).m_fSecsHeld;
}

CString InputFilter::GetButtonComment( const DeviceInput &di ) const
{
	LockMut(*queuemutex);
	return GetButtonState(di.device, di.button).m_sComment;
}

void InputFilter::ResetKeyRepeat( const DeviceInput &di )
{
	LockMut(*queuemutex);
	GetButtonState(di.device, di.button).m_fSecsHeld = 0;
}

void InputFilter::GetInputEvents( InputEventArray &array )
{
	LockMut(*queuemutex);
	array = queue;
	queue.clear();
}

void InputFilter::GetPressedButtons( vector<DeviceInput> &array )
{
	LockMut(*queuemutex);
	FOREACHM( DeviceInput, ButtonState, g_ButtonStates, b )
	{
		if( b->second.m_BeingHeld )
		{
			DeviceInput di = b->first;
			di.level = b->second.m_Level;
			array.push_back( di );
		}
	}
}

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

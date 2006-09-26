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
	RString m_sComment;
	float m_fSecsHeld;

	// Timestamp of m_BeingHeld changing.
	RageTimer m_BeingHeldTime;

	// The time that we actually reported the last event (used for debouncing).
	RageTimer m_LastReportTime;

	// The timestamp of the last reported change.  Unlike m_BeingHeldTime, this
	// value is debounced along with the input state.  (This is the same as
	// m_fSecsHeld, except this isn't affected by Update scaling.)
	RageTimer m_LastInputTime;
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
	ButtonState &GetButtonState( const DeviceInput &di )
	{
		return g_ButtonStates[di];
	}

	DeviceInputList g_CurrentState;
	set<DeviceInput> g_DisableRepeat;
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

static const float TIME_BEFORE_REPEATS = 0.375f;

static const float REPEATS_PER_SEC = 8;

static float g_fTimeBeforeRepeats, g_fTimeBetweenRepeats;


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
	FOREACH_InputDevice( i )
		ResetDevice( InputDevice(i) );
}

void InputFilter::SetRepeatRate( float fDelay, float fRepeatRate )
{
	g_fTimeBeforeRepeats = fDelay;
	g_fTimeBetweenRepeats = 1/fRepeatRate;
}

void InputFilter::ResetRepeatRate()
{
	SetRepeatRate( TIME_BEFORE_REPEATS, REPEATS_PER_SEC );
}

ButtonState::ButtonState():
	m_BeingHeldTime(RageZeroTimer),
	m_LastReportTime(RageZeroTimer)
{
	m_BeingHeld = false;
	m_bLastReportedHeld = false;
	m_fSecsHeld = 0;
}

void InputFilter::ButtonPressed( const DeviceInput &di, bool Down )
{
	LockMut(*queuemutex);

	if( di.ts.IsZero() )
		LOG->Warn( "InputFilter::ButtonPressed: zero timestamp is invalid" );

	ASSERT_M( di.device < NUM_InputDevice, ssprintf("%i", di.device) );
	ASSERT_M( di.button < NUM_DeviceButton, ssprintf("%i", di.button) );

	ButtonState &bs = GetButtonState( di );

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

void InputFilter::SetButtonComment( const DeviceInput &di, const RString &sComment )
{
	LockMut(*queuemutex);
	ButtonState &bs = GetButtonState( di );
	bs.m_sComment = sComment;
}

/* Release all buttons on the given device. */
void InputFilter::ResetDevice( InputDevice device )
{
	LockMut(*queuemutex);
	RageTimer now;

	const ButtonStateMap ButtonStates( g_ButtonStates );
	FOREACHM_CONST( DeviceInput, ButtonState, ButtonStates, b )
	{
		const DeviceInput &di = b->first;
		if( di.device == device )
			ButtonPressed( DeviceInput(device, di.button, 0, now), false );
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
	bs.m_LastInputTime = bs.m_BeingHeldTime;

	di.ts = bs.m_BeingHeldTime;
	ReportButtonChange( di, bs.m_bLastReportedHeld? IET_FIRST_PRESS:IET_RELEASE );

	if( !bs.m_bLastReportedHeld )
		g_DisableRepeat.erase( di );
}

void InputFilter::ReportButtonChange( const DeviceInput &di, InputEventType t )
{
	queue.push_back( InputEvent() );
	InputEvent &ie = queue.back();
	ie.type = t;
	ie.di = di;

	/*
	 * Include a list of all buttons that were pressed at the time of this event.  We
	 * can create this efficiently using g_ButtonStates.  Use a vector and not a
	 * map, for efficiency; most code will not use this information.  Iterating over
	 * g_ButtonStates will be in DeviceInput order, so users can binary search this
	 * list (eg. std::lower_bound).
	 */
	MakeButtonStateList( ie.m_ButtonState );
	g_CurrentState = ie.m_ButtonState;
}

void InputFilter::MakeButtonStateList( vector<DeviceInput> &aInputOut ) const
{
	aInputOut.reserve( g_ButtonStates.size() );
	for( ButtonStateMap::const_iterator it = g_ButtonStates.begin(); it != g_ButtonStates.end(); ++it )
	{
		const DeviceInput &di = it->first;
		const ButtonState &bs = it->second;
		if( !bs.m_bLastReportedHeld )
			continue;
		aInputOut.push_back( di );
		aInputOut.back().ts = bs.m_LastInputTime;
	}
}

void InputFilter::Update( float fDeltaTime )
{
	RageTimer now;

	INPUTMAN->Update();

	/* Make sure that nothing gets inserted while we do this, to prevent
	 * things like "key pressed, key release, key repeat". */
	LockMut(*queuemutex);

	DeviceInput di( DEVICE_NONE, DeviceButton_Invalid, 1.0f, now );

	vector<ButtonStateMap::iterator> ButtonsToErase;
	
	FOREACHM( DeviceInput, ButtonState, g_ButtonStates, b )
	{
		di.device = b->first.device;
		di.button = b->first.button;
		ButtonState &bs = b->second;

		/* Generate IET_FIRST_PRESS and IET_RELEASE events that were delayed. */
		CheckButtonChange( bs, di, now );

		/* Generate IET_REPEAT events. */
		if( !bs.m_bLastReportedHeld )
		{
			// If the key isn't pressed, and hasn't been pressed for a while (so debouncing
			// isn't interested in it), purge the entry.
			if( now - bs.m_LastReportTime > g_fInputDebounceTime )
				ButtonsToErase.push_back( b );
			continue;
		}

		/* If repeats are disabled for this button, skip. */
		if( g_DisableRepeat.find(di) != g_DisableRepeat.end() )
			continue;

		const float fOldHoldTime = bs.m_fSecsHeld;
		bs.m_fSecsHeld += fDeltaTime;
		const float fNewHoldTime = bs.m_fSecsHeld;

		if( fNewHoldTime <= g_fTimeBeforeRepeats )
			continue;

		float fRepeatTime;
		if( fOldHoldTime < g_fTimeBeforeRepeats )
		{
			fRepeatTime = g_fTimeBeforeRepeats;
		}
		else
		{
			float fAdjustedOldHoldTime = fOldHoldTime - g_fTimeBeforeRepeats;
			float fAdjustedNewHoldTime = fNewHoldTime - g_fTimeBeforeRepeats;
			if( int(fAdjustedOldHoldTime/g_fTimeBetweenRepeats) == int(fAdjustedNewHoldTime/g_fTimeBetweenRepeats) )
				continue;
			fRepeatTime = ftruncf( fNewHoldTime, g_fTimeBetweenRepeats );
		}

		/* Set the timestamp to the exact time of the repeat.  This way,
		 * as long as tab/` aren't being used, the timestamp will always
		 * increase steadily during repeats. */
		di.ts = bs.m_LastInputTime + fRepeatTime;

		ReportButtonChange( di, IET_REPEAT );
	}

	FOREACH( ButtonStateMap::iterator, ButtonsToErase, it )
		g_ButtonStates.erase( *it );
}

template<typename T, typename IT>
const T *FindItemBinarySearch( IT begin, IT end, const T &i )
{
	IT it = lower_bound( begin, end, i );
	if( it == end || *it != i )
		return NULL;

	return &*it;
}

bool InputFilter::IsBeingPressed( const DeviceInput &di, const DeviceInputList *pButtonState ) const
{
	LockMut(*queuemutex);
	if( pButtonState == NULL )
		pButtonState = &g_CurrentState;
	const DeviceInput *pDI = FindItemBinarySearch( pButtonState->begin(), pButtonState->end(), di );
	return pDI != NULL;
}

float InputFilter::GetSecsHeld( const DeviceInput &di, const DeviceInputList *pButtonState ) const
{
	LockMut(*queuemutex);
	if( pButtonState == NULL )
		pButtonState = &g_CurrentState;
	const DeviceInput *pDI = FindItemBinarySearch( pButtonState->begin(), pButtonState->end(), di );
	if( pDI == NULL )
		return 0;
	return pDI->ts.Ago();
}

RString InputFilter::GetButtonComment( const DeviceInput &di ) const
{
	LockMut(*queuemutex);
	return GetButtonState( di ).m_sComment;
}

void InputFilter::ResetKeyRepeat( const DeviceInput &di )
{
	LockMut(*queuemutex);
	GetButtonState( di ).m_fSecsHeld = 0;
}

/* Stop repeating the specified key until released. */
void InputFilter::RepeatStopKey( const DeviceInput &di )
{
	LockMut(*queuemutex);

	/* If the button is up, do nothing. */
	ButtonState &bs = GetButtonState( di );
	if( !bs.m_bLastReportedHeld )
		return;

	g_DisableRepeat.insert( di );
}

void InputFilter::GetInputEvents( vector<InputEvent> &array )
{
	array.clear();
	LockMut(*queuemutex);
	array.swap( queue );
}

void InputFilter::GetPressedButtons( vector<DeviceInput> &array ) const
{
	LockMut(*queuemutex);
	array = g_CurrentState;
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

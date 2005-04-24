#include "global.h"
#include "InputFilter.h"
#include "RageLog.h"
#include "RageInput.h"
#include "RageThreads.h"


InputFilter*	INPUTFILTER = NULL;	// global and accessable from anywhere in our program

static const float TIME_BEFORE_SLOW_REPEATS = 0.25f;
static const float TIME_BEFORE_FAST_REPEATS = 1.5f;

static const float SLOW_REPEATS_PER_SEC = 8;
static const float FAST_REPEATS_PER_SEC = 8;

static float g_fTimeBeforeSlow, g_fTimeBeforeFast, g_fTimeBetweenSlow, g_fTimeBetweenFast;


InputFilter::InputFilter()
{
	queuemutex = new RageMutex("InputFilter");
	memset( m_ButtonState, 0, sizeof(m_ButtonState) );

	Reset();
	ResetRepeatRate();
}

InputFilter::~InputFilter()
{
	delete queuemutex;
}

void InputFilter::Reset()
{
	for( int i=0; i<NUM_INPUT_DEVICES; i++ )
		ResetDevice( InputDevice(i) );
}

void InputFilter::SetRepeatRate( float fSlowDelay, float fSlowRate, float fFastDelay, float fFastRate )
{
	g_fTimeBeforeSlow = fSlowDelay;
	g_fTimeBeforeFast = fFastDelay;
	g_fTimeBetweenSlow = 1/fSlowRate;
	g_fTimeBetweenFast = 1/fFastRate;
}

void InputFilter::ResetRepeatRate()
{
	SetRepeatRate( TIME_BEFORE_SLOW_REPEATS, SLOW_REPEATS_PER_SEC, TIME_BEFORE_FAST_REPEATS, FAST_REPEATS_PER_SEC );
}

void InputFilter::ButtonPressed( DeviceInput di, bool Down )
{
	LockMut(*queuemutex);

	ButtonState &bs = m_ButtonState[di.device][di.button];

	bs.m_Level = di.level;

	if( bs.m_BeingHeld == Down )
		return;

	bs.m_BeingHeld = Down;
	bs.m_fSecsHeld = 0;

	queue.push_back( InputEvent(di,Down? IET_FIRST_PRESS:IET_RELEASE) );
}

/* Release all buttons on the given device. */
void InputFilter::ResetDevice( InputDevice device )
{
	for( int button = 0; button < GetNumDeviceButtons(device); ++button )
		ButtonPressed( DeviceInput(device, button), false );
}

void InputFilter::Update(float fDeltaTime)
{
	RageTimer now;

	// Constructing the DeviceInput inside the nested loops caues terrible 
	// performance.  So, construct it once outside the loop, then change 
	// .device and .button inside the loop.  I have no idea what is causing 
	// the slowness.  DeviceInput is a very small and simple structure, but
	// it's constructor was being called NUM_INPUT_DEVICES*NUM_DEVICE_BUTTONS
	// (>2000) times per Update().
	/* This should be fixed: DeviceInput's ctor uses an init list, so RageTimer
	 * isn't initialized each time. */
//	DeviceInput di( (InputDevice)0,0,now);

	INPUTMAN->Update( fDeltaTime );

	/* Make sure that nothing gets inserted while we do this, to prevent
	 * things like "key pressed, key release, key repeat". */
	LockMut(*queuemutex);

	// Don't reconstruct "di" inside the loop.  This line alone is 
	// taking 4% of the CPU on a P3-666.
	DeviceInput di( (InputDevice)0,0,1.0f,now);

	FOREACH_InputDevice( d )
	{
		di.device = d;

		for( int b=0; b < GetNumDeviceButtons(d); b++ )	// foreach button
		{
			ButtonState &bs = m_ButtonState[d][b];
			di.button = b;
			di.level = bs.m_Level;

			/* Generate IET_LEVEL_CHANGED events. */
			if( bs.m_LastLevel != bs.m_Level )
			{
				queue.push_back( InputEvent(di,IET_LEVEL_CHANGED) );
				bs.m_LastLevel = bs.m_Level;
			}

			/* Generate IET_FAST_REPEAT and IET_SLOW_REPEAT events. */
			if( !bs.m_BeingHeld )
				continue;

			const float fOldHoldTime = bs.m_fSecsHeld;
			bs.m_fSecsHeld += fDeltaTime;
			const float fNewHoldTime = bs.m_fSecsHeld;

			float fTimeBetweenRepeats;
			InputEventType iet;
			if( fOldHoldTime > g_fTimeBeforeSlow )
			{
				if( fOldHoldTime > g_fTimeBeforeFast )
				{
					fTimeBetweenRepeats = g_fTimeBetweenFast;
					iet = IET_FAST_REPEAT;
				}
				else
				{
					fTimeBetweenRepeats = g_fTimeBetweenSlow;
					iet = IET_SLOW_REPEAT;
				}
				if( int(fOldHoldTime/fTimeBetweenRepeats) != int(fNewHoldTime/fTimeBetweenRepeats) )
				{
					queue.push_back( InputEvent(di,iet) );
				}
			}
		}
	}

}

bool InputFilter::IsBeingPressed( DeviceInput di )
{
	return m_ButtonState[di.device][di.button].m_BeingHeld;
}

float InputFilter::GetSecsHeld( DeviceInput di )
{
	return m_ButtonState[di.device][di.button].m_fSecsHeld;
}

void InputFilter::ResetKeyRepeat( DeviceInput di )
{
	m_ButtonState[di.device][di.button].m_fSecsHeld = 0;
}

void InputFilter::GetInputEvents( InputEventArray &array )
{
	LockMut(*queuemutex);
	array = queue;
	queue.clear();
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

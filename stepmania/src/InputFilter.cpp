#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: InputFilter

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "InputFilter.h"
#include "RageLog.h"
#include "RageInput.h"


InputFilter*	INPUTFILTER = NULL;	// global and accessable from anywhere in our program

InputFilter::InputFilter()
{
	Reset();
}

void InputFilter::Reset()
{
	for( int i=0; i<NUM_INPUT_DEVICES; i++ )
		ResetDevice( InputDevice(i) );
}

void InputFilter::ButtonPressed( DeviceInput di, bool Down )
{
	if(m_BeingHeld[di.device][di.button] == Down)
		return;

	m_BeingHeld[di.device][di.button] = Down;
	m_fSecsHeld[di.device][di.button] = 0;

	InputEventType iet = Down? IET_FIRST_PRESS:IET_RELEASE;

	LockMut(queuemutex);
	queue.push_back( InputEvent(di,iet) );
}

/* Release all buttons on the given device. */
void InputFilter::ResetDevice( InputDevice dev )
{
	for( int button = 0; button < NUM_DEVICE_BUTTONS; ++button )
		ButtonPressed( DeviceInput(dev, button), false );
}

void InputFilter::Update(float fDeltaTime)
{
	INPUTMAN->Update( fDeltaTime );

	/* Make sure that nothing gets inserted while we do this, to prevent
	 * things like "key pressed, key release, key repeat". */
	LockMut(queuemutex);

	for( int d=0; d<NUM_INPUT_DEVICES; d++ )	// foreach InputDevice
	{
		for( int b=0; b < NUM_DEVICE_BUTTONS; b++ )	// foreach button
		{
			if(!m_BeingHeld[d][b])
				continue;

			const float fOldHoldTime = m_fSecsHeld[d][b];
			m_fSecsHeld[d][b] += fDeltaTime;
			const float fNewHoldTime = m_fSecsHeld[d][b];

			float fTimeBetweenRepeats;
			InputEventType iet;
			if( fOldHoldTime > TIME_BEFORE_SLOW_REPEATS )
			{
				if( fOldHoldTime > TIME_BEFORE_FAST_REPEATS )
				{
					fTimeBetweenRepeats = TIME_BETWEEN_FAST_REPEATS;
					iet = IET_FAST_REPEAT;
				}
				else
				{
					fTimeBetweenRepeats = TIME_BETWEEN_SLOW_REPEATS;
					iet = IET_SLOW_REPEAT;
				}
				if( int(fOldHoldTime/fTimeBetweenRepeats) != int(fNewHoldTime/fTimeBetweenRepeats) )
				{
					RageTimer now;
					DeviceInput di( (InputDevice)d,b,now);
					queue.push_back( InputEvent(di,iet) );
				}
			}
		}
	}

}

bool InputFilter::IsBeingPressed( DeviceInput di )
{
	return m_BeingHeld[di.device][di.button];
}

float InputFilter::GetSecsHeld( DeviceInput di )
{
	return m_fSecsHeld[di.device][di.button];
}

void InputFilter::GetInputEvents( InputEventArray &array )
{
	LockMut(queuemutex);
	array = queue;
	queue.clear();
}

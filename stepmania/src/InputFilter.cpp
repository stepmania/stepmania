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
#include "RageThreads.h"


InputFilter*	INPUTFILTER = NULL;	// global and accessable from anywhere in our program



InputFilter::InputFilter()
{
	queuemutex = new RageMutex;
	memset( m_BeingHeld, 0, sizeof(m_BeingHeld) );
	memset( m_BeingForced, 0, sizeof(m_BeingForced) );
	memset( m_fSecsHeld, 0, sizeof(m_fSecsHeld) );
	for( int d=0; d<NUM_INPUT_DEVICES; d++ )	// foreach InputDevice
		for( int b=0; b < NUM_DEVICE_BUTTONS; b++ )	// foreach button
			m_fSecsToForce[d][b] = -1;	

	Reset();
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

void InputFilter::ButtonPressed( DeviceInput di, bool Down )
{
	LockMut(*queuemutex);

	if(m_BeingHeld[di.device][di.button] == Down)
		return;

	const bool WasBeingPressed = IsBeingPressed( di );

	m_BeingHeld[di.device][di.button] = Down;
	m_fSecsHeld[di.device][di.button] = 0;

	if( WasBeingPressed != IsBeingPressed(di) )
	{
		InputEventType iet = IsBeingPressed(di)? IET_FIRST_PRESS:IET_RELEASE;
		queue.push_back( InputEvent(di,iet) );
	}
}

/* Force a key down.  Duration is the amount of time to force the key, or 0 to force
 * it until we explicitly call StopForcingKey. */
void InputFilter::ForceKey( DeviceInput di, float duration )
{
	LockMut(*queuemutex);

	if( m_BeingForced[di.device][di.button] )
		return;

	const bool WasBeingPressed = IsBeingPressed( di );

	m_BeingForced[di.device][di.button] = true;
	m_fSecsToForce[di.device][di.button] = duration;
	m_fSecsHeld[di.device][di.button] = 0;

	/* Send an IET_FIRST_PRESS event if the key wasn't already down. */
	if( WasBeingPressed != IsBeingPressed( di ) )
	{
		InputEventType iet = IsBeingPressed(di)? IET_FIRST_PRESS:IET_RELEASE;
		queue.push_back( InputEvent(di,iet) );
	}
}

void InputFilter::StopForcingKey( DeviceInput di )
{
	LockMut(*queuemutex);

	if( !m_BeingForced[di.device][di.button] )
		return;

	const bool WasBeingPressed = IsBeingPressed( di );

	m_BeingForced[di.device][di.button] = false;
	m_fSecsToForce[di.device][di.button] = 0;
	m_fSecsHeld[di.device][di.button] = 0;
	
	/* Send an IET_RELEASE event if the key is no longer down. */
	if( WasBeingPressed != IsBeingPressed(di) )
	{
		InputEventType iet = IsBeingPressed(di)? IET_FIRST_PRESS:IET_RELEASE;
		queue.push_back( InputEvent(di,iet) );
	}
}

/* Release all buttons on the given device. */
void InputFilter::ResetDevice( InputDevice dev )
{
	for( int button = 0; button < NUM_DEVICE_BUTTONS; ++button )
		ButtonPressed( DeviceInput(dev, button), false );
}

void InputFilter::Update(float fDeltaTime)
{
	RageTimer now;

	INPUTMAN->Update( fDeltaTime );

	/* Make sure that nothing gets inserted while we do this, to prevent
	 * things like "key pressed, key release, key repeat". */
	LockMut(*queuemutex);

	for( int d=0; d<NUM_INPUT_DEVICES; d++ )	// foreach InputDevice
	{
		for( int b=0; b < NUM_DEVICE_BUTTONS; b++ )	// foreach button
		{
			DeviceInput di( (InputDevice)d,b,now);

			if( m_fSecsToForce[d][b] > 0 )
			{
				m_fSecsToForce[d][b] -= fDeltaTime;
				if( m_fSecsToForce[d][b] <= 0 )
					StopForcingKey( di );
			}

			if( !IsBeingPressed(di) )
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
					queue.push_back( InputEvent(di,iet) );
				}
			}
		}
	}

}

bool InputFilter::IsBeingPressed( DeviceInput di )
{
	return m_BeingHeld[di.device][di.button] || m_BeingForced[di.device][di.button];
}

float InputFilter::GetSecsHeld( DeviceInput di )
{
	return m_fSecsHeld[di.device][di.button];
}

void InputFilter::GetInputEvents( InputEventArray &array )
{
	LockMut(*queuemutex);
	array = queue;
	queue.clear();
}

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
#if 0
	if( 1 ) // PREFSMAN->m_bJoytechInput && di.IsJoystick() )
	{
		/* If this is a release of the right button, force it down for a little while, so
		 * the axis motion has a chance to come in before we register the right arrow release. */
		const int RightButton = 'd';//JOY_13;
//		const int LeftButton = 'a';//JOY_15;
		if( !Down && di.button == RightButton )
		{
			LOG->Trace("temp force d");
			ForceKey( DeviceInput(di.device, RightButton), 1 );
		}
	}
#endif
	if( WasBeingPressed != IsBeingPressed(di) )
	{
		InputEventType iet = IsBeingPressed(di)? IET_FIRST_PRESS:IET_RELEASE;
		queue.push_back( InputEvent(di,iet) );
	}
#if 0
	if( 1 ) // PREFSMAN->m_bJoytechInput && di.IsJoystick() )
	{
		const int RightButton = 'd';//JOY_13;
		const int LeftButton = 'a';//JOY_15;
		bool NeedToForceBothButtons =
			IsBeingPressed( DeviceInput(di.device, 'e'/*JOY_RIGHT*/) ) &&
			!IsBeingPressed( DeviceInput(di.device, RightButton) );
			LOG->Trace("--- %i", NeedToForceBothButtons );
		if( NeedToForceBothButtons )
		{
			ForceKey( DeviceInput(di.device, RightButton), 0 );
			ForceKey( DeviceInput(di.device, LeftButton), 0 );
		} else {
			StopForcingKey( DeviceInput(di.device, RightButton) );
			StopForcingKey( DeviceInput(di.device, LeftButton) );
		}
	}
#endif
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
//	static float AutoStartTime=0;
//	AutoStartTime -= fDeltaTime;
//	if( AutoStartTime <= 0 )
//	{
//		AutoStartTime = 0.5f;
//		ButtonPressed( DeviceInput(DEVICE_KEYBOARD, 'a'), true );
//		ButtonPressed( DeviceInput(DEVICE_KEYBOARD, 'a'), false );
//	}
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

#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: InputFilter

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include <math.h>	// for fmod
#include "InputFilter.h"
#include "RageLog.h"
#include "RageInput.h"


InputFilter*	INPUTFILTER = NULL;	// global and accessable from anywhere in our program

InputFilter::InputFilter()
{
	for( int i=0; i<NUM_INPUT_DEVICES; i++ )
	{
		for( int j=0; j<NUM_DEVICE_BUTTONS; j++ )
			m_fSecsHeld[i][j] = 0;
	}
}

bool InputFilter::BeingPressed( DeviceInput di, bool Prev )
{
	if(di.device == DEVICE_JOY1 || di.device == DEVICE_JOY2 || di.device == DEVICE_JOY3 || di.device == DEVICE_JOY4)
	switch( di.button ) {
	case JOY_Z_UP: case JOY_Z_DOWN:
	case JOY_Z_ROT_UP: case JOY_Z_ROT_DOWN:
	case JOY_HAT_LEFT: case JOY_HAT_RIGHT: case JOY_HAT_UP: case JOY_HAT_DOWN:
		/* For now, ignore these. */
		return false;
	}

	return INPUTMAN->BeingPressed(di, Prev);
}

bool InputFilter::WasBeingPressed( DeviceInput di )
{
	return BeingPressed(di, true);
}

bool InputFilter::IsBeingPressed( DeviceInput di )
{
	return BeingPressed(di, false);
}

float InputFilter::GetSecsHeld( DeviceInput di )
{
	return m_fSecsHeld[di.device][di.button];
}

void InputFilter::GetInputEvents( InputEventArray &array, float fDeltaTime )
{
	INPUTMAN->Update();

	for( int d=0; d<NUM_INPUT_DEVICES; d++ )	// foreach InputDevice
	{
		int iNumButtonsToCheck = DeviceInput::NumButtons(InputDevice(d));

		for( int b=0; b<iNumButtonsToCheck; b++ )	// foreach button
		{
			const DeviceInput di = DeviceInput(InputDevice(d),b);

			if( WasBeingPressed(di) )
			{
				if( IsBeingPressed(di) )
				{
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
							array.Add( InputEvent(di,iet) );
					}
				}
				else {	// !IsBeingPressed(di)
					m_fSecsHeld[d][b] = 0;
					array.Add( InputEvent(di,IET_RELEASE) );
				}
			}
			else	// !WasBeingPressed(di)
			{
				if( IsBeingPressed(di) )
					array.Add( InputEvent(di,IET_FIRST_PRESS) );
				else	// !IsBeingPressed(di)
					;	// don't care
			}
		}
	}
}

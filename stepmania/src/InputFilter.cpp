#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: InputFilter

 Desc: See header.

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include <math.h>	// for fmod
#include "InputFilter.h"
#include "RageLog.h"
#include "RageInput.h"
#include "ErrorCatcher/ErrorCatcher.h"

InputFilter*	FILTER = NULL;	// global and accessable from anywhere in our program



void InputFilter::GetInputEvents( InputEventArray &array, float fDeltaTime )
{
	INPUTM->Update();

	for( int d=0; d<NUM_INPUT_DEVICES; d++ )	// foreach InputDevice
	{
		int iNumButtonsToCheck;
		switch( d )
		{
		case DEVICE_KEYBOARD:	
			iNumButtonsToCheck = NUM_KEYBOARD_BUTTONS;	
			break;
		case DEVICE_JOY1:
		case DEVICE_JOY2:
		case DEVICE_JOY3:
		case DEVICE_JOY4:
			iNumButtonsToCheck = NUM_JOYSTICK_BUTTONS;	
			break;
		default:
			ASSERT( false );
		}

		for( int b=0; b<iNumButtonsToCheck; b++ )	// foreach button
		{
			const DeviceInput di = DeviceInput(InputDevice(d),b);

			if( INPUTM->WasBeingPressed(di) )
			{
				if( INPUTM->IsBeingPressed(di) )
				{
					const float fOldHoldTime = m_fTimeHeld[d][b];
					m_fTimeHeld[d][b] += fDeltaTime;
					const float fNewHoldTime = m_fTimeHeld[d][b];
					
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
				else	// !INPUTM->IsBeingPressed(di)
					m_fTimeHeld[d][b] = 0;
			}
			else	// !INPUTM->WasBeingPressed(di)
			{
				if( INPUTM->IsBeingPressed(di) )
					array.Add( InputEvent(di,IET_FIRST_PRESS) );
				else	// !INPUTM->IsBeingPressed(di)
					;	// don't care
			}
		}
	}
}
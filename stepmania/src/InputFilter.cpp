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



void InputFilter::GetInputEvents( InputEventArray &array, float fDeltaTime )
{
	INPUTMAN->Update();

	for( int d=0; d<NUM_INPUT_DEVICES; d++ )	// foreach InputDevice
	{
		int iNumButtonsToCheck = DeviceInput::NumButtons(InputDevice(d));

		for( int b=0; b<iNumButtonsToCheck; b++ )	// foreach button
		{
			const DeviceInput di = DeviceInput(InputDevice(d),b);

			if( INPUTMAN->WasBeingPressed(di) )
			{
				if( INPUTMAN->IsBeingPressed(di) )
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
				else	// !INPUTMAN->IsBeingPressed(di)
					m_fTimeHeld[d][b] = 0;
			}
			else	// !INPUTMAN->WasBeingPressed(di)
			{
				if( INPUTMAN->IsBeingPressed(di) )
					array.Add( InputEvent(di,IET_FIRST_PRESS) );
				else	// !INPUTMAN->IsBeingPressed(di)
					;	// don't care
			}
		}
	}
}

#include "global.h"
#include "InputFilter.h"
#include "InputHandler.h"

void InputHandler::Update(float fDeltaTime)
{
	m_LastUpdate.Touch();
}

void InputHandler::ButtonPressed( DeviceInput di, bool Down )
{
	if( di.ts.IsZero() )
	{
		const RageTimer now;
		const float ProbableDelay = -(now - m_LastUpdate) / 2;
		di.ts = now + ProbableDelay;
	}

	INPUTFILTER->ButtonPressed( di, Down );
}

/*
 * Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
 *	Glenn Maynard
 */

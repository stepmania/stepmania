#include "global.h"
#include "InputFilter.h"
#include "InputHandler.h"

void InputHandler::UpdateTimer()
{
	m_LastUpdate.Touch();
}

void InputHandler::ButtonPressed( DeviceInput di, bool Down )
{
	if( di.ts.IsZero() )
		di.ts = m_LastUpdate.Half();

	INPUTFILTER->ButtonPressed( di, Down );
}

/*
 * Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
 *	Glenn Maynard
 */

#include "global.h"
/*
-----------------------------------------------------------------------------
 File: RageInput.h

 This file just starts up input drivers, which generate InputEvents.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "RageInput.h"
#include "RageLog.h"
#include "RageException.h"
#include "arch/InputHandler/InputHandler.h"

RageInput*		INPUTMAN	= NULL;		// globally accessable input device

RageInput::RageInput()
{
	LOG->Trace( "RageInput::RageInput()" );

	/* Init optional devices. */
	MakeInputHandlers(Devices);

	/* If no input devices are loaded, the user won't be able to input anything.
	 * That should never happen. */
	if(Devices.size() == 0)
		RageException::Throw("No input devices were loaded.  This shouldn't happen; please file a bug.");
}

RageInput::~RageInput()
{
	/* Delete optional devices. */
	for(unsigned i = 0; i < Devices.size(); ++i)
		delete Devices[i];
}

void RageInput::Update( float fDeltaTime )
{
	/* Update optional devices. */
	for(unsigned i = 0; i < Devices.size(); ++i)
		Devices[i]->Update(fDeltaTime);
}

void RageInput::GetDevicesAndDescriptions(vector<InputDevice>& vDevicesOut, vector<CString>& vDescriptionsOut)
{
	for(unsigned i = 0; i < Devices.size(); ++i)
		Devices[i]->GetDevicesAndDescriptions( vDevicesOut, vDescriptionsOut );	
}

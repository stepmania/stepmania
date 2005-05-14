#include "global.h"
#include "RageInput.h"
#include "RageLog.h"
#include "RageException.h"
#include "arch/InputHandler/InputHandler.h"

#include "arch/arch.h"

RageInput*		INPUTMAN	= NULL;		// globally accessable input device

RageInput::RageInput( CString drivers )
{
	LOG->Trace( "RageInput::RageInput()" );

	/* Init optional devices. */
	MakeInputHandlers(drivers,Devices);

	/* If no input devices are loaded, the user won't be able to input anything. */
	if( Devices.size() == 0 )
		LOG->Warn( "No input devices were loaded." );
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

void RageInput::WindowReset()
{
	for(unsigned i = 0; i < Devices.size(); ++i)
		Devices[i]->WindowReset();
}

void RageInput::AddHandler( InputHandler *pHandler )
{
	ASSERT( pHandler != NULL );
	Devices.push_back( pHandler );
}

/*
 * Copyright (c) 2001-2004 Chris Danford, Glenn Maynard
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

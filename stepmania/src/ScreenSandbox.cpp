#include "global.h"
/*
-----------------------------------------------------------------------------
 File: ScreenSandbox.h

 Desc: Area for testing.  Throw whatever you're working on in here.  If you
 don't want stuff in here to be wiped out by the next guy who works on something,
 make a separate screen and add a hook into ScreenTest; this one's just a
 scratchpad.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
  (there is no actual code here)
-----------------------------------------------------------------------------
*/

#include "ScreenSandbox.h"
#include "GameConstantsAndTypes.h"
#include "ThemeManager.h"
#include "RageDisplay.h"
#include "RageLog.h"


ScreenSandbox::ScreenSandbox( CString sClassName ) : Screen( sClassName )
{
}

void ScreenSandbox::HandleScreenMessage( const ScreenMessage SM )
{
	Screen::HandleScreenMessage( SM );
}

void ScreenSandbox::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenSandbox::Update( float fDeltaTime )
{
	Screen::Update(fDeltaTime);
}

void ScreenSandbox::DrawPrimitives()
{
	Screen::DrawPrimitives();
}

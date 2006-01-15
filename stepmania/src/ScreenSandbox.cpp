/*
 * Area for testing.  Throw whatever you're working on in here.  If you
 * don't want stuff in here to be wiped out by the next guy who works on something,
 * make a separate screen and add a hook into ScreenTest; this one's just a
 * scratchpad.
 */

#include "global.h"
#include "ScreenSandbox.h"
#include "GameConstantsAndTypes.h"
#include "ThemeManager.h"
#include "RageDisplay.h"
#include "RageLog.h"


REGISTER_SCREEN_CLASS_NEW( ScreenSandbox );

void ScreenSandbox::HandleScreenMessage( const ScreenMessage SM )
{
	Screen::HandleScreenMessage( SM );
}

void ScreenSandbox::Input( const InputEventPlus &input )
{
	Screen::Input( input );
}

void ScreenSandbox::Update( float fDeltaTime )
{
	Screen::Update(fDeltaTime);
}

void ScreenSandbox::DrawPrimitives()
{
	Screen::DrawPrimitives();
}

/*
 * (c) 2004 Chris Danford
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

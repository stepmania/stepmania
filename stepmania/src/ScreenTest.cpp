#include "global.h"
#include "ScreenTest.h"
#include "ScreenManager.h"
#include "GameSoundManager.h"
#include "ThemeManager.h"
#include "CommonMetrics.h"
#include "InputEventPlus.h"


ScreenTest::~ScreenTest()
{
	delete current;
}

void ScreenTest::SetScreen(int num)
{
	cur_screen = num;
	delete current;
	if(num > 2) num = 0;
	if(num == 0)
		current=SCREENMAN->MakeNewScreen("ScreenSandbox");
	else if(num == 1)
		current=SCREENMAN->MakeNewScreen("ScreenTestSound");
	else if(num == 2)
		current=SCREENMAN->MakeNewScreen("ScreenTestFonts");
}

REGISTER_SCREEN_CLASS_NEW( ScreenTest );
ScreenTest::ScreenTest()
{
	current = NULL;
}

void ScreenTest::Init()
{
	Screen::Init();

	cur_screen = -1;

	SOUND->StopMusic();

	SetScreen(0);
}

void ScreenTest::Update(float f) { current->Update(f); Screen::Update(f); }
void ScreenTest::HandleScreenMessage( const ScreenMessage SM ) { current->HandleScreenMessage(SM); }
void ScreenTest::Draw() { current->Draw(); }

void ScreenTest::Input( const InputEventPlus &input )
{
	if( input.DeviceI.device == DEVICE_KEYBOARD )
	{
		if( input.DeviceI.button >= KEY_F9 && input.DeviceI.button <= KEY_F12 )
		{
			if( input.type != IET_FIRST_PRESS ) return;
			SetScreen( input.DeviceI.button - KEY_F9 );
			return;
		}
		if( input.DeviceI.button == KEY_ESC )
		{
			if( input.type != IET_FIRST_PRESS ) return;
			SCREENMAN->SetNewScreen( CommonMetrics::INITIAL_SCREEN );
			return;
		}
	}
	
	current->Input( input );
}

/*
 * (c) 2003 Chris Danford, Glenn Maynard, Lance Gilbert
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

#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: ScreenTest.h

 Desc: Area for testing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Glenn Maynard (OpenGL Code)
	Lance Gilbert (OpenGL/Usability Modifications)
-----------------------------------------------------------------------------
*/

#include "stdafx.h"
#include "ScreenTest.h"
#include "ScreenSandbox.h"
#include "ScreenTestSound.h"
#include "ScreenTestFonts.h"
#include "ScreenManager.h"


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
		current=new ScreenSandbox;
	else if(num == 1)
		current=new ScreenTestSound;
	else if(num == 2)
		current=new ScreenTestFonts;
}

ScreenTest::ScreenTest()
{
	current = NULL;
	cur_screen = -1;

	SOUNDMAN->music->StopPlaying();

	SetScreen(0);
}

void ScreenTest::Update(float f) { current->Update(f); }
void ScreenTest::HandleScreenMessage( const ScreenMessage SM ) { current->HandleScreenMessage(SM); }
void ScreenTest::Draw() { current->Draw(); }

void ScreenTest::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if(DeviceI.device == DEVICE_KEYBOARD)
	{
		if(DeviceI.button >= SDLK_F9 && DeviceI.button <= SDLK_F12)
		{
			if( type != IET_FIRST_PRESS ) return;
			SetScreen(DeviceI.button - SDLK_F9);
			return;
		}
		if(DeviceI.button == SDLK_ESCAPE)
		{
			if( type != IET_FIRST_PRESS ) return;
			SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
			return;
		}
	}
	
	current->Input( DeviceI, type, GameI, MenuI, StyleI );
}

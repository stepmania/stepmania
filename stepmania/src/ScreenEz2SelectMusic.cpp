#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: ScreenSandbox.h

 Desc: Area for testing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Glenn Maynard (OpenGL Code)
	Lance Gilbert (OpenGL/Usability Modifications)
-----------------------------------------------------------------------------
*/

#include "stdafx.h"

#include "ScreenEz2SelectMusic.h"

#include "RageDisplay.h"
#include <math.h>

#include "SDL.h"
#include "SDL_opengl.h"
#include "ScreenSandbox.h"
#include "ScreenManager.h"
#include "RageMusic.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "ThemeManager.h"


ScreenEz2SelectMusic::ScreenEz2SelectMusic()
{	
	MUSIC->Stop();
	m_MusicBannerWheel.SetX(CENTER_X);
	m_MusicBannerWheel.SetY(CENTER_Y);
	this->AddChild( &m_MusicBannerWheel );
}

void ScreenEz2SelectMusic::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( type != IET_FIRST_PRESS )
		return;	// ignore

	switch( DeviceI.device)
	{
	case DEVICE_KEYBOARD:
		switch( DeviceI.button )
		{
		case SDLK_LEFT:
			m_MusicBannerWheel.BannersLeft();
			break;
		case SDLK_RIGHT:
			m_MusicBannerWheel.BannersRight();
			break;
		case SDLK_UP:

			break;
		case SDLK_DOWN:

			break;
		case SDLK_t: 
			{
				SDL_Event *event;
				event = (SDL_Event *) malloc(sizeof(event));
				event->type = SDL_QUIT;
				SDL_PushEvent(event);
			}
		case SDLK_ESCAPE: 
			{
				SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
			}
		}

	}

}

void ScreenEz2SelectMusic::Update( float fDeltaTime )
{
	Screen::Update( fDeltaTime );
}

void ScreenEz2SelectMusic::DrawPrimitives()
{
//	m_Menu.DrawBottomLayer();
	Screen::DrawPrimitives();
//	m_Menu.DrawTopLayer();
}


void ScreenEz2SelectMusic::HandleScreenMessage( const ScreenMessage SM )
{
	Screen::HandleScreenMessage( SM );
	switch( SM )
	{
	case SM_DoneClosingWipingLeft:
		break;
	case SM_DoneClosingWipingRight:
		break;
	case SM_DoneOpeningWipingLeft:
		break;
	case SM_DoneOpeningWipingRight:
		break;
	}
}

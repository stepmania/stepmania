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

#include "ScreenSandbox.h"

#include "RageDisplay.h"
#include <math.h>

#include "SDL.h"
#include "SDL_opengl.h"
#include "ScreenSandbox.h"
#include "ScreenManager.h"
#include "RageMusic.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "Quad.h"
#include "ThemeManager.h"
#include "RageNetwork.h"


ScreenSandbox::ScreenSandbox()
{	
	m_quad.StretchTo( RectI(SCREEN_LEFT, SCREEN_TOP, SCREEN_RIGHT, SCREEN_BOTTOM) );
	m_quad.SetDiffuse( RageColor(1,1,1,1) );
	this->AddChild( &m_quad );

	m_sprite.Load( "C:\\stepmania\\stepmania\\RandomMovies\\cm301[1].avi" );
	m_sprite.SetXY( CENTER_X, CENTER_Y );
	this->AddChild( &m_sprite );

	obj.SetXY(CENTER_X, CENTER_Y);
	this->AddChild(&obj);
}

void ScreenSandbox::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( type != IET_FIRST_PRESS )
		return;	// ignore

	switch( DeviceI.device)
	{
	case DEVICE_KEYBOARD:
		switch( DeviceI.button )
		{
		case SDLK_LEFT:
			obj.SetX(obj.GetX() - 10);
			break;
		case SDLK_RIGHT:
			obj.SetX(obj.GetX() + 10);
			break;
		case SDLK_UP:
			obj.SetY(obj.GetY() - 10);
			break;
		case SDLK_DOWN:
			obj.SetY(obj.GetY() + 10);
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
void ScreenSandbox::HandleScreenMessage( const ScreenMessage SM )
{
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

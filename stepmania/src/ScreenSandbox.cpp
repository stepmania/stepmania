#include "global.h"
/*
-----------------------------------------------------------------------------
 File: ScreenSandbox.h

 Desc: Area for testing.  Throw whatever you're working on in here.  If you
 don't want stuff in here to be wiped out by the next guy who works on something,
 make a separate screen and add a hook into ScreenTest; this one's just a
 scratchpad.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Glenn Maynard (OpenGL Code)
	Lance Gilbert (OpenGL/Usability Modifications)
-----------------------------------------------------------------------------
*/

#include "ScreenSandbox.h"
#include "GameConstantsAndTypes.h"


ScreenSandbox::ScreenSandbox()
{
	m_quad.StretchTo( RectI(SCREEN_LEFT, SCREEN_TOP, SCREEN_RIGHT, SCREEN_BOTTOM) );
	m_quad.SetDiffuse( RageColor(1,1,1,1) );
//	this->AddChild( &m_quad );

//	m_sprite.Load( "C:\\stepmania\\stepmania\\RandomMovies\\cm301[1].avi" );
//	m_sprite.SetXY( CENTER_X, CENTER_Y );
//	this->AddChild( &m_sprite );

	obj.SetXY(CENTER_X, CENTER_Y);
//	obj.SetZoom(50);
	this->AddChild(&obj);
}

void ScreenSandbox::HandleScreenMessage( const ScreenMessage SM )
{
}

void ScreenSandbox::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
}

void ScreenSandbox::Update( float fDeltaTime )
{
	Screen::Update(fDeltaTime);
}

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
#include "ThemeManager.h"
#include "RageDisplay.h"
#include "RageLog.h"


ScreenSandbox::ScreenSandbox() : Screen("ScreenSandbox")
{
//	m_quad1.StretchTo( RectI(SCREEN_LEFT, SCREEN_TOP, SCREEN_RIGHT, SCREEN_BOTTOM) );
//	m_quad1.SetDiffuse( RageColor(0,0,1,1) );
//	m_quad1.SetZ( 0 );
//	m_quad1.SetUseZBuffer( true );
//	this->AddChild( &m_quad1 );

//	m_quad2.StretchTo( RectI(SCREEN_LEFT, SCREEN_TOP, SCREEN_RIGHT, SCREEN_BOTTOM) );
//	m_quad2.SetDiffuse( RageColor(0,1,0,1) );
//	m_quad2.SetZ( -1 );
//	m_quad2.SetUseZBuffer( true );
//	this->AddChild( &m_quad2 );

//	m_sprite.Load( "C:\\stepmania\\stepmania\\RandomMovies\\cm301[1].avi" );
//	m_sprite.SetXY( CENTER_X, CENTER_Y );
//	this->AddChild( &m_sprite );

	//m_model.LoadMilkshapeAscii( "C:\\stepmania\\stepmania\\Themes\\groove\\BGAnimations\\ScreenCaution background\\2 platforms and backbar.txt" );

	m_model.LoadMilkshapeAscii( "Characters\\DancePads-DDR.txt" );
	m_model.SetXY(CENTER_X, CENTER_Y);
	m_model.SetRotationX( 15 );
	m_model.SetZoom(5);
	this->AddChild(&m_model);

//	this->AddChild( &m_In );

//	this->AddChild( &m_Out );
}

void ScreenSandbox::HandleScreenMessage( const ScreenMessage SM )
{
}

void ScreenSandbox::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( DeviceI.button == SDLK_LEFT )
		m_model.SetRotationY( m_model.GetRotationY()+5 );
	if( DeviceI.button == SDLK_DOWN )
		m_model.SetRotationX( m_model.GetRotationX()+5 );
	if( DeviceI.button == SDLK_UP )
		m_model.SetRotationX( m_model.GetRotationX()-5 );
	if( DeviceI.button == SDLK_RIGHT )
		m_model.SetRotationY( m_model.GetRotationY()-5 );

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenSandbox::Update( float fDeltaTime )
{
	Screen::Update(fDeltaTime);
}

void ScreenSandbox::DrawPrimitives()
{
	DISPLAY->SetLighting( true );
	DISPLAY->SetLightDirectional( 
		0, 
		RageColor(0.5,0.5,0.5,1), 
		RageColor(1,1,1,1),
		RageColor(0,0,0,1),
		RageVector3(0, 0, 1) );

	Screen::DrawPrimitives();

	DISPLAY->SetLightOff( 0 );
	DISPLAY->SetLighting( false );
}

void ScreenSandbox::MenuLeft( PlayerNumber pn )
{
//	m_In.Load( THEME->GetPathToB("_menu in") );
//	m_In.StartTransitioning();
}

void ScreenSandbox::MenuRight( PlayerNumber pn )
{
//	m_Out.Load( THEME->GetPathToB("_menu out") );
//	m_Out.StartTransitioning();
}

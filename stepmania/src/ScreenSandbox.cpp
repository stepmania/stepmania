#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: ScreenSandbox.h

 Desc: Area for testing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/


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
//	m_Menu.Load( 	
//		THEME->GetPathTo(GRAPHIC_SELECT_STYLE_BACKGROUND), 
//		THEME->GetPathTo(GRAPHIC_SELECT_STYLE_TOP_EDGE),
//		ssprintf("Use %c %c to select, then press START", char(1), char(2) ),
//		false, true, 40 
//		);
//	this->AddChild( &m_Menu );
//	m_Menu.TweenOnScreenFromBlack( SM_None );

	m_sprite.Load( THEME->GetPathTo("Graphics","title menu logo dance") );
	m_sprite.SetXY( CENTER_X, CENTER_Y );
	m_sprite.SetEffectGlowing();
	this->AddChild( &m_sprite );
}


void ScreenSandbox::Update( float fDeltaTime )
{
	Screen::Update( fDeltaTime );

	CArray<Packet,Packet> aPackets;
	NETWORK->Recv( aPackets );
}

void ScreenSandbox::DrawPrimitives()
{
	Screen::DrawPrimitives();
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
		case DIK_LEFT:
			break;
		case DIK_RIGHT:
			break;
		case DIK_T:
			break;
		}
	}

//	m_sprBG.SetEffectCamelion( 5, D3DXCOLOR(1,0.8f,0.8f,1), D3DXCOLOR(1,0.2f,0.2f,1) );
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
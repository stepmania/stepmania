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


ScreenSandbox::ScreenSandbox()
{	
	m_spr.Load( THEME->GetPathTo("Graphics","title menu logo game 0") );
	m_spr.SetXY( CENTER_X, CENTER_Y );
	this->AddSubActor( &m_spr );

//	m_Menu.Load( 	
//		THEME->GetPathTo(GRAPHIC_SELECT_STYLE_BACKGROUND), 
//		THEME->GetPathTo(GRAPHIC_SELECT_STYLE_TOP_EDGE),
//		ssprintf("Use %c %c to select, then press START", char(1), char(2) ),
//		false, true, 40 
//		);
//	this->AddSubActor( &m_Menu );
//	m_Menu.TweenOnScreenFromBlack( SM_None );

	//this->AddSubActor( &m_spr );
}


void ScreenSandbox::Update( float fDeltaTime )
{
	Screen::Update( fDeltaTime );
}

void ScreenSandbox::DrawPrimitives()
{
	Screen::DrawPrimitives();
}

void ScreenSandbox::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( MenuI.IsValid() )
	{
		switch( MenuI.button )
		{
		case MENU_BUTTON_LEFT:
			break;
		case MENU_BUTTON_RIGHT:
			break;
		case MENU_BUTTON_BACK:
			//SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
			return;
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
#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenCenterImage

 Desc: Where the player maps device input to pad input.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenCenterImage.h"
#include "PrefsManager.h"
#include "ScreenManager.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "InputMapper.h"
#include "GameManager.h"
#include "GameState.h"
#include "RageSounds.h"
#include "ThemeManager.h"
#include "RageDisplay.h"




ScreenCenterImage::ScreenCenterImage( CString sClassName ) : Screen( sClassName )
{
	LOG->Trace( "ScreenCenterImage::ScreenCenterImage()" );
	
	m_textInstructions.LoadFromFont( THEME->GetPathToF("Common normal") );
	m_textInstructions.SetText( "" );
	m_textInstructions.SetXY( CENTER_X, CENTER_Y );
	m_textInstructions.SetDiffuse( RageColor(0,1,0,0) );
	m_textInstructions.SetZoom( 0.8f );
	this->AddChild( &m_textInstructions );

	m_Menu.Load( "ScreenCenterImage" );
	this->AddChild( &m_Menu );

	SOUND->PlayMusic( THEME->GetPathToS("ScreenCenterImage music") );
}



ScreenCenterImage::~ScreenCenterImage()
{
	LOG->Trace( "ScreenCenterImage::~ScreenCenterImage()" );
}



void ScreenCenterImage::DrawPrimitives()
{
	m_Menu.DrawBottomLayer();
	Screen::DrawPrimitives();
	m_Menu.DrawTopLayer();
}

void ScreenCenterImage::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( m_Menu.IsTransitioning() )
		return;

	if( type != IET_RELEASE )
	{
		if( INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, SDLK_RSHIFT)) ||
			INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, SDLK_LSHIFT)) )
		{
			if( DeviceI.button == SDLK_LEFT )
				DISPLAY->CenteringScale( 0.99, 0, 0 );
			if( DeviceI.button == SDLK_DOWN )
				DISPLAY->CenteringScale( 0, 1/0.99, 0 );
			if( DeviceI.button == SDLK_UP )
				DISPLAY->CenteringScale( 0, 0.99, 0 );
			if( DeviceI.button == SDLK_RIGHT )
				DISPLAY->CenteringScale( 1/0.99, 0, 0 );
		}
		else
		{
			if( DeviceI.button == SDLK_LEFT )
				DISPLAY->CenteringTranslate( -1, 0, 0 );
			if( DeviceI.button == SDLK_DOWN )
				DISPLAY->CenteringTranslate( 0, +1, 0 );
			if( DeviceI.button == SDLK_UP )
				DISPLAY->CenteringTranslate( 0, -1, 0 );
			if( DeviceI.button == SDLK_RIGHT )
				DISPLAY->CenteringTranslate( +1, 0, 0 );
		}
	}
}

void ScreenCenterImage::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_GoToNextScreen:
		SCREENMAN->SetNewScreen( "ScreenOptionsMenu" );
		break;
	}
}

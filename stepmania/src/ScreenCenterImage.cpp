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

	if( type == IET_RELEASE )
		return;


	bool bHoldingShift = 
		INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, SDLK_RSHIFT)) ||
		INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, SDLK_LSHIFT));

	int iScale;
	switch( type )
	{
	case IET_SLOW_REPEAT:	iScale = 4;		break;
	case IET_FAST_REPEAT:	iScale = 16;	break;
	default:				iScale = 1;		break;
	}

	switch( DeviceI.button )
	{
	case SDLK_SPACE:
		PREFSMAN->m_iCenterImageTranslateX = 0;
		PREFSMAN->m_iCenterImageTranslateY = 0;
		PREFSMAN->m_fCenterImageScaleX = 1;
		PREFSMAN->m_fCenterImageScaleY = 1;
		break;
	case SDLK_LEFT:
		if( bHoldingShift )
			PREFSMAN->m_fCenterImageScaleX -= 0.001f * iScale;
		else
			PREFSMAN->m_iCenterImageTranslateX -= 1 * iScale;
		break;
	case SDLK_RIGHT:
		if( bHoldingShift )
			PREFSMAN->m_fCenterImageScaleX += 0.001f * iScale;
		else
			PREFSMAN->m_iCenterImageTranslateX += 1 * iScale;
		break;
	case SDLK_UP:
		if( bHoldingShift )
			PREFSMAN->m_fCenterImageScaleY -= 0.001f * iScale;
		else
			PREFSMAN->m_iCenterImageTranslateY -= 1 * iScale;
		break;
	case SDLK_DOWN:
		if( bHoldingShift )
			PREFSMAN->m_fCenterImageScaleY += 0.001f * iScale;
		else
			PREFSMAN->m_iCenterImageTranslateY += 1 * iScale;
		break;
	}

	switch( DeviceI.button )
	{
	case SDLK_SPACE:
	case SDLK_LEFT:
	case SDLK_RIGHT:
	case SDLK_UP:
	case SDLK_DOWN:
		DISPLAY->ChangeCentering(
			PREFSMAN->m_iCenterImageTranslateX, 
			PREFSMAN->m_iCenterImageTranslateY,
			PREFSMAN->m_fCenterImageScaleX,
			PREFSMAN->m_fCenterImageScaleY );
		break;
	}

	switch( DeviceI.button )
	{
	case SDLK_ESCAPE:
		if(!m_Menu.IsTransitioning())
		{
			SOUND->PlayOnce( THEME->GetPathToS("Common back") );
			m_Menu.StartTransitioning( SM_GoToPrevScreen );		
		}
	case SDLK_RETURN:
	case SDLK_KP_ENTER:
		if(!m_Menu.IsTransitioning())
		{
			SOUND->PlayOnce( THEME->GetPathToS("Common start") );
			m_Menu.StartTransitioning( SM_GoToNextScreen );		
		}
	}
}

void ScreenCenterImage::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_GoToPrevScreen:
	case SM_GoToNextScreen:
		SCREENMAN->SetNewScreen( "ScreenOptionsMenu" );
		break;
	}
}

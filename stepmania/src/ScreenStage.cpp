#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenStage

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenStage.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "GameConstantsAndTypes.h"
#include "BitmapText.h"
#include "TransitionFadeWipe.h"
#include "TransitionFade.h"
#include "ScreenSelectMusic.h"
#include "ScreenGameplay.h"
#include "SongManager.h"
#include "Sprite.h"
#include "AnnouncerManager.h"
#include "GameState.h"


const ScreenMessage SM_StartFadingOut	=	ScreenMessage(SM_User + 1);
const ScreenMessage SM_DoneFadingIn		=	ScreenMessage(SM_User + 2);
const ScreenMessage SM_GoToNextState	=	ScreenMessage(SM_User + 3);


ScreenStage::ScreenStage()
{
	m_pNextScreen = NULL;

	m_textStage.Load( THEME->GetPathTo(FONT_STAGE) );
	m_textStage.TurnShadowOff();

	if( GAMESTATE->m_PlayMode == PLAY_MODE_ONI )
	{
		m_textStage.SetText( "Challenge Mode" );
		m_textStage.SetDiffuseColor( D3DXCOLOR(0.8f,0.8f,1,1) );	// light blue
		SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_STAGE_CHALLENGE) );
	}
	else if( GAMESTATE->IsExtraStage() )
	{
		m_textStage.SetText( "Extra Stage" );
		m_textStage.SetDiffuseColor( D3DXCOLOR(1,0.1f,0.1f,1) );	// red
		SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_STAGE_EXTRA) );
	}
	else if( GAMESTATE->IsExtraStage2() )
	{
		m_textStage.SetText( "Another Extra Stage" );	// red
		m_textStage.SetDiffuseColor( D3DXCOLOR(1,0.1f,0.1f,1) );
		SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_STAGE_ANOTHER_EXTRA) );
	}
	else if( GAMESTATE->IsFinalStage() )
	{
		m_textStage.SetText( "Final Stage" );
		m_textStage.SetDiffuseColor( D3DXCOLOR(1,1,1,1) );
		SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_STAGE_FINAL) );
	}
	else
	{
		m_textStage.SetText( GAMESTATE->GetStageText() + " Stage" );
		m_textStage.SetDiffuseColor( D3DXCOLOR(1,1,1,1) );

		const int iStageNo = GAMESTATE->GetStageIndex()+1;
		switch( iStageNo )
		{
		case 1:	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_STAGE_1) );	break;
		case 2:	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_STAGE_2) );	break;
		case 3:	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_STAGE_3) );	break;
		case 4:	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_STAGE_4) );	break;
		case 5:	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_STAGE_5) );	break;
		default:	;	break;	// play nothing
		}
	}


	m_textStage.SetXY( CENTER_X, CENTER_Y );
	m_textStage.SetZoomX( 2 );
	m_textStage.SetZoomY( 0 );
	m_textStage.SetDiffuseColor( D3DXCOLOR(0,0,0,1) );
	m_textStage.BeginTweening( 0.5f );
	m_textStage.SetTweenZoom( 2 );
	m_textStage.SetTweenDiffuseColor( D3DXCOLOR(1,1,1,1) );
	this->AddSubActor( &m_textStage );

	this->SendScreenMessage( SM_DoneFadingIn, 0.6f );
	this->SendScreenMessage( SM_StartFadingOut, 1.2f );
}


void ScreenStage::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_StartFadingOut:
		m_textStage.BeginTweening( 0.8f );
		m_textStage.SetTweenDiffuseColor( D3DXCOLOR(1,1,1,0) );
		this->SendScreenMessage( SM_GoToNextState, 0.8f );
		break;
	case SM_DoneFadingIn:
		m_pNextScreen = new ScreenGameplay;
		break;
	case SM_GoToNextState:
		SCREENMAN->SetNewScreen( m_pNextScreen );
		break;
	}
}


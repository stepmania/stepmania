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
#include "ThemeManager.h"
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

const ScreenMessage SM_StartFadingOut	=	ScreenMessage(SM_User + 1);
const ScreenMessage SM_LoadNoteData		=	ScreenMessage(SM_User + 2);	// this is a super-hack.  We'll load the note data from cache while the message is displaying
const ScreenMessage SM_GoToNextState	=	ScreenMessage(SM_User + 3);

const float BASE_X		= CENTER_X;
const float BASE_Y		= CENTER_Y + 36;
const float BASE_HEIGHT = 6;

ScreenStage::ScreenStage( bool bTryExtraStage )
{
	m_bTryExtraStage = false;

	m_ptextStage = new BitmapText();
	m_ptextStage->Load( THEME->GetPathTo(FONT_STAGE) );
	m_ptextStage->TurnShadowOff();
	if( bTryExtraStage )
	{
		m_ptextStage->SetText( "Try Extra Stage!" );
		m_ptextStage->SetDiffuseColor( D3DXCOLOR(1,0.1f,0.1f,1) );
		SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_STAGE_EXTRA) );
	}
	else // !bTryExtraStage
	{
		int iStageNo = PREFS->GetStageNumber();
		bool bFinal = PREFS->IsFinalStage();

		CString sStagePrefix = PREFS->GetStageText();
		m_ptextStage->SetText( sStagePrefix + " Stage" );
		m_ptextStage->SetDiffuseColor( D3DXCOLOR(1,1,1,1) );

		if( bFinal )
			SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_STAGE_FINAL) );
		else
		{
			switch( iStageNo )
			{
			case 1:	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_STAGE_1) );	break;
			case 2:	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_STAGE_2) );	break;
			case 3:	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_STAGE_3) );	break;
			case 4:	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_STAGE_4) );	break;
			case 5:	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_STAGE_5) );	break;
			}
		}
	}
	m_ptextStage->SetXY( BASE_X, BASE_Y );
	m_ptextStage->SetVertAlign( Actor::align_top );
	m_ptextStage->SetZoomX( 2 );
	m_ptextStage->SetZoomY( 0 );
	m_ptextStage->BeginTweeningQueued( 0.6f );	// sleep
	m_ptextStage->BeginTweeningQueued( 0.5f );	// sleep
	m_ptextStage->SetTweenZoom( 2 );
	m_ptextStage->SetTweenXY( CENTER_X, CENTER_Y );
	this->AddActor( m_ptextStage );


	m_psprUnderscore = new Sprite;
	m_psprUnderscore->Load( THEME->GetPathTo(GRAPHIC_STAGE_UNDERSCORE) );
	m_psprUnderscore->SetXY( BASE_X, BASE_Y );
	m_psprUnderscore->SetZoomX( 0 );
	m_psprUnderscore->SetDiffuseColor( bTryExtraStage ? D3DXCOLOR(1,0.1f,0.1f,1) : D3DXCOLOR(1,1,1,1) );
	m_psprUnderscore->BeginTweeningQueued( 0.4f );	// open
	float fDestZoom = m_ptextStage->GetWidestLineWidthInSourcePixels() * m_ptextStage->GetZoomX() / m_psprUnderscore->GetUnzoomedWidth();
	m_psprUnderscore->SetTweenZoomX( fDestZoom );
	m_psprUnderscore->BeginTweeningQueued( 0.1f );	// sleep
	m_psprUnderscore->BeginTweeningQueued( 0.4f );	// close
	m_psprUnderscore->SetTweenZoomX( 0 );
	this->AddActor( m_psprUnderscore );


	m_pWipe = new TransitionFadeWipe;
	m_pWipe->OpenWipingRight();
	this->AddActor( m_ptextStage );

	this->SendScreenMessage( SM_LoadNoteData, 1 );
	this->SendScreenMessage( SM_StartFadingOut, 3 );
}


ScreenStage::~ScreenStage()
{
	delete m_ptextStage;
	delete m_pWipe;
}	

void ScreenStage::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_StartFadingOut:
		m_ptextStage->BeginTweening( 0.8f );
		m_ptextStage->SetTweenDiffuseColor( D3DXCOLOR(1,1,1,0) );
		this->SendScreenMessage( SM_GoToNextState, 0.8f );
		break;
	case SM_GoToNextState:
		SCREENMAN->SetNewScreen( new ScreenGameplay );
		break;
	case SM_LoadNoteData:
		SONGMAN->m_pCurSong->LoadFromCacheFile( true );	// load note data
		break;
	}
}


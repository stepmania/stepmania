#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenEz2Stage

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Andrew Livy
-----------------------------------------------------------------------------
*/

#include "ScreenEz2Stage.h"
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


ScreenEz2Stage::ScreenEz2Stage()
{
	
	MUSIC->Stop();

	m_pNextScreen = NULL;

	m_textStage.Load( THEME->GetPathTo(FONT_STAGE) );
	m_textStage.TurnShadowOff();
	m_textStageString.Load( THEME->GetPathTo(FONT_STAGE) );
	m_textStageString.TurnShadowOff();

	for (int i=0; i<20; i++)
	{
		m_blobs_a[i].Load( THEME->GetPathTo(FONT_STAGE) );
		m_blobs_a[i].TurnShadowOff();
		m_blobs_b[i].Load( THEME->GetPathTo(FONT_STAGE) );
		m_blobs_b[i].TurnShadowOff();
	}

	for (i=0; i<2; i++)
	{
		m_ez2ukm[i].Load( THEME->GetPathTo(FONT_STAGE) );
		m_ez2ukm[i].TurnShadowOff();		
	}

	if( GAMESTATE->m_PlayMode == PLAY_MODE_ONI )
	{
		m_textStage.SetText( "Nonstop Challenge" );
		m_textStage.SetDiffuseColor( D3DXCOLOR(0.8f,0.8f,1,1) );	// light blue
		SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_STAGE_ONI) );
	}
	else if( GAMESTATE->m_PlayMode == PLAY_MODE_ENDLESS )
	{
		m_textStage.SetText( "Endless Challenge" );
		m_textStage.SetDiffuseColor( D3DXCOLOR(0.8f,0.8f,1,1) );	// light blue
		SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_STAGE_ENDLESS) );
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

		m_textStageString.SetText( "NEXT NEXT NEXT NEXT NEXT NEXT NEXT NEXT NEXT NEXT" );
		m_textStageString.SetDiffuseColor( D3DXCOLOR(1,1,1,1) );

		for ( i=0; i<20; i++)
		{
			m_blobs_a[i].SetText( "$" );
			m_blobs_a[i].SetDiffuseColor( D3DXCOLOR(1,1,1,1) );
			m_blobs_b[i].SetText( "$" );
			m_blobs_b[i].SetDiffuseColor( D3DXCOLOR(1,1,1,1) );
		}

		for (i=0; i<2; i++)
		{
			m_ez2ukm[i].SetText( "EZ2DANCER UK MOVE" );
			m_ez2ukm[i].SetDiffuseColor( D3DXCOLOR(1,1,1,1) );
		}

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


	m_textStageString.SetXY(CENTER_X - 300, CENTER_Y - 150);
	m_textStageString.SetZoomX( 1.5f );
	m_textStageString.SetZoomY( 1.5f );
	m_textStageString.SetDiffuseColor( D3DXCOLOR(0,0,0,1) );

	for ( i=0; i<20; i++)
	{
		m_blobs_a[i].SetZoomX( 1.2f );
		m_blobs_a[i].SetZoomY( 1.2f );
		m_blobs_b[i].SetZoomX( 1.2f );
		m_blobs_b[i].SetZoomY( 1.2f );
		m_blobs_a[i].SetXY( CENTER_X-500-((i*i)*4), CENTER_Y + 140 );
		m_blobs_a[i].SetDiffuseColor( D3DXCOLOR(1.0f/255.0f*245.0f,1.0f/255.0f*237.0f,1.0f/255.0f*19.0f,0) );
		m_blobs_b[i].SetXY( CENTER_X+500+((i*i)*4), CENTER_Y + 170 );
		m_blobs_b[i].SetDiffuseColor( D3DXCOLOR(1.0f/255.0f*245.0f,1.0f/255.0f*237.0f,1.0f/255.0f*19.0f,0) );
	}

	for (i=0; i<2; i++)
	{
		m_ez2ukm[i].SetZoomX( 0.9f );
		m_ez2ukm[i].SetZoomY( 0.9f );
	}
	m_ez2ukm[0].SetXY( SCREEN_WIDTH + 100, 30 );
	m_ez2ukm[1].SetXY( 0 - 100, SCREEN_HEIGHT - 30 );

	m_textStage.SetXY( CENTER_X, CENTER_Y );
	m_textStage.SetZoomX( 4 );
	m_textStage.SetZoomY( 4 );
	m_textStage.SetDiffuseColor( D3DXCOLOR(0,0,0,1) );
	
	m_textStage.BeginTweening( 0.5f );
	m_textStageString.BeginTweening( 0.3f );
	for (i=0; i<2; i++)
	{
		m_ez2ukm[i].BeginTweening(0.5f);
	}
	for ( i=0; i<20; i++)
	{
		m_blobs_a[i].BeginTweening(0.2f * i / 7);
		m_blobs_b[i].BeginTweening(0.2f * i / 7);
		m_blobs_a[i].SetTweenX(40+(i*30.0f));
		m_blobs_b[i].SetTweenX(SCREEN_WIDTH-40-(i*30.0f));
		m_blobs_a[i].SetTweenDiffuseColor( D3DXCOLOR(1.0f/255.0f*245.0f,1.0f/255.0f*237.0f,1.0f/255.0f*19.0f,1) );
		m_blobs_b[i].SetTweenDiffuseColor( D3DXCOLOR(1.0f/255.0f*245.0f,1.0f/255.0f*237.0f,1.0f/255.0f*19.0f,1) );
	}

	m_ez2ukm[0].SetTweenX(0+150);
	m_ez2ukm[1].SetTweenX(SCREEN_WIDTH-150);

	m_textStage.SetTweenZoom( 4 );
	m_textStageString.SetTweenX(CENTER_X);
	m_textStage.SetTweenDiffuseColor( D3DXCOLOR(1,1,1,1) );
	m_textStageString.SetTweenDiffuseColor( D3DXCOLOR(1.0f/255.0f*245.0f,1.0f/255.0f*237.0f,1.0f/255.0f*19.0f,1) );


	this->AddSubActor( &m_textStage );
	this->AddSubActor( &m_textStageString );
	for ( i=0; i<20; i++)
	{
		this->AddSubActor( &m_blobs_a[i] );
		this->AddSubActor( &m_blobs_b[i] );
	}
	for (i=0; i<2; i++)
	{
		this->AddSubActor( &m_ez2ukm[i] );
	}

	this->SendScreenMessage( SM_DoneFadingIn, 0.6f );
	this->SendScreenMessage( SM_StartFadingOut, 1.2f );
}


void ScreenEz2Stage::HandleScreenMessage( const ScreenMessage SM )
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


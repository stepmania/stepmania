#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenGameplay

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenGameplay.h"
#include "SongManager.h"
#include "ScreenManager.h"

#include "ScreenSelectMusic.h"
#include "ScreenResults.h"
#include "GameConstantsAndTypes.h"
#include "ThemeManager.h"
#include "GameManager.h"
#include "SongManager.h"
#include "RageLog.h"
#include "AnnouncerManager.h"
#include "ScreenGameOver.h"

//
// Defines specific to GameScreenTitleMenu
//

const float LIFE_X[NUM_PLAYERS] = { CENTER_X-180, CENTER_X+180 };
const float LIFE_Y				= SCREEN_TOP+28;

const float SCORE_X[NUM_PLAYERS] = { CENTER_X-214, CENTER_X+214 };
const float SCORE_Y				 = SCREEN_BOTTOM-38;

const float PLAYER_OPTIONS_X[NUM_PLAYERS]	= { CENTER_X-214, CENTER_X+214 };
const float PLAYER_OPTIONS_Y[NUM_PLAYERS]	= { SCREEN_BOTTOM-60, SCREEN_BOTTOM-40 };

const float DIFFICULTY_X[NUM_PLAYERS]	= { SCREEN_LEFT+70, SCREEN_RIGHT-70 };
const float DIFFICULTY_Y				= SCREEN_BOTTOM-80;

const float MAX_SECONDS_CAN_BE_OFF_BY	=	0.20f;

const float TIME_BETWEEN_DANCING_COMMENTS	=	15;


// received while STATE_DANCING
const ScreenMessage	SM_SongEnded			= ScreenMessage(SM_User+102);
const ScreenMessage	SM_LifeIs0				= ScreenMessage(SM_User+103);


// received while STATE_OUTRO
const ScreenMessage	SM_ShowCleared			= ScreenMessage(SM_User+111);
const ScreenMessage	SM_HideCleared			= ScreenMessage(SM_User+112);
const ScreenMessage	SM_GoToResults			= ScreenMessage(SM_User+113);

const ScreenMessage	SM_BeginFailed			= ScreenMessage(SM_User+121);
const ScreenMessage	SM_ShowFailed			= ScreenMessage(SM_User+122);
const ScreenMessage	SM_PlayFailComment		= ScreenMessage(SM_User+123);
const ScreenMessage	SM_HideFailed			= ScreenMessage(SM_User+124);
const ScreenMessage	SM_GoToScreenAfterFail		= ScreenMessage(SM_User+125);



ScreenGameplay::ScreenGameplay()
{
	LOG->WriteLine( "ScreenGameplay::ScreenGameplay()" );

	m_DancingState = STATE_INTRO;
	m_fTimeLeftBeforeDancingComment = TIME_BETWEEN_DANCING_COMMENTS;

	m_bHasFailed = false;
	m_pSong = SONGMAN->m_pCurSong;


	m_Background.LoadFromSong( SONGMAN->m_pCurSong );
	m_Background.SetDiffuseColor( D3DXCOLOR(0.4f,0.4f,0.4f,1) );
	this->AddActor( &m_Background );


	// Get the current StyleDef definition (used below)
	StyleDef* pStyleDef = GAMEMAN->GetCurrentStyleDef();

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMEMAN->IsPlayerEnabled(PlayerNumber(p)) )
			continue;

		float fDifficultyY = DIFFICULTY_Y;
		if( PREFSMAN->m_PlayerOptions[p].m_bReverseScroll )
			fDifficultyY = SCREEN_HEIGHT - DIFFICULTY_Y;
		m_DifficultyBanner[p].SetXY( DIFFICULTY_X[p], fDifficultyY );
		m_DifficultyBanner[p].SetFromNotes( SONGMAN->m_pCurNotes[p] );
		this->AddActor( &m_DifficultyBanner[p] );


		NoteData* pOriginalNoteData = SONGMAN->m_pCurNotes[p]->GetNoteData();
		
		NoteData newNoteData;
		GAMEMAN->GetCurrentStyleDef()->GetTransformedNoteDataForStyle( (PlayerNumber)p, pOriginalNoteData, newNoteData );

		m_Player[p].SetX( (float) GAMEMAN->GetCurrentStyleDef()->m_iCenterX[p] );
		m_Player[p].Load( 
			(PlayerNumber)p,
			GAMEMAN->GetCurrentStyleDef(),
			&newNoteData, 
			PREFSMAN->m_PlayerOptions[p],
			&m_LifeMeter[p],
			&m_ScoreDisplay[p]
		);
		

		this->AddActor( &m_Player[p] );
	}


	for( p=0; p<NUM_PLAYERS; p++ )
	{
		m_quadLifeMeterBG[p].SetXY( LIFE_X[p], LIFE_Y );
		m_quadLifeMeterBG[p].SetDiffuseColor( D3DXCOLOR(0,0,0,1) );
		m_quadLifeMeterBG[p].SetZoomX( 256 );
		m_quadLifeMeterBG[p].SetZoomY( (p==PLAYER_1) ? 20.0f : -20.0f );
		this->AddActor( &m_quadLifeMeterBG[p] );

		if( !GAMEMAN->IsPlayerEnabled(PlayerNumber(p)) )
			continue;

		m_LifeMeter[p].SetPlayerOptions( PREFSMAN->m_PlayerOptions[p] );
		m_LifeMeter[p].SetXY( LIFE_X[p], LIFE_Y );
		m_LifeMeter[p].SetZoomX( 256 );
		m_LifeMeter[p].SetZoomY( (p==PLAYER_1) ? 20.0f : -20.0f );
		this->AddActor( &m_LifeMeter[p] );
	}

	m_sprTopFrame.Load( THEME->GetPathTo(GRAPHIC_GAMEPLAY_TOP_FRAME) );
	m_sprTopFrame.SetXY( CENTER_X, SCREEN_TOP + m_sprTopFrame.GetZoomedHeight()/2 );
	this->AddActor( &m_sprTopFrame );

	m_sprBottomFrame.Load( THEME->GetPathTo(GRAPHIC_GAMEPLAY_BOTTOM_FRAME) );
	m_sprBottomFrame.SetXY( CENTER_X, SCREEN_BOTTOM - m_sprBottomFrame.GetZoomedHeight()/2 );
	this->AddActor( &m_sprBottomFrame );

	m_textStageNumber.Load( THEME->GetPathTo(FONT_HEADER1) );
	m_textStageNumber.TurnShadowOff();
	m_textStageNumber.SetXY( CENTER_X, 60 );
	m_textStageNumber.SetDiffuseColor( D3DXCOLOR(0.3f,1,1,1) );
	m_textStageNumber.SetText( PREFSMAN->GetStageText() );
	this->AddActor( &m_textStageNumber );


	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMEMAN->IsPlayerEnabled(PlayerNumber(p)) )
			continue;

		m_ScoreDisplay[p].SetXY( SCORE_X[p], SCORE_Y );
		m_ScoreDisplay[p].SetZoom( 0.8f );
		this->AddActor( &m_ScoreDisplay[p] );

		m_textPlayerOptions[p].Load( THEME->GetPathTo(FONT_NORMAL) );
		m_textPlayerOptions[p].TurnShadowOff();
		m_textPlayerOptions[p].SetXY( PLAYER_OPTIONS_X[p], PLAYER_OPTIONS_Y[p] );
		m_textPlayerOptions[p].SetZoom( 0.5f );
		m_textPlayerOptions[p].SetDiffuseColor( D3DXCOLOR(1,1,1,1) );
		m_textPlayerOptions[p].SetText( PREFSMAN->m_PlayerOptions[p].GetString() );
		this->AddActor( &m_textPlayerOptions[p] );
	}

	



	m_StarWipe.SetClosed();
	this->AddActor( &m_StarWipe );

	m_sprReady.Load( THEME->GetPathTo(GRAPHIC_GAMEPLAY_READY) );
	m_sprReady.SetXY( CENTER_X, CENTER_Y );
	m_sprReady.SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
	this->AddActor( &m_sprReady );

	m_sprHereWeGo.Load( THEME->GetPathTo(GRAPHIC_GAMEPLAY_HERE_WE_GO) );
	m_sprHereWeGo.SetXY( CENTER_X, CENTER_Y );
	m_sprHereWeGo.SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
	this->AddActor( &m_sprHereWeGo );

	m_sprCleared.Load( THEME->GetPathTo(GRAPHIC_GAMEPLAY_CLEARED) );
	m_sprCleared.SetXY( CENTER_X, CENTER_Y );
	m_sprCleared.SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
	this->AddActor( &m_sprCleared );

	m_sprFailed.Load( THEME->GetPathTo(GRAPHIC_GAMEPLAY_FAILED) );
	m_sprFailed.SetXY( CENTER_X, CENTER_Y );
	m_sprFailed.SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
	this->AddActor( &m_sprFailed );


	m_soundMusic.Load( m_pSong->GetMusicPath() );

	m_soundFail.Load(			THEME->GetPathTo(SOUND_GAMEPLAY_FAILED) );
	m_announcerReady.Load(		ANNOUNCER->GetPathTo(ANNOUNCER_GAMEPLAY_READY) );
	m_announcerHereWeGo.Load(	ANNOUNCER->GetPathTo(ANNOUNCER_GAMEPLAY_HERE_WE_GO_NORMAL) );
	m_announcerGood.Load(		ANNOUNCER->GetPathTo(ANNOUNCER_GAMEPLAY_COMMENT_GOOD) );
	m_announcerBad.Load(		ANNOUNCER->GetPathTo(ANNOUNCER_GAMEPLAY_COMMENT_BAD) );
	m_announcerCleared.Load(	ANNOUNCER->GetPathTo(ANNOUNCER_GAMEPLAY_CLEARED) );
	m_announcerFailComment.Load(ANNOUNCER->GetPathTo(ANNOUNCER_GAMEPLAY_FAILED) );

	m_soundAssistTick.Load(		THEME->GetPathTo(SOUND_GAMEPLAY_ASSIST_TICK) );


	// Send some messages every have second to we can get the introduction rolling
	for( int i=0; i<30; i++ )
		this->SendScreenMessage( ScreenMessage(SM_User+i), i/2.0f );


	m_StarWipe.SetClosed();
}

ScreenGameplay::~ScreenGameplay()
{
	LOG->WriteLine( "ScreenGameplay::~ScreenGameplay()" );
	m_soundMusic.Stop();
}


void ScreenGameplay::Update( float fDeltaTime )
{
	//LOG->WriteLine( "ScreenGameplay::Update(%f)", fDeltaTime );
	Screen::Update( fDeltaTime );


	float fSongBeat, fBPS;
	float fPositionSeconds = m_soundMusic.GetPositionSeconds();
	
	m_pSong->GetBeatAndBPSFromElapsedTime( fPositionSeconds, fSongBeat, fBPS );

	

	
	//LOG->WriteLine( "m_fOffsetInBeats = %f, m_fBeatsPerSecond = %f, m_Music.GetPositionSeconds = %f", m_fOffsetInBeats, m_fBeatsPerSecond, m_Music.GetPositionSeconds() );

	float fMaxBeatDifference = fBPS*MAX_SECONDS_CAN_BE_OFF_BY;

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMEMAN->IsPlayerEnabled(PlayerNumber(p)) )
			continue;
		m_Player[p].Update( fDeltaTime, fSongBeat, fMaxBeatDifference );
	}

	// check for fail
	switch( PREFSMAN->m_SongOptions.m_FailType )
	{
	case SongOptions::FAIL_ARCADE:
	case SongOptions::FAIL_END_OF_SONG:

		if( m_bHasFailed )
			break;		// if they have already failed, don't bother checking again

		{
		// check for both players fail
		bool bAllAboutToFail = true;
		bool bAllFailed = true;
		for( int p=0; p<NUM_PLAYERS; p++ )
		{
			if( !GAMEMAN->IsPlayerEnabled(PlayerNumber(p)) )
				continue;

			if( !m_LifeMeter[p].IsAboutToFail() )
			{
				bAllAboutToFail = false;
				bAllFailed = false;
			}
			else if( !m_LifeMeter[p].HasFailed() )
			{
				bAllFailed = false;
			}
			break;
		}
		if( bAllAboutToFail )	m_Background.TurnDangerOn();
		else					m_Background.TurnDangerOff();

		if( bAllFailed )
		{
			m_bHasFailed = true;
			SCREENMAN->SendMessageToTopScreen( SM_LifeIs0, 0 );
		}
		}
		break;
	case SongOptions::FAIL_OFF:
		break;
	}


	switch( m_DancingState )
	{
	case STATE_DANCING:
		
		// Check for end of song
		if( m_DancingState == STATE_DANCING  &&
			m_soundMusic.GetLengthSeconds() - m_soundMusic.GetPositionSeconds() < 2 )
			this->SendScreenMessage( SM_SongEnded, 1 );
	
		// Check to see if it's time to play a gameplay comment
		if( PREFSMAN->m_bAnnouncer )
		{
			m_fTimeLeftBeforeDancingComment -= fDeltaTime;
			if( m_fTimeLeftBeforeDancingComment <= 0 )
			{
				m_fTimeLeftBeforeDancingComment = TIME_BETWEEN_DANCING_COMMENTS;	// reset for the next comment

				if( m_Background.IsDangerOn() )
					m_announcerBad.PlayRandom();
				else
					m_announcerGood.PlayRandom();
			}
		}
	}




	if( PREFSMAN->m_SongOptions.m_AssistType == SongOptions::ASSIST_TICK )
	{
		// 
		// play assist ticks
		//
		// Sound cards have a latency between when a sample is Play()ed and when the sound
		// will start coming out the speaker.  Compensate for this by boosting
		// fPositionSeconds ahead
		fPositionSeconds += (SOUND->GetPlayLatency()+0.06f) * m_soundMusic.GetPlaybackRate();	// HACK:  Add 0.06 seconds to make them play a tiny bit earlier
		m_pSong->GetBeatAndBPSFromElapsedTime( fPositionSeconds, fSongBeat, fBPS );

		int iIndexNow = BeatToNoteIndexNotRounded( fSongBeat );
		static int iIndexLastCrossed = 0;

		bool bAnyoneHasANote = false;	// set this to true if any player has a note at one of the indicies we crossed

		for( int i=iIndexLastCrossed+1; i<=iIndexNow; i++ )  // for each index we crossed since the last update
		{
			for( int p=0; p<NUM_PLAYERS; p++ )
			{
				if( !GAMEMAN->IsPlayerEnabled( (PlayerNumber)p ) )
					continue;		// skip

				bAnyoneHasANote |= m_Player[p].IsThereANoteAtIndex( i );
				break;	// this will only play the tick for the first player that is joined
			}
		}

		if( bAnyoneHasANote )
			m_soundAssistTick.PlayRandom();


		iIndexLastCrossed = iIndexNow;
	}

}


void ScreenGameplay::DrawPrimitives()
{
	Screen::DrawPrimitives();
}


void ScreenGameplay::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	//LOG->WriteLine( "ScreenGameplay::Input()" );

	float fSongBeat, fBPS;
	m_pSong->GetBeatAndBPSFromElapsedTime( m_soundMusic.GetPositionSeconds(), fSongBeat, fBPS );

	
	if( MenuI.IsValid() )
	{
		switch( MenuI.button )
		{
		case MENU_BUTTON_BACK:
			if( m_DancingState == STATE_DANCING 
			&& !m_bHasFailed )
			{
				m_bHasFailed = true;
				SCREENMAN->SendMessageToTopScreen( SM_BeginFailed, 0 );
			}
			else
				; // do not let user go back!
			break;
		}
	}

	float fMaxBeatsCanBeOffBy = MAX_SECONDS_CAN_BE_OFF_BY * fBPS;

	if( type == IET_FIRST_PRESS )
	{
		if( StyleI.IsValid() )
		{
			if( GAMEMAN->IsPlayerEnabled( StyleI.player ) )
				m_Player[StyleI.player].HandlePlayerStep( fSongBeat, StyleI.col, fMaxBeatsCanBeOffBy ); 
		}
	}
}

void ScreenGameplay::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
		// received while STATE_INTRO
	case SM_User+0:
		m_StarWipe.OpenWipingRight(SM_None);
		break;
	case SM_User+1:
		break;
	case SM_User+2:
		m_sprReady.StartFocusing();
		if( PREFSMAN->m_bAnnouncer )
			m_announcerReady.PlayRandom();
		break;
	case SM_User+3:
		break;
	case SM_User+4:
		m_sprReady.StartBlurring();
		break;
	case SM_User+5:
		m_sprHereWeGo.StartFocusing();
		if( PREFSMAN->m_bAnnouncer )
			m_announcerHereWeGo.PlayRandom();
		m_Background.SetDiffuseColor( D3DXCOLOR(0.8f,0.8f,0.8f,1) );
		m_soundMusic.Play();
		m_soundMusic.SetPlaybackRate( PREFSMAN->m_SongOptions.m_fMusicRate );
		break;
	case SM_User+6:
		break;
	case SM_User+7:
		break;
	case SM_User+8:
		m_sprHereWeGo.StartBlurring();
		m_DancingState = STATE_DANCING;		// STATE CHANGE!  Now the user is allowed to press Back
		break;
	case SM_User+9:
		break;

	// received while STATE_DANCING
	case SM_LifeIs0:
		if( PREFSMAN->m_SongOptions.m_FailType == SongOptions::FAIL_ARCADE )	// fail them now!
			this->SendScreenMessage( SM_BeginFailed, 0 );
		m_DancingState = STATE_OUTRO;
		break;
	case SM_SongEnded:
		if( m_DancingState == STATE_OUTRO )	// gameplay already ended
			return;		// ignore

		if( m_bHasFailed )	// fail them
		{
			this->SendScreenMessage( SM_BeginFailed, 0 );
		}
		else	// cleared
		{
			m_StarWipe.CloseWipingRight( SM_ShowCleared );
			if( PREFSMAN->m_bAnnouncer )
				m_announcerCleared.PlayRandom();	// crowd cheer
		}
		m_DancingState = STATE_OUTRO;
		break;

		// received while STATE_OUTRO
	case SM_ShowCleared:
		m_sprCleared.StartFocusing();
		SCREENMAN->SendMessageToTopScreen( SM_HideCleared, 2.5 );
		break;
	case SM_HideCleared:
		m_sprCleared.StartBlurring();
		SCREENMAN->SendMessageToTopScreen( SM_GoToResults, 1 );
		break;
	case SM_GoToResults:
		// send score summaries to the PREFSMAN object so ScreenResults can grab it.
		PREFSMAN->m_ScoreSummary[PLAYER_1][PREFSMAN->m_iNumArcadeStages-1] = m_Player[PLAYER_1].GetScoreSummary();
		PREFSMAN->m_ScoreSummary[PLAYER_2][PREFSMAN->m_iNumArcadeStages-1] = m_Player[PLAYER_2].GetScoreSummary();
		SCREENMAN->SetNewScreen( new ScreenResults(false) );
		break;


	case SM_BeginFailed:
		m_DancingState = STATE_OUTRO;
		m_soundMusic.Pause();
		m_StarWipe.CloseWipingRight( SM_None );
		this->SendScreenMessage( SM_ShowFailed, 0.2f );
		break;
	case SM_ShowFailed:
		if( PREFSMAN->m_bAnnouncer )
			m_soundFail.PlayRandom();

		// make the background invisible so we don't waste mem bandwidth drawing it
		m_Background.BeginTweening( 1 );
		m_Background.SetTweenDiffuseColor( D3DXCOLOR(1,1,1,0) );

		m_sprFailed.SetZoom( 4 );
		m_sprFailed.BeginBlurredTweening( 0.8f, TWEEN_BIAS_END );
		m_sprFailed.SetTweenZoom( 0.5f );			// zoom out
		m_sprFailed.SetTweenDiffuseColor( D3DXCOLOR(1,1,1,0.7f) );	// and fade in
		m_sprFailed.BeginTweeningQueued( 0.3f );
		m_sprFailed.SetTweenZoom( 1.1f );			// bounce
		m_sprFailed.SetTweenDiffuseColor( D3DXCOLOR(1,1,1,0.7f) );	// and fade in
		m_sprFailed.BeginTweeningQueued( 0.2f );
		m_sprFailed.SetTweenZoom( 1.0f );			// come to rest
		m_sprFailed.SetTweenDiffuseColor( D3DXCOLOR(1,1,1,0.7f) );	// and fade in

		SCREENMAN->SendMessageToTopScreen( SM_PlayFailComment, 1.5f );
		SCREENMAN->SendMessageToTopScreen( SM_HideFailed, 3.0f );
		break;
	case SM_PlayFailComment:
		if( PREFSMAN->m_bAnnouncer )
			m_announcerFailComment.PlayRandom();
		break;
	case SM_HideFailed:
		m_sprFailed.StopTweening();
		m_sprFailed.BeginTweening(1.0f);
		m_sprFailed.SetTweenDiffuseColor( D3DXCOLOR(1,1,1,0) );

		SCREENMAN->SendMessageToTopScreen( SM_GoToScreenAfterFail, 1.5f );
		break;
	case SM_GoToScreenAfterFail:
		if( PREFSMAN->m_bEventMode )
			SCREENMAN->SetNewScreen( new ScreenSelectMusic );
		else
			SCREENMAN->SetNewScreen( new ScreenGameOver );
		break;
	}


}

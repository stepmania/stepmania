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
#include "ScreenEvaluation.h"
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

const float LIFE_LOCAL_X[NUM_PLAYERS] = { -180, +180 };
const float LIFE_LOCAL_Y[NUM_PLAYERS] = { -10, -10 };

const float STAGE_NUMBER_LOCAL_X = 0;
const float STAGE_NUMBER_LOCAL_Y = +20;

const float SCORE_LOCAL_X[NUM_PLAYERS] = { -214, +214 };
const float SCORE_LOCAL_Y[NUM_PLAYERS] = { -6, -6 };

const float PLAYER_OPTIONS_LOCAL_X[NUM_PLAYERS]	= { -0, +0 };
const float PLAYER_OPTIONS_LOCAL_Y[NUM_PLAYERS]	= { -10, +10 };

const float DIFFICULTY_X[NUM_PLAYERS]	= { SCREEN_LEFT+60, SCREEN_RIGHT-60 };
const float DIFFICULTY_Y[NUM_PLAYERS]	= { SCREEN_BOTTOM-70, SCREEN_BOTTOM-70 };

const float DEBUG_X	= CENTER_X;
const float DEBUG_Y	= CENTER_Y-70;

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

	m_pCurSong = NULL;

	switch( PREFSMAN->m_PlayMode )
	{
	case PLAY_MODE_ARCADE:
		{
			m_apSongQueue.Add( SONGMAN->GetCurrentSong() );
			for( int p=0; p<NUM_PLAYERS; p++ )
				m_apNotesQueue[p].Add( SONGMAN->GetCurrentNotes(PlayerNumber(p)) );
		}
		break;
	case PLAY_MODE_ONI:
		{
			Course* pCourse = SONGMAN->m_pCurCourse;
			ASSERT( pCourse != NULL );
			for( int i=0; i<pCourse->m_iStages; i++ )
			{
				Song* pSong = pCourse->m_apSongs[i];
				DifficultyClass dc = pCourse->m_aDifficultyClasses[i];

				for( int j=0; j<pSong->m_arrayNotes.GetSize(); j++ )
				{
					Notes* pNotes = pSong->m_arrayNotes[j];
					if( pSong->m_arrayNotes[j]->m_DifficultyClass == dc )
					{
						m_apSongQueue.Add( pSong );
						for( int p=0; p<NUM_PLAYERS; p++ )
							m_apNotesQueue[p].Add( pNotes );
						break;
					}
				}
			}
		}
		break;
	}


	m_DancingState = STATE_INTRO;
	m_fTimeLeftBeforeDancingComment = TIME_BETWEEN_DANCING_COMMENTS;
	m_bBothHaveFailed = false;


	m_Background.SetDiffuseColor( D3DXCOLOR(0.4f,0.4f,0.4f,1) );
	this->AddActor( &m_Background );



	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMEMAN->IsPlayerEnabled(PlayerNumber(p)) )
			continue;

		m_Player[p].SetX( (float) GAMEMAN->GetCurrentStyleDef()->m_iCenterX[p] );
		this->AddActor( &m_Player[p] );
	}



	//////////////////////////////////
	// Add all Actors to m_frameTop
	//////////////////////////////////
	this->AddActor( &m_frameTop );

	// LifeMeter goes underneath top frame
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		m_quadLifeMeterBG[p].SetXY( LIFE_LOCAL_X[p], LIFE_LOCAL_Y[p] );
		m_quadLifeMeterBG[p].SetDiffuseColor( D3DXCOLOR(0,0,0,1) );
		m_quadLifeMeterBG[p].SetZoomX( 256 );
		m_quadLifeMeterBG[p].SetZoomY( (p==PLAYER_1) ? 20.0f : -20.0f );
		m_frameTop.AddActor( &m_quadLifeMeterBG[p] );

		if( !GAMEMAN->IsPlayerEnabled(PlayerNumber(p)) )
			continue;

		m_LifeMeter[p].SetPlayerOptions( PREFSMAN->m_PlayerOptions[p] );
		m_LifeMeter[p].SetXY( LIFE_LOCAL_X[p], LIFE_LOCAL_Y[p] );
		m_LifeMeter[p].SetZoomX( 256 );
		m_LifeMeter[p].SetZoomY( (p==PLAYER_1) ? 20.0f : -20.0f );
		m_frameTop.AddActor( &m_LifeMeter[p] );
	}

	// TopFrame goes above LifeMeter
	m_sprTopFrame.Load( THEME->GetPathTo(GRAPHIC_GAMEPLAY_TOP_FRAME_ARCADE) );
	m_frameTop.AddActor( &m_sprTopFrame );

	m_frameTop.SetXY( CENTER_X, SCREEN_TOP + m_sprTopFrame.GetZoomedHeight()/2 );

	m_textStageNumber.Load( THEME->GetPathTo(FONT_HEADER2) );
	m_textStageNumber.TurnShadowOff();
	m_textStageNumber.SetXY( STAGE_NUMBER_LOCAL_X, STAGE_NUMBER_LOCAL_Y );
	m_textStageNumber.SetText( PREFSMAN->GetStageText() );
	m_textStageNumber.SetDiffuseColor( PREFSMAN->GetStageColor() );
	m_frameTop.AddActor( &m_textStageNumber );



	//////////////////////////////////
	// Add all Actors to m_frameBottom
	//////////////////////////////////
	this->AddActor( &m_frameBottom );

	m_sprBottomFrame.Load( THEME->GetPathTo(GRAPHIC_GAMEPLAY_BOTTOM_FRAME) );
	m_frameBottom.AddActor( &m_sprBottomFrame );

	m_frameBottom.SetXY( CENTER_X, SCREEN_BOTTOM - m_sprBottomFrame.GetZoomedHeight()/2 );


	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMEMAN->IsPlayerEnabled(PlayerNumber(p)) )
			continue;

		m_ScoreDisplay[p].SetXY( SCORE_LOCAL_X[p], SCORE_LOCAL_Y[p] );
		m_ScoreDisplay[p].SetZoom( 0.8f );
		m_ScoreDisplay[p].SetDiffuseColor( PlayerToColor(p) );
		m_frameBottom.AddActor( &m_ScoreDisplay[p] );

		m_textPlayerOptions[p].Load( THEME->GetPathTo(FONT_NORMAL) );
		m_textPlayerOptions[p].TurnShadowOff();
		m_textPlayerOptions[p].SetXY( PLAYER_OPTIONS_LOCAL_X[p], PLAYER_OPTIONS_LOCAL_Y[p] );
		m_textPlayerOptions[p].SetZoom( 0.5f );
		m_textPlayerOptions[p].SetDiffuseColor( D3DXCOLOR(1,1,1,1) );
		m_textPlayerOptions[p].SetText( PREFSMAN->m_PlayerOptions[p].GetString() );
		m_frameBottom.AddActor( &m_textPlayerOptions[p] );
	}



	// Get the current StyleDef definition (used below)
	StyleDef* pStyleDef = GAMEMAN->GetCurrentStyleDef();

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMEMAN->IsPlayerEnabled(PlayerNumber(p)) )
			continue;

		float fDifficultyY = DIFFICULTY_Y[p];
		if( PREFSMAN->m_PlayerOptions[p].m_bReverseScroll )
			fDifficultyY = SCREEN_HEIGHT - DIFFICULTY_Y[p];
		m_DifficultyBanner[p].SetXY( DIFFICULTY_X[p], fDifficultyY );
		this->AddActor( &m_DifficultyBanner[p] );
	}


	m_textDebug.Load( THEME->GetPathTo(FONT_NORMAL) );
	m_textDebug.SetXY( DEBUG_X, DEBUG_Y );
	m_textDebug.SetDiffuseColor( D3DXCOLOR(1,1,1,1) );
	this->AddActor( &m_textDebug );

	
	


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



	m_soundFail.Load(			THEME->GetPathTo(SOUND_GAMEPLAY_FAILED) );
	m_announcerReady.Load(		ANNOUNCER->GetPathTo(ANNOUNCER_GAMEPLAY_READY) );
	m_announcerHereWeGo.Load(	ANNOUNCER->GetPathTo(ANNOUNCER_GAMEPLAY_HERE_WE_GO_NORMAL) );
	m_announcerDanger.Load(		ANNOUNCER->GetPathTo(ANNOUNCER_GAMEPLAY_COMMENT_DANGER) );
	m_announcerGood.Load(		ANNOUNCER->GetPathTo(ANNOUNCER_GAMEPLAY_COMMENT_GOOD) );
	m_announcerHot.Load(		ANNOUNCER->GetPathTo(ANNOUNCER_GAMEPLAY_COMMENT_HOT) );
	m_announcerCleared.Load(	ANNOUNCER->GetPathTo(ANNOUNCER_GAMEPLAY_CLEARED) );
	m_announcerFailComment.Load(ANNOUNCER->GetPathTo(ANNOUNCER_GAMEPLAY_FAILED) );

	m_soundAssistTick.Load(		THEME->GetPathTo(SOUND_GAMEPLAY_ASSIST_TICK) );


	LoadNextSong();

	// Send some messages every have second to we can get the introduction rolling
	for( int i=0; i<30; i++ )
		this->SendScreenMessage( ScreenMessage(SM_User+i), i/2.0f );
}

ScreenGameplay::~ScreenGameplay()
{
	LOG->WriteLine( "ScreenGameplay::~ScreenGameplay()" );
	m_soundMusic.Stop();
}


void ScreenGameplay::SaveSummary()
{
	// save score summaries
	if( m_pCurSong != NULL )
	{
		for( int p=0; p<NUM_PLAYERS; p++ )
		{
			if( !GAMEMAN->IsPlayerEnabled((PlayerNumber)p) )
				continue;		// skip

			SONGMAN->m_aGameplayStatistics[p].Add( m_Player[p].GetGameplayStatistics( m_pCurSong, m_pCurNotes[p]) );
		}

		m_pCurSong = NULL;
		for( int p=0; p<NUM_PLAYERS; p++ )
			m_pCurNotes[p] = NULL;
	}
}


void ScreenGameplay::LoadNextSong()
{
	int p;


	SaveSummary();

	m_pCurSong = m_apSongQueue[m_apSongQueue.GetSize()-1];
	m_apSongQueue.RemoveAt(m_apSongQueue.GetSize()-1);

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		m_pCurNotes[p] = m_apNotesQueue[p][m_apNotesQueue[p].GetSize()-1];
		m_apNotesQueue[p].RemoveAt(m_apNotesQueue[p].GetSize()-1);
	}


	// Get the current StyleDef definition (used below)
	StyleDef* pStyleDef = GAMEMAN->GetCurrentStyleDef();

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMEMAN->IsPlayerEnabled(PlayerNumber(p)) )
			continue;

		m_DifficultyBanner[p].SetFromNotes( PlayerNumber(p), m_pCurNotes[p] );


		NoteData originalNoteData;
		m_pCurNotes[p]->GetNoteData( &originalNoteData );
		
		NoteData newNoteData;
		pStyleDef->GetTransformedNoteDataForStyle( (PlayerNumber)p, &originalNoteData, &newNoteData );

		m_Player[p].Load( 
			(PlayerNumber)p,
			GAMEMAN->GetCurrentStyleDef(),
			&newNoteData, 
			PREFSMAN->m_PlayerOptions[p],
			&m_LifeMeter[p],
			&m_ScoreDisplay[p],
			originalNoteData.GetNumTapNotes(),
			m_pCurNotes[p]->m_iMeter
		);
	}

	m_soundMusic.Load( m_pCurSong->GetMusicPath() );
	m_soundMusic.SetPlaybackRate( PREFSMAN->m_SongOptions.m_fMusicRate );
	
	m_Background.LoadFromSong( m_pCurSong );
	m_Background.SetDiffuseColor( D3DXCOLOR(0.5f,0.5f,0.5f,1) );
	m_Background.BeginTweeningQueued( 2 );
	m_Background.SetTweenDiffuseColor( D3DXCOLOR(1,1,1,1) );
}


void ScreenGameplay::Update( float fDeltaTime )
{
	//LOG->WriteLine( "ScreenGameplay::Update(%f)", fDeltaTime );
	Screen::Update( fDeltaTime );


	if( m_pCurSong == NULL )
		return;


	float fPositionSeconds = m_soundMusic.GetPositionSeconds();
	float fSongBeat, fBPS;
	bool bFreeze;	
	m_pCurSong->GetBeatAndBPSFromElapsedTime( fPositionSeconds, fSongBeat, fBPS, bFreeze );

	
	m_Background.SetSongBeat( fSongBeat, bFreeze );

	
	//LOG->WriteLine( "m_fOffsetInBeats = %f, m_fBeatsPerSecond = %f, m_Music.GetPositionSeconds = %f", m_fOffsetInBeats, m_fBeatsPerSecond, m_Music.GetPositionSeconds() );

	const float fMaxBeatDifference = fBPS * PREFSMAN->m_fJudgeWindow * PREFSMAN->m_SongOptions.m_fMusicRate;

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

		if( m_bBothHaveFailed )
			break;		// if they have already failed, don't bother checking again

		{
		// check for both players fail
		bool bAllInDanger = true;
		bool bAllFailed = true;
		for( int p=0; p<NUM_PLAYERS; p++ )
		{
			if( !GAMEMAN->IsPlayerEnabled(PlayerNumber(p)) )
				continue;

			if( !m_LifeMeter[p].IsInDanger() )
				bAllInDanger = false;

			if( !m_LifeMeter[p].HasFailed() )
				bAllFailed = false;
		}

		if( bAllInDanger )	m_Background.TurnDangerOn();
		else				m_Background.TurnDangerOff();

		if( bAllFailed )
		{
			m_bBothHaveFailed = true;
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

				bool bAllInDanger = true;
				for( int p=0; p<NUM_PLAYERS; p++ )
					if( !m_LifeMeter[p].IsInDanger() )
						bAllInDanger = false;

				bool bOneIsHot = false;
				for( p=0; p<NUM_PLAYERS; p++ )
					if( m_LifeMeter[p].IsHot() )
						bOneIsHot = true;

				if( bOneIsHot )
					m_announcerHot.PlayRandom();
				else if( bAllInDanger )
					m_announcerDanger.PlayRandom();
				else
					m_announcerGood.PlayRandom();
			}
		}
	}



	//
	// Send crossed row messages to Player
	//
	int iRowNow = BeatToNoteRowNotRounded( fSongBeat );
	static int iRowLastCrossed = 0;

	for( int r=iRowLastCrossed+1; r<=iRowNow; r++ )  // for each index we crossed since the last update
	{
		for( int p=0; p<NUM_PLAYERS; p++ )
		{
			if( !GAMEMAN->IsPlayerEnabled( (PlayerNumber)p ) )
				continue;		// skip

			m_Player[p].CrossedRow( r, fSongBeat, fMaxBeatDifference );
		}
	}

	iRowLastCrossed = iRowNow;



	// 
	// play assist ticks
	//
	// Sound cards have a latency between when a sample is Play()ed and when the sound
	// will start coming out the speaker.  Compensate for this by boosting
	// fPositionSeconds ahead
	if( PREFSMAN->m_SongOptions.m_AssistType == SongOptions::ASSIST_TICK )
	{
		fPositionSeconds += (SOUND->GetPlayLatency()+0.04f) * m_soundMusic.GetPlaybackRate();	// HACK:  Add 0.04 seconds to make them play a tiny bit earlier
		m_pCurSong->GetBeatAndBPSFromElapsedTime( fPositionSeconds, fSongBeat, fBPS, bFreeze );

		int iRowNow = BeatToNoteRowNotRounded( fSongBeat );
		static int iRowLastCrossed = 0;

		bool bAnyoneHasANote = false;	// set this to true if any player has a note at one of the indicies we crossed

		for( int r=iRowLastCrossed+1; r<=iRowNow; r++ )  // for each index we crossed since the last update
		{
			for( int p=0; p<NUM_PLAYERS; p++ )
			{
				if( !GAMEMAN->IsPlayerEnabled( (PlayerNumber)p ) )
					continue;		// skip

				m_Player[p].CrossedRow( r, fSongBeat, fMaxBeatDifference );
				bAnyoneHasANote |= m_Player[p].IsThereANoteAtRow( r );
				break;	// this will only play the tick for the first player that is joined
			}
		}

		if( bAnyoneHasANote )
			m_soundAssistTick.PlayRandom();


		iRowLastCrossed = iRowNow;
	}

	//
	if( m_bBothHaveFailed )
	{
		for( int p=0; p<NUM_PLAYERS; p++ )
		{
			if( !GAMEMAN->IsPlayerEnabled((PlayerNumber)p) )
				continue;
			
			float fOverrideAdd = m_Player[p].GetOverrideAdd();
			if( fOverrideAdd == -1 )
			{
				m_Player[p].SetOverrideAdd( 0 );
			}
			else
			{
				float fNewAdd = min( 1, fOverrideAdd + fDeltaTime );
                m_Player[p].SetOverrideAdd( fNewAdd );
			}

		}
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
	bool bFreeze;
	m_pCurSong->GetBeatAndBPSFromElapsedTime( m_soundMusic.GetPositionSeconds(), fSongBeat, fBPS, bFreeze );


	// Handle special keys to adjust the offset
	if( DeviceI.device == DEVICE_KEYBOARD )
	{
		switch( DeviceI.button )
		{
		case DIK_F11:
		case DIK_F12:
			{
				float fOffsetDelta;
				switch( DeviceI.button )
				{
				case DIK_F11:	fOffsetDelta = -0.025f;		break;
				case DIK_F12:	fOffsetDelta = +0.025f;		break;
				default:	ASSERT(0);
				}
				if( type == IET_FAST_REPEAT )
					fOffsetDelta *= 40;

				m_pCurSong->m_fBeat0OffsetInSeconds += fOffsetDelta;

				m_textDebug.SetText( ssprintf("Offset = %f.", m_pCurSong->m_fBeat0OffsetInSeconds) );
				m_textDebug.SetDiffuseColor( D3DXCOLOR(1,1,1,1) );
				m_textDebug.StopTweening();
				m_textDebug.BeginTweeningQueued( 3 );		// sleep
				m_textDebug.BeginTweeningQueued( 0.5f );	// fade out
				m_textDebug.SetTweenDiffuseColor( D3DXCOLOR(1,1,1,0) );
			}
			break;
		}
	}


	
	if( MenuI.IsValid()  &&  MenuI.button == MENU_BUTTON_BACK  &&  type == IET_SLOW_REPEAT &&  m_DancingState == STATE_DANCING  &&  !m_bBothHaveFailed )
	{
		m_bBothHaveFailed = true;
		SCREENMAN->SendMessageToTopScreen( SM_BeginFailed, 0 );
	}

	const float fMaxBeatDifference = fBPS * PREFSMAN->m_fJudgeWindow * PREFSMAN->m_SongOptions.m_fMusicRate;

	if( type == IET_FIRST_PRESS )
	{
		if( StyleI.IsValid() )
		{
			if( GAMEMAN->IsPlayerEnabled( StyleI.player ) )
				m_Player[StyleI.player].HandlePlayerStep( fSongBeat, StyleI.col, fMaxBeatDifference ); 
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

		if( m_bBothHaveFailed )	// fail them
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
		SaveSummary();
		SCREENMAN->SetNewScreen( new ScreenEvaluation(false) );
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


void ScreenGameplay::TweenOnScreen()
{

}

void ScreenGameplay::TweenOffScreen()
{

}


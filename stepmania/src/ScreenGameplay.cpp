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
#include "PrefsManager.h"
#include "GameManager.h"
#include "SongManager.h"
#include "RageLog.h"
#include "AnnouncerManager.h"
#include "ScreenGameOver.h"
#include "LifeMeterBar.h"
#include "LifeMeterBattery.h"
#include "GameState.h"
#include "ScoreDisplayNormal.h"
#include "ScoreDisplayOni.h"

//
// Defines
//

const float LIFE_LOCAL_X[NUM_PLAYERS] = { -180, +180 };
const float LIFE_LOCAL_Y[NUM_PLAYERS] = { -8, -8 };

const float STAGE_NUMBER_LOCAL_X = 0;
const float STAGE_NUMBER_LOCAL_Y = +20;
const float STAGE_NUMBER_LOCALEZ2_Y = -30;

const float SCORE_LOCAL_X[NUM_PLAYERS] = { -214, +214 };
const float SCORE_LOCAL_Y[NUM_PLAYERS] = { -6, -6 };

const float SCORE_LOCALEZ2_X[NUM_PLAYERS] = { -240, +240 };
const float SCORE_LOCALEZ2_Y[NUM_PLAYERS] = { -32, -32 };

const float PLAYER_OPTIONS_LOCAL_X[NUM_PLAYERS]	= { -0, +0 };
const float PLAYER_OPTIONS_LOCAL_Y[NUM_PLAYERS]	= { -10, +10 };

const float DIFFICULTY_X[NUM_PLAYERS]	= { SCREEN_LEFT+60, SCREEN_RIGHT-60 };
const float DIFFICULTY_Y[NUM_PLAYERS]	= { SCREEN_BOTTOM-70, SCREEN_BOTTOM-70 };

const float DEBUG_X	= CENTER_X;
const float DEBUG_Y	= CENTER_Y-70;

const float TIME_BETWEEN_DANCING_COMMENTS	=	13;


// received while STATE_DANCING
const ScreenMessage	SM_NotesEnded			= ScreenMessage(SM_User+102);
const ScreenMessage	SM_LastNotesEnded		= ScreenMessage(SM_User+103);
const ScreenMessage	SM_LifeIs0				= ScreenMessage(SM_User+104);


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

	switch( GAMESTATE->m_PlayMode )
	{
	case PLAY_MODE_ARCADE:
		{
			m_apSongQueue.Add( GAMESTATE->m_pCurSong );
			for( int p=0; p<NUM_PLAYERS; p++ )
				m_apNotesQueue[p].Add( GAMESTATE->m_pCurNotes[p] );
		}
		break;
	case PLAY_MODE_ONI:
		{
			Course* pCourse = GAMESTATE->m_pCurCourse;
			ASSERT( pCourse != NULL );

			m_apSongQueue.RemoveAll();
			for( int p=0; p<NUM_PLAYERS; p++ )
				m_apNotesQueue[p].RemoveAll();

			pCourse->GetSongAndNotesForCurrentStyle( m_apSongQueue, m_apNotesQueue );

			// store possible dance points in GAMESTATE
			GAMESTATE->m_iCoursePossibleDancePoints = 0;
			for( int i=0; i<m_apNotesQueue[PLAYER_1].GetSize(); i++ )
			{
				NoteData nd;
				m_apNotesQueue[PLAYER_1][i]->GetNoteData( &nd );
				GAMESTATE->m_iCoursePossibleDancePoints += nd.GetPossibleDancePoints();
			}
		}
		break;
	}


	m_DancingState = STATE_INTRO;
	m_fTimeLeftBeforeDancingComment = TIME_BETWEEN_DANCING_COMMENTS;
	m_bBothHaveFailed = false;


	m_Background.SetDiffuseColor( D3DXCOLOR(0.4f,0.4f,0.4f,1) );
	this->AddSubActor( &m_Background );



	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled(PlayerNumber(p)) )
			continue;

		m_Player[p].SetX( (float) GAMESTATE->GetCurrentStyleDef()->m_iCenterX[p] );
		this->AddSubActor( &m_Player[p] );
	}


	//////////////////////////////////
	// Add all Actors to m_frameTop
	//////////////////////////////////
	this->AddSubActor( &m_frameTop );

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		switch( GAMESTATE->m_PlayMode )
		{
		case PLAY_MODE_ARCADE:
			m_pLifeMeter[p] = new LifeMeterBar;
			break;
		case PLAY_MODE_ONI:
			m_pLifeMeter[p] = new LifeMeterBattery;
			break;
		default:
			ASSERT(0);
		}

		m_pLifeMeter[p]->Load( (PlayerNumber)p, GAMESTATE->m_PlayerOptions[p] );
		m_pLifeMeter[p]->SetXY( LIFE_LOCAL_X[p], LIFE_LOCAL_Y[p] );
		m_frameTop.AddSubActor( m_pLifeMeter[p] );
		
		if( GAMESTATE->GetCurGame() == GAME_EZ2 )
		{	
			m_pScoreDisplay[p]->SetXY( SCORE_LOCALEZ2_X[p], SCORE_LOCALEZ2_Y[p] );
			m_pScoreDisplay[p]->SetZoom( 0.5f );
			m_pScoreDisplay[p]->SetDiffuseColor( PlayerToColor(p) );
			m_frameTop.AddSubActor( m_pScoreDisplay[p] );
		}
	}

	// TopFrame goes above LifeMeter
	m_sprTopFrame.Load( THEME->GetPathTo(GRAPHIC_GAMEPLAY_TOP_FRAME) );
	m_frameTop.AddSubActor( &m_sprTopFrame );

	m_frameTop.SetXY( CENTER_X, SCREEN_TOP + m_sprTopFrame.GetZoomedHeight()/2 );

	m_textStageNumber.Load( THEME->GetPathTo(FONT_HEADER2) );
	m_textStageNumber.TurnShadowOff();
	m_textStageNumber.SetXY( STAGE_NUMBER_LOCAL_X, STAGE_NUMBER_LOCAL_Y );
	m_textStageNumber.SetText( GAMESTATE->GetStageText() );
	m_textStageNumber.SetDiffuseColor( GAMESTATE->GetStageColor() );

	if( GAMESTATE->GetCurGame() == GAME_EZ2 )
	{
		m_textStageNumber.SetXY( STAGE_NUMBER_LOCAL_X, STAGE_NUMBER_LOCALEZ2_Y );
	}
	else
	{
		m_textStageNumber.SetXY( STAGE_NUMBER_LOCAL_X, STAGE_NUMBER_LOCAL_Y );
	}
	m_textStageNumber.SetText( GAMESTATE->GetStageText() );
	m_textStageNumber.SetDiffuseColor( GAMESTATE->GetStageColor() );

	m_frameTop.AddSubActor( &m_textStageNumber );


	//////////////////////////////////
	// Add all Actors to m_frameBottom
	//////////////////////////////////
	this->AddSubActor( &m_frameBottom );

	m_sprBottomFrame.Load( THEME->GetPathTo(GRAPHIC_GAMEPLAY_BOTTOM_FRAME) );
	m_frameBottom.AddSubActor( &m_sprBottomFrame );

	m_frameBottom.SetXY( CENTER_X, SCREEN_BOTTOM - m_sprBottomFrame.GetZoomedHeight()/2 );

	if( GAMESTATE->GetCurGame() != GAME_EZ2 )
	{
		m_sprBottomFrame.Load( THEME->GetPathTo(GRAPHIC_GAMEPLAY_BOTTOM_FRAME) );
		m_frameBottom.AddSubActor( &m_sprBottomFrame );
		m_frameBottom.SetXY( CENTER_X, SCREEN_BOTTOM - m_sprBottomFrame.GetZoomedHeight()/2 );
	}



	for( p=0; p<NUM_PLAYERS; p++ )
	{
		switch( GAMESTATE->m_PlayMode )
		{
		case PLAY_MODE_ARCADE:
			m_pScoreDisplay[p] = new ScoreDisplayNormal;
			break;
		case PLAY_MODE_ONI:
			m_pScoreDisplay[p] = new ScoreDisplayOni;
			break;
		default:
			ASSERT(0);
		}

		m_pScoreDisplay[p]->Init( (PlayerNumber)p, GAMESTATE->m_PlayerOptions[p], 100, 7 );
		m_pScoreDisplay[p]->SetXY( SCORE_LOCAL_X[p], SCORE_LOCAL_Y[p] );
		m_pScoreDisplay[p]->SetZoom( 0.8f );
		m_pScoreDisplay[p]->SetDiffuseColor( PlayerToColor(p) );
		m_frameBottom.AddSubActor( m_pScoreDisplay[p] );

		m_textPlayerOptions[p].Load( THEME->GetPathTo(FONT_NORMAL) );

		if( !GAMESTATE->IsPlayerEnabled(PlayerNumber(p)) )
			continue;
		if( GAMESTATE->GetCurGame() != GAME_EZ2 )
		{	
			m_pScoreDisplay[p]->SetXY( SCORE_LOCAL_X[p], SCORE_LOCAL_Y[p] );
			m_pScoreDisplay[p]->SetZoom( 0.8f );
			m_pScoreDisplay[p]->SetDiffuseColor( PlayerToColor(p) );
			m_frameBottom.AddSubActor( m_pScoreDisplay[p] );
		}
		
		m_textPlayerOptions[p].Load( THEME->GetPathTo(FONT_NORMAL) );

		m_textPlayerOptions[p].TurnShadowOff();
		m_textPlayerOptions[p].SetXY( PLAYER_OPTIONS_LOCAL_X[p], PLAYER_OPTIONS_LOCAL_Y[p] );
		m_textPlayerOptions[p].SetZoom( 0.5f );
		m_textPlayerOptions[p].SetDiffuseColor( D3DXCOLOR(1,1,1,1) );
		m_textPlayerOptions[p].SetText( GAMESTATE->m_PlayerOptions[p].GetString() );
		m_frameBottom.AddSubActor( &m_textPlayerOptions[p] );
	}



	// Get the current StyleDef definition (used below)
	StyleDef* pStyleDef = GAMESTATE->GetCurrentStyleDef();

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled(PlayerNumber(p)) )
			continue;

		float fDifficultyY = DIFFICULTY_Y[p];
		if( GAMESTATE->m_PlayerOptions[p].m_bReverseScroll )
			fDifficultyY = SCREEN_HEIGHT - DIFFICULTY_Y[p];
		m_DifficultyBanner[p].SetXY( DIFFICULTY_X[p], fDifficultyY );
		this->AddSubActor( &m_DifficultyBanner[p] );

		if( GAMESTATE->GetCurGame() != GAME_EZ2 )
		{
			float fDifficultyY = DIFFICULTY_Y[p];
			if( GAMESTATE->m_PlayerOptions[p].m_bReverseScroll )
				fDifficultyY = SCREEN_HEIGHT - DIFFICULTY_Y[p];
			m_DifficultyBanner[p].SetXY( DIFFICULTY_X[p], fDifficultyY );
			this->AddSubActor( &m_DifficultyBanner[p] );
		}
	}


	m_textDebug.Load( THEME->GetPathTo(FONT_NORMAL) );
	m_textDebug.SetXY( DEBUG_X, DEBUG_Y );
	m_textDebug.SetDiffuseColor( D3DXCOLOR(1,1,1,1) );
	this->AddSubActor( &m_textDebug );

	
	


	m_StarWipe.SetClosed();
	this->AddSubActor( &m_StarWipe );

	m_sprReady.Load( THEME->GetPathTo(GRAPHIC_GAMEPLAY_READY) );
	m_sprReady.SetXY( CENTER_X, CENTER_Y );
	m_sprReady.SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
	this->AddSubActor( &m_sprReady );

	m_sprHereWeGo.Load( THEME->GetPathTo(GRAPHIC_GAMEPLAY_HERE_WE_GO) );
	m_sprHereWeGo.SetXY( CENTER_X, CENTER_Y );
	m_sprHereWeGo.SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
	this->AddSubActor( &m_sprHereWeGo );

	m_sprCleared.Load( THEME->GetPathTo(GRAPHIC_GAMEPLAY_CLEARED) );
	m_sprCleared.SetXY( CENTER_X, CENTER_Y );
	m_sprCleared.SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
	this->AddSubActor( &m_sprCleared );

	m_sprFailed.Load( THEME->GetPathTo(GRAPHIC_GAMEPLAY_FAILED) );
	m_sprFailed.SetXY( CENTER_X, CENTER_Y );
	m_sprFailed.SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
	this->AddSubActor( &m_sprFailed );



	m_soundFail.Load(			THEME->GetPathTo(SOUND_GAMEPLAY_FAILED) );
	m_announcerReady.Load(		ANNOUNCER->GetPathTo(ANNOUNCER_GAMEPLAY_READY) );
	if( GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2() )
		m_announcerHereWeGo.Load(	ANNOUNCER->GetPathTo(ANNOUNCER_GAMEPLAY_HERE_WE_GO_EXTRA) );
	else if( GAMESTATE->IsFinalStage() )
		m_announcerHereWeGo.Load(	ANNOUNCER->GetPathTo(ANNOUNCER_GAMEPLAY_HERE_WE_GO_FINAL) );
	else
		m_announcerHereWeGo.Load(	ANNOUNCER->GetPathTo(ANNOUNCER_GAMEPLAY_HERE_WE_GO_NORMAL) );
	m_announcerDanger.Load(		ANNOUNCER->GetPathTo(ANNOUNCER_GAMEPLAY_COMMENT_DANGER) );
	m_announcerGood.Load(		ANNOUNCER->GetPathTo(ANNOUNCER_GAMEPLAY_COMMENT_GOOD) );
	m_announcerHot.Load(		ANNOUNCER->GetPathTo(ANNOUNCER_GAMEPLAY_COMMENT_HOT) );

	m_announcer100Combo.Load(	ANNOUNCER->GetPathTo(ANNOUNCER_GAMEPLAY_100_COMBO) );
	m_announcer200Combo.Load(	ANNOUNCER->GetPathTo(ANNOUNCER_GAMEPLAY_200_COMBO) );
	m_announcer300Combo.Load(	ANNOUNCER->GetPathTo(ANNOUNCER_GAMEPLAY_300_COMBO) );
	m_announcer400Combo.Load(	ANNOUNCER->GetPathTo(ANNOUNCER_GAMEPLAY_400_COMBO) );
	m_announcer500Combo.Load(	ANNOUNCER->GetPathTo(ANNOUNCER_GAMEPLAY_500_COMBO) );
	m_announcer600Combo.Load(	ANNOUNCER->GetPathTo(ANNOUNCER_GAMEPLAY_600_COMBO) );
	m_announcer700Combo.Load(	ANNOUNCER->GetPathTo(ANNOUNCER_GAMEPLAY_700_COMBO) );
	m_announcer800Combo.Load(	ANNOUNCER->GetPathTo(ANNOUNCER_GAMEPLAY_800_COMBO) );
	m_announcer900Combo.Load(	ANNOUNCER->GetPathTo(ANNOUNCER_GAMEPLAY_900_COMBO) );
	m_announcer1000Combo.Load(	ANNOUNCER->GetPathTo(ANNOUNCER_GAMEPLAY_1000_COMBO) );
	m_announcerComboStopped.Load(	ANNOUNCER->GetPathTo(ANNOUNCER_GAMEPLAY_COMBO_STOPPED) );

	m_announcerCleared.Load(	ANNOUNCER->GetPathTo(ANNOUNCER_GAMEPLAY_CLEARED) );
	m_announcerFailComment.Load(ANNOUNCER->GetPathTo(ANNOUNCER_GAMEPLAY_FAILED) );

	m_soundAssistTick.Load(		THEME->GetPathTo(SOUND_GAMEPLAY_ASSIST_TICK) );


	LoadNextSong( false );

	// Send some messages every have second to we can get the introduction rolling
	for( int i=0; i<30; i++ )
		this->SendScreenMessage( ScreenMessage(SM_User+i), i/2.0f );
}

ScreenGameplay::~ScreenGameplay()
{
	LOG->WriteLine( "ScreenGameplay::~ScreenGameplay()" );
	
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		delete m_pLifeMeter[p];
		delete m_pScoreDisplay[p];
	}

	m_soundMusic.Stop();
}


void ScreenGameplay::LoadNextSong( bool bPlayMusic )
{
	if( m_apSongQueue.GetSize() == 0 )
	{
		this->SendScreenMessage( SM_LastNotesEnded, 0 );
		return;
	}


	int p;

	// add a new GameplayStatistic for this song
	GAMESTATE->m_aGameplayStatistics.Add( GameplayStatistics() );

	GAMESTATE->m_pCurSong = m_apSongQueue[0];
	m_apSongQueue.RemoveAt(0);

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		GAMESTATE->m_pCurNotes[p] = m_apNotesQueue[p][0];
		m_apNotesQueue[p].RemoveAt(0);
	}


	// Get the current StyleDef definition (used below)
	StyleDef* pStyleDef = GAMESTATE->GetCurrentStyleDef();

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled(PlayerNumber(p)) )
			continue;

		m_DifficultyBanner[p].SetFromNotes( PlayerNumber(p), GAMESTATE->m_pCurNotes[p] );


		NoteData originalNoteData;
		GAMESTATE->m_pCurNotes[p]->GetNoteData( &originalNoteData );
		
		NoteData newNoteData;
		pStyleDef->GetTransformedNoteDataForStyle( (PlayerNumber)p, &originalNoteData, &newNoteData );


		// Fill in info about these notes in the latest GameplayStatistics
		GAMESTATE->GetLatestGameplayStatistics().dc[p] = GAMESTATE->m_pCurNotes[p]->m_DifficultyClass;
		GAMESTATE->GetLatestGameplayStatistics().meter[p] = GAMESTATE->m_pCurNotes[p]->m_iMeter;
		GAMESTATE->GetLatestGameplayStatistics().iPossibleDancePoints[p] = newNoteData.GetPossibleDancePoints();
		for( int r=0; r<NUM_RADAR_VALUES; r++ )
			GAMESTATE->GetLatestGameplayStatistics().fRadarPossible[p][r] = newNoteData.GetRadarValue( (RadarCategory)r, GAMESTATE->m_pCurSong->m_fMusicLengthSeconds );

		m_Player[p].Load( 
			(PlayerNumber)p,
			GAMESTATE->GetCurrentStyleDef(),
			&newNoteData, 
			GAMESTATE->m_PlayerOptions[p],
			m_pLifeMeter[p],
			m_pScoreDisplay[p],
			originalNoteData.GetNumTapNotes(),
			GAMESTATE->m_pCurNotes[p]->m_iMeter
		);
	}

	
	m_Background.LoadFromSong( GAMESTATE->m_pCurSong );
	m_Background.SetDiffuseColor( D3DXCOLOR(0.5f,0.5f,0.5f,1) );
	m_Background.BeginTweeningQueued( 2 );
	m_Background.SetTweenDiffuseColor( D3DXCOLOR(1,1,1,1) );

	m_soundMusic.Load( GAMESTATE->m_pCurSong->GetMusicPath(), true );	// enable accurate sync
	float fStartSeconds = min( 0, -4+GAMESTATE->m_pCurSong->GetElapsedTimeFromBeat(GAMESTATE->m_pCurSong->m_fFirstBeat) );
	m_soundMusic.SetPositionSeconds( fStartSeconds );
	m_soundMusic.SetPlaybackRate( GAMESTATE->m_SongOptions.m_fMusicRate );
	if( bPlayMusic )
		m_soundMusic.Play();

	if( bPlayMusic )
		for( int p=0; p<NUM_PLAYERS; p++ )
			if( GAMESTATE->IsPlayerEnabled(PlayerNumber(p)) )
				m_pLifeMeter[p]->NextSong( NULL );
}


void ScreenGameplay::Update( float fDeltaTime )
{
	//LOG->WriteLine( "ScreenGameplay::Update(%f)", fDeltaTime );
	Screen::Update( fDeltaTime );

	m_soundMusic.Update( fDeltaTime );

	if( GAMESTATE->m_pCurSong == NULL )
		return;


	float fPositionSeconds = m_soundMusic.GetPositionSeconds();
	float fSongBeat, fBPS;
	bool bFreeze;	
	GAMESTATE->m_pCurSong->GetBeatAndBPSFromElapsedTime( fPositionSeconds, fSongBeat, fBPS, bFreeze );


	// update the global music statistics for other classes to access
	GAMESTATE->m_fMusicSeconds = fPositionSeconds;
	GAMESTATE->m_fMusicBeat = fSongBeat;
	GAMESTATE->m_fCurBPS = fBPS;
	GAMESTATE->m_bFreeze = bFreeze;
	

	m_Background.SetSongBeat( fSongBeat, bFreeze, fPositionSeconds );

	
	//LOG->WriteLine( "m_fOffsetInBeats = %f, m_fBeatsPerSecond = %f, m_Music.GetPositionSeconds = %f", m_fOffsetInBeats, m_fBeatsPerSecond, m_Music.GetPositionSeconds() );

	const float fMaxBeatDifference = fBPS * PREFSMAN->m_fJudgeWindow * GAMESTATE->m_SongOptions.m_fMusicRate;

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled(PlayerNumber(p)) )
			continue;
		m_Player[p].Update( fDeltaTime, fSongBeat, fMaxBeatDifference );
	}

	// check for fail
	switch( GAMESTATE->m_SongOptions.m_FailType )
	{
	case SongOptions::FAIL_ARCADE:
	case SongOptions::FAIL_END_OF_SONG:
		{

			if( m_bBothHaveFailed )
				break;		// if they have already failed, don't bother checking again

			// check for both players fail
			bool bAllInDanger = true;
			bool bAllAreFailing = true;
			for( int p=0; p<NUM_PLAYERS; p++ )
			{
				if( !GAMESTATE->IsPlayerEnabled(PlayerNumber(p)) )
					continue;

				if( !m_pLifeMeter[p]->IsInDanger() )
					bAllInDanger = false;

				if( !m_pLifeMeter[p]->IsFailing() )
					bAllAreFailing = false;
			}

			if( bAllInDanger )	m_Background.TurnDangerOn();
			else				m_Background.TurnDangerOff();

			if( bAllAreFailing )
			{
				m_bBothHaveFailed = true;
				SCREENMAN->SendMessageToTopScreen( SM_LifeIs0, 0 );
			}
		}
		break;
	case SongOptions::FAIL_OFF:
		break;
	}

	// update seconds into play
	for( p=0; p<NUM_PLAYERS; p++ )
		if( GAMESTATE->IsPlayerEnabled((PlayerNumber)p)  &&  !m_pLifeMeter[p]->FailedEarlier() )
			GAMESTATE->GetLatestGameplayStatistics().fSecsIntoPlay[p] = max( GAMESTATE->GetLatestGameplayStatistics().fSecsIntoPlay[p], GAMESTATE->m_fMusicSeconds );


	switch( m_DancingState )
	{
	case STATE_DANCING:
		
		// Check for end of song
		if( fSongBeat > GAMESTATE->m_pCurSong->m_fLastBeat+4 )
			this->SendScreenMessage( SM_NotesEnded, 0 );
	
		// Check to see if it's time to play a gameplay comment
		m_fTimeLeftBeforeDancingComment -= fDeltaTime;
		if( m_fTimeLeftBeforeDancingComment <= 0 )
		{
			m_fTimeLeftBeforeDancingComment = TIME_BETWEEN_DANCING_COMMENTS;	// reset for the next comment

			bool bAllInDanger = true;
			for( int p=0; p<NUM_PLAYERS; p++ )
				if( !m_pLifeMeter[p]->IsInDanger() )
					bAllInDanger = false;

			bool bOneIsHot = false;
			for( p=0; p<NUM_PLAYERS; p++ )
				if( m_pLifeMeter[p]->IsHot() )
					bOneIsHot = true;

			if( bOneIsHot )
				m_announcerHot.PlayRandom();
			else if( bAllInDanger )
				m_announcerDanger.PlayRandom();
			else
				m_announcerGood.PlayRandom();
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
			if( !GAMESTATE->IsPlayerEnabled( (PlayerNumber)p ) )
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
	if( GAMESTATE->m_SongOptions.m_AssistType == SongOptions::ASSIST_TICK )
	{
		fPositionSeconds += (SOUND->GetPlayLatency()+0.04f) * m_soundMusic.GetPlaybackRate();	// HACK:  Add 0.04 seconds to make them play a tiny bit earlier
		GAMESTATE->m_pCurSong->GetBeatAndBPSFromElapsedTime( fPositionSeconds, fSongBeat, fBPS, bFreeze );

		int iRowNow = BeatToNoteRowNotRounded( fSongBeat );
		static int iRowLastCrossed = 0;

		bool bAnyoneHasANote = false;	// set this to true if any player has a note at one of the indicies we crossed

		for( int r=iRowLastCrossed+1; r<=iRowNow; r++ )  // for each index we crossed since the last update
		{
			for( int p=0; p<NUM_PLAYERS; p++ )
			{
				if( !GAMESTATE->IsPlayerEnabled( (PlayerNumber)p ) )
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
			if( !GAMESTATE->IsPlayerEnabled((PlayerNumber)p) )
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
	GAMESTATE->m_pCurSong->GetBeatAndBPSFromElapsedTime( m_soundMusic.GetPositionSeconds(), fSongBeat, fBPS, bFreeze );


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

				GAMESTATE->m_pCurSong->m_fBeat0OffsetInSeconds += fOffsetDelta;

				m_textDebug.SetText( ssprintf("Offset = %f.", GAMESTATE->m_pCurSong->m_fBeat0OffsetInSeconds) );
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

	const float fMaxBeatDifference = fBPS * PREFSMAN->m_fJudgeWindow * GAMESTATE->m_SongOptions.m_fMusicRate;

	if( type == IET_FIRST_PRESS )
	{
		if( StyleI.IsValid() )
		{
			if( GAMESTATE->IsPlayerEnabled( StyleI.player ) )
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
		m_announcerReady.PlayRandom();
		break;
	case SM_User+3:
		break;
	case SM_User+4:
		m_sprReady.StartBlurring();
		break;
	case SM_User+5:
		{
			m_sprHereWeGo.StartFocusing();
			m_announcerHereWeGo.PlayRandom();
			m_Background.FadeIn();
			m_soundMusic.Play();
		}
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
		if( GAMESTATE->m_SongOptions.m_FailType == SongOptions::FAIL_ARCADE )	// fail them now!
			this->SendScreenMessage( SM_BeginFailed, 0 );
		m_DancingState = STATE_OUTRO;
		break;
	case SM_NotesEnded:
		{
			for( int p=0; p<NUM_PLAYERS; p++ )
				if( GAMESTATE->IsPlayerEnabled( (PlayerNumber)p ) )
					m_Player[p].SaveGameplayStatistics();
			LoadNextSong( true );
		}
		break;
	case SM_LastNotesEnded:
		if( m_DancingState == STATE_OUTRO )	// gameplay already ended
			return;		// ignore

		if( m_bBothHaveFailed )	// fail them
		{
			this->SendScreenMessage( SM_BeginFailed, 0 );
		}
		else	// cleared
		{
			m_StarWipe.CloseWipingRight( SM_ShowCleared );
			m_announcerCleared.PlayRandom();	// crowd cheer
		}
		m_DancingState = STATE_OUTRO;
		break;

	case SM_100Combo:
		if( m_fTimeLeftBeforeDancingComment < 12 )
		{
			m_announcer100Combo.PlayRandom();
			m_fTimeLeftBeforeDancingComment = TIME_BETWEEN_DANCING_COMMENTS;
		}
		break;
	case SM_200Combo:
		if( m_fTimeLeftBeforeDancingComment < 12 )
		{
			m_announcer200Combo.PlayRandom();
			m_fTimeLeftBeforeDancingComment = TIME_BETWEEN_DANCING_COMMENTS;
		}
		break;
	case SM_300Combo:
		if( m_fTimeLeftBeforeDancingComment < 12 )
		{
			m_announcer300Combo.PlayRandom();
			m_fTimeLeftBeforeDancingComment = TIME_BETWEEN_DANCING_COMMENTS;
		}
		break;
	case SM_400Combo:
		if( m_fTimeLeftBeforeDancingComment < 12 )
		{
			m_announcer400Combo.PlayRandom();
			m_fTimeLeftBeforeDancingComment = TIME_BETWEEN_DANCING_COMMENTS;
		}
		break;
	case SM_500Combo:
		if( m_fTimeLeftBeforeDancingComment < 12 )
		{
			m_announcer500Combo.PlayRandom();
			m_fTimeLeftBeforeDancingComment = TIME_BETWEEN_DANCING_COMMENTS;
		}
		break;
	case SM_600Combo:
		if( m_fTimeLeftBeforeDancingComment < 12 )
		{
			m_announcer600Combo.PlayRandom();
			m_fTimeLeftBeforeDancingComment = TIME_BETWEEN_DANCING_COMMENTS;
		}
		break;
	case SM_700Combo:
		if( m_fTimeLeftBeforeDancingComment < 12 )
		{
			m_announcer700Combo.PlayRandom();
			m_fTimeLeftBeforeDancingComment = TIME_BETWEEN_DANCING_COMMENTS;
		}
		break;
	case SM_800Combo:
		if( m_fTimeLeftBeforeDancingComment < 12 )
		{
			m_announcer800Combo.PlayRandom();
			m_fTimeLeftBeforeDancingComment = TIME_BETWEEN_DANCING_COMMENTS;
		}
		break;
	case SM_900Combo:
		if( m_fTimeLeftBeforeDancingComment < 12 )
		{
			m_announcer900Combo.PlayRandom();
			m_fTimeLeftBeforeDancingComment = TIME_BETWEEN_DANCING_COMMENTS;
		}
		break;
	case SM_1000Combo:
		if( m_fTimeLeftBeforeDancingComment < 12 )
		{
			m_announcer1000Combo.PlayRandom();
			m_fTimeLeftBeforeDancingComment = TIME_BETWEEN_DANCING_COMMENTS;
		}
		break;
	case SM_ComboStopped:
		if( m_fTimeLeftBeforeDancingComment < 12 )
		{
			m_announcerComboStopped.PlayRandom();
			m_fTimeLeftBeforeDancingComment = TIME_BETWEEN_DANCING_COMMENTS;
		}
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
		SCREENMAN->SetNewScreen( new ScreenEvaluation(false) );
		break;


	case SM_BeginFailed:
		m_DancingState = STATE_OUTRO;
		m_soundMusic.Pause();
		m_StarWipe.CloseWipingRight( SM_None );
		this->SendScreenMessage( SM_ShowFailed, 0.2f );
		break;
	case SM_ShowFailed:
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

		// BUGFIX by ANDY: Stage will now reset back to 0 when game ends.
		GAMESTATE->m_iCurrentStageIndex = 0;

		SCREENMAN->SendMessageToTopScreen( SM_PlayFailComment, 1.5f );
		SCREENMAN->SendMessageToTopScreen( SM_HideFailed, 3.0f );
		break;
	case SM_PlayFailComment:
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


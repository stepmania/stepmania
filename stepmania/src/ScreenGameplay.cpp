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
#include "ScreenSelectCourse.h"
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
#include "ScreenPrompt.h"
#include "ScreenTitleMenu.h"

//
// Defines
//

const float LIFE_LOCAL_X[NUM_PLAYERS] = { -180, +180 };
const float LIFE_LOCAL_Y[NUM_PLAYERS] = { -8, -8 };

const float STAGE_NUMBER_LOCAL_X = 0;
const float STAGE_NUMBER_LOCAL_Y = +20;
const float STAGE_NUMBER_LOCALEZ2_Y = -30;

const float SONG_NUMBER_LOCAL_X[NUM_PLAYERS] = { STAGE_NUMBER_LOCAL_X-20, STAGE_NUMBER_LOCAL_X+20 };
const float SONG_NUMBER_LOCAL_Y[NUM_PLAYERS] = { STAGE_NUMBER_LOCAL_Y, STAGE_NUMBER_LOCAL_Y };


const float SCORE_LOCAL_X[NUM_PLAYERS] = { -214, +214 };
const float SCORE_LOCAL_Y[NUM_PLAYERS] = { -6, -6 };

const float SCORE_LOCALEZ2_X[NUM_PLAYERS] = { -240, +240 };
const float SCORE_LOCALEZ2_Y[NUM_PLAYERS] = { -32, -32 };

const float PLAYER_OPTIONS_LOCAL_X[NUM_PLAYERS]	= { -0, +0 };
const float PLAYER_OPTIONS_LOCAL_Y[NUM_PLAYERS]	= { -12, +2 };

const float DIFFICULTY_X[NUM_PLAYERS]	= { SCREEN_LEFT+60, SCREEN_RIGHT-60 };
const float DIFFICULTY_Y[NUM_PLAYERS]	= { SCREEN_BOTTOM-70, SCREEN_BOTTOM-70 };

const float DEBUG_X	= CENTER_X;
const float DEBUG_Y	= CENTER_Y-70;

const float SURVIVE_TIME_X	= CENTER_X;
const float SURVIVE_TIME_Y	= CENTER_Y+100;

const float TIME_BETWEEN_DANCING_COMMENTS	=	13;

const float DEMONSTRATION_TIME	=	30;


// received while STATE_DANCING
const ScreenMessage	SM_NotesEnded			= ScreenMessage(SM_User+101);
const ScreenMessage	SM_BeginLoadingNextSong	= ScreenMessage(SM_User+102);
const ScreenMessage	SM_BeginFadingToTitleMenu	= ScreenMessage(SM_User+103);


// received while STATE_OUTRO
const ScreenMessage	SM_ShowCleared			= ScreenMessage(SM_User+111);
const ScreenMessage	SM_HideCleared			= ScreenMessage(SM_User+112);
const ScreenMessage	SM_SaveChangedBeforeGoingBack	= ScreenMessage(SM_User+113);
const ScreenMessage	SM_GoToScreenAfterBack	= ScreenMessage(SM_User+114);
const ScreenMessage	SM_GoToStateAfterCleared= ScreenMessage(SM_User+115);

const ScreenMessage	SM_BeginFailed			= ScreenMessage(SM_User+121);
const ScreenMessage	SM_ShowFailed			= ScreenMessage(SM_User+122);
const ScreenMessage	SM_PlayFailComment		= ScreenMessage(SM_User+123);
const ScreenMessage	SM_HideFailed			= ScreenMessage(SM_User+124);
const ScreenMessage	SM_GoToScreenAfterFail	= ScreenMessage(SM_User+125);
const ScreenMessage	SM_GoToTitleMenu			= ScreenMessage(SM_User+126);



ScreenGameplay::ScreenGameplay()
{
	LOG->Trace( "ScreenGameplay::ScreenGameplay()" );


	GAMESTATE->ResetStageStatistics();	// clear values

	// Update possible dance points
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled(p) )
			continue;	// skip

		NoteData notedata;
		switch( GAMESTATE->m_PlayMode )
		{
		case PLAY_MODE_ARCADE:
			GAMESTATE->m_pCurNotes[p]->GetNoteData( &notedata );
			GAMESTATE->m_iPossibleDancePoints[p] = notedata.GetPossibleDancePoints();
			break;
		case PLAY_MODE_ONI:
		case PLAY_MODE_ENDLESS:
			{
				GAMESTATE->m_iPossibleDancePoints[p] = 0;

				Course* pCourse = GAMESTATE->m_pCurCourse;
				CArray<Song*,Song*> apSongs;
				CArray<Notes*,Notes*> apNotes;
				CStringArray asModifiers;
				pCourse->GetSongAndNotesForCurrentStyle( apSongs, apNotes, asModifiers );

				for( int i=0; i<apNotes.GetSize(); i++ )
				{
					apNotes[i]->GetNoteData( &notedata );
					int iPossibleDancePoints = notedata.GetPossibleDancePoints();
					GAMESTATE->m_iPossibleDancePoints[p] += iPossibleDancePoints;
				}
			}
			break;
		}

	}



	GAMESTATE->m_bUsedAutoPlayer |= PREFSMAN->m_bAutoPlay;
	m_bChangedOffsetOrBPM = false;


	m_DancingState = STATE_INTRO;
	m_fTimeLeftBeforeDancingComment = TIME_BETWEEN_DANCING_COMMENTS;


	m_Background.SetDiffuseColor( D3DXCOLOR(0.4f,0.4f,0.4f,1) );
	this->AddSubActor( &m_Background );


	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled(PlayerNumber(p)) )
			continue;

		float fPlayerX = (float) GAMESTATE->GetCurrentStyleDef()->m_iCenterX[p];
		m_Player[p].SetX( fPlayerX );
		this->AddSubActor( &m_Player[p] );
	
		m_sprOniGameOver[p].Load( THEME->GetPathTo(GRAPHIC_GAMEPLAY_ONI_GAMEOVER) );
		m_sprOniGameOver[p].SetX( fPlayerX );
		m_sprOniGameOver[p].SetY( SCREEN_TOP - m_sprOniGameOver[p].GetZoomedHeight()/2 );
		m_sprOniGameOver[p].SetDiffuseColor( D3DXCOLOR(1,1,1,0) );	// 0 alpha so we don't waste time drawing while not visible
		this->AddSubActor( &m_sprOniGameOver[p] );
	}


	m_OniFade.SetOpened();
	this->AddSubActor( &m_OniFade );


	//////////////////////////////////
	// Add all Actors to m_frameTop
	//////////////////////////////////
	this->AddSubActor( &m_frameTop );

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		switch( GAMESTATE->m_SongOptions.m_LifeType )
		{
		case SongOptions::LIFE_BAR:
			m_pLifeMeter[p] = new LifeMeterBar;
			break;
		case SongOptions::LIFE_BATTERY:
			m_pLifeMeter[p] = new LifeMeterBattery;
			break;
		default:
			ASSERT(0);
		}

		m_pLifeMeter[p]->Load( (PlayerNumber)p );
		m_pLifeMeter[p]->SetXY( LIFE_LOCAL_X[p], LIFE_LOCAL_Y[p] );
		m_frameTop.AddSubActor( m_pLifeMeter[p] );

		if( GAMESTATE->m_CurGame == GAME_EZ2 )
		{
			m_pScoreDisplay[p] = new ScoreDisplayNormal;
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

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		m_textCourseSongNumber[p].Load( THEME->GetPathTo(FONT_HEADER2) );
		m_textCourseSongNumber[p].TurnShadowOff();
		m_textCourseSongNumber[p].SetXY( SONG_NUMBER_LOCAL_X[p], SONG_NUMBER_LOCAL_Y[p] );
		m_textCourseSongNumber[p].SetText( "" );
		m_textCourseSongNumber[p].SetDiffuseColor( D3DXCOLOR(0.8f,0.8f,1,1) );	// light blue
	}

	if( GAMESTATE->m_CurGame == GAME_EZ2 )
	{
		m_textStageNumber.SetXY( STAGE_NUMBER_LOCAL_X, STAGE_NUMBER_LOCALEZ2_Y );
	}
	else
	{
		m_textStageNumber.SetXY( STAGE_NUMBER_LOCAL_X, STAGE_NUMBER_LOCAL_Y );
	}
	m_textStageNumber.SetText( GAMESTATE->GetStageText() );
	m_textStageNumber.SetDiffuseColor( GAMESTATE->GetStageColor() );

	switch( GAMESTATE->m_PlayMode )
	{
	case PLAY_MODE_ARCADE:
		m_frameTop.AddSubActor( &m_textStageNumber );
		break;
	case PLAY_MODE_ONI:
	case PLAY_MODE_ENDLESS:
		for( p=0; p<NUM_PLAYERS; p++ )
			if( GAMESTATE->IsPlayerEnabled(p) )
				m_frameTop.AddSubActor( &m_textCourseSongNumber[p] );
		break;
	default:
		ASSERT(0);	// invalid GameMode
	}


	//////////////////////////////////
	// Add all Actors to m_frameBottom
	//////////////////////////////////
	this->AddSubActor( &m_frameBottom );

	m_sprBottomFrame.Load( THEME->GetPathTo(GRAPHIC_GAMEPLAY_BOTTOM_FRAME) );
	m_frameBottom.AddSubActor( &m_sprBottomFrame );

	m_frameBottom.SetXY( CENTER_X, SCREEN_BOTTOM - m_sprBottomFrame.GetZoomedHeight()/2 );

	if( GAMESTATE->m_CurGame != GAME_EZ2 )
	{
		m_sprBottomFrame.Load( THEME->GetPathTo(GRAPHIC_GAMEPLAY_BOTTOM_FRAME) );
		m_frameBottom.AddSubActor( &m_sprBottomFrame );
		m_frameBottom.SetXY( CENTER_X, SCREEN_BOTTOM - m_sprBottomFrame.GetZoomedHeight()/2 );
	}



	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( GAMESTATE->m_CurGame != GAME_EZ2 )
		{
			switch( GAMESTATE->m_PlayMode )
			{
			case PLAY_MODE_ARCADE:
				m_pScoreDisplay[p] = new ScoreDisplayNormal;
				break;
			case PLAY_MODE_ONI:
			case PLAY_MODE_ENDLESS:
				m_pScoreDisplay[p] = new ScoreDisplayOni;
				break;
			default:
				ASSERT(0);
			}

			m_pScoreDisplay[p]->Init( (PlayerNumber)p );
			m_pScoreDisplay[p]->SetXY( SCORE_LOCAL_X[p], SCORE_LOCAL_Y[p] );
			m_pScoreDisplay[p]->SetZoom( 0.8f );
			m_pScoreDisplay[p]->SetDiffuseColor( PlayerToColor(p) );
			m_frameBottom.AddSubActor( m_pScoreDisplay[p] );
		}

		m_textPlayerOptions[p].Load( THEME->GetPathTo(FONT_NORMAL) );

		if( !GAMESTATE->IsPlayerEnabled(PlayerNumber(p)) )
			continue;

		if( GAMESTATE->m_CurGame != GAME_EZ2 )
		{	
			m_pScoreDisplay[p]->SetXY( SCORE_LOCAL_X[p], SCORE_LOCAL_Y[p] );
			m_pScoreDisplay[p]->SetZoom( 0.8f );
			m_pScoreDisplay[p]->SetDiffuseColor( PlayerToColor(p) );
			m_frameBottom.AddSubActor( m_pScoreDisplay[p] );
	//	}
		
	//	m_textPlayerOptions[p].Load( THEME->GetPathTo(FONT_NORMAL) );

		m_textPlayerOptions[p].TurnShadowOff();
		m_textPlayerOptions[p].SetXY( PLAYER_OPTIONS_LOCAL_X[p], PLAYER_OPTIONS_LOCAL_Y[p] );
		m_textPlayerOptions[p].SetZoom( 0.5f );
		m_textPlayerOptions[p].SetDiffuseColor( D3DXCOLOR(1,1,1,1) );
		m_textPlayerOptions[p].SetText( GAMESTATE->m_PlayerOptions[p].GetString() );
		m_frameBottom.AddSubActor( &m_textPlayerOptions[p] );
		}
	}



	// Get the current StyleDef definition (used below)
	StyleDef* pStyleDef = GAMESTATE->GetCurrentStyleDef();

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled(PlayerNumber(p)) )
			continue;

// CONFLICT RESOLUTION:
// BY ANDY
// <<<<<<< ScreenGameplay.cpp
// =======
// YOURS:
/*
		float fDifficultyY = DIFFICULTY_Y[p];
		if( GAMESTATE->m_PlayerOptions[p].m_bReverseScroll )
			fDifficultyY = SCREEN_HEIGHT - DIFFICULTY_Y[p] -10;	// HACK: move difficulty banner up 10 if reverse
		m_DifficultyBanner[p].SetXY( DIFFICULTY_X[p], fDifficultyY );
		this->AddSubActor( &m_DifficultyBanner[p] );
*/
//>>>>>>> 1.32 MINE
		/*
		if( GAMESTATE->m_CurGame != GAME_EZ2 )
		{	
			float fDifficultyY = DIFFICULTY_Y[p];
			if( GAMESTATE->m_PlayerOptions[p].m_bReverseScroll )
				fDifficultyY = SCREEN_HEIGHT - DIFFICULTY_Y[p];
			m_DifficultyBanner[p].SetXY( DIFFICULTY_X[p], fDifficultyY );
			this->AddSubActor( &m_DifficultyBanner[p] );
		}
*/
// HOW I THINK IT SHOULD BE FIXED:
		if( GAMESTATE->m_CurGame != GAME_EZ2 )
		{	
			float fDifficultyY = DIFFICULTY_Y[p];
			if( GAMESTATE->m_PlayerOptions[p].m_bReverseScroll )
				fDifficultyY = SCREEN_HEIGHT - DIFFICULTY_Y[p] -10;	// HACK: move difficulty banner up 10 if reverse
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

	if( GAMESTATE->m_bDemonstration )
	{
		m_quadDemonstrationBox.SetDiffuseColor( D3DXCOLOR(0,0,0,0.7f) );
		m_quadDemonstrationBox.StretchTo( CRect(SCREEN_LEFT, int(CENTER_Y-60), SCREEN_RIGHT, int(CENTER_Y+60)) );
		this->AddSubActor( &m_quadDemonstrationBox );

		m_sprDemonstration.Load( THEME->GetPathTo(GRAPHIC_GAMEPLAY_DEMONSTRATION) );
		m_sprDemonstration.SetXY( CENTER_X, CENTER_Y );
		m_sprDemonstration.SetEffectBlinking();
		this->AddSubActor( &m_sprDemonstration );

		m_Fade.OpenWipingRight();
	}

	m_Fade.SetOpened();
	this->AddSubActor( &m_Fade );


	m_textSurviveTime.Load( THEME->GetPathTo(FONT_HEADER1) );
	m_textSurviveTime.TurnShadowOff();
	m_textSurviveTime.SetXY( SURVIVE_TIME_X, SURVIVE_TIME_Y );
	m_textSurviveTime.SetText( "" );
	m_textSurviveTime.SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
	this->AddSubActor( &m_textSurviveTime );



	if( !GAMESTATE->m_bDemonstration )	// don't load sounds if just playing demonstration
	{
		m_soundFail.Load(			THEME->GetPathTo(SOUND_GAMEPLAY_FAILED) );
		m_soundOniDie.Load(			THEME->GetPathTo(SOUND_GAMEPLAY_ONI_DIE) );
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
	}

	m_soundAssistTick.Load(		THEME->GetPathTo(SOUND_GAMEPLAY_ASSIST_TICK) );


	LoadNextSong( true );


	if( GAMESTATE->m_bDemonstration )
	{
		m_StarWipe.SetOpened();
		m_DancingState = STATE_DANCING;
		m_soundMusic.Play();
		this->SendScreenMessage( SM_BeginFadingToTitleMenu, DEMONSTRATION_TIME );
	}
	else
	{
		for( int i=0; i<30; i++ )
			this->SendScreenMessage( ScreenMessage(SM_User+i), i/2.0f );	// Send messages to we can get the introduction rolling
	}
}

ScreenGameplay::~ScreenGameplay()
{
	LOG->Trace( "ScreenGameplay::~ScreenGameplay()" );
	
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		delete m_pLifeMeter[p];
		delete m_pScoreDisplay[p];
	}

	m_soundMusic.Stop();
}

bool ScreenGameplay::IsLastSong()
{
	switch( GAMESTATE->m_PlayMode )
	{
	case PLAY_MODE_ARCADE:
		return true;
	case PLAY_MODE_ONI:
	case PLAY_MODE_ENDLESS:
		{
			Course* pCourse = GAMESTATE->m_pCurCourse;

			if( pCourse->m_bRepeat )
				return false;

			CArray<Song*,Song*> apSongs;
			CArray<Notes*,Notes*> apNotes;
			CStringArray asModifiers;
			pCourse->GetSongAndNotesForCurrentStyle( apSongs, apNotes, asModifiers );

			return GAMESTATE->m_iSongsIntoCourse >= apSongs.GetSize();	// there are no more songs left
		}
		break;
	default:
		ASSERT(0);
		return true;
	}
}

void ScreenGameplay::LoadNextSong( bool bFirstLoad )
{
	GAMESTATE->ResetMusicStatistics();

	switch( GAMESTATE->m_PlayMode )
	{
	case PLAY_MODE_ARCADE:
		break;
	case PLAY_MODE_ONI:
	case PLAY_MODE_ENDLESS:
		{
			for( int p=0; p<NUM_PLAYERS; p++ )
				m_textCourseSongNumber[p].SetText( ssprintf("%d", GAMESTATE->m_iSongsBeforeFail[p]+1) );

			Course* pCourse = GAMESTATE->m_pCurCourse;
			CArray<Song*,Song*> apSongs;
			CArray<Notes*,Notes*> apNotes;
			CStringArray asModifiers;
			pCourse->GetSongAndNotesForCurrentStyle( apSongs, apNotes, asModifiers );

			int iPlaySongIndex = -1;
			if( pCourse->m_bRandomize )
				iPlaySongIndex = rand() % apSongs.GetSize();
			else
				iPlaySongIndex = GAMESTATE->m_iSongsIntoCourse;

			GAMESTATE->m_pCurSong = apSongs[iPlaySongIndex];
			for( p=0; p<NUM_PLAYERS; p++ )
			{
				GAMESTATE->m_pCurNotes[p] = apNotes[iPlaySongIndex];
				if( asModifiers[iPlaySongIndex] != "" )		// some modifiers specified
					GAMESTATE->m_PlayerOptions[p].FromString( asModifiers[iPlaySongIndex] );	// put them into effect
			}
		}
		break;
	default:
		ASSERT(0);
		break;
	}

	m_textStageNumber.SetText( GAMESTATE->GetStageText() );



	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled(p) )
			continue;

		// reset oni game over graphic
		m_sprOniGameOver[p].SetY( SCREEN_TOP - m_sprOniGameOver[p].GetZoomedHeight()/2 );
		m_sprOniGameOver[p].SetDiffuseColor( D3DXCOLOR(1,1,1,0) );	// 0 alpha so we don't waste time drawing while not visible

		if( GAMESTATE->m_SongOptions.m_LifeType == SongOptions::LIFE_BATTERY  &&  GAMESTATE->m_fSecondsBeforeFail[p] != -1 )	// already failed
			ShowOniGameOver((PlayerNumber)p);


		m_DifficultyBanner[p].SetFromNotes( PlayerNumber(p), GAMESTATE->m_pCurNotes[p] );


		NoteData originalNoteData;
		GAMESTATE->m_pCurNotes[p]->GetNoteData( &originalNoteData );
		
		StyleDef* pStyleDef = GAMESTATE->GetCurrentStyleDef();
		NoteData newNoteData;
		pStyleDef->GetTransformedNoteDataForStyle( (PlayerNumber)p, &originalNoteData, &newNoteData );

		m_Player[p].Load( (PlayerNumber)p, &newNoteData, m_pLifeMeter[p], m_pScoreDisplay[p] );
	}

	
	m_Background.LoadFromSong( GAMESTATE->m_pCurSong );
	m_Background.SetDiffuseColor( D3DXCOLOR(0.5f,0.5f,0.5f,1) );
	m_Background.BeginTweeningQueued( 2 );
	m_Background.SetTweenDiffuseColor( D3DXCOLOR(1,1,1,1) );

	m_soundMusic.Load( GAMESTATE->m_pCurSong->GetMusicPath(), true );	// enable accurate sync
	float fStartSeconds = min( 0, -4+GAMESTATE->m_pCurSong->GetElapsedTimeFromBeat(GAMESTATE->m_pCurSong->m_fFirstBeat) );
	m_soundMusic.SetPositionSeconds( fStartSeconds );
	m_soundMusic.SetPlaybackRate( GAMESTATE->m_SongOptions.m_fMusicRate );
	if( !bFirstLoad )
		m_soundMusic.Play();
}

bool ScreenGameplay::OneIsHot()
{
	for( int p=0; p<NUM_PLAYERS; p++ )
		if( GAMESTATE->IsPlayerEnabled(PlayerNumber(p)) )
			if( !m_pLifeMeter[p]->IsHot() )
				return true;
	return false;
}

bool ScreenGameplay::AllAreInDanger()
{
	for( int p=0; p<NUM_PLAYERS; p++ )
		if( GAMESTATE->IsPlayerEnabled(PlayerNumber(p)) )
			if( !m_pLifeMeter[p]->IsInDanger() )
				return false;
	return true;
}

bool ScreenGameplay::AllAreFailing()
{
	for( int p=0; p<NUM_PLAYERS; p++ )
		if( GAMESTATE->IsPlayerEnabled(PlayerNumber(p)) )
			if( !m_pLifeMeter[p]->IsFailing() )
				return false;
	return true;
}

bool ScreenGameplay::AllFailedEarlier()
{
	for( int p=0; p<NUM_PLAYERS; p++ )
		if( GAMESTATE->IsPlayerEnabled(p) )
			if( !m_pLifeMeter[p]->FailedEarlier() )
				return false;
	return true;
}


void ScreenGameplay::Update( float fDeltaTime )
{
	//LOG->Trace( "ScreenGameplay::Update(%f)", fDeltaTime );

	m_soundMusic.Update( fDeltaTime );

	if( GAMESTATE->m_pCurSong == NULL )
		return;


	// update the global music statistics for other classes to access
	float fPositionSeconds = m_soundMusic.GetPositionSeconds();
	float fSongBeat, fBPS;
	bool bFreeze;	
	GAMESTATE->m_pCurSong->GetBeatAndBPSFromElapsedTime( fPositionSeconds, fSongBeat, fBPS, bFreeze );


	GAMESTATE->m_fMusicSeconds = fPositionSeconds;
	GAMESTATE->m_fSongBeat = fSongBeat;
	GAMESTATE->m_fCurBPS = fBPS;
	GAMESTATE->m_bFreeze = bFreeze;

//	LOG->Trace( "GAMESTATE->m_fMusicSeconds = %f", GAMESTATE->m_fMusicSeconds );
	

	m_Background.SetSongBeat( fSongBeat, bFreeze, fPositionSeconds );

	//LOG->Trace( "m_fOffsetInBeats = %f, m_fBeatsPerSecond = %f, m_Music.GetPositionSeconds = %f", m_fOffsetInBeats, m_fBeatsPerSecond, m_Music.GetPositionSeconds() );


	switch( m_DancingState )
	{
	case STATE_DANCING:
		
		//
		// Check for end of song
		//
		if( fSongBeat > GAMESTATE->m_pCurSong->m_fLastBeat+4 )
		{
			GAMESTATE->m_fSongBeat = 0;
			m_soundMusic.Stop();
			this->SendScreenMessage( SM_NotesEnded, 0 );
		}

		//
		// check for fail
		//
		switch( GAMESTATE->m_SongOptions.m_FailType )
		{
		case SongOptions::FAIL_ARCADE:
			switch( GAMESTATE->m_SongOptions.m_LifeType )
			{
			case SongOptions::LIFE_BAR:
				if( AllAreFailing() )	SCREENMAN->SendMessageToTopScreen( SM_BeginFailed, 0 );
				if( AllAreInDanger() )	m_Background.TurnDangerOn();
				else					m_Background.TurnDangerOff();
				break;
			case SongOptions::LIFE_BATTERY:
				if( AllFailedEarlier() )SCREENMAN->SendMessageToTopScreen( SM_BeginFailed, 0 );
				if( AllAreInDanger() )	m_Background.TurnDangerOn();
				else					m_Background.TurnDangerOff();

				// check for individual fail
				for( int p=0; p<NUM_PLAYERS; p++ )
					if( GAMESTATE->IsPlayerEnabled(p) )
						if( m_pLifeMeter[p]->IsFailing()  &&  GAMESTATE->m_fSecondsBeforeFail[p] == -1 )	// not yet failed
							if( !AllFailedEarlier() )	// if not the last one to fail
							{
								// kill them!
								GAMESTATE->m_fSecondsBeforeFail[p] = GAMESTATE->GetElapsedSeconds();
								m_soundOniDie.PlayRandom();
								ShowOniGameOver((PlayerNumber)p);
								m_Player[p].Init();		// remove all notes and scoring
								m_Player[p].FadeToFail();	// tell the NoteField to fade to white
							}
				break;
			}
			break;
		case SongOptions::FAIL_END_OF_SONG:
		case SongOptions::FAIL_OFF:
			break;	// don't check for fail
		default:
			ASSERT(0);
		}

		//
		// Check to see if it's time to play a gameplay comment
		//
		if( !GAMESTATE->m_bDemonstration )		// don't play announcer comments in demonstration
		{
			m_fTimeLeftBeforeDancingComment -= fDeltaTime;
			if( m_fTimeLeftBeforeDancingComment <= 0 )
			{
				m_fTimeLeftBeforeDancingComment = TIME_BETWEEN_DANCING_COMMENTS;	// reset for the next comment



				if( OneIsHot() )
					m_announcerHot.PlayRandom();
				else if( AllAreInDanger() )
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
	CLAMP( iRowNow, 0, MAX_TAP_NOTE_ROWS-1 );
	static int iRowLastCrossed = 0;
	CLAMP( iRowLastCrossed, 0, MAX_TAP_NOTE_ROWS-1 );

	for( int r=iRowLastCrossed+1; r<=iRowNow; r++ )  // for each index we crossed since the last update
	{
		for( int p=0; p<NUM_PLAYERS; p++ )
		{
			if( GAMESTATE->IsPlayerEnabled(p) )
				m_Player[p].CrossedRow( r );
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
		fPositionSeconds += (SOUND->GetPlayLatency()+0.01f) * m_soundMusic.GetPlaybackRate();	// HACK:  Add 0.02 seconds to account for the fact that the sound file has 0.01 seconds of silence at the beginning
		GAMESTATE->m_pCurSong->GetBeatAndBPSFromElapsedTime( fPositionSeconds, fSongBeat, fBPS, bFreeze );

		int iRowNow = BeatToNoteRowNotRounded( fSongBeat );
		iRowNow = max( 0, iRowNow );
		static int iRowLastCrossed = 0;

		bool bAnyoneHasANote = false;	// set this to true if any player has a note at one of the indicies we crossed

		for( int r=iRowLastCrossed+1; r<=iRowNow; r++ )  // for each index we crossed since the last update
		{
			for( int p=0; p<NUM_PLAYERS; p++ )
			{
				if( !GAMESTATE->IsPlayerEnabled( (PlayerNumber)p ) )
					continue;		// skip

				m_Player[p].CrossedRow( r );
				bAnyoneHasANote |= m_Player[p].IsThereANoteAtRow( r );
				break;	// this will only play the tick for the first player that is joined
			}
		}

		if( bAnyoneHasANote )
			m_soundAssistTick.PlayRandom();


		iRowLastCrossed = iRowNow;
	}


	Screen::Update( fDeltaTime );
}


void ScreenGameplay::DrawPrimitives()
{
	Screen::DrawPrimitives();
}


void ScreenGameplay::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	//LOG->Trace( "ScreenGameplay::Input()" );

	if( GAMESTATE->m_bDemonstration )
	{
		if( MenuI.button == MENU_BUTTON_START )
		{
			m_soundMusic.Stop();
			SOUND->PlayOnceStreamed( THEME->GetPathTo(SOUND_INSERT_COIN) );
			m_Fade.CloseWipingRight( SM_GoToTitleMenu );
		}
		return;	// don't fall through below
	}


	// Handle special keys to adjust the offset
	if( DeviceI.device == DEVICE_KEYBOARD )
	{
		switch( DeviceI.button )
		{
		case DIK_F8:
			PREFSMAN->m_bAutoPlay = !PREFSMAN->m_bAutoPlay;
			GAMESTATE->m_bUsedAutoPlayer |= PREFSMAN->m_bAutoPlay;
			m_textDebug.SetText( ssprintf("Autoplayer %s.", (PREFSMAN->m_bAutoPlay ? "ON" : "OFF")) );
			m_textDebug.SetDiffuseColor( D3DXCOLOR(1,1,1,1) );
			m_textDebug.StopTweening();
			m_textDebug.BeginTweeningQueued( 3 );		// sleep
			m_textDebug.BeginTweeningQueued( 0.5f );	// fade out
			m_textDebug.SetTweenDiffuseColor( D3DXCOLOR(1,1,1,0) );
			break;
		case DIK_F9:
		case DIK_F10:
			{
				m_bChangedOffsetOrBPM = true;

				float fOffsetDelta;
				switch( DeviceI.button )
				{
				case DIK_F9:	fOffsetDelta = -0.025f;		break;
				case DIK_F10:	fOffsetDelta = +0.025f;		break;
				default:	ASSERT(0);
				}
				if( type == IET_FAST_REPEAT )
					fOffsetDelta *= 10;
				BPMSegment& seg = GAMESTATE->m_pCurSong->GetBPMSegmentAtBeat( GAMESTATE->m_fSongBeat );

				seg.m_fBPM += fOffsetDelta;

				m_textDebug.SetText( ssprintf("Cur BPM = %f.", GAMESTATE->m_pCurSong->m_fBeat0OffsetInSeconds) );
				m_textDebug.SetDiffuseColor( D3DXCOLOR(1,1,1,1) );
				m_textDebug.StopTweening();
				m_textDebug.BeginTweeningQueued( 3 );		// sleep
				m_textDebug.BeginTweeningQueued( 0.5f );	// fade out
				m_textDebug.SetTweenDiffuseColor( D3DXCOLOR(1,1,1,0) );
			}
			break;
		case DIK_F11:
		case DIK_F12:
			{
				m_bChangedOffsetOrBPM = true;

				float fOffsetDelta;
				switch( DeviceI.button )
				{
				case DIK_F11:	fOffsetDelta = -0.02f;		break;
				case DIK_F12:	fOffsetDelta = +0.02f;		break;
				default:	ASSERT(0);
				}
				if( type == IET_FAST_REPEAT )
					fOffsetDelta *= 10;

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

	if( MenuI.IsValid()  &&  
		MenuI.button == MENU_BUTTON_BACK  &&  
		m_DancingState != STATE_OUTRO  &&
		!m_Fade.IsClosing() )
	{
		if( (DeviceI.device==DEVICE_KEYBOARD && type==IET_SLOW_REPEAT)  ||
			(DeviceI.device!=DEVICE_KEYBOARD && type==IET_FAST_REPEAT) )
		{
			m_DancingState = STATE_OUTRO;
			SOUND->PlayOnceStreamed( THEME->GetPathTo(SOUND_MENU_BACK) );
			m_soundMusic.Stop();
			this->ClearMessageQueue();
			m_Fade.CloseWipingLeft( SM_SaveChangedBeforeGoingBack );
		}
	}

	//
	// handle a step
	//
	if( m_DancingState == STATE_DANCING )
	{
		if( type == IET_FIRST_PRESS )
		{
			if( StyleI.IsValid() )
			{
				if( GAMESTATE->IsPlayerEnabled( StyleI.player ) )
					m_Player[StyleI.player].Step( StyleI.col ); 
			}
		}
	}
}

void SaveChanges()
{
	switch( GAMESTATE->m_PlayMode )
	{
	case PLAY_MODE_ARCADE:
		GAMESTATE->m_pCurSong->SaveToSMFile();
		break;
	case PLAY_MODE_ONI:
	case PLAY_MODE_ENDLESS:
		{
			for( int i=0; i<GAMESTATE->m_pCurCourse->m_iStages; i++ )
			{
				Song* pSong = GAMESTATE->m_pCurCourse->m_apSongs[i];
				pSong->SaveToSMFile();
			}
		}
		break;
	default:
		ASSERT(0);
	}
}

void DontSaveChanges()
{
	switch( GAMESTATE->m_PlayMode )
	{
	case PLAY_MODE_ARCADE:
		GAMESTATE->m_pCurSong->LoadFromSMFile( GAMESTATE->m_pCurSong->GetCacheFilePath() );
		break;
	case PLAY_MODE_ONI:
	case PLAY_MODE_ENDLESS:
		{
			for( int i=0; i<GAMESTATE->m_pCurCourse->m_iStages; i++ )
			{
				Song* pSong = GAMESTATE->m_pCurCourse->m_apSongs[i];
				pSong->LoadFromSMFile( GAMESTATE->m_pCurSong->GetCacheFilePath() );
			}
		}
		break;
	default:
		ASSERT(0);
	}
}

void ShowSavePrompt( ScreenMessage SM_SendWhenDone )
{
	CString sMessage;
	switch( GAMESTATE->m_PlayMode )
	{
	case PLAY_MODE_ARCADE:
		sMessage = ssprintf( 
			"You have changed the offset or BPM of\n"
			"%s.\n"
			"Would you like to save these changes back\n"
			"to the song file?\n"
			"Choosing NO will disgard your changes.",
			GAMESTATE->m_pCurSong->GetFullTitle() );
		break;
	case PLAY_MODE_ONI:
	case PLAY_MODE_ENDLESS:
		sMessage = ssprintf( 
			"You have changed the offset or BPM of\n"
			"one or more songs in this course.\n"
			"Would you like to save these changes back\n"
			"to the song file(s)?\n"
			"Choosing NO will disgard your changes." );
		break;
	default:
		ASSERT(0);
	}

	SCREENMAN->AddScreenToTop( new ScreenPrompt(
		SM_SendWhenDone,
		sMessage,
		PROMPT_YES_NO,
		true,
		SaveChanges,
		DontSaveChanges
		)
	);
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
	case SM_NotesEnded:
		{
			// save any statistics
			for( int p=0; p<NUM_PLAYERS; p++ )
			{
				if( GAMESTATE->IsPlayerEnabled(p) )
				{
					for( int r=0; r<NUM_RADAR_CATEGORIES; r++ )
					{
						RadarCategory rc = (RadarCategory)r;
						GAMESTATE->m_fRadarPossible[p][r] = m_Player[p].GetRadarValue( rc, GAMESTATE->m_pCurSong->m_fMusicLengthSeconds );
						GAMESTATE->m_fRadarActual[p][r] = m_Player[p].GetActualRadarValue( rc, GAMESTATE->m_pCurSong->m_fMusicLengthSeconds );
					}
				}
			}

			GAMESTATE->m_apSongsPlayed.Add( GAMESTATE->m_pCurSong );
			GAMESTATE->m_iSongsIntoCourse++;
			for( p=0; p<NUM_PLAYERS; p++ )
				if( GAMESTATE->IsPlayerEnabled(p) )
					if( !m_pLifeMeter[p]->FailedEarlier() )
						GAMESTATE->m_iSongsBeforeFail[p]++;


			if( !IsLastSong() )
			{
				for( int p=0; p<NUM_PLAYERS; p++ )
					if( GAMESTATE->IsPlayerEnabled(p) )
						if( !m_pLifeMeter[p]->FailedEarlier() )
							m_pLifeMeter[p]->SongEnded();	// let the oni life meter give them back a life

				m_OniFade.CloseWipingRight( SM_BeginLoadingNextSong );
			}
			else	// IsLastSong
			{
				if( m_DancingState == STATE_OUTRO )	// gameplay already ended
					return;		// ignore

				m_DancingState = STATE_OUTRO;

				GAMESTATE->AccumulateStageStatistics();	// accumulate values for final evaluation

				if( GAMESTATE->m_SongOptions.m_FailType == SongOptions::FAIL_END_OF_SONG  &&  AllFailedEarlier() )
				{
					this->SendScreenMessage( SM_BeginFailed, 0 );
				}
				else
				{
					m_StarWipe.CloseWipingRight( SM_ShowCleared );
					SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_GAMEPLAY_CLEARED) );
				}
			}
		}
		break;

	case SM_BeginLoadingNextSong:
		LoadNextSong( false );
		m_OniFade.OpenWipingRight( SM_None );
		break;
	case SM_BeginFadingToTitleMenu:
		m_Fade.CloseWipingRight( SM_GoToTitleMenu );
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
		SCREENMAN->SendMessageToTopScreen( SM_GoToStateAfterCleared, 1 );
		break;
	case SM_SaveChangedBeforeGoingBack:
		if( m_bChangedOffsetOrBPM )
		{
			m_bChangedOffsetOrBPM = false;
			ShowSavePrompt( SM_GoToScreenAfterBack );
		}
		else
			this->SendScreenMessage( SM_GoToScreenAfterBack, 0 );

		break;
	case SM_GoToScreenAfterBack:
		switch( GAMESTATE->m_PlayMode )
		{
		case PLAY_MODE_ARCADE:	
			SCREENMAN->SetNewScreen( new ScreenSelectMusic );
			break;
		case PLAY_MODE_ONI:
		case PLAY_MODE_ENDLESS:
			SCREENMAN->SetNewScreen( new ScreenSelectCourse );
			break;
		default:	ASSERT(0);
		}
		break;
	case SM_GoToStateAfterCleared:
		if( m_bChangedOffsetOrBPM )
		{
			m_bChangedOffsetOrBPM = false;
			ShowSavePrompt( SM_GoToStateAfterCleared );
			break;
		}
		SCREENMAN->SetNewScreen( new ScreenEvaluation(false) );
		break;


	case SM_BeginFailed:
		m_DancingState = STATE_OUTRO;
		m_soundMusic.Pause();
		m_StarWipe.CloseWipingRight( SM_None );
		int p;
		for( p=0; p<NUM_PLAYERS; p++ )
			if( GAMESTATE->IsPlayerEnabled(p) )
				m_Player[p].FadeToFail();

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

		// show the survive time if extra stage
		if( GAMESTATE->IsExtraStage()  ||  GAMESTATE->IsExtraStage2() )
		{
			float fMaxSurviveSeconds = -1;
			for( int p=0; p<NUM_PLAYERS; p++ )
				if( GAMESTATE->IsPlayerEnabled(p) )
					fMaxSurviveSeconds = max( fMaxSurviveSeconds, GAMESTATE->GetPlayerSurviveTime((PlayerNumber)p) );
			ASSERT( fMaxSurviveSeconds != -1 );
			m_textSurviveTime.SetText( "TIME  " + SecondsToTime(fMaxSurviveSeconds) );
			m_textSurviveTime.BeginTweening( 0.3f );	// sleep
			m_textSurviveTime.BeginTweeningQueued( 0.3f );	// fade in
			m_textSurviveTime.SetTweenDiffuseColor( D3DXCOLOR(1,1,1,1) );
		}

		SCREENMAN->SendMessageToTopScreen( SM_PlayFailComment, 1.5f );
		SCREENMAN->SendMessageToTopScreen( SM_HideFailed, 3.0f );
		break;
	case SM_PlayFailComment:
		SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_GAMEPLAY_FAILED) );
		break;
	case SM_HideFailed:
		m_sprFailed.StopTweening();
		m_sprFailed.BeginTweening(1.0f);
		m_sprFailed.SetTweenDiffuseColor( D3DXCOLOR(1,1,1,0) );

		m_textSurviveTime.BeginTweening( 0.5f );	// sleep
		m_textSurviveTime.BeginTweeningQueued( 0.5f );	// fade out
		m_textSurviveTime.SetTweenDiffuseColor( D3DXCOLOR(1,1,1,0) );

		SCREENMAN->SendMessageToTopScreen( SM_GoToScreenAfterFail, 1.5f );
		break;
	case SM_GoToScreenAfterFail:
		if( m_bChangedOffsetOrBPM )
		{
			m_bChangedOffsetOrBPM = false;
			ShowSavePrompt( SM_GoToScreenAfterFail );
			break;
		}
		if( PREFSMAN->m_bEventMode )
			this->SendScreenMessage( SM_GoToScreenAfterBack, 0 );
		else if( GAMESTATE->IsExtraStage()  ||  GAMESTATE->IsExtraStage2() )
			SCREENMAN->SetNewScreen( new ScreenEvaluation(true) );
		else if( GAMESTATE->m_PlayMode == PLAY_MODE_ONI  ||  GAMESTATE->m_PlayMode == PLAY_MODE_ENDLESS )
			SCREENMAN->SetNewScreen( new ScreenEvaluation(false) );
		else
			SCREENMAN->SetNewScreen( new ScreenGameOver );
		break;
	case SM_GoToTitleMenu:
		SCREENMAN->SetNewScreen( new ScreenTitleMenu );
		break;
	}
}


void ScreenGameplay::TweenOnScreen()
{

}

void ScreenGameplay::TweenOffScreen()
{

}

void ScreenGameplay::ShowOniGameOver( PlayerNumber p )
{
	m_sprOniGameOver[p].SetDiffuseColor( D3DXCOLOR(1,1,1,1) );
	m_sprOniGameOver[p].BeginTweening( 0.5f, Actor::TWEEN_BOUNCE_END );
	m_sprOniGameOver[p].SetTweenY( CENTER_Y );
	m_sprOniGameOver[p].SetEffectBobbing( D3DXVECTOR3(0,6,0), 4 );
}

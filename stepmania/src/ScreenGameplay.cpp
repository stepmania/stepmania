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
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "GameManager.h"
#include "SongManager.h"
#include "RageLog.h"
#include "AnnouncerManager.h"
#include "LifeMeterBar.h"
#include "LifeMeterBattery.h"
#include "GameState.h"
#include "ScoreDisplayNormal.h"
#include "ScoreDisplayOni.h"
#include "ScreenPrompt.h"
#include "GrooveRadar.h"
#include "NotesLoaderSM.h"

//
// Defines
//

#define TOP_FRAME_X						THEME->GetMetricF("ScreenGameplay","TopFrameX")
#define TOP_FRAME_Y( e )				THEME->GetMetricF("ScreenGameplay",ssprintf("TopFrame%sY",e?"Extra":""))
#define BOTTOM_FRAME_X					THEME->GetMetricF("ScreenGameplay","BottomFrameX")
#define BOTTOM_FRAME_Y( e )				THEME->GetMetricF("ScreenGameplay",ssprintf("BottomFrame%sY",e?"Extra":""))
#define MIDDLE_FRAME_X					THEME->GetMetricF("ScreenGameplay","MiddleFrameX")
#define MIDDLE_FRAME_Y					THEME->GetMetricF("ScreenGameplay","MiddleFrameY")
#define LIFE_X( p )						THEME->GetMetricF("ScreenGameplay",ssprintf("LifeP%dX",p+1))
#define LIFE_Y( p, e )					THEME->GetMetricF("ScreenGameplay",ssprintf("LifeP%d%sY",p+1,e?"Extra":""))
#define STAGE_X							THEME->GetMetricF("ScreenGameplay","StageX")
#define STAGE_Y( e )					THEME->GetMetricF("ScreenGameplay",ssprintf("Stage%sY",e?"Extra":""))
#define SONG_NUMBER_X( p )				THEME->GetMetricF("ScreenGameplay",ssprintf("SongNumberP%dX",p+1))
#define SONG_NUMBER_Y( p, e )			THEME->GetMetricF("ScreenGameplay",ssprintf("SongNumberP%d%sY",p+1,e?"Extra":""))
#define SCORE_X( p )					THEME->GetMetricF("ScreenGameplay",ssprintf("ScoreP%dX",p+1))
#define SCORE_Y( p, e )					THEME->GetMetricF("ScreenGameplay",ssprintf("ScoreP%d%sY",p+1,e?"Extra":""))
#define PLAYER_OPTIONS_X( p )			THEME->GetMetricF("ScreenGameplay",ssprintf("PlayerOptionsP%dX",p+1))
#define PLAYER_OPTIONS_Y( p, e )		THEME->GetMetricF("ScreenGameplay",ssprintf("PlayerOptionsP%d%sY",p+1,e?"Extra":""))
#define SONG_OPTIONS_X					THEME->GetMetricF("ScreenGameplay","SongOptionsX")
#define SONG_OPTIONS_Y( e )				THEME->GetMetricF("ScreenGameplay",ssprintf("SongOptions%sY",e?"Extra":""))
#define DIFFICULTY_X( p )				THEME->GetMetricF("ScreenGameplay",ssprintf("DifficultyP%dX",p+1))
#define DIFFICULTY_Y( p, e, r )			THEME->GetMetricF("ScreenGameplay",ssprintf("DifficultyP%d%s%sY",p+1,e?"Extra":"",r?"Reverse":""))
#define DEBUG_X							THEME->GetMetricF("ScreenGameplay","DebugX")
#define DEBUG_Y							THEME->GetMetricF("ScreenGameplay","DebugY")
#define AUTOPLAY_X						THEME->GetMetricF("ScreenGameplay","AutoPlayX")
#define AUTOPLAY_Y						THEME->GetMetricF("ScreenGameplay","AutoPlayY")
#define SURVIVE_TIME_X					THEME->GetMetricF("ScreenGameplay","SurviveTimeX")
#define SURVIVE_TIME_Y					THEME->GetMetricF("ScreenGameplay","SurviveTimeY")
#define SECONDS_BETWEEN_COMMENTS		THEME->GetMetricF("ScreenGameplay","SecondsBetweenComments")
#define DEMONSTRATION_SECONDS			THEME->GetMetricF("ScreenGameplay","DemonstrationSeconds")
#define TICK_EARLY_SECONDS				THEME->GetMetricF("ScreenGameplay","TickEarlySeconds")

float g_fTickEarlySecondsCache = 0;		// reading directly out of theme metrics is slow


// received while STATE_DANCING
const ScreenMessage	SM_NotesEnded			= ScreenMessage(SM_User+101);
const ScreenMessage	SM_BeginLoadingNextSong	= ScreenMessage(SM_User+102);
const ScreenMessage	SM_BeginFadingToTitleMenu	= ScreenMessage(SM_User+103);
const ScreenMessage SM_PlayToastySound		= ScreenMessage(SM_User+105);

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

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		m_pLifeMeter[p] = NULL;
		m_pScoreDisplay[p] = NULL;
	}

	g_fTickEarlySecondsCache = TICK_EARLY_SECONDS;


	MUSIC->Stop();

	GAMESTATE->ResetStageStatistics();	// clear values

	const bool bExtra = GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2();
	const bool bReverse[NUM_PLAYERS] = { 
		GAMESTATE->m_PlayerOptions[0].m_bReverseScroll,
		GAMESTATE->m_PlayerOptions[1].m_bReverseScroll
	};

	// Update possible dance points
	for( p=0; p<NUM_PLAYERS; p++ )
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
				pCourse->GetSongAndNotesForCurrentStyle( apSongs, apNotes, asModifiers, true );

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


	m_bChangedOffsetOrBPM = false;


	m_DancingState = STATE_INTRO;
	m_fTimeLeftBeforeDancingComment = SECONDS_BETWEEN_COMMENTS;


	m_Background.SetDiffuse( D3DXCOLOR(0.4f,0.4f,0.4f,1) );
	this->AddChild( &m_Background );


	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled(PlayerNumber(p)) )
			continue;

		float fPlayerX = (float) GAMESTATE->GetCurrentStyleDef()->m_iCenterX[p];
		m_Player[p].SetX( fPlayerX );
		this->AddChild( &m_Player[p] );
	
		m_sprOniGameOver[p].Load( THEME->GetPathTo("Graphics","gameplay oni gameover") );
		m_sprOniGameOver[p].SetX( fPlayerX );
		m_sprOniGameOver[p].SetY( SCREEN_TOP - m_sprOniGameOver[p].GetZoomedHeight()/2 );
		m_sprOniGameOver[p].SetDiffuse( D3DXCOLOR(1,1,1,0) );	// 0 alpha so we don't waste time drawing while not visible
		this->AddChild( &m_sprOniGameOver[p] );
	}


	m_OniFade.SetOpened();
	this->AddChild( &m_OniFade );


	m_sprMiddleFrame.Load( THEME->GetPathTo("Graphics","Gameplay Middle Frame") );
	m_sprMiddleFrame.SetXY( MIDDLE_FRAME_X, MIDDLE_FRAME_Y );


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
		m_pLifeMeter[p]->SetXY( LIFE_X(p), LIFE_Y(p,bExtra) );
		this->AddChild( m_pLifeMeter[p] );		
	}

	// TopFrame goes above LifeMeter
	m_sprTopFrame.Load( THEME->GetPathTo("Graphics",bExtra?"gameplay extra top frame":"gameplay top frame") );
	m_sprTopFrame.SetXY( TOP_FRAME_X, TOP_FRAME_Y(bExtra) );
	this->AddChild( &m_sprTopFrame );


	m_textStageNumber.LoadFromFont( THEME->GetPathTo("Fonts","gameplay stage") );
	m_textStageNumber.TurnShadowOff();
	m_textStageNumber.SetXY( STAGE_X, STAGE_Y(bExtra) );
	m_textStageNumber.SetText( GAMESTATE->GetStageText() );
	m_textStageNumber.SetDiffuse( GAMESTATE->GetStageColor() );

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		m_textCourseSongNumber[p].LoadFromFont( THEME->GetPathTo("Fonts","Header2") );
		m_textCourseSongNumber[p].TurnShadowOff();
		m_textCourseSongNumber[p].SetXY( SONG_NUMBER_X(p), SONG_NUMBER_Y(p,bExtra) );
		m_textCourseSongNumber[p].SetText( "" );
		m_textCourseSongNumber[p].SetDiffuse( GAMESTATE->GetStageColor() );	// light blue
	}

	switch( GAMESTATE->m_PlayMode )
	{
	case PLAY_MODE_ARCADE:
		this->AddChild( &m_textStageNumber );
		break;
	case PLAY_MODE_ONI:
	case PLAY_MODE_ENDLESS:
		for( p=0; p<NUM_PLAYERS; p++ )
			if( GAMESTATE->IsPlayerEnabled(p) )
				this->AddChild( &m_textCourseSongNumber[p] );
		break;
	default:
		ASSERT(0);	// invalid GameMode
	}


	//
	// Add all Actors in bottom frame
	//
	m_sprBottomFrame.Load( THEME->GetPathTo("Graphics",bExtra?"gameplay extra bottom frame":"gameplay bottom frame") );
	m_sprBottomFrame.SetXY( BOTTOM_FRAME_X, BOTTOM_FRAME_Y(bExtra) );
	this->AddChild( &m_sprBottomFrame );

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled(PlayerNumber(p)) )
			continue;

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
		m_pScoreDisplay[p]->SetXY( SCORE_X(p), SCORE_Y(p,bExtra) );
		m_pScoreDisplay[p]->SetZoom( 0.8f );
		m_pScoreDisplay[p]->SetDiffuse( PlayerToColor(p) );
		this->AddChild( m_pScoreDisplay[p] );
		
		m_textPlayerOptions[p].LoadFromFont( THEME->GetPathTo("Fonts","normal") );
		m_textPlayerOptions[p].TurnShadowOff();
		m_textPlayerOptions[p].SetXY( PLAYER_OPTIONS_X(p), PLAYER_OPTIONS_Y(p,bExtra) );
		m_textPlayerOptions[p].SetZoom( 0.5f );
		m_textPlayerOptions[p].SetDiffuse( D3DXCOLOR(1,1,1,1) );
		m_textPlayerOptions[p].SetText( GAMESTATE->m_PlayerOptions[p].GetString() );
		this->AddChild( &m_textPlayerOptions[p] );
	}

	m_textSongOptions.LoadFromFont( THEME->GetPathTo("Fonts","normal") );
	m_textSongOptions.TurnShadowOff();
	m_textSongOptions.SetXY( SONG_OPTIONS_X, SONG_OPTIONS_Y(bExtra) );
	m_textSongOptions.SetZoom( 0.5f );
	m_textSongOptions.SetDiffuse( D3DXCOLOR(1,1,1,1) );
	m_textSongOptions.SetText( GAMESTATE->m_SongOptions.GetString() );
	this->AddChild( &m_textSongOptions );


	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled(PlayerNumber(p)) )
			continue;

		m_DifficultyBanner[p].SetXY( DIFFICULTY_X(p), DIFFICULTY_Y(p,bExtra,bReverse[p]) );
		this->AddChild( &m_DifficultyBanner[p] );
	}


	m_textDebug.LoadFromFont( THEME->GetPathTo("Fonts","normal") );
	m_textDebug.SetXY( DEBUG_X, DEBUG_Y );
	m_textDebug.SetDiffuse( D3DXCOLOR(1,1,1,1) );
	this->AddChild( &m_textDebug );
	
	m_textAutoPlay.LoadFromFont( THEME->GetPathTo("Fonts","normal") );
	m_textAutoPlay.SetXY( AUTOPLAY_X, AUTOPLAY_Y );
	m_textAutoPlay.SetText( "AutoPlay is ON" );
	m_textAutoPlay.SetDiffuse( D3DXCOLOR(1,1,1,1) );
	this->AddChild( &m_textAutoPlay );
	
	
	m_StarWipe.SetClosed();
	this->AddChild( &m_StarWipe );

	m_sprReady.Load( THEME->GetPathTo("Graphics","gameplay ready") );
	m_sprReady.SetXY( CENTER_X, CENTER_Y );
	m_sprReady.SetDiffuse( D3DXCOLOR(1,1,1,0) );
	this->AddChild( &m_sprReady );

	m_sprHereWeGo.Load( THEME->GetPathTo("Graphics","gameplay here we go") );
	m_sprHereWeGo.SetXY( CENTER_X, CENTER_Y );
	m_sprHereWeGo.SetDiffuse( D3DXCOLOR(1,1,1,0) );
	this->AddChild( &m_sprHereWeGo );

	m_sprCleared.Load( THEME->GetPathTo("Graphics","gameplay cleared") );
	m_sprCleared.SetXY( CENTER_X, CENTER_Y );
	m_sprCleared.SetDiffuse( D3DXCOLOR(1,1,1,0) );
	this->AddChild( &m_sprCleared );

	m_sprFailed.Load( THEME->GetPathTo("Graphics","gameplay failed") );
	m_sprFailed.SetXY( CENTER_X, CENTER_Y );
	m_sprFailed.SetDiffuse( D3DXCOLOR(1,1,1,0) );
	this->AddChild( &m_sprFailed );

	if( GAMESTATE->m_bDemonstration )
	{
		m_quadDemonstrationBox.SetDiffuse( D3DXCOLOR(0,0,0,0.7f) );
		m_quadDemonstrationBox.StretchTo( CRect(SCREEN_LEFT, int(CENTER_Y-60), SCREEN_RIGHT, int(CENTER_Y+60)) );
		this->AddChild( &m_quadDemonstrationBox );

		m_sprDemonstration.Load( THEME->GetPathTo("Graphics","gameplay demonstration") );
		m_sprDemonstration.SetXY( CENTER_X, CENTER_Y );
		m_sprDemonstration.SetEffectBlinking();
		this->AddChild( &m_sprDemonstration );

		m_Fade.OpenWipingRight();
	}

	m_Fade.SetOpened();
	this->AddChild( &m_Fade );


	m_textSurviveTime.LoadFromFont( THEME->GetPathTo("Fonts","survive time") );
	m_textSurviveTime.TurnShadowOff();
	m_textSurviveTime.SetXY( SURVIVE_TIME_X, SURVIVE_TIME_Y );
	m_textSurviveTime.SetText( "" );
	m_textSurviveTime.SetDiffuse( D3DXCOLOR(1,1,1,0) );
	this->AddChild( &m_textSurviveTime );


	m_sprToasty.Load( THEME->GetPathTo("Graphics","gameplay toasty") );
	m_sprToasty.SetDiffuse( D3DXCOLOR(1,1,1,0) );
	this->AddChild( &m_sprToasty );
	
	m_soundToasty.Load( THEME->GetPathTo("Sounds","gameplay toasty") );


	if( !GAMESTATE->m_bDemonstration )	// don't load sounds if just playing demonstration
	{
		m_soundFail.Load(				THEME->GetPathTo("Sounds","gameplay failed") );
		m_soundOniDie.Load(				THEME->GetPathTo("Sounds","gameplay oni die") );
		m_announcerReady.Load(			ANNOUNCER->GetPathTo("gameplay ready") );
		if( GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2() )
			m_announcerHereWeGo.Load(	ANNOUNCER->GetPathTo("gameplay here we go extra") );
		else if( GAMESTATE->IsFinalStage() )
			m_announcerHereWeGo.Load(	ANNOUNCER->GetPathTo("gameplay here we go final") );
		else
			m_announcerHereWeGo.Load(	ANNOUNCER->GetPathTo("gameplay here we go normal") );
		m_announcerDanger.Load(			ANNOUNCER->GetPathTo("gameplay comment danger") );
		m_announcerGood.Load(			ANNOUNCER->GetPathTo("gameplay comment good") );
		m_announcerHot.Load(			ANNOUNCER->GetPathTo("gameplay comment hot") );
		m_announcerOni.Load(			ANNOUNCER->GetPathTo("gameplay comment oni") );

		m_announcer100Combo.Load(		ANNOUNCER->GetPathTo("gameplay 100 combo") );
		m_announcer200Combo.Load(		ANNOUNCER->GetPathTo("gameplay 200 combo") );
		m_announcer300Combo.Load(		ANNOUNCER->GetPathTo("gameplay 300 combo") );
		m_announcer400Combo.Load(		ANNOUNCER->GetPathTo("gameplay 400 combo") );
		m_announcer500Combo.Load(		ANNOUNCER->GetPathTo("gameplay 500 combo") );
		m_announcer600Combo.Load(		ANNOUNCER->GetPathTo("gameplay 600 combo") );
		m_announcer700Combo.Load(		ANNOUNCER->GetPathTo("gameplay 700 combo") );
		m_announcer800Combo.Load(		ANNOUNCER->GetPathTo("gameplay 800 combo") );
		m_announcer900Combo.Load(		ANNOUNCER->GetPathTo("gameplay 900 combo") );
		m_announcer1000Combo.Load(		ANNOUNCER->GetPathTo("gameplay 1000 combo") );
		m_announcerComboStopped.Load(	ANNOUNCER->GetPathTo("gameplay combo stopped") );
	}

	m_iRowLastCrossed = -1;

	m_soundAssistTick.Load(		THEME->GetPathTo("Sounds","gameplay assist tick") );

	TweenOnScreen();

	/* XXX: We set m_textPlayerOptions[p] above, so that won't
	 * include options set by the course.  Should it? -glenn */
	LoadNextSong( true );


	if( GAMESTATE->m_bDemonstration )
	{
		m_StarWipe.SetOpened();
		m_DancingState = STATE_DANCING;
		m_soundMusic.Play();
		this->SendScreenMessage( SM_BeginFadingToTitleMenu, DEMONSTRATION_SECONDS );
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
		SAFE_DELETE( m_pLifeMeter[p] );
		SAFE_DELETE( m_pScoreDisplay[p] );
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
			pCourse->GetSongAndNotesForCurrentStyle( apSongs, apNotes, asModifiers, true );

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

			pCourse->GetSongAndNotesForCurrentStyle( apSongs, apNotes, asModifiers, true );

			int iPlaySongIndex = GAMESTATE->m_iSongsIntoCourse;
			iPlaySongIndex %= apSongs.GetSize();

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
		m_sprOniGameOver[p].SetDiffuse( D3DXCOLOR(1,1,1,0) );	// 0 alpha so we don't waste time drawing while not visible

		if( GAMESTATE->m_SongOptions.m_LifeType == SongOptions::LIFE_BATTERY  &&  GAMESTATE->m_fSecondsBeforeFail[p] != -1 )	// already failed
			ShowOniGameOver((PlayerNumber)p);


		m_DifficultyBanner[p].SetFromNotes( PlayerNumber(p), GAMESTATE->m_pCurNotes[p] );


		NoteData originalNoteData;
		GAMESTATE->m_pCurNotes[p]->GetNoteData( &originalNoteData );
		
		const StyleDef* pStyleDef = GAMESTATE->GetCurrentStyleDef();
		NoteData newNoteData;
		pStyleDef->GetTransformedNoteDataForStyle( (PlayerNumber)p, &originalNoteData, &newNoteData );

		m_Player[p].Load( (PlayerNumber)p, &newNoteData, m_pLifeMeter[p], m_pScoreDisplay[p] );
	}

	m_Background.LoadFromSong( GAMESTATE->m_pCurSong );
	m_Background.SetDiffuse( D3DXCOLOR(0.5f,0.5f,0.5f,1) );
	m_Background.BeginTweening( 2 );
	m_Background.SetTweenDiffuse( D3DXCOLOR(1,1,1,1) );

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
			if( m_pLifeMeter[p]->IsHot() )
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

	/* Before the music starts, we have no song position, so set it toto
	 * -1; if we leave it alone we'll get 0, which will cause movies to start
	 * playing before the music starts.
	 *
	 * We really should be setting the beat to negative numbers before the song
	 * starts, leading up to 0, but we need a separate time source to do that.
	 * XXX: do this once we have a new sound infrastructure. */
	if(!m_soundMusic.IsPlaying())
		GAMESTATE->m_fSongBeat = -1;

//	LOG->Trace( "GAMESTATE->m_fMusicSeconds = %f", GAMESTATE->m_fMusicSeconds );
	

	//LOG->Trace( "m_fOffsetInBeats = %f, m_fBeatsPerSecond = %f, m_Music.GetPositionSeconds = %f", m_fOffsetInBeats, m_fBeatsPerSecond, m_Music.GetPositionSeconds() );


	switch( m_DancingState )
	{
	case STATE_DANCING:
		
		//
		// Check for end of song
		//
		if( fSongBeat > GAMESTATE->m_pCurSong->m_fLastBeat+2  &&  !m_soundMusic.IsPlaying() )
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
				switch( GAMESTATE->m_PlayMode )
				{
				case PLAY_MODE_ARCADE:
					if( OneIsHot() )			m_announcerHot.PlayRandom();
					else if( AllAreInDanger() )	m_announcerDanger.PlayRandom();
					else						m_announcerGood.PlayRandom();
					break;
				case PLAY_MODE_ONI:
				case PLAY_MODE_ENDLESS:
					m_announcerOni.PlayRandom();
					break;
				default:
					ASSERT(0);
				}

				m_fTimeLeftBeforeDancingComment = SECONDS_BETWEEN_COMMENTS;	// reset for the next comment
			}

		}
	}



	//
	// Send crossed row messages to Player
	//
	int iRowNow = BeatToNoteRowNotRounded( fSongBeat );
	if( iRowNow >= 0  &&  iRowNow < MAX_TAP_NOTE_ROWS )
	{
		for( int r=m_iRowLastCrossed+1; r<=iRowNow; r++ )  // for each index we crossed since the last update
		{
			for( int p=0; p<NUM_PLAYERS; p++ )
			{
				if( GAMESTATE->IsPlayerEnabled(p) )
					m_Player[p].CrossedRow( r );
			}
		}

		m_iRowLastCrossed = iRowNow;
	}


	// 
	// play assist ticks
	//
	// Sound cards have a latency between when a sample is Play()ed and when the sound
	// will start coming out the speaker.  Compensate for this by boosting
	// fPositionSeconds ahead
	if( GAMESTATE->m_SongOptions.m_AssistType == SongOptions::ASSIST_TICK )
	{
		fPositionSeconds += (SOUND->GetPlayLatency()+g_fTickEarlySecondsCache) * m_soundMusic.GetPlaybackRate();	// HACK:  Play the sound a little bit early to account for the fact that the middle of the tick sounds occurs 0.015 seconds into playing.
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

				bAnyoneHasANote |= m_Player[p].IsThereANoteAtRow( r );
				break;	// this will only play the tick for the first player that is joined
			}
		}

		if( bAnyoneHasANote )
			m_soundAssistTick.Play();


		iRowLastCrossed = iRowNow;
	}

	if( PREFSMAN->m_bAutoPlay  &&  !GAMESTATE->m_bDemonstration )
		m_textAutoPlay.SetDiffuse( D3DXCOLOR(1,1,1,1) );
	else
		m_textAutoPlay.SetDiffuse( D3DXCOLOR(1,1,1,0) );

	Screen::Update( fDeltaTime );
}


void ScreenGameplay::DrawPrimitives()
{
	Screen::DrawPrimitives();
}


void ScreenGameplay::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	//LOG->Trace( "ScreenGameplay::Input()" );
	if(type == IET_RELEASE) return; // don't care

	if( GAMESTATE->m_bDemonstration )
	{
		/* Special case:always allow escape. */
		if( DeviceI.device==DEVICE_KEYBOARD  &&  DeviceI.button==DIK_ESCAPE  &&  !m_Fade.IsClosing() )
		{
			this->SendScreenMessage( SM_BeginFadingToTitleMenu, 0 );
		}
		/* Since escape backs out, we want to allow any other back buttons to
		 * work, too, to avoid confusion. */
		else if( (MenuI.button==MENU_BUTTON_START || MenuI.button==MENU_BUTTON_BACK)
			&&  !m_Fade.IsClosing() )
		{
			m_soundMusic.Stop();
			SOUND->PlayOnceStreamed( THEME->GetPathTo("Sounds","insert coin") );
			::Sleep( 1000 );	// do a little pause, like the arcade does
			this->SendScreenMessage( SM_GoToTitleMenu, 0 );
		}
		return;	// don't fall through below
	}

	// Handle special keys to adjust the offset
	if( DeviceI.device == DEVICE_KEYBOARD )
	{
		switch( DeviceI.button )
		{
//		case DIK_F6:
//			this->SendScreenMessage( SM_BeginToasty, 0 );
//			break;
		case DIK_F7:
			if( GAMESTATE->m_SongOptions.m_AssistType == SongOptions::ASSIST_NONE )
				GAMESTATE->m_SongOptions.m_AssistType = SongOptions::ASSIST_TICK;
			else
				GAMESTATE->m_SongOptions.m_AssistType = SongOptions::ASSIST_NONE;
			m_textDebug.SetText( ssprintf( "Assist tick is %s.", (GAMESTATE->m_SongOptions.m_AssistType==SongOptions::ASSIST_NONE)?"OFF":"ON") );
			m_textDebug.SetDiffuse( D3DXCOLOR(1,1,1,1) );
			m_textDebug.StopTweening();
			m_textDebug.BeginTweening( 3 );		// sleep
			m_textDebug.BeginTweening( 0.5f );	// fade out
			m_textDebug.SetTweenDiffuse( D3DXCOLOR(1,1,1,0) );
			break;
		case DIK_F8:
			PREFSMAN->m_bAutoPlay = !PREFSMAN->m_bAutoPlay;
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
				default:	ASSERT(0);						return;
				}
				if( type == IET_FAST_REPEAT )
					fOffsetDelta *= 10;
				BPMSegment& seg = GAMESTATE->m_pCurSong->GetBPMSegmentAtBeat( GAMESTATE->m_fSongBeat );

				seg.m_fBPM += fOffsetDelta;

				m_textDebug.SetText( ssprintf("Cur BPM = %f", seg.m_fBPM) );
				m_textDebug.SetDiffuse( D3DXCOLOR(1,1,1,1) );
				m_textDebug.StopTweening();
				m_textDebug.BeginTweening( 3 );		// sleep
				m_textDebug.BeginTweening( 0.5f );	// fade out
				m_textDebug.SetTweenDiffuse( D3DXCOLOR(1,1,1,0) );
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
				default:	ASSERT(0);						return;
				}
				if( type == IET_FAST_REPEAT )
					fOffsetDelta *= 10;

				GAMESTATE->m_pCurSong->m_fBeat0OffsetInSeconds += fOffsetDelta;

				m_textDebug.SetText( ssprintf("Offset = %f", GAMESTATE->m_pCurSong->m_fBeat0OffsetInSeconds) );
				m_textDebug.SetDiffuse( D3DXCOLOR(1,1,1,1) );
				m_textDebug.StopTweening();
				m_textDebug.BeginTweening( 3 );		// sleep
				m_textDebug.BeginTweening( 0.5f );	// fade out
				m_textDebug.SetTweenDiffuse( D3DXCOLOR(1,1,1,0) );
			}
			break;
		}
	}

	if( MenuI.IsValid()  &&  
		MenuI.button == MENU_BUTTON_BACK  &&  
		m_DancingState != STATE_OUTRO  &&
		!m_Fade.IsClosing() )
	{
		if( !PREFSMAN->m_bDelayedEscape ||
			(DeviceI.device==DEVICE_KEYBOARD && type==IET_SLOW_REPEAT)  ||
			(DeviceI.device!=DEVICE_KEYBOARD && type==IET_FAST_REPEAT) )
		{
			m_DancingState = STATE_OUTRO;
			SOUND->PlayOnceStreamed( THEME->GetPathTo("Sounds","menu back") );
			/* Hmm.  There are a bunch of subtly different ways we can
			 * tween out: 
			 *   1. Keep rendering the song, and keep it moving.  This might
			 *      cause problems if the cancel and the end of the song overlap.
			 *   2. Stop the song completely, so all song motion under the tween
			 *      ceases.
			 *   3. Stop the song, but keep effects (eg. Drunk) running.
			 *   4. Don't display the song at all.
			 *
			 * We're doing #3.  I'm not sure which is best.
			 *
			 * We have to pause the music, not stop it.  If we stop it,
			 * its position will be 0, and we'll render the *start*
			 * of the song while we tween out, which looks really strange.
			 * -glenn
			 */
			m_soundMusic.Pause();

			this->ClearMessageQueue();
			m_Fade.CloseWipingLeft( SM_SaveChangedBeforeGoingBack );
		}
	}

	//
	// handle a step
	//
	if( m_DancingState == STATE_DANCING  &&  type == IET_FIRST_PRESS  &&  !PREFSMAN->m_bAutoPlay   &&  StyleI.IsValid() )
		if( GAMESTATE->IsPlayerEnabled( StyleI.player ) )
			m_Player[StyleI.player].Step( StyleI.col ); 
}

void SaveChanges()
{
	switch( GAMESTATE->m_PlayMode )
	{
	case PLAY_MODE_ARCADE:
		GAMESTATE->m_pCurSong->SaveToSongFile();
		break;
	case PLAY_MODE_ONI:
	case PLAY_MODE_ENDLESS:
		{
			for( int i=0; i<GAMESTATE->m_pCurCourse->m_iStages; i++ )
			{
				Song* pSong = GAMESTATE->m_pCurCourse->m_apSongs[i];
				pSong->SaveToSongFile();
			}
		}
		break;
	default:
		ASSERT(0);
	}
}

void DontSaveChanges()
{
	SMLoader ld;
	switch( GAMESTATE->m_PlayMode )
	{
	case PLAY_MODE_ARCADE:
		ld.LoadFromSMFile(GAMESTATE->m_pCurSong->GetCacheFilePath(),
			*GAMESTATE->m_pCurSong);
		break;
	case PLAY_MODE_ONI:
	case PLAY_MODE_ENDLESS:
		{
			for( int i=0; i<GAMESTATE->m_pCurCourse->m_iStages; i++ )
			{
				Song* pSong = GAMESTATE->m_pCurCourse->m_apSongs[i];
				ld.LoadFromSMFile( GAMESTATE->m_pCurSong->GetCacheFilePath(), *pSong );
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
			"Choosing NO will discard your changes.",
			GAMESTATE->m_pCurSong->GetFullTitle() );
		break;
	case PLAY_MODE_ONI:
	case PLAY_MODE_ENDLESS:
		sMessage = ssprintf( 
			"You have changed the offset or BPM of\n"
			"one or more songs in this course.\n"
			"Would you like to save these changes back\n"
			"to the song file(s)?\n"
			"Choosing NO will discard your changes." );
		break;
	default:
		ASSERT(0);
	}

	SCREENMAN->Prompt( SM_SendWhenDone, sMessage, true, true, SaveChanges, DontSaveChanges );
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
					m_StarWipe.CloseWipingRight( SM_None );
					this->SendScreenMessage( SM_ShowCleared, 1 );
					SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo("gameplay cleared") );
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

	case SM_BeginToasty:
		this->SendScreenMessage( SM_PlayToastySound, 0.3f );

		// set off screen
		m_sprToasty.StopTweening();
		m_sprToasty.SetDiffuse( D3DXCOLOR(1,1,1,1) );
		m_sprToasty.SetX( SCREEN_RIGHT+m_sprToasty.GetUnzoomedWidth()/2 );
		m_sprToasty.SetY( SCREEN_BOTTOM-m_sprToasty.GetUnzoomedHeight()/2 );
		m_sprToasty.BeginTweening( 0.2f, Actor::TWEEN_BIAS_BEGIN ); // slide on
		m_sprToasty.SetTweenX( SCREEN_RIGHT-m_sprToasty.GetUnzoomedWidth()/2 );
		m_sprToasty.BeginTweening( 0.6f );	// sleep
		m_sprToasty.BeginTweening( 0.3f, Actor::TWEEN_BIAS_END );	// slide off
		m_sprToasty.SetTweenX( SCREEN_RIGHT+m_sprToasty.GetUnzoomedWidth()/2 );
		m_sprToasty.BeginTweening( 0.001f );	// fade out
		m_sprToasty.SetDiffuse( D3DXCOLOR(1,1,1,0) );
		break;

	case SM_PlayToastySound:
		m_soundToasty.Play();
		break;

	case SM_100Combo:
		if( m_fTimeLeftBeforeDancingComment < 12 )
		{
			m_announcer100Combo.PlayRandom();
			m_fTimeLeftBeforeDancingComment = SECONDS_BETWEEN_COMMENTS;
		}
		break;
	case SM_200Combo:
		if( m_fTimeLeftBeforeDancingComment < 12 )
		{
			m_announcer200Combo.PlayRandom();
			m_fTimeLeftBeforeDancingComment = SECONDS_BETWEEN_COMMENTS;
		}
		break;
	case SM_300Combo:
		if( m_fTimeLeftBeforeDancingComment < 12 )
		{
			m_announcer300Combo.PlayRandom();
			m_fTimeLeftBeforeDancingComment = SECONDS_BETWEEN_COMMENTS;
		}
		break;
	case SM_400Combo:
		if( m_fTimeLeftBeforeDancingComment < 12 )
		{
			m_announcer400Combo.PlayRandom();
			m_fTimeLeftBeforeDancingComment = SECONDS_BETWEEN_COMMENTS;
		}
		break;
	case SM_500Combo:
		if( m_fTimeLeftBeforeDancingComment < 12 )
		{
			m_announcer500Combo.PlayRandom();
			m_fTimeLeftBeforeDancingComment = SECONDS_BETWEEN_COMMENTS;
		}
		break;
	case SM_600Combo:
		if( m_fTimeLeftBeforeDancingComment < 12 )
		{
			m_announcer600Combo.PlayRandom();
			m_fTimeLeftBeforeDancingComment = SECONDS_BETWEEN_COMMENTS;
		}
		break;
	case SM_700Combo:
		if( m_fTimeLeftBeforeDancingComment < 12 )
		{
			m_announcer700Combo.PlayRandom();
			m_fTimeLeftBeforeDancingComment = SECONDS_BETWEEN_COMMENTS;
		}
		break;
	case SM_800Combo:
		if( m_fTimeLeftBeforeDancingComment < 12 )
		{
			m_announcer800Combo.PlayRandom();
			m_fTimeLeftBeforeDancingComment = SECONDS_BETWEEN_COMMENTS;
		}
		break;
	case SM_900Combo:
		if( m_fTimeLeftBeforeDancingComment < 12 )
		{
			m_announcer900Combo.PlayRandom();
			m_fTimeLeftBeforeDancingComment = SECONDS_BETWEEN_COMMENTS;
		}
		break;
	case SM_1000Combo:
		if( m_fTimeLeftBeforeDancingComment < 12 )
		{
			m_announcer1000Combo.PlayRandom();
			m_fTimeLeftBeforeDancingComment = SECONDS_BETWEEN_COMMENTS;
		}
		break;
	case SM_ComboStopped:
		if( m_fTimeLeftBeforeDancingComment < 12 )
		{
			m_announcerComboStopped.PlayRandom();
			m_fTimeLeftBeforeDancingComment = SECONDS_BETWEEN_COMMENTS;
		}
		break;


	// received while STATE_OUTRO
	case SM_ShowCleared:
		m_sprCleared.BeginTweening(1.0f);
		m_sprCleared.SetTweenDiffuse( D3DXCOLOR(1,1,1,1) );
		SCREENMAN->SendMessageToTopScreen( SM_HideCleared, 1.5 );
		break;
	case SM_HideCleared:
		m_sprCleared.BeginTweening(0.7f);
		m_sprCleared.SetTweenDiffuse( D3DXCOLOR(1,1,1,0) );
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
			SCREENMAN->SetNewScreen( "ScreenSelectMusic" );
			break;
		case PLAY_MODE_ONI:
		case PLAY_MODE_ENDLESS:
			SCREENMAN->SetNewScreen( "ScreenSelectCourse" );
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
		SCREENMAN->SetNewScreen( "ScreenEvaluation" );
		break;


	case SM_BeginFailed:
		m_DancingState = STATE_OUTRO;
		m_soundMusic.Pause();
		m_StarWipe.SetTransitionTime( 1.5f );
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
		m_Background.SetTweenDiffuse( D3DXCOLOR(1,1,1,0) );

		m_sprFailed.SetZoom( 4 );
		m_sprFailed.BeginBlurredTweening( 0.8f, TWEEN_BIAS_END );
		m_sprFailed.SetTweenZoom( 0.5f );			// zoom out
		m_sprFailed.SetTweenDiffuse( D3DXCOLOR(1,1,1,0.7f) );	// and fade in
		m_sprFailed.BeginTweening( 0.3f );
		m_sprFailed.SetTweenZoom( 1.1f );			// bounce
		m_sprFailed.SetTweenDiffuse( D3DXCOLOR(1,1,1,0.7f) );	// and fade in
		m_sprFailed.BeginTweening( 0.2f );
		m_sprFailed.SetTweenZoom( 1.0f );			// come to rest
		m_sprFailed.SetTweenDiffuse( D3DXCOLOR(1,1,1,0.7f) );	// and fade in

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
			m_textSurviveTime.BeginTweening( 0.3f );	// fade in
			m_textSurviveTime.SetTweenDiffuse( D3DXCOLOR(1,1,1,1) );
		}

		SCREENMAN->SendMessageToTopScreen( SM_PlayFailComment, 1.0f );
		SCREENMAN->SendMessageToTopScreen( SM_HideFailed, 2.0f );
		break;
	case SM_PlayFailComment:
		SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo("gameplay failed") );
		break;
	case SM_HideFailed:
		m_sprFailed.StopTweening();
		m_sprFailed.BeginTweening(1.0f);
		m_sprFailed.SetTweenDiffuse( D3DXCOLOR(1,1,1,0) );

		m_textSurviveTime.BeginTweening( 0.5f );	// sleep
		m_textSurviveTime.BeginTweening( 0.5f );	// fade out
		m_textSurviveTime.SetTweenDiffuse( D3DXCOLOR(1,1,1,0) );

		SCREENMAN->SendMessageToTopScreen( SM_GoToScreenAfterFail, 1.5f );
		break;
	case SM_GoToScreenAfterFail:
		if( m_bChangedOffsetOrBPM )
		{
			m_bChangedOffsetOrBPM = false;
			ShowSavePrompt( SM_GoToScreenAfterFail );
			break;
		}
		if( GAMESTATE->m_PlayMode == PLAY_MODE_ONI  ||  GAMESTATE->m_PlayMode == PLAY_MODE_ENDLESS )
			SCREENMAN->SetNewScreen( "ScreenEvaluation" );
		else if( PREFSMAN->m_bEventMode )
			this->SendScreenMessage( SM_GoToScreenAfterBack, 0 );
		else if( GAMESTATE->IsExtraStage()  ||  GAMESTATE->IsExtraStage2() )
			SCREENMAN->SetNewScreen( "ScreenFinalEvaluation" );
		else
			SCREENMAN->SetNewScreen( "ScreenGameOver" );
		break;
	case SM_GoToTitleMenu:
		SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
		break;
	}
}


void ScreenGameplay::TweenOnScreen()
{
	int i, p;

	CArray<Actor*,Actor*> apActorsInTopFrame;
	apActorsInTopFrame.Add(	&m_sprTopFrame );
	for( p=0; p<NUM_PLAYERS; p++ )
		apActorsInTopFrame.Add(	m_pLifeMeter[p] );
	apActorsInTopFrame.Add(	&m_textStageNumber );
	for( p=0; p<NUM_PLAYERS; p++ )
		apActorsInTopFrame.Add(	&m_textCourseSongNumber[p] );
	for( i=0; i<apActorsInTopFrame.GetSize(); i++ )
	{
		float fOriginalY = apActorsInTopFrame[i]->GetY();
		apActorsInTopFrame[i]->SetY( fOriginalY-100 );
		if( !GAMESTATE->m_bDemonstration )
			apActorsInTopFrame[i]->BeginTweening( 0.5f );	// sleep
		apActorsInTopFrame[i]->BeginTweening( 1 );
		apActorsInTopFrame[i]->SetTweenY( fOriginalY );
	}


	CArray<Actor*,Actor*> apActorsInBottomFrame;
	apActorsInBottomFrame.Add( &m_sprBottomFrame );
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled(p) )
			continue;
		apActorsInBottomFrame.Add( m_pScoreDisplay[p] );
		apActorsInBottomFrame.Add( &m_textPlayerOptions[p] );
	}
	apActorsInBottomFrame.Add( &m_textSongOptions );
	for( i=0; i<apActorsInBottomFrame.GetSize(); i++ )
	{
		float fOriginalY = apActorsInBottomFrame[i]->GetY();
		apActorsInBottomFrame[i]->SetY( fOriginalY+100 );
		if( !GAMESTATE->m_bDemonstration )
			apActorsInBottomFrame[i]->BeginTweening( 0.5f );	// sleep
		apActorsInBottomFrame[i]->BeginTweening( 1 );
		apActorsInBottomFrame[i]->SetTweenY( fOriginalY );
	}

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		float fOriginalX = m_DifficultyBanner[p].GetX();
		m_DifficultyBanner[p].SetX( (p==PLAYER_1) ? fOriginalX-200 : fOriginalX+200 );
		if( !GAMESTATE->m_bDemonstration )
			m_DifficultyBanner[p].BeginTweening( 0.5f );	// sleep
		m_DifficultyBanner[p].BeginTweening( 1 );
		m_DifficultyBanner[p].SetTweenX( fOriginalX );
	}
}

void ScreenGameplay::TweenOffScreen()
{

}

void ScreenGameplay::ShowOniGameOver( PlayerNumber pn )
{
	m_sprOniGameOver[pn].SetDiffuse( D3DXCOLOR(1,1,1,1) );
	m_sprOniGameOver[pn].BeginTweening( 0.5f, Actor::TWEEN_BOUNCE_END );
	m_sprOniGameOver[pn].SetTweenY( CENTER_Y );
	m_sprOniGameOver[pn].SetEffectBobbing( D3DXVECTOR3(0,6,0), 4 );
}

#include "global.h"
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
#include "ThemeManager.h"

#include "RageTimer.h"

//
// Defines
//
#define SONGSEL_SCREEN					THEME->GetMetric("ScreenGameplay","SongSelectScreen")
#define MAXCOMBO_X						THEME->GetMetricF("ScreenGameplay","MAXCOMBOX")		// Please follow the capitalization conventions used everywhere else.  This is case sensitive.  -Chris
#define MAXCOMBO_Y						THEME->GetMetricF("ScreenGameplay","MAXCOMBOY")
#define MAXCOMBO_ZOOM					THEME->GetMetricF("ScreenGameplay","MAXCOMBOZoom")
#define BPM_X							THEME->GetMetricF("ScreenGameplay","BPMX")
#define BPM_Y							THEME->GetMetricF("ScreenGameplay","BPMY")
#define BPM_ZOOM						THEME->GetMetricF("ScreenGameplay","BPMZoom")
#define STAGENAME_X						THEME->GetMetricF("ScreenGameplay","StagenameX")
#define STAGENAME_Y						THEME->GetMetricF("ScreenGameplay","StagenameY")
#define STAGENAME_ZOOM					THEME->GetMetricF("ScreenGameplay","StagenameZoom")
#define LIFE_FRAME_X					THEME->GetMetricF("ScreenGameplay","LifeFrameX")
#define LIFE_FRAME_Y( e )				THEME->GetMetricF("ScreenGameplay",ssprintf("LifeFrame%sY",e?"Extra":""))
#define SCORE_FRAME_X					THEME->GetMetricF("ScreenGameplay","ScoreFrameX")
#define SCORE_FRAME_Y( e )				THEME->GetMetricF("ScreenGameplay",ssprintf("ScoreFrame%sY",e?"Extra":""))
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
#define SCORE_ZOOM						THEME->GetMetricF("ScreenGameplay","ScoreZoom")
#define PLAYER_OPTIONS_X( p )			THEME->GetMetricF("ScreenGameplay",ssprintf("PlayerOptionsP%dX",p+1))
#define PLAYER_OPTIONS_Y( p, e )		THEME->GetMetricF("ScreenGameplay",ssprintf("PlayerOptionsP%d%sY",p+1,e?"Extra":""))
#define SONG_OPTIONS_X					THEME->GetMetricF("ScreenGameplay","SongOptionsX")
#define SONG_OPTIONS_Y( e )				THEME->GetMetricF("ScreenGameplay",ssprintf("SongOptions%sY",e?"Extra":""))
#define DIFFICULTY_X( p )				THEME->GetMetricF("ScreenGameplay",ssprintf("DifficultyP%dX",p+1))
#define DIFFICULTY_Y( p, e, r )			THEME->GetMetricF("ScreenGameplay",ssprintf("DifficultyP%d%s%sY",p+1,e?"Extra":"",r?"Reverse":""))
#define DEBUG_X							THEME->GetMetricF("ScreenGameplay","DebugX")
#define DEBUG_Y							THEME->GetMetricF("ScreenGameplay","DebugY")
#define STATUS_ICONS_X					THEME->GetMetricF("ScreenGameplay","StatusIconsX")
#define STATUS_ICONS_Y					THEME->GetMetricF("ScreenGameplay","StatusIconsY")
#define SURVIVE_TIME_X					THEME->GetMetricF("ScreenGameplay","SurviveTimeX")
#define SURVIVE_TIME_Y					THEME->GetMetricF("ScreenGameplay","SurviveTimeY")
#define SECONDS_BETWEEN_COMMENTS		THEME->GetMetricF("ScreenGameplay","SecondsBetweenComments")
#define TICK_EARLY_SECONDS				THEME->GetMetricF("ScreenGameplay","TickEarlySeconds")

static float g_fTickEarlySecondsCache = 0;		// reading directly out of theme metrics is slow


// received while STATE_DANCING
const ScreenMessage	SM_NotesEnded			= ScreenMessage(SM_User+101);
const ScreenMessage	SM_BeginLoadingNextSong	= ScreenMessage(SM_User+102);
const ScreenMessage SM_PlayToastySound		= ScreenMessage(SM_User+105);

// received while STATE_OUTRO
const ScreenMessage	SM_ShowCleared			= ScreenMessage(SM_User+111);
const ScreenMessage	SM_ShowTryExtraStage	= ScreenMessage(SM_User+112);
const ScreenMessage	SM_SaveChangedBeforeGoingBack	= ScreenMessage(SM_User+113);
const ScreenMessage	SM_GoToScreenAfterBack	= ScreenMessage(SM_User+114);
const ScreenMessage	SM_GoToStateAfterCleared= ScreenMessage(SM_User+115);

const ScreenMessage	SM_BeginFailed			= ScreenMessage(SM_User+121);
const ScreenMessage	SM_ShowFailed			= ScreenMessage(SM_User+122);
const ScreenMessage	SM_PlayFailComment		= ScreenMessage(SM_User+123);
const ScreenMessage	SM_GoToScreenAfterFail	= ScreenMessage(SM_User+125);

// received while STATE_INTRO
const ScreenMessage	SM_StartHereWeGo		= ScreenMessage(SM_User+130);
const ScreenMessage	SM_StopHereWeGo			= ScreenMessage(SM_User+131);


ScreenGameplay::ScreenGameplay( bool bDemonstration )
{
	LOG->Trace( "ScreenGameplay::ScreenGameplay()" );

	if( GAMESTATE->m_pCurSong == NULL && GAMESTATE->m_pCurCourse == NULL )
		return;	// ScreenDemonstration will move us to the next scren.  We just need to survive for one update without crashing.

	{
		for( int p=0; p<NUM_PLAYERS; p++ )
		{
			m_pLifeMeter[p] = NULL;
			m_pScoreDisplay[p] = NULL;
		}
	}

	g_fTickEarlySecondsCache = TICK_EARLY_SECONDS;


	GAMESTATE->m_CurStageStats = StageStats();	// clear values

	// Fill in m_CurStageStats
	NoteData notedata;
	switch( GAMESTATE->m_PlayMode )
	{
	case PLAY_MODE_ARCADE:
		{
			GAMESTATE->m_CurStageStats.pSong = GAMESTATE->m_pCurSong;
			for( int p=0; p<NUM_PLAYERS; p++ )
			{
				if( !GAMESTATE->IsPlayerEnabled(p) )
					continue;	// skip
				GAMESTATE->m_CurStageStats.iMeter[p] = GAMESTATE->m_pCurNotes[p]->GetMeter();
				GAMESTATE->m_pCurNotes[p]->GetNoteData( &notedata );
				GAMESTATE->m_CurStageStats.iPossibleDancePoints[p] = notedata.GetPossibleDancePoints();
			}
		}
		break;
	case PLAY_MODE_NONSTOP:
	case PLAY_MODE_ONI:
	case PLAY_MODE_ENDLESS:
		{
			Course* pCourse = GAMESTATE->m_pCurCourse;
			pCourse->GetStageInfo( m_apCourseSongs, m_apCourseNotes, m_asCourseModifiers, GAMESTATE->GetCurrentStyleDef()->m_NotesType, false );

			int iTotalMeter = 0;
			int iTotalPossibleDancePoints = 0;
			for( unsigned i=0; i<m_apCourseNotes.size(); i++ )
			{
				iTotalMeter += m_apCourseNotes[i]->GetMeter();
				m_apCourseNotes[i]->GetNoteData( &notedata );
				iTotalPossibleDancePoints += notedata.GetPossibleDancePoints();
			}

			GAMESTATE->m_CurStageStats.pSong = NULL;
			for( int p=0; p<NUM_PLAYERS; p++ )
			{
				if( !GAMESTATE->IsPlayerEnabled(p) )
					continue;	// skip
				GAMESTATE->m_CurStageStats.iMeter[p] = iTotalMeter;
				GAMESTATE->m_CurStageStats.iPossibleDancePoints[p] = iTotalPossibleDancePoints;
			}
		}
		break;
	default:
		ASSERT(0);
	}



	const bool bExtra = GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2();
	const bool bReverse[NUM_PLAYERS] = { 
		GAMESTATE->m_PlayerOptions[0].m_bReverseScroll,
		GAMESTATE->m_PlayerOptions[1].m_bReverseScroll
	};



	m_bChangedOffsetOrBPM = GAMESTATE->m_SongOptions.m_bAutoSync;


	m_DancingState = STATE_INTRO;
	m_fTimeLeftBeforeDancingComment = SECONDS_BETWEEN_COMMENTS;


	m_Background.SetDiffuse( RageColor(0.4f,0.4f,0.4f,1) );
	this->AddChild( &m_Background );

	int p;
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
		m_sprOniGameOver[p].SetDiffuse( RageColor(1,1,1,0) );	// 0 alpha so we don't waste time drawing while not visible
		this->AddChild( &m_sprOniGameOver[p] );
	}


	m_OniFade.SetOpened();
	this->AddChild( &m_OniFade );


	m_sprMiddleFrame.Load( THEME->GetPathTo("Graphics","Gameplay Middle Frame") );
	m_sprMiddleFrame.SetXY( MIDDLE_FRAME_X, MIDDLE_FRAME_Y );


	// LifeFrame goes below LifeMeter
	CString sLifeFrameName;
	if( bExtra )
		sLifeFrameName = "gameplay extra life frame";
	else if( GAMESTATE->m_SongOptions.m_LifeType == SongOptions::LIFE_BATTERY )
		sLifeFrameName = "gameplay oni life frame";
	else 
		sLifeFrameName = "gameplay life frame";
	m_sprLifeFrame.Load( THEME->GetPathTo("Graphics",sLifeFrameName) );
	m_sprLifeFrame.SetXY( LIFE_FRAME_X, LIFE_FRAME_Y(bExtra) );
	this->AddChild( &m_sprLifeFrame );

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
	case PLAY_MODE_NONSTOP:
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
	// Add all Actors in score frame
	//
	CString sScoreFrameName;
	if( bExtra )
		sScoreFrameName = "gameplay extra score frame";
	else if( GAMESTATE->m_SongOptions.m_LifeType == SongOptions::LIFE_BATTERY )
		sScoreFrameName = "gameplay oni score frame";
	else 
		sScoreFrameName = "gameplay score frame";
	m_sprScoreFrame.Load( THEME->GetPathTo("Graphics",sScoreFrameName) );
	m_sprScoreFrame.SetXY( SCORE_FRAME_X, SCORE_FRAME_Y(bExtra) );
	this->AddChild( &m_sprScoreFrame );

	m_StageName.LoadFromFont( THEME->GetPathTo("Fonts","musicwheel text banner") );
	m_StageName.TurnShadowOff();
	m_StageName.SetXY( STAGENAME_X, STAGENAME_Y );
	m_StageName.SetZoom( STAGENAME_ZOOM );
	this->AddChild( &m_StageName );

	m_MaxCombo.LoadFromNumbers( THEME->GetPathTo("Numbers","gameplay score numbers") );
	m_MaxCombo.SetXY( MAXCOMBO_X, MAXCOMBO_Y );
	m_MaxCombo.SetZoom( MAXCOMBO_ZOOM );
	m_MaxCombo.SetText( ssprintf("%d", m_Player[GAMESTATE->m_MasterPlayerNumber].GetPlayersMaxCombo()) ); /* MAKE THIS WORK FOR BOTH PLAYERS! */
	this->AddChild( &m_MaxCombo );

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled(PlayerNumber(p)) )
			continue;

		switch( GAMESTATE->m_PlayMode )
		{
		case PLAY_MODE_ARCADE:
			m_pScoreDisplay[p] = new ScoreDisplayNormal;
			break;
		case PLAY_MODE_NONSTOP:
		case PLAY_MODE_ONI:
		case PLAY_MODE_ENDLESS:
			m_pScoreDisplay[p] = new ScoreDisplayOni;
			break;
		default:
			ASSERT(0);
		}

		m_pScoreDisplay[p]->Init( (PlayerNumber)p );
		m_pScoreDisplay[p]->SetXY( SCORE_X(p), SCORE_Y(p,bExtra) );
		m_pScoreDisplay[p]->SetZoom( SCORE_ZOOM );
		m_pScoreDisplay[p]->SetDiffuse( PlayerToColor(p) );
		this->AddChild( m_pScoreDisplay[p] );
		
		m_textPlayerOptions[p].LoadFromFont( THEME->GetPathTo("Fonts","normal") );
		m_textPlayerOptions[p].TurnShadowOff();
		m_textPlayerOptions[p].SetXY( PLAYER_OPTIONS_X(p), PLAYER_OPTIONS_Y(p,bExtra) );
		m_textPlayerOptions[p].SetZoom( 0.5f );
		m_textPlayerOptions[p].SetDiffuse( RageColor(1,1,1,1) );
		this->AddChild( &m_textPlayerOptions[p] );
	}

	m_textSongOptions.LoadFromFont( THEME->GetPathTo("Fonts","normal") );
	m_textSongOptions.TurnShadowOff();
	m_textSongOptions.SetXY( SONG_OPTIONS_X, SONG_OPTIONS_Y(bExtra) );
	m_textSongOptions.SetZoom( 0.5f );
	m_textSongOptions.SetDiffuse( RageColor(1,1,1,1) );
	m_textSongOptions.SetText( GAMESTATE->m_SongOptions.GetString() );
	this->AddChild( &m_textSongOptions );


	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled(PlayerNumber(p)) )
			continue;

		m_DifficultyIcon[p].Load( THEME->GetPathTo("graphics","gameplay difficulty icons 2x5") );
		m_DifficultyIcon[p].SetXY( DIFFICULTY_X(p), DIFFICULTY_Y(p,bExtra,bReverse[p]) );
		this->AddChild( &m_DifficultyIcon[p] );
	}


	m_textDebug.LoadFromFont( THEME->GetPathTo("Fonts","normal") );
	m_textDebug.SetXY( DEBUG_X, DEBUG_Y );
	m_textDebug.SetDiffuse( RageColor(1,1,1,1) );
	this->AddChild( &m_textDebug );
	

	for( int s=0; s<NUM_STATUS_ICONS; s++ )
	{
		m_sprStatusIcons[s].Load( THEME->GetPathTo("Graphics",ssprintf("gameplay status icons 1x%d",NUM_STATUS_ICONS)) );
		m_sprStatusIcons[s].StopAnimating();
		m_sprStatusIcons[s].SetState(s);
		this->AddChild( &m_sprStatusIcons[s] );
	}	

	PositionStatusIcons();	// position them
	
	
	m_StarWipe.SetClosed();
	this->AddChild( &m_StarWipe );

	m_sprReady.Load( THEME->GetPathTo("Graphics","gameplay ready") );
	m_sprReady.SetXY( CENTER_X, CENTER_Y );
	m_sprReady.SetDiffuse( RageColor(1,1,1,0) );
	this->AddChild( &m_sprReady );

	m_sprHereWeGo.Load( THEME->GetPathTo("Graphics","gameplay here we go") );
	m_sprHereWeGo.SetXY( CENTER_X, CENTER_Y );
	m_sprHereWeGo.SetDiffuse( RageColor(1,1,1,0) );
	this->AddChild( &m_sprHereWeGo );

	m_sprCleared.Load( THEME->GetPathTo("Graphics","gameplay cleared") );
	m_sprCleared.SetXY( CENTER_X, CENTER_Y );
	m_sprCleared.SetDiffuse( RageColor(1,1,1,0) );
	this->AddChild( &m_sprCleared );

	m_sprFailed.Load( THEME->GetPathTo("Graphics","gameplay failed") );
	m_sprFailed.SetXY( CENTER_X, CENTER_Y );
	m_sprFailed.SetDiffuse( RageColor(1,1,1,0) );
	this->AddChild( &m_sprFailed );

	if( GAMESTATE->IsFinalStage() )
		m_sprTryExtraStage.Load( THEME->GetPathTo("Graphics","gameplay try extra stage1") );
	else if( GAMESTATE->IsExtraStage() )
		m_sprTryExtraStage.Load( THEME->GetPathTo("Graphics","gameplay try extra stage2") );
	m_sprTryExtraStage.SetXY( CENTER_X, CENTER_Y );
	m_sprTryExtraStage.SetDiffuse( RageColor(1,1,1,0) );
	this->AddChild( &m_sprTryExtraStage );

	m_Fade.SetOpened();
	this->AddChild( &m_Fade );

	m_textSurviveTime.LoadFromFont( THEME->GetPathTo("Fonts","survive time") );
	m_textSurviveTime.TurnShadowOff();
	m_textSurviveTime.SetXY( SURVIVE_TIME_X, SURVIVE_TIME_Y );
	m_textSurviveTime.SetText( "" );
	m_textSurviveTime.SetDiffuse( RageColor(1,1,1,0) );
	this->AddChild( &m_textSurviveTime );


	m_sprToasty.Load( THEME->GetPathTo("Graphics","gameplay toasty") );
	m_sprToasty.SetDiffuse( RageColor(1,1,1,0) );
	this->AddChild( &m_sprToasty );


	m_BPMDisplay.SetXY( BPM_X, BPM_Y );
	m_BPMDisplay.SetZoom( BPM_ZOOM );
	this->AddChild( &m_BPMDisplay );

	TweenOnScreen();

	for( int i=0; i<30; i++ )
		this->SendScreenMessage( ScreenMessage(SM_User+i), i/2.0f );	// Send messages to we can get the introduction rolling
	LoadNextSong();
	m_soundToasty.Load( THEME->GetPathTo("Sounds","gameplay toasty") );


	if( !bDemonstration )	// don't load sounds if just playing demonstration
	{
		m_soundFail.Load(				THEME->GetPathTo("Sounds","gameplay failed") );
		m_soundTryExtraStage.Load(		THEME->GetPathTo("Sounds","gameplay try extra stage") );
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
	else
	{
		StartPlayingSong( 0, 0 );
	}


	m_iRowLastCrossed = -1;

	m_soundAssistTick.Load(		THEME->GetPathTo("Sounds","gameplay assist tick") );
}

ScreenGameplay::~ScreenGameplay()
{
	LOG->Trace( "ScreenGameplay::~ScreenGameplay()" );
	
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		SAFE_DELETE( m_pLifeMeter[p] );
		SAFE_DELETE( m_pScoreDisplay[p] );
	}

	m_soundMusic.StopPlaying();
}

bool ScreenGameplay::IsLastSong()
{
	switch( GAMESTATE->m_PlayMode )
	{
	case PLAY_MODE_ARCADE:
		return true;
	case PLAY_MODE_NONSTOP:
	case PLAY_MODE_ONI:
	case PLAY_MODE_ENDLESS:
		{
			Course* pCourse = GAMESTATE->m_pCurCourse;
			if( pCourse->m_bRepeat )
				return false;
			else
                return GAMESTATE->GetCourseSongIndex() == (int)m_apCourseSongs.size();
		}
		break;
	default:
		ASSERT(0);
		return true;
	}
}

void ScreenGameplay::LoadNextSong()
{
	GAMESTATE->ResetMusicStatistics();

	switch( GAMESTATE->m_PlayMode )
	{
	case PLAY_MODE_ARCADE:
		break;
	case PLAY_MODE_NONSTOP:
	case PLAY_MODE_ONI:
	case PLAY_MODE_ENDLESS:
		{
			int p;
			for( p=0; p<NUM_PLAYERS; p++ )
			{
				m_textCourseSongNumber[p].SetText( ssprintf("%d", GAMESTATE->m_CurStageStats.iSongsPassed[p]+1) );
				// If it's the first song, record the options the player selected for later.
				if( GAMESTATE->m_CurStageStats.iSongsPassed[p] == 0 )
					GAMESTATE->m_SelectedOptions[p] = GAMESTATE->m_PlayerOptions[p];
			}
			int iPlaySongIndex = GAMESTATE->GetCourseSongIndex();
			iPlaySongIndex %= m_apCourseSongs.size();
			GAMESTATE->m_pCurSong = m_apCourseSongs[iPlaySongIndex];

			for( p=0; p<NUM_PLAYERS; p++ )
			{
				GAMESTATE->m_pCurNotes[p] = m_apCourseNotes[iPlaySongIndex];
				// Restore the player's originally selected options.
				GAMESTATE->m_PlayerOptions[p] = GAMESTATE->m_SelectedOptions[p];
				// Put courses options into effect.
				GAMESTATE->m_PlayerOptions[p].FromString( m_asCourseModifiers[iPlaySongIndex] );
				GAMESTATE->m_SongOptions.Init();
				GAMESTATE->m_SongOptions.FromString( m_asCourseModifiers[iPlaySongIndex] );
			}
		}
		break;
	default:
		ASSERT(0);
		break;
	}

	m_textStageNumber.SetText( GAMESTATE->GetStageText() );

	m_textSongOptions.SetText( GAMESTATE->m_SongOptions.GetString() );

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled(p) )
			continue;

		m_textPlayerOptions[p].SetText( GAMESTATE->m_PlayerOptions[p].GetString() );

		// reset oni game over graphic
		m_sprOniGameOver[p].SetY( SCREEN_TOP - m_sprOniGameOver[p].GetZoomedHeight()/2 );
		m_sprOniGameOver[p].SetDiffuse( RageColor(1,1,1,0) );	// 0 alpha so we don't waste time drawing while not visible

		if( GAMESTATE->m_SongOptions.m_LifeType==SongOptions::LIFE_BATTERY && GAMESTATE->m_CurStageStats.bFailed[p] )	// already failed
			ShowOniGameOver((PlayerNumber)p);


		m_DifficultyIcon[p].SetFromNotes( PlayerNumber(p), GAMESTATE->m_pCurNotes[p] );


		NoteData pOriginalNoteData;
		GAMESTATE->m_pCurNotes[p]->GetNoteData( &pOriginalNoteData );
		
		const StyleDef* pStyleDef = GAMESTATE->GetCurrentStyleDef();
		NoteData pNewNoteData;
		pStyleDef->GetTransformedNoteDataForStyle( (PlayerNumber)p, &pOriginalNoteData, &pNewNoteData );

		m_Player[p].Load( (PlayerNumber)p, &pNewNoteData, m_pLifeMeter[p], m_pScoreDisplay[p] );
	}

	/* Set up song-specific graphics. */
	m_Background.LoadFromSong( GAMESTATE->m_pCurSong );
	m_Background.SetDiffuse( RageColor(0.5f,0.5f,0.5f,1) );
	m_Background.BeginTweening( 2 );
	m_Background.SetTweenDiffuse( RageColor(1,1,1,1) );

	m_StageName.SetText( GAMESTATE->m_pCurSong->m_sMainTitle );

	/* XXX: set it to the current BPM, not the range */
	float fMinBPM, fMaxBPM;
	GAMESTATE->m_pCurSong->GetMinMaxBPM( fMinBPM, fMaxBPM );
	m_BPMDisplay.SetBPMRange( fMinBPM, fMaxBPM );

	m_soundMusic.Load( GAMESTATE->m_pCurSong->GetMusicPath() );
}

float ScreenGameplay::StartPlayingSong(float MinTimeToNotes, float MinTimeToMusic)
{
	ASSERT(MinTimeToNotes >= 0);
	ASSERT(MinTimeToMusic >= 0);

	/* XXX: We want the first beat *in use*, so we don't delay needlessly. */
	const float fFirstBeat = GAMESTATE->m_pCurSong->m_fFirstBeat;
	const float fFirstSecond = GAMESTATE->m_pCurSong->GetElapsedTimeFromBeat( fFirstBeat );
	float fStartSecond = fFirstSecond - MinTimeToNotes;

	fStartSecond = min(fStartSecond, -MinTimeToMusic);
	
	m_soundMusic.SetPositionSeconds( fStartSecond );
	m_soundMusic.SetPlaybackRate( GAMESTATE->m_SongOptions.m_fMusicRate );

	/* Keep the music playing after it's finished; we'll stop it. */
	m_soundMusic.SetStopMode(RageSound::M_CONTINUE);
	m_soundMusic.StartPlaying();

	/* Return the amount of time until the first beat. */
	return fFirstSecond - fStartSecond;
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

// play assist ticks
bool ScreenGameplay::IsTimeToPlayTicks() const
{
	// Sound cards have a latency between when a sample is Play()ed and when the sound
	// will start coming out the speaker.  Compensate for this by boosting
	// fPositionSeconds ahead
	float fPositionSeconds = GAMESTATE->m_fMusicSeconds;
	fPositionSeconds += (SOUNDMAN->GetPlayLatency()+g_fTickEarlySecondsCache) * m_soundMusic.GetPlaybackRate();
	float fSongBeat = GAMESTATE->m_pCurSong->GetBeatFromElapsedTime( fPositionSeconds );

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

	iRowLastCrossed = iRowNow;

	return bAnyoneHasANote;
}


void ScreenGameplay::Update( float fDeltaTime )
{
	/* Very important:  Update GAMESTATE's song beat information
	 * -before- calling update on all the classes that depend on it.
	 * If you don't do this first, the classes are all acting on old 
	 * information and will lag.  -Chris */

	/* If ScreenJukebox is changing screens, it'll stop m_soundMusic to tell
	 * us not to update the time here.  (In that case, we've already created
	 * a new ScreenJukebox and reset music statistics, and if we do this then
	 * we'll un-reset them.) */
	if(m_soundMusic.IsPlaying())
		GAMESTATE->UpdateSongPosition(m_soundMusic.GetPositionSeconds());

	Screen::Update( fDeltaTime );


	if( GAMESTATE->m_MasterPlayerNumber != PLAYER_INVALID )
		m_MaxCombo.SetText( ssprintf("%d", m_Player[GAMESTATE->m_MasterPlayerNumber].GetPlayersMaxCombo()) ); /* MAKE THIS WORK FOR BOTH PLAYERS! */
	
	
	//LOG->Trace( "m_fOffsetInBeats = %f, m_fBeatsPerSecond = %f, m_Music.GetPositionSeconds = %f", m_fOffsetInBeats, m_fBeatsPerSecond, m_Music.GetPositionSeconds() );


	if( GAMESTATE->m_pCurSong == NULL )
		return;


	int pn;
	switch( m_DancingState )
	{
	case STATE_DANCING:
		
		//
		// Update players' alive time
		//
		for( pn=0; pn<NUM_PLAYERS; pn++ )
			if( GAMESTATE->IsPlayerEnabled(pn) )
				GAMESTATE->m_CurStageStats.fAliveSeconds[pn] += fDeltaTime;

		//
		// Check for end of song
		//
		float fSecondsToStop = GAMESTATE->m_pCurSong->GetElapsedTimeFromBeat( GAMESTATE->m_pCurSong->m_fLastBeat ) + 1;
		if( GAMESTATE->m_fMusicSeconds > fSecondsToStop  &&  !m_OniFade.IsClosing() )
		{
			GAMESTATE->m_fSongBeat = 0;
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

				// check for individual fail
				for ( pn=0; pn<NUM_PLAYERS; pn++ )
				{
					if ( m_pLifeMeter[pn]->IsFailing() && !GAMESTATE->m_CurStageStats.bFailed[pn] )
						GAMESTATE->m_CurStageStats.bFailed[pn] = true;	// fail
				}
						
				break;
			case SongOptions::LIFE_BATTERY:
				if( AllFailedEarlier() )SCREENMAN->SendMessageToTopScreen( SM_BeginFailed, 0 );
				if( AllAreInDanger() )	m_Background.TurnDangerOn();
				else					m_Background.TurnDangerOff();

				// check for individual fail
				for( pn=0; pn<NUM_PLAYERS; pn++ ) 
				{
					if( !GAMESTATE->IsPlayerEnabled(pn) )
						continue;
					if( !m_pLifeMeter[pn]->IsFailing())
						continue; /* isn't failing */
					if( GAMESTATE->m_CurStageStats.bFailed[pn] )
						continue; /* failed and is already dead */
					
					if( !AllFailedEarlier() )	// if not the last one to fail
					{
						// kill them!
						m_soundOniDie.PlayRandom();
						ShowOniGameOver((PlayerNumber)pn);
						m_Player[pn].Init();		// remove all notes and scoring
						m_Player[pn].FadeToFail();	// tell the NoteField to fade to white
					}
					GAMESTATE->m_CurStageStats.bFailed[pn] = true;
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
			case PLAY_MODE_NONSTOP:
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



	//
	// Send crossed row messages to Player
	//

	// Why was this originally "BeatToNoteRowNotRounded"?  It should be rounded.  -Chris
	int iRowNow = BeatToNoteRow( GAMESTATE->m_fSongBeat );
	if( iRowNow >= 0  &&  iRowNow < MAX_TAP_NOTE_ROWS )
	{
		for( int r=m_iRowLastCrossed+1; r<=iRowNow; r++ )  // for each index we crossed since the last update
		{
			for( pn=0; pn<NUM_PLAYERS; pn++ )
			{
				if( GAMESTATE->IsPlayerEnabled(pn) )
					m_Player[pn].CrossedRow( r );
			}
		}

		m_iRowLastCrossed = iRowNow;
	}

	if( GAMESTATE->m_SongOptions.m_AssistType == SongOptions::ASSIST_TICK )
		if( IsTimeToPlayTicks() )
			m_soundAssistTick.Play();
}


void ScreenGameplay::DrawPrimitives()
{
	Screen::DrawPrimitives();
}


void ScreenGameplay::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	//LOG->Trace( "ScreenGameplay::Input()" );

	if( MenuI.IsValid()  &&  
		MenuI.button == MENU_BUTTON_BACK  &&  
		m_DancingState != STATE_OUTRO  &&
		!m_Fade.IsClosing() )
	{
		if( PREFSMAN->m_bDelayedEscape && type==IET_FIRST_PRESS)
		{
			m_textDebug.SetText( "Continue holding BACK to quit" );
			m_textDebug.StopTweening();
			m_textDebug.SetDiffuse( RageColor(1,1,1,0) );
			m_textDebug.BeginTweening( 1/8.f );
			m_textDebug.SetTweenDiffuse( RageColor(1,1,1,1) );
			return;
		}
		
		if( PREFSMAN->m_bDelayedEscape && type==IET_RELEASE )
		{
			m_textDebug.StopTweening();
			m_textDebug.BeginTweening( 1/8.f );
			m_textDebug.SetTweenDiffuse( RageColor(1,1,1,0) );
			return;
		}
		
		if( (!PREFSMAN->m_bDelayedEscape && type==IET_FIRST_PRESS) ||
			(DeviceI.device==DEVICE_KEYBOARD && type==IET_SLOW_REPEAT)  ||
			(DeviceI.device!=DEVICE_KEYBOARD && type==IET_FAST_REPEAT) )
		{
			m_DancingState = STATE_OUTRO;
			SOUNDMAN->PlayOnce( THEME->GetPathTo("Sounds","menu back") );
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
			 */
			m_soundMusic.StopPlaying();

			this->ClearMessageQueue();
			m_Fade.CloseWipingLeft( SM_SaveChangedBeforeGoingBack );
			return;
		}
	}

	/* Nothing else cares about releases. */
	if(type == IET_RELEASE) return;

	// Handle special keys to adjust the offset
	if( DeviceI.device == DEVICE_KEYBOARD )
	{
		switch( DeviceI.button )
		{
		case SDLK_F6:
			m_bChangedOffsetOrBPM = true;
			GAMESTATE->m_SongOptions.m_bAutoSync = !GAMESTATE->m_SongOptions.m_bAutoSync;	// toggle
			PositionStatusIcons();
			break;
		case SDLK_F7:
			if( GAMESTATE->m_SongOptions.m_AssistType == SongOptions::ASSIST_NONE )
				GAMESTATE->m_SongOptions.m_AssistType = SongOptions::ASSIST_TICK;
			else
				GAMESTATE->m_SongOptions.m_AssistType = SongOptions::ASSIST_NONE;
			
			m_textDebug.SetText( ssprintf("Assist Tick is %s", GAMESTATE->m_SongOptions.m_AssistType==SongOptions::ASSIST_TICK?"ON":"OFF") );
			m_textDebug.SetDiffuse( RageColor(1,1,1,1) );
			m_textDebug.StopTweening();
			m_textDebug.BeginTweening( 3 );		// sleep
			m_textDebug.BeginTweening( 0.5f );	// fade out
			m_textDebug.SetTweenDiffuse( RageColor(1,1,1,0) );
			break;
		case SDLK_F8:
			PREFSMAN->m_bAutoPlay = !PREFSMAN->m_bAutoPlay;
			PositionStatusIcons();
			break;
		case SDLK_F9:
		case SDLK_F10:
			{
				m_bChangedOffsetOrBPM = true;

				float fOffsetDelta;
				switch( DeviceI.button )
				{
				case SDLK_F9:	fOffsetDelta = -0.025f;		break;
				case SDLK_F10:	fOffsetDelta = +0.025f;		break;
				default:	ASSERT(0);						return;
				}
				if( type == IET_FAST_REPEAT )
					fOffsetDelta *= 10;
				BPMSegment& seg = GAMESTATE->m_pCurSong->GetBPMSegmentAtBeat( GAMESTATE->m_fSongBeat );

				seg.m_fBPM += fOffsetDelta;

				m_textDebug.SetText( ssprintf("Cur BPM = %f", seg.m_fBPM) );
				m_textDebug.SetDiffuse( RageColor(1,1,1,1) );
				m_textDebug.StopTweening();
				m_textDebug.BeginTweening( 3 );		// sleep
				m_textDebug.BeginTweening( 0.5f );	// fade out
				m_textDebug.SetTweenDiffuse( RageColor(1,1,1,0) );
			}
			break;
		case SDLK_F11:
		case SDLK_F12:
			{
				m_bChangedOffsetOrBPM = true;

				float fOffsetDelta;
				switch( DeviceI.button )
				{
				case SDLK_F11:	fOffsetDelta = -0.02f;		break;
				case SDLK_F12:	fOffsetDelta = +0.02f;		break;
				default:	ASSERT(0);						return;
				}
				if( type == IET_FAST_REPEAT )
					fOffsetDelta *= 10;

				GAMESTATE->m_pCurSong->m_fBeat0OffsetInSeconds += fOffsetDelta;

				m_textDebug.SetText( ssprintf("Offset = %f", GAMESTATE->m_pCurSong->m_fBeat0OffsetInSeconds) );
				m_textDebug.SetDiffuse( RageColor(1,1,1,1) );
				m_textDebug.StopTweening();
				m_textDebug.BeginTweening( 3 );		// sleep
				m_textDebug.BeginTweening( 0.5f );	// fade out
				m_textDebug.SetTweenDiffuse( RageColor(1,1,1,0) );
			}
			break;
		}
	}

	//
	// handle a step
	//
	if( type==IET_FIRST_PRESS && 
		!PREFSMAN->m_bAutoPlay && 
		StyleI.IsValid() &&
		GAMESTATE->IsPlayerEnabled( StyleI.player ) )
		m_Player[StyleI.player].Step( StyleI.col ); 
}

void ScreenGameplay::PositionStatusIcons()
{
	bool status[NUM_STATUS_ICONS] = {
		PREFSMAN->m_bAutoPlay,
		GAMESTATE->m_SongOptions.m_bAutoSync
	};

	vector<Actor*> vActorsToShow;

	for( int i=0; i<NUM_STATUS_ICONS; i++ )
	{
		m_sprStatusIcons[i].SetDiffuse( RageColor(1,1,1,status[i]?1.f:0.f) );
		if( status[i] )
			vActorsToShow.push_back( &m_sprStatusIcons[i] );
	}

	float width_of_one = m_sprStatusIcons[0].GetUnzoomedWidth();
	float center_x = STATUS_ICONS_X;
	float left_most_x = center_x-(vActorsToShow.size()-1)*width_of_one/2;
	float right_most_x = center_x+(vActorsToShow.size()-1)*width_of_one/2;

	for( unsigned a=0; a<vActorsToShow.size(); a++ )
	{
		vActorsToShow[a]->SetY( STATUS_ICONS_Y );
		if( vActorsToShow.size()-1 == 0 )
			vActorsToShow[a]->SetX( center_x );
		else
			vActorsToShow[a]->SetX( SCALE(a, 0.0f, vActorsToShow.size()-1, left_most_x, right_most_x) );		
	}
}

void SaveChanges()
{
	/* XXX: Hmm.  Better would be to make sure m_pCurCourse is only set when we're
	 * playing out of a course, and use that here, so these things wouldn't need to
	 * special case play modes.  Need to make sure m_pCurCourse gets erased
	 * correctly, though. -glenn */
	/* That's a very clever idea!   I should look into this.  -Chris */
	switch( GAMESTATE->m_PlayMode )
	{
	case PLAY_MODE_ARCADE:
		GAMESTATE->m_pCurSong->Save();
		break;
	case PLAY_MODE_NONSTOP:
	case PLAY_MODE_ONI:
	case PLAY_MODE_ENDLESS:
		{
			// FIXME
			//for( int i=0; i<m_apCourseSongs.size(); i++ )
			//	m_apCourseSongs[i]->Save();
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
	case PLAY_MODE_NONSTOP:
	case PLAY_MODE_ONI:
	case PLAY_MODE_ENDLESS:
		{
			// FIXME
//			for( int i=0; i<GAMESTATE->m_pCurCourse->GetNumStages(); i++ )
//			{
//				Song* pSong = GAMESTATE->m_pCurCourse->GetSong(i);
//				ld.LoadFromSMFile( GAMESTATE->m_pCurSong->GetCacheFilePath(), *pSong );
//			}
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
			GAMESTATE->m_pCurSong->GetFullDisplayTitle().GetString() );
		break;
	case PLAY_MODE_NONSTOP:
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
	int p;
	switch( SM )
	{
		// received while STATE_INTRO
	case SM_User+0:
		{
			m_StarWipe.OpenWipingRight(SM_None);

			/* It may just be me, but having Here We Go start so late into the song
			 * ruins the rhythm of the announcer.  MAX/EX starts Here We Go
			 * a fixed amount of time after Are You Ready.  However, 1-5th
			 * start HWG a certain amount of time before the notes.
			 * We may want to metric this, but for now, leave it like MAX/EX. -Chris */

			/*
			 * Don't actually reach any notes for at least 5.5 seconds.
			 * "Here We Go" takes about two seconds, we want a half second or so of
			 * a clear view, so we don't want any notes for 2.5 seconds, plus three
			 * seconds until we actually begin the HWG.
			 */
			const float MinTimeToNotes = 5.5f;

			/*
			 * Tell the music to start, but don't actually make any noise for
			 * at least 2.5 (or 1.5) seconds.  (This is so we scroll on screen smoothly.)
			 *
			 * Use 1.5 seconds to (try to) start when "Ready" is fading in;
			 * 2.5 for when it's fading out.
			 *
			 * The last set of timings was aiming for 1. fade in "Ready", 2. fade
			 * out "Ready", 3. music starts, 4. fade in "HWG"; the current timing
			 * is 1. fade in "Ready" and start music, 2. fade out "Ready", 3. fade
			 * in "HWG".
			 *
			 * This is only a minimum: the music might be started later, to meet
			 * the minimum-time-to-notes value.  If you're writing song data,
			 * and you want to make sure we get ideal timing here, make sure there's
			 * a bit of space at the beginning of the music with no steps. */
			const float MinTimeToMusic = 1.5f;

			float delay = StartPlayingSong( MinTimeToNotes, MinTimeToMusic );

			/* Start the Here We Go sequence 2.5 seconds before notes, so we
			 * have time to fade in and fade out. */
			this->SendScreenMessage( SM_StartHereWeGo, 3.0f );
			// use this for HWG right as the notes are starting
			// this->SendScreenMessage( SM_StartHereWeGo, delay - 3.0f );
			delay; /* hush compiler */
		}
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
		m_Background.FadeIn();
		break;
	case SM_User+6:
		break;
	case SM_User+7:
		break;
	case SM_User+8:
		break;
	case SM_User+9:
		break;

	case SM_StartHereWeGo:
		/* We have 2.5 seconds until the notes start. */
		m_sprHereWeGo.StartFocusing();
		m_announcerHereWeGo.PlayRandom();
		GAMESTATE->m_bPastHereWeGo = true;

		/* This gives the HWG about a second of screen time. */
		this->SendScreenMessage( SM_StopHereWeGo, 1.5f );
		break;

	case SM_StopHereWeGo:
		/* We have one second until the notes start. */
		m_sprHereWeGo.StartBlurring();

		m_DancingState = STATE_DANCING;		// STATE CHANGE!  Now the user is allowed to press Back
		break;

	// received while STATE_DANCING
	case SM_NotesEnded:
		{
			// save any statistics
			int p;
			for( p=0; p<NUM_PLAYERS; p++ )
			{
				if( GAMESTATE->IsPlayerEnabled(p) )
				{
					for( int r=0; r<NUM_RADAR_CATEGORIES; r++ )
					{
						RadarCategory rc = (RadarCategory)r;
						GAMESTATE->m_CurStageStats.fRadarPossible[p][r] = NoteDataUtil::GetRadarValue( m_Player[p], rc, GAMESTATE->m_pCurSong->m_fMusicLengthSeconds );
						GAMESTATE->m_CurStageStats.fRadarActual[p][r] = m_Player[p].GetActualRadarValue( rc, GAMESTATE->m_pCurSong->m_fMusicLengthSeconds );
					}
				}
			}

			for( p=0; p<NUM_PLAYERS; p++ )
				if( GAMESTATE->IsPlayerEnabled(p) && !GAMESTATE->m_CurStageStats.bFailed[p] )
					GAMESTATE->m_CurStageStats.iSongsPassed[p]++;

			if( !IsLastSong() )
			{
				for( p=0; p<NUM_PLAYERS; p++ )
				if( GAMESTATE->IsPlayerEnabled(p) && !GAMESTATE->m_CurStageStats.bFailed[p] )
					m_pLifeMeter[p]->OnSongEnded();	// give a little life back between stages
				m_OniFade.CloseWipingRight( SM_BeginLoadingNextSong );
			}
			else	// IsLastSong
			{
				if( m_DancingState == STATE_OUTRO )	// gameplay already ended
					return;		// ignore
				m_DancingState = STATE_OUTRO;

				if( GAMESTATE->m_SongOptions.m_FailType == SongOptions::FAIL_END_OF_SONG  &&  AllFailedEarlier() )
				{
					this->SendScreenMessage( SM_BeginFailed, 0 );
					return;
				}

				m_StarWipe.CloseWipingRight( SM_None );

				// do they deserve an extra stage?
				if( GAMESTATE->HasEarnedExtraStage() )
					this->SendScreenMessage( SM_ShowTryExtraStage, 1 );
				else
				{
					this->SendScreenMessage( SM_ShowCleared, 1 );
					SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("gameplay cleared") );
				}
			}
		}
		break;

	case SM_BeginLoadingNextSong:
		LoadNextSong();
		GAMESTATE->m_bPastHereWeGo = true;
		/* We're fading in, so don't hit any notes for a few seconds; they'll be
		 * obscured by the fade. */
		StartPlayingSong( 3, 0 );
		m_OniFade.OpenWipingRight( SM_None );
		break;

	case SM_BeginToasty:
		if( PREFSMAN->m_bEasterEggs )
		{
			m_soundToasty.Play();

			// set off screen
			m_sprToasty.StopTweening();
			m_sprToasty.SetDiffuse( RageColor(1,1,1,1) );
			m_sprToasty.SetX( SCREEN_RIGHT+m_sprToasty.GetUnzoomedWidth()/2 );
			m_sprToasty.SetY( SCREEN_BOTTOM-m_sprToasty.GetUnzoomedHeight()/2 );
			m_sprToasty.BeginTweening( 0.2f, Actor::TWEEN_BIAS_BEGIN ); // slide on
			m_sprToasty.SetTweenX( SCREEN_RIGHT-m_sprToasty.GetUnzoomedWidth()/2 );
			m_sprToasty.BeginTweening( 0.6f );	// sleep
			m_sprToasty.BeginTweening( 0.3f, Actor::TWEEN_BIAS_END );	// slide off
			m_sprToasty.SetTweenX( SCREEN_RIGHT+m_sprToasty.GetUnzoomedWidth()/2 );
			m_sprToasty.BeginTweening( 0.001f );	// fade out
			m_sprToasty.SetDiffuse( RageColor(1,1,1,0) );
		}
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
		m_sprCleared.SetTweenDiffuse( RageColor(1,1,1,1) );
		m_sprCleared.BeginTweening(1.5f); // sleep
		m_sprCleared.BeginTweening(0.7f);
		m_sprCleared.SetTweenDiffuse( RageColor(1,1,1,0) );
		SCREENMAN->SendMessageToTopScreen( SM_GoToStateAfterCleared, 4 );
		break;

	case SM_ShowTryExtraStage:
		{
			m_soundTryExtraStage.PlayRandom();

			// make the background invisible so we don't waste mem bandwidth drawing it
			m_Background.BeginTweening( 1 );
			m_Background.SetTweenDiffuse( RageColor(1,1,1,0) );

			RageColor colorStage = GAMESTATE->GetStageColor();
			colorStage.a *= 0.7f;

			m_sprTryExtraStage.SetZoom( 4 );
			m_sprTryExtraStage.BeginBlurredTweening( 0.8f, TWEEN_BIAS_END );
			m_sprTryExtraStage.SetTweenZoom( 0.4f );			// zoom out
			m_sprTryExtraStage.SetTweenDiffuse( colorStage );	// and fade in
			m_sprTryExtraStage.BeginTweening( 0.2f );
			m_sprTryExtraStage.SetTweenZoom( 0.8f );			// bounce
			m_sprTryExtraStage.SetTweenDiffuse( colorStage );	// and fade in
			m_sprTryExtraStage.BeginTweening( 0.2f );
			m_sprTryExtraStage.SetTweenZoom( 1.0f );			// come to rest
			m_sprTryExtraStage.SetTweenDiffuse( colorStage );	// and fade in

			colorStage.a = 0;

			m_sprTryExtraStage.BeginTweening( 2 );	// sleep
			m_sprTryExtraStage.BeginTweening( 1 );	// fade out
			m_sprTryExtraStage.SetTweenDiffuse( colorStage );
			SCREENMAN->SendMessageToTopScreen( SM_GoToStateAfterCleared, 5 );
		}
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
		/* Reset options.  (Should this be done in ScreenSelect*?) */
		for( p=0; p<NUM_PLAYERS; p++ )
			GAMESTATE->m_PlayerOptions[p] = GAMESTATE->m_SelectedOptions[p];
		GAMESTATE->m_SongOptions.Init();
		switch( GAMESTATE->m_PlayMode )
		{
		case PLAY_MODE_ARCADE:	
			// SCREENMAN->SetNewScreen( "ScreenSelectMusic" );
			SCREENMAN->SetNewScreen( SONGSEL_SCREEN );
			break;
		case PLAY_MODE_NONSTOP:
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
		m_soundMusic.StopPlaying();
		m_StarWipe.SetTransitionTime( 1.5f );
		m_StarWipe.CloseWipingRight( SM_None );
		for( p=0; p<NUM_PLAYERS; p++ )
			if( GAMESTATE->IsPlayerEnabled(p) )
				m_Player[p].FadeToFail();

		this->SendScreenMessage( SM_ShowFailed, 0.2f );
		break;
	case SM_ShowFailed:
		m_soundFail.PlayRandom();

		// make the background invisible so we don't waste mem bandwidth drawing it
		m_Background.BeginTweening( 1 );
		m_Background.SetTweenDiffuse( RageColor(1,1,1,0) );

		m_sprFailed.SetZoom( 4 );
		m_sprFailed.BeginBlurredTweening( 0.8f, TWEEN_BIAS_END );
		m_sprFailed.SetTweenZoom( 0.5f );			// zoom out
		m_sprFailed.SetTweenDiffuse( RageColor(1,1,1,0.7f) );	// and fade in
		m_sprFailed.BeginTweening( 0.3f );
		m_sprFailed.SetTweenZoom( 1.1f );			// bounce
		m_sprFailed.SetTweenDiffuse( RageColor(1,1,1,0.7f) );	// and fade in
		m_sprFailed.BeginTweening( 0.2f );
		m_sprFailed.SetTweenZoom( 1.0f );			// come to rest
		m_sprFailed.SetTweenDiffuse( RageColor(1,1,1,0.7f) );	// and fade in
		m_sprFailed.BeginTweening( 2 );		// sleep
		m_sprFailed.BeginTweening( 1 );
		m_sprFailed.SetTweenDiffuse( RageColor(1,1,1,0) );

		// show the survive time if extra stage
		if( GAMESTATE->IsExtraStage()  ||  GAMESTATE->IsExtraStage2() )
		{
			float fMaxSurviveSeconds = 0;
			for( int p=0; p<NUM_PLAYERS; p++ )
				if( GAMESTATE->IsPlayerEnabled(p) )
					fMaxSurviveSeconds = max( fMaxSurviveSeconds, GAMESTATE->m_CurStageStats.fAliveSeconds[p] );
			ASSERT( fMaxSurviveSeconds > 0 );
			m_textSurviveTime.SetText( "TIME  " + SecondsToTime(fMaxSurviveSeconds) );
			m_textSurviveTime.BeginTweening( 0.3f );	// sleep
			m_textSurviveTime.BeginTweening( 0.3f );	// fade in
			m_textSurviveTime.SetTweenDiffuse( RageColor(1,1,1,1) );
			m_textSurviveTime.BeginTweening( 3.5f );	// sleep
			m_textSurviveTime.BeginTweening( 0.5f );	// fade out
			m_textSurviveTime.SetTweenDiffuse( RageColor(1,1,1,0) );
		}

		SCREENMAN->SendMessageToTopScreen( SM_PlayFailComment, 1.0f );
		SCREENMAN->SendMessageToTopScreen( SM_GoToScreenAfterFail, 5.0f );
		break;
	case SM_PlayFailComment:
		SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("gameplay failed") );
		break;

	case SM_GoToScreenAfterFail:
		if( m_bChangedOffsetOrBPM )
		{
			m_bChangedOffsetOrBPM = false;
			ShowSavePrompt( SM_GoToScreenAfterFail );
			break;
		}

		if( GAMESTATE->m_PlayMode == PLAY_MODE_NONSTOP  ||
			GAMESTATE->m_PlayMode == PLAY_MODE_ONI  ||  
			GAMESTATE->m_PlayMode == PLAY_MODE_ENDLESS ||
			GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2())
			SCREENMAN->SetNewScreen( "ScreenEvaluation" );
		else if( PREFSMAN->m_bEventMode )
			this->SendScreenMessage( SM_GoToScreenAfterBack, 0 );
		else
			SCREENMAN->SetNewScreen( "ScreenGameOver" );
		break;
	}
}


void ScreenGameplay::TweenOnScreen()
{
	unsigned i, p;

	vector<Actor*> apActorsInLifeFrame;
	apActorsInLifeFrame.push_back(	&m_sprLifeFrame );
	for( p=0; p<NUM_PLAYERS; p++ )
		apActorsInLifeFrame.push_back(	m_pLifeMeter[p] );
	apActorsInLifeFrame.push_back(	&m_textStageNumber );
	for( p=0; p<NUM_PLAYERS; p++ )
		apActorsInLifeFrame.push_back(	&m_textCourseSongNumber[p] );
	for( i=0; i<apActorsInLifeFrame.size(); i++ )
	{
		float fOriginalY = apActorsInLifeFrame[i]->GetY();
		apActorsInLifeFrame[i]->SetY( fOriginalY-100 );
		apActorsInLifeFrame[i]->BeginTweening( 0.5f );	// sleep
		apActorsInLifeFrame[i]->BeginTweening( 1 );
		apActorsInLifeFrame[i]->SetTweenY( fOriginalY );
	}


	vector<Actor*> apActorsInScoreFrame;
	apActorsInScoreFrame.push_back( &m_sprScoreFrame );
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled(p) )
			continue;
		apActorsInScoreFrame.push_back( m_pScoreDisplay[p] );
		apActorsInScoreFrame.push_back( &m_textPlayerOptions[p] );
	}
	apActorsInScoreFrame.push_back( &m_textSongOptions );
	for( i=0; i<apActorsInScoreFrame.size(); i++ )
	{
		float fOriginalY = apActorsInScoreFrame[i]->GetY();
		apActorsInScoreFrame[i]->SetY( fOriginalY+100 );
		apActorsInScoreFrame[i]->BeginTweening( 0.5f );	// sleep
		apActorsInScoreFrame[i]->BeginTweening( 1 );
		apActorsInScoreFrame[i]->SetTweenY( fOriginalY );
	}

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		float fOriginalX = m_DifficultyIcon[p].GetX();
		m_DifficultyIcon[p].SetX( (p==PLAYER_1) ? fOriginalX-200 : fOriginalX+200 );
		m_DifficultyIcon[p].BeginTweening( 0.5f );	// sleep
		m_DifficultyIcon[p].BeginTweening( 1 );
		m_DifficultyIcon[p].SetTweenX( fOriginalX );
	}
}

void ScreenGameplay::TweenOffScreen()
{

}

void ScreenGameplay::ShowOniGameOver( PlayerNumber pn )
{
	m_sprOniGameOver[pn].SetDiffuse( RageColor(1,1,1,1) );
	m_sprOniGameOver[pn].BeginTweening( 0.5f, Actor::TWEEN_BOUNCE_END );
	m_sprOniGameOver[pn].SetTweenY( CENTER_Y );
	m_sprOniGameOver[pn].SetEffectBobbing( RageVector3(0,6,0), 4 );
}

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
#include "ScoreDisplayBattle.h"
#include "ScoreDisplayRave.h"
#include "ScreenPrompt.h"
#include "GrooveRadar.h"
#include "NotesLoaderSM.h"
#include "ThemeManager.h"
#include "RageTimer.h"
#include "ScoreKeeperMAX2.h"
#include "NoteFieldPositioning.h"
#include "LyricsLoader.h"
#include "ActorUtil.h"
#include "NoteSkinManager.h"
#include "RageTextureManager.h"
#include "EnemyHealth.h"
#include "EnemyFace.h"

//
// Defines
//
#define PREV_SCREEN( play_mode )		THEME->GetMetric ("ScreenGameplay","PrevScreen"+Capitalize(PlayModeToString(play_mode)))
#define NEXT_SCREEN( play_mode )		THEME->GetMetric ("ScreenGameplay","NextScreen"+Capitalize(PlayModeToString(play_mode)))

CachedThemeMetricF SECONDS_BETWEEN_COMMENTS	("ScreenGameplay","SecondsBetweenComments");
CachedThemeMetricF G_TICK_EARLY_SECONDS		("ScreenGameplay","TickEarlySeconds");


const ScreenMessage	SM_PlayReady			= ScreenMessage(SM_User+0);
const ScreenMessage	SM_PlayGo				= ScreenMessage(SM_User+1);


// received while STATE_DANCING
const ScreenMessage	SM_NotesEnded			= ScreenMessage(SM_User+10);
const ScreenMessage	SM_LoadNextSong			= ScreenMessage(SM_User+11);

// received while STATE_OUTRO
const ScreenMessage	SM_SaveChangedBeforeGoingBack	= ScreenMessage(SM_User+20);
const ScreenMessage	SM_GoToScreenAfterBack	= ScreenMessage(SM_User+21);
const ScreenMessage	SM_GoToStateAfterCleared= ScreenMessage(SM_User+22);

const ScreenMessage	SM_BeginFailed			= ScreenMessage(SM_User+30);
const ScreenMessage	SM_GoToScreenAfterFail	= ScreenMessage(SM_User+31);

// received while STATE_INTRO
const ScreenMessage	SM_StartHereWeGo		= ScreenMessage(SM_User+40);
const ScreenMessage	SM_StopHereWeGo			= ScreenMessage(SM_User+41);


ScreenGameplay::ScreenGameplay( bool bDemonstration ) : Screen("ScreenGameplay")
{
	LOG->Trace( "ScreenGameplay::ScreenGameplay()" );

	int p;

	m_bDemonstration = bDemonstration;

	SECONDS_BETWEEN_COMMENTS.Refresh();
	G_TICK_EARLY_SECONDS.Refresh();


	if( GAMESTATE->m_pCurSong == NULL && GAMESTATE->m_pCurCourse == NULL )
		return;	// ScreenDemonstration will move us to the next scren.  We just need to survive for one update without crashing.

	/* Save selected options before we change them. */
	GAMESTATE->StoreSelectedOptions();

	GAMESTATE->RemoveAllInventory();


	for( p=0; p<NUM_PLAYERS; p++ )
	{
		m_pLifeMeter[p] = NULL;
		m_pScoreDisplay[p] = NULL;
		m_pScoreKeeper[p] = NULL;
	}


	// fill in difficulty of CPU players with that of the first human player
	for( p=0; p<NUM_PLAYERS; p++ )
		if( GAMESTATE->IsCpuPlayer(p) )
			GAMESTATE->m_pCurNotes[p] = GAMESTATE->m_pCurNotes[ GAMESTATE->GetFirstHumanPlayer() ];


	GAMESTATE->m_CurStageStats = StageStats();	// clear values

	switch( GAMESTATE->m_PlayMode )
	{
	case PLAY_MODE_BATTLE:
	case PLAY_MODE_RAVE:
		{
			// cache NoteSkin graphics
			CStringArray asNames;
			NOTESKIN->GetNoteSkinNames( asNames );
			for( unsigned i=0; i<asNames.size(); i++ )
			{
				CString sDir = NOTESKIN->GetNoteSkinDir( asNames[i] );
				CStringArray asGraphicPaths;
				GetDirListing( sDir+"*.png", asGraphicPaths, false, true ); 
				GetDirListing( sDir+"*.jpg", asGraphicPaths, false, true ); 
				GetDirListing( sDir+"*.gif", asGraphicPaths, false, true ); 
				GetDirListing( sDir+"*.bmp", asGraphicPaths, false, true ); 
				for( unsigned j=0; j<asGraphicPaths.size(); j++ )
					TEXTUREMAN->CacheTexture( asGraphicPaths[j] );
			}
		}
		break;
	}



	//
	// fill in m_apSongsQueue, m_apNotesQueue, m_asModifiersQueue
	//
	if( GAMESTATE->IsCourseMode() )
	{
		Course* pCourse = GAMESTATE->m_pCurCourse;
		pCourse->GetStageInfo( m_apSongsQueue, m_apNotesQueue[0], m_asModifiersQueue[0], GAMESTATE->GetCurrentStyleDef()->m_NotesType, false );
		for( int p=1; p<NUM_PLAYERS; p++ )
		{
			m_apNotesQueue[p] = m_apNotesQueue[0];
			m_asModifiersQueue[p] = m_asModifiersQueue[0];
		}
	}
	else
	{
		m_apSongsQueue.push_back( GAMESTATE->m_pCurSong );
		for( int p=0; p<NUM_PLAYERS; p++ )
		{
			m_apNotesQueue[p].push_back( GAMESTATE->m_pCurNotes[p] );
			m_asModifiersQueue[p].push_back( "" );
		}
	}

	
	//
	// Init ScoreKeepers
	//
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled(p) )
			continue;	// skip
		m_pScoreKeeper[p] = new ScoreKeeperMAX2( m_apNotesQueue[p], (PlayerNumber)p );
	}




	m_bChangedOffsetOrBPM = GAMESTATE->m_SongOptions.m_bAutoSync;


	m_DancingState = STATE_INTRO;
	m_fTimeLeftBeforeDancingComment = SECONDS_BETWEEN_COMMENTS;

	
	m_bZeroDeltaOnNextUpdate = false;




	// If this is beginner mode, show the helper
	/* !! Working on this.. having probs loading the BG sequences -- Miryokuteki */
		
		//this->AddChild( &m_bhDancer );
		//m_bhDancer.Load( THEME->GetPathToG( "select difficulty ex picture easy") );
			//m_bhDancer.SetXY( -100,-100 );  //<-- causing entire screen to offset!
		//m_bhDancer.SetDiffuse( RageColor(1,1,1,1) );
		//m_bhDancer.SetEffectGlowShift( 0.5f );
		//m_bhDancer.BeginDraw();

	/* */




	const bool bExtra = GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2();


	m_Background.SetDiffuse( RageColor(0.4f,0.4f,0.4f,1) );
	this->AddChild( &m_Background );

	if( !bDemonstration )	// only load if we're going to use it
	{
		m_Toasty.Load( THEME->GetPathToB("ScreenGameplay toasty") );
		this->AddChild( &m_Toasty );
	}

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled(PlayerNumber(p)) )
			continue;

		float fPlayerX = (float) GAMESTATE->GetCurrentStyleDef()->m_iCenterX[p];

		// If solo-single is enabled..
		/* XXX: Maybe this should be enabled for all ONE_PLAYER_ONE_CREDIT modes.
		 * I don't feel like testing that right now, though. */
		if( PREFSMAN->m_bSoloSingle && 
			GAMESTATE->m_CurStyle == STYLE_DANCE_SINGLE &&
			GAMESTATE->GetNumSidesJoined() == 1 )
			fPlayerX = SCREEN_WIDTH/2;

		m_Player[p].SetX( fPlayerX );
		this->AddChild( &m_Player[p] );
	
		m_TimingAssist.Load((PlayerNumber)p, &m_Player[p]);

		if( GAMESTATE->m_PlayMode == PLAY_MODE_BATTLE )
		{
			m_ActiveItemList[p].Init( (PlayerNumber)p );
			/* Position it in LoadNextSong. */
			this->AddChild( &m_ActiveItemList[p] );
		}

		m_sprOniGameOver[p].Load( THEME->GetPathToG("ScreenGameplay oni gameover") );
		m_sprOniGameOver[p].SetX( fPlayerX );
		m_sprOniGameOver[p].SetY( SCREEN_TOP - m_sprOniGameOver[p].GetZoomedHeight()/2 );
		m_sprOniGameOver[p].SetDiffuse( RageColor(1,1,1,0) );	// 0 alpha so we don't waste time drawing while not visible
		this->AddChild( &m_sprOniGameOver[p] );
	}

	if( GAMESTATE->m_PlayMode == PLAY_MODE_BATTLE )
	{
		m_pEnemyFace = new EnemyFace;
		m_pEnemyFace->SetName( "EnemyFace" );
		this->AddChild( m_pEnemyFace );
		SET_XY( *m_pEnemyFace );

		m_pEnemyHealth = new EnemyHealth;
		m_pEnemyHealth->SetName( "EnemyHealth" );
		this->AddChild( m_pEnemyHealth );
		SET_XY( *m_pEnemyHealth );
	}

	this->AddChild(&m_TimingAssist);

	this->AddChild( &m_NextSongIn );

	this->AddChild( &m_NextSongOut );



	// LifeFrame goes below LifeMeter
	CString sLifeFrameName;
	if( bExtra )
		sLifeFrameName = "ScreenGameplay extra life frame";
	else if( GAMESTATE->m_SongOptions.m_LifeType == SongOptions::LIFE_BATTERY )
		sLifeFrameName = "ScreenGameplay oni life frame";
	else 
		sLifeFrameName = "ScreenGameplay life frame";
	m_sprLifeFrame.Load( THEME->GetPathToG(sLifeFrameName) );
	m_sprLifeFrame.SetName( bExtra?"LifeFrameExtra":"LifeFrame" );
	SET_XY( m_sprLifeFrame );
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
		m_pLifeMeter[p]->SetName( ssprintf("LifeP%d%s",p+1,bExtra?"Extra":"") );
		SET_XY( *m_pLifeMeter[p] );
		this->AddChild( m_pLifeMeter[p] );		
	}


	m_sprStage.SetName( ssprintf("Stage%s",bExtra?"Extra":"") );
	SET_XY( m_sprStage );

	
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		m_textCourseSongNumber[p].LoadFromNumbers( THEME->GetPathToN("ScreenGameplay song num") );
		m_textCourseSongNumber[p].EnableShadow( false );
		m_textCourseSongNumber[p].SetName( ssprintf("SongNumberP%d%s",p+1,bExtra?"Extra":"") );
		SET_XY( m_textCourseSongNumber[p] );
		m_textCourseSongNumber[p].SetText( "" );
		m_textCourseSongNumber[p].SetDiffuse( RageColor(0,0.5f,1,1) );	// light blue
	}

	switch( GAMESTATE->m_PlayMode )
	{
	case PLAY_MODE_ARCADE:
	case PLAY_MODE_BATTLE:
	case PLAY_MODE_RAVE:
		m_sprStage.Load( THEME->GetPathToG("ScreenGameplay stage "+GAMESTATE->GetStageText()) );
		this->AddChild( &m_sprStage );
		break;
	case PLAY_MODE_NONSTOP:
	case PLAY_MODE_ONI:
	case PLAY_MODE_ENDLESS:
		for( p=0; p<NUM_PLAYERS; p++ )
			if( GAMESTATE->IsPlayerEnabled(p) )
			{
				this->AddChild( &m_textCourseSongNumber[p] );
			}
		break;
	default:
		ASSERT(0);	// invalid GameMode
	}


	//
	// Add all Actors in score frame
	//
	CString sScoreFrameName;
	if( bExtra )
		sScoreFrameName = "ScreenGameplay extra score frame";
	else if( GAMESTATE->m_SongOptions.m_LifeType == SongOptions::LIFE_BATTERY )
		sScoreFrameName = "ScreenGameplay oni score frame";
	else 
		sScoreFrameName = "ScreenGameplay score frame";
	m_sprScoreFrame.Load( THEME->GetPathToG(sScoreFrameName) );
	m_sprScoreFrame.SetName( ssprintf("ScoreFrame%s",bExtra?"Extra":"") );
	SET_XY( m_sprScoreFrame );
	this->AddChild( &m_sprScoreFrame );

	m_textSongTitle.LoadFromFont( THEME->GetPathToF("ScreenGameplay song title") );
	m_textSongTitle.EnableShadow( false );
	m_textSongTitle.SetName( "SongTitle" );
	SET_XY( m_textSongTitle );
	this->AddChild( &m_textSongTitle );

	m_MaxCombo.LoadFromNumbers( THEME->GetPathToN("ScreenGameplay max combo") );
	m_MaxCombo.SetName( "MaxCombo" );
	SET_XY( m_MaxCombo );
	m_MaxCombo.SetText( ssprintf("%d", GAMESTATE->m_CurStageStats.iCurCombo[GAMESTATE->m_MasterPlayerNumber]) ); // TODO: Make this work for both players
	this->AddChild( &m_MaxCombo );

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled(PlayerNumber(p)) )
			continue;

		switch( GAMESTATE->m_PlayMode )
		{
		case PLAY_MODE_ARCADE:
		case PLAY_MODE_NONSTOP:
			m_pScoreDisplay[p] = new ScoreDisplayNormal;
			break;
		case PLAY_MODE_ONI:
		case PLAY_MODE_ENDLESS:
			m_pScoreDisplay[p] = new ScoreDisplayOni;
			break;
		case PLAY_MODE_BATTLE:
			m_pScoreDisplay[p] = new ScoreDisplayBattle;
			break;
		case PLAY_MODE_RAVE:
			m_pScoreDisplay[p] = new ScoreDisplayRave;
			break;
		default:
			ASSERT(0);
		}

		m_pScoreDisplay[p]->Init( (PlayerNumber)p );
		m_pScoreDisplay[p]->SetName( ssprintf("ScoreP%d%s",p+1,bExtra?"Extra":"") );
		SET_XY( *m_pScoreDisplay[p] );
		this->AddChild( m_pScoreDisplay[p] );
		
		m_textPlayerOptions[p].LoadFromFont( THEME->GetPathToF("Common normal") );
		m_textPlayerOptions[p].EnableShadow( false );
		m_textPlayerOptions[p].SetName( ssprintf("PlayerOptionsP%d%s",p+1,bExtra?"Extra":"") );
		SET_XY( m_textPlayerOptions[p] );
		this->AddChild( &m_textPlayerOptions[p] );
	}

	m_textSongOptions.LoadFromFont( THEME->GetPathToF("Common normal") );
	m_textSongOptions.EnableShadow( false );
	m_textSongOptions.SetName( ssprintf("SongOptions%s",bExtra?"Extra":"") );
	SET_XY( m_textSongOptions );
	m_textSongOptions.SetText( GAMESTATE->m_SongOptions.GetString() );
	this->AddChild( &m_textSongOptions );


	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled(PlayerNumber(p)) )
			continue;

		m_DifficultyIcon[p].Load( THEME->GetPathToG("ScreenGameplay difficulty icons 2x5") );
		/* Position it in LoadNextSong. */
		this->AddChild( &m_DifficultyIcon[p] );
	}


	if(PREFSMAN->m_bShowLyrics)
		this->AddChild(&m_LyricDisplay);
	

	m_textAutoPlay.LoadFromFont( THEME->GetPathToF("ScreenGameplay autoplay") );
	m_textAutoPlay.SetName( "AutoPlay" );
	SET_XY( m_textAutoPlay );
	if( !bDemonstration )	// only load if we're not in demonstration of jukebox
		this->AddChild( &m_textAutoPlay );
	UpdateAutoPlayText();
	

	m_BPMDisplay.SetName( "BPM" );
	SET_XY( m_BPMDisplay );
	this->AddChild( &m_BPMDisplay );

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( GAMESTATE->m_PlayMode==PLAY_MODE_BATTLE )
		{
			m_Inventory[p].Load( (PlayerNumber)p );
			this->AddChild( &m_Inventory[p] );
		}

		if( GAMESTATE->m_PlayMode==PLAY_MODE_RAVE )
		{
			m_RaveHelper[p].Load( (PlayerNumber)p );
			this->AddChild( &m_RaveHelper[p] );
		}
	}

	
	if( !bDemonstration )	// only load if we're going to use it
	{
		m_Ready.Load( THEME->GetPathToB("ScreenGameplay ready") );
		this->AddChild( &m_Ready );

		m_Go.Load( THEME->GetPathToB("ScreenGameplay go") );
		this->AddChild( &m_Go );

		m_Cleared.Load( THEME->GetPathToB("ScreenGameplay cleared") );
		this->AddChild( &m_Cleared );

		m_Failed.Load( THEME->GetPathToB("ScreenGameplay failed") );
		this->AddChild( &m_Failed );

		if( GAMESTATE->IsFinalStage() )	// only load if we're going to use it
			m_Extra.Load( THEME->GetPathToB("ScreenGameplay extra1") );
		if( GAMESTATE->IsExtraStage() )	// only load if we're going to use it
			m_Extra.Load( THEME->GetPathToB("ScreenGameplay extra2") );
		this->AddChild( &m_Extra );

		// only load if we're going to use it
		switch( GAMESTATE->m_PlayMode )
		{
		case PLAY_MODE_BATTLE:
		case PLAY_MODE_RAVE:
			for( p=0; p<NUM_PLAYERS; p++ )
			{
				m_Win[p].Load( THEME->GetPathToB(ssprintf("ScreenGameplay win p%d",p+1)) );
				this->AddChild( &m_Win[p] );
			}
			m_Draw.Load( THEME->GetPathToB("ScreenGameplay draw") );
			this->AddChild( &m_Draw );
			break;
		}

		m_In.Load( THEME->GetPathToB("ScreenGameplay in") );
		this->AddChild( &m_In );


		m_textDebug.LoadFromFont( THEME->GetPathToF("Common normal") );
		m_textDebug.SetName( "Debug" );
		SET_XY( m_textDebug );
		this->AddChild( &m_textDebug );


		m_Back.Load( THEME->GetPathToB("Common back") );
		this->AddChild( &m_Back );


		if( GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2() )	// only load if we're going to use it
		{
			m_textSurviveTime.LoadFromFont( THEME->GetPathToF("ScreenGameplay survive time") );
			m_textSurviveTime.EnableShadow( false );
			m_textSurviveTime.SetName( "SurviveTime" );
			SET_XY( m_textSurviveTime );
			m_textSurviveTime.SetDiffuse( RageColor(1,1,1,0) );
			this->AddChild( &m_textSurviveTime );
		}
	}


	/* LoadNextSong first, since that positions some elements which need to be
	 * positioned before we TweenOnScreen. */
	LoadNextSong();

	TweenOnScreen();


	if( !bDemonstration )	// only load if we're going to use it
	{
		m_soundOniDie.Load(				THEME->GetPathToS("ScreenGameplay oni die") );
		m_announcerReady.Load(			ANNOUNCER->GetPathTo("gameplay ready"), 1 );
		if( GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2() )
			m_announcerHereWeGo.Load(	ANNOUNCER->GetPathTo("gameplay here we go extra"), 1 );
		else if( GAMESTATE->IsFinalStage() )
			m_announcerHereWeGo.Load(	ANNOUNCER->GetPathTo("gameplay here we go final"), 1 );
		else
			m_announcerHereWeGo.Load(	ANNOUNCER->GetPathTo("gameplay here we go normal"), 1 );
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
		m_soundAssistTick.Load(			THEME->GetPathToS("ScreenGameplay assist tick") );

		if( GAMESTATE->m_PlayMode == PLAY_MODE_BATTLE )
		{
			m_announcerBattleTrickLevel1.Load(	ANNOUNCER->GetPathTo("gameplay battle trick level1") );
			m_announcerBattleTrickLevel2.Load(	ANNOUNCER->GetPathTo("gameplay battle trick level2") );
			m_announcerBattleTrickLevel3.Load(	ANNOUNCER->GetPathTo("gameplay battle trick level3") );
			m_soundBattleTrickLevel1.Load(	THEME->GetPathToS("ScreenGameplay battle trick level1") );
			m_soundBattleTrickLevel2.Load(	THEME->GetPathToS("ScreenGameplay battle trick level2") );
			m_soundBattleTrickLevel3.Load(	THEME->GetPathToS("ScreenGameplay battle trick level3") );
			m_announcerBattleDamageLevel1.Load(	ANNOUNCER->GetPathTo("gameplay battle damage level1") );
			m_announcerBattleDamageLevel2.Load(	ANNOUNCER->GetPathTo("gameplay battle damage level2") );
			m_announcerBattleDamageLevel3.Load(	ANNOUNCER->GetPathTo("gameplay battle damage level3") );
		}
	}

	// Get the transitions rolling on the first update.
	// We can't do this in the constructor because ScreenGameplay is constructed 
	// in the middle of ScreenStage.
}

ScreenGameplay::~ScreenGameplay()
{
	LOG->Trace( "ScreenGameplay::~ScreenGameplay()" );
	
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		SAFE_DELETE( m_pLifeMeter[p] );
		SAFE_DELETE( m_pScoreDisplay[p] );
		SAFE_DELETE( m_pScoreKeeper[p] );
	}

	delete m_pEnemyHealth;
	delete m_pEnemyFace;

	m_soundMusic.StopPlaying();
}

bool ScreenGameplay::IsLastSong()
{
	if( GAMESTATE->m_pCurCourse  &&  GAMESTATE->m_pCurCourse->m_bRepeat )
		return false;

	return GAMESTATE->GetCourseSongIndex()+1 == (int)m_apSongsQueue.size(); // GetCourseSongIndex() is 0-based but size() is not
}

void ScreenGameplay::LoadNextSong()
{
	m_iRowLastCrossed = -1;

	GAMESTATE->ResetMusicStatistics();
	int p;
	for( p=0; p<NUM_PLAYERS; p++ )
		if( GAMESTATE->IsPlayerEnabled(p) )
		{
			GAMESTATE->m_CurStageStats.iSongsPlayed[p]++;
			m_textCourseSongNumber[p].SetText( ssprintf("%d", GAMESTATE->m_CurStageStats.iSongsPlayed[p]) );
		}

	int iPlaySongIndex = GAMESTATE->GetCourseSongIndex();
	iPlaySongIndex %= m_apSongsQueue.size();
	GAMESTATE->m_pCurSong = m_apSongsQueue[iPlaySongIndex];

	// Restore the player's originally selected options.
	GAMESTATE->RestoreSelectedOptions();

	m_textSongOptions.SetText( GAMESTATE->m_SongOptions.GetString() );

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled(p) )
			continue;

		GAMESTATE->m_pCurNotes[p] = m_apNotesQueue[p][iPlaySongIndex];
		m_pScoreKeeper[p]->OnNextSong( GAMESTATE->GetCourseSongIndex(), GAMESTATE->m_pCurNotes[p] );

		// Put courses options into effect.
		GAMESTATE->ApplyModifiers( (PlayerNumber)p, m_asModifiersQueue[p][iPlaySongIndex] );

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

		m_Player[p].Load( (PlayerNumber)p, &pNewNoteData, m_pLifeMeter[p], m_pScoreDisplay[p], &m_Inventory[p], m_pScoreKeeper[p] );
		if( m_bDemonstration )
		{
			GAMESTATE->m_PlayerController[p] = PC_CPU;
			GAMESTATE->m_iCpuSkill[p] = 5;
		}
		else if( GAMESTATE->IsCpuPlayer(p) )
		{
			GAMESTATE->m_PlayerController[p] = PC_CPU;
			if( GAMESTATE->m_iCpuSkill[p] == -1 )
			{
				switch( GAMESTATE->m_pCurNotes[p]->GetDifficulty() )
				{
				case DIFFICULTY_BEGINNER:	GAMESTATE->m_iCpuSkill[p] = 1;	break;
				case DIFFICULTY_EASY:		GAMESTATE->m_iCpuSkill[p] = 3;	break;
				case DIFFICULTY_MEDIUM:		GAMESTATE->m_iCpuSkill[p] = 5;	break;
				case DIFFICULTY_HARD:		GAMESTATE->m_iCpuSkill[p] = 7;	break;
				case DIFFICULTY_CHALLENGE:	GAMESTATE->m_iCpuSkill[p] = 9;	break;
				default:	ASSERT(0);
				}
			}
		}
		else if( PREFSMAN->m_bAutoPlay )
			GAMESTATE->m_PlayerController[p] = PC_AUTOPLAY;
		else
			GAMESTATE->m_PlayerController[p] = PC_HUMAN;

		m_TimingAssist.Reset();
	}

	m_textSongTitle.SetText( GAMESTATE->m_pCurSong->m_sMainTitle );

	/* XXX: set it to the current BPM, not the range */
	float fMinBPM, fMaxBPM;
	GAMESTATE->m_pCurSong->GetMinMaxBPM( fMinBPM, fMaxBPM );
	m_BPMDisplay.SetBPMRange( fMinBPM, fMaxBPM );

	const bool bExtra = GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2();
	const bool bReverse[NUM_PLAYERS] = { 
		GAMESTATE->m_PlayerOptions[0].m_fReverseScroll == 1,
		GAMESTATE->m_PlayerOptions[1].m_fReverseScroll == 1
	};

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled(PlayerNumber(p)) )
			continue;

		m_ActiveItemList[p].SetName( ssprintf("ActiveItemsP%d%s%s",p+1,bExtra?"Extra":"",bReverse[p]?"Reverse":"") );
		SET_XY( m_ActiveItemList[p] );

		m_DifficultyIcon[p].SetName( ssprintf("DifficultyP%d%s%s",p+1,bExtra?"Extra":"",bReverse[p]?"Reverse":"") );
		SET_XY( m_DifficultyIcon[p] );
	}

	/* Load the Oni transitions */
	m_NextSongIn.Load( THEME->GetPathToB("ScreenGameplay next song in") );
	// Instead, load this right before it's used
//	m_NextSongOut.Load( THEME->GetPathToB("ScreenGameplay next song out") );

	/* XXX: We want to put the lyrics out of the way, but it's likely that one
	 * player is in reverse and the other isn't.  What to do? */
	m_LyricDisplay.SetName( "Lyrics" );
	SET_XY( m_LyricDisplay );

	// Load lyrics
	// XXX: don't load this here
	LyricsLoader LL;
	if( GAMESTATE->m_pCurSong->HasLyrics()  )
		LL.LoadFromLRCFile(GAMESTATE->m_pCurSong->GetLyricsPath(), *GAMESTATE->m_pCurSong);

	
	m_soundMusic.Load( GAMESTATE->m_pCurSong->GetMusicPath() );
	
	/* Set up song-specific graphics. */
	m_Background.LoadFromSong( GAMESTATE->m_pCurSong );
	m_Background.SetDiffuse( RageColor(0.5f,0.5f,0.5f,1) );
	m_Background.BeginTweening( 2 );
	m_Background.SetDiffuse( RageColor(1,1,1,1) );


	/* m_soundMusic and m_Background take a very long time to load,
	 * so cap fDelta at 0 so m_NextSongIn will show up on screen.
	 * -Chris */ 
	m_bZeroDeltaOnNextUpdate = true;
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
	fPositionSeconds += (SOUNDMAN->GetPlayLatency()+(float)G_TICK_EARLY_SECONDS) * m_soundMusic.GetPlaybackRate();
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

			bAnyoneHasANote |= m_Player[p].IsThereATapAtRow( r );
			break;	// this will only play the tick for the first player that is joined
		}
	}

	iRowLastCrossed = iRowNow;

	return bAnyoneHasANote;
}


void ScreenGameplay::Update( float fDeltaTime )
{
	if( GAMESTATE->m_pCurSong == NULL  )
	{
		/* ScreenDemonstration will move us to the next screen.  We just need to
		 * survive for one update without crashing.  We need to call Screen::Update
		 * to make sure we receive the next-screen message. */
		Screen::Update( fDeltaTime );
		return;
	}

	if( m_bFirstUpdate )
	{
		SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("gameplay intro") );	// crowd cheer

		//
		// Get the transitions rolling
		//
		if( m_bDemonstration )
		{
			StartPlayingSong( 0, 0 );	// *kick* (no transitions)
		}
		else
		{
			float fMinTimeToMusic = m_In.GetLengthSeconds();	// start of m_Ready
			float fMinTimeToNotes = fMinTimeToMusic + m_Ready.GetLengthSeconds() + m_Go.GetLengthSeconds()+2;	// end of Go

			/*
			 * Tell the music to start, but don't actually make any noise for
			 * at least 2.5 (or 1.5) seconds.  (This is so we scroll on screen smoothly.)
			 *
			 * This is only a minimum: the music might be started later, to meet
			 * the minimum-time-to-notes value.  If you're writing song data,
			 * and you want to make sure we get ideal timing here, make sure there's
			 * a bit of space at the beginning of the music with no steps. 
			 */

			/*float delay =*/ StartPlayingSong( fMinTimeToNotes, fMinTimeToMusic );

			m_In.StartTransitioning( SM_PlayReady );
		}
	}



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

	if( m_bZeroDeltaOnNextUpdate )
	{
		Screen::Update( 0 );
		m_bZeroDeltaOnNextUpdate = false;
	}
	else
		Screen::Update( fDeltaTime );

	if( GAMESTATE->m_pCurSong == NULL )
		return;

	if( GAMESTATE->m_MasterPlayerNumber != PLAYER_INVALID )
		m_MaxCombo.SetText( ssprintf("%d", GAMESTATE->m_CurStageStats.iCurCombo[GAMESTATE->m_MasterPlayerNumber]) ); /* MAKE THIS WORK FOR BOTH PLAYERS! */
	
	//LOG->Trace( "m_fOffsetInBeats = %f, m_fBeatsPerSecond = %f, m_Music.GetPositionSeconds = %f", m_fOffsetInBeats, m_fBeatsPerSecond, m_Music.GetPositionSeconds() );

	int pn;
	switch( m_DancingState )
	{
	case STATE_DANCING:
		//
		// Update living players' alive time
		//
		for( pn=0; pn<NUM_PLAYERS; pn++ )
			if( GAMESTATE->IsPlayerEnabled(pn) && !GAMESTATE->m_CurStageStats.bFailed[pn])
				GAMESTATE->m_CurStageStats.fAliveSeconds [pn] += fDeltaTime;

		//
		// Check for end of song
		//
		float fSecondsToStop = GAMESTATE->m_pCurSong->GetElapsedTimeFromBeat( GAMESTATE->m_pCurSong->m_fLastBeat ) + 1;
		if( GAMESTATE->m_fMusicSeconds > fSecondsToStop  &&  !m_NextSongOut.IsTransitioning() )
		{
			GAMESTATE->m_fSongBeat = 0;
			this->PostScreenMessage( SM_NotesEnded, 0 );
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
				if( AllAreFailing() )	SCREENMAN->PostMessageToTopScreen( SM_BeginFailed, 0 );
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
				if( AllFailedEarlier() )SCREENMAN->PostMessageToTopScreen( SM_BeginFailed, 0 );
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
		// Check to see if it's time to play a ScreenGameplay comment
		//
		m_fTimeLeftBeforeDancingComment -= fDeltaTime;
		if( m_fTimeLeftBeforeDancingComment <= 0 )
		{
			switch( GAMESTATE->m_PlayMode )
			{
			case PLAY_MODE_ARCADE:
			case PLAY_MODE_BATTLE:
			case PLAY_MODE_RAVE:
				if( OneIsHot() )			m_announcerHot.PlayRandom();
				else if( AllAreInDanger() )	m_announcerDanger.PlayRandom();
				else						m_announcerGood.PlayRandom();
				if( m_pEnemyFace )
					m_pEnemyFace->SetFace( EnemyFace::taunt );
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
	if( iRowNow >= 0 )
	{
		for( ; m_iRowLastCrossed <= iRowNow; m_iRowLastCrossed++ )  // for each index we crossed since the last update
			for( pn=0; pn<NUM_PLAYERS; pn++ )
				if( GAMESTATE->IsPlayerEnabled(pn) )
				{
					if(GAMESTATE->m_CurrentPlayerOptions[pn].m_fAppearances[PlayerOptions::APPEARANCE_RANDOMVANISH]==1) // if we're doing random vanish
					{
						m_Player[pn].RandomiseNotes( m_iRowLastCrossed ); // randomise notes on the fly
					}
					m_Player[pn].CrossedRow( m_iRowLastCrossed );
				}
	}

	if( GAMESTATE->m_SongOptions.m_bAssistTick && IsTimeToPlayTicks())
		m_soundAssistTick.Play();


	// launch battle attacks

	if( GAMESTATE->m_PlayMode == PLAY_MODE_BATTLE )
	{
#define CROSSED_SONG_SECONDS( s ) ((GAMESTATE->m_fMusicSeconds-fDeltaTime) < s  &&  (GAMESTATE->m_fMusicSeconds) >= s )

		static const CString sPossibleModifiers[NUM_ATTACK_LEVELS][3] = 
		{
			{
				"1.5x",
				"dizzy",
				"drunk"
			},
			{
				"sudden",
				"hidden",
				"wave",
			},
			{
				"expand",
				"tornado",
				"flip"
			}
		};

		if( CROSSED_SONG_SECONDS(20) || CROSSED_SONG_SECONDS(40) )
		{
			GameState::Attack a;
			a.fSecsRemaining = 10;
			a.level = ATTACK_LEVEL_1;
			a.sModifier = sPossibleModifiers[a.level][rand()%3];
			for( int p=0; p<NUM_PLAYERS; p++ )
				if( GAMESTATE->IsPlayerEnabled(p) )
					GAMESTATE->LaunchAttack( (PlayerNumber)p, a );
			this->HandleScreenMessage( SM_BattleTrickLevel1 );
		}
		if( CROSSED_SONG_SECONDS(60) || CROSSED_SONG_SECONDS(80) )
		{
			GameState::Attack a;
			a.fSecsRemaining = 10;
			a.level = ATTACK_LEVEL_2;
			a.sModifier = sPossibleModifiers[a.level][rand()%3];
			for( int p=0; p<NUM_PLAYERS; p++ )
				if( GAMESTATE->IsPlayerEnabled(p) )
					GAMESTATE->LaunchAttack( (PlayerNumber)p, a );
			this->HandleScreenMessage( SM_BattleTrickLevel2 );
		}
		if( CROSSED_SONG_SECONDS(100) )
		{
			GameState::Attack a;
			a.fSecsRemaining = 10;
			a.level = ATTACK_LEVEL_3;
			a.sModifier = sPossibleModifiers[a.level][rand()%3];
			for( int p=0; p<NUM_PLAYERS; p++ )
				if( GAMESTATE->IsPlayerEnabled(p) )
					GAMESTATE->LaunchAttack( (PlayerNumber)p, a );
			this->HandleScreenMessage( SM_BattleTrickLevel3 );
		}
	}
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
		!m_Back.IsTransitioning() )
	{
		if( PREFSMAN->m_bDelayedEscape && type==IET_FIRST_PRESS)
		{
			m_textDebug.SetText( "Continue holding BACK to quit" );
			m_textDebug.StopTweening();
			m_textDebug.SetDiffuse( RageColor(1,1,1,0) );
			m_textDebug.BeginTweening( 1/8.f );
			m_textDebug.SetDiffuse( RageColor(1,1,1,1) );
			return;
		}
		
		if( PREFSMAN->m_bDelayedEscape && type==IET_RELEASE )
		{
			m_textDebug.StopTweening();
			m_textDebug.BeginTweening( 1/8.f );
			m_textDebug.SetDiffuse( RageColor(1,1,1,0) );
			return;
		}
		
		if( (!PREFSMAN->m_bDelayedEscape && type==IET_FIRST_PRESS) ||
			(DeviceI.device==DEVICE_KEYBOARD && type==IET_SLOW_REPEAT)  ||
			(DeviceI.device!=DEVICE_KEYBOARD && type==IET_FAST_REPEAT) )
		{
			m_DancingState = STATE_OUTRO;
			SOUNDMAN->PlayOnce( THEME->GetPathToS("Common back") );
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
			m_Back.StartTransitioning( SM_SaveChangedBeforeGoingBack );
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
			UpdateAutoPlayText();
			break;
		case SDLK_F7:
			GAMESTATE->m_SongOptions.m_bAssistTick ^= 1;
			
			m_textDebug.SetText( ssprintf("Assist Tick is %s", GAMESTATE->m_SongOptions.m_bAssistTick?"ON":"OFF") );
			m_textDebug.SetDiffuse( RageColor(1,1,1,1) );
			m_textDebug.StopTweening();
			m_textDebug.BeginTweening( 3 );		// sleep
			m_textDebug.BeginTweening( 0.5f );	// fade out
			m_textDebug.SetDiffuse( RageColor(1,1,1,0) );
			break;
		case SDLK_F8:
			{
				PREFSMAN->m_bAutoPlay = !PREFSMAN->m_bAutoPlay;
				UpdateAutoPlayText();
				for( int p=0; p<NUM_PLAYERS; p++ )
					if( GAMESTATE->IsHumanPlayer(p) )
						GAMESTATE->m_PlayerController[p] = PREFSMAN->m_bAutoPlay?PC_AUTOPLAY:PC_HUMAN;
			}
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
				m_textDebug.SetDiffuse( RageColor(1,1,1,0) );
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
				m_textDebug.SetDiffuse( RageColor(1,1,1,0) );
			}
			break;
		}
	}

	//
	// handle a step or battle item activate
	//
	if( type==IET_FIRST_PRESS && 
		!PREFSMAN->m_bAutoPlay && 
		StyleI.IsValid() &&
		GAMESTATE->IsPlayerEnabled( StyleI.player ) )
	{
		m_Player[StyleI.player].Step( StyleI.col ); 
	}
	else if( type==IET_FIRST_PRESS && 
		!PREFSMAN->m_bAutoPlay && 
		MenuI.IsValid() &&
		GAMESTATE->IsPlayerEnabled( MenuI.player ) )
	{
		int iItemSlot;
		switch( MenuI.button )
		{
		case MENU_BUTTON_LEFT:	iItemSlot = 0;	break;
		case MENU_BUTTON_START:	iItemSlot = 1;	break;
		case MENU_BUTTON_RIGHT:	iItemSlot = 2;	break;
		default:				iItemSlot = -1;	break;
		}
		
		if( iItemSlot != -1 )
			m_Inventory[MenuI.player].UseItem( iItemSlot );
	}
}

void ScreenGameplay::UpdateAutoPlayText()
{
	CString sText;

	if( PREFSMAN->m_bAutoPlay )
		sText += "AutoPlay     ";
	if( GAMESTATE->m_SongOptions.m_bAutoSync )
		sText += "AutoSync     ";

	if( sText.length() > 0 )
		sText.resize( sText.length()-5 );

	m_textAutoPlay.SetText( sText );
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
	case PLAY_MODE_BATTLE:
	case PLAY_MODE_RAVE:
		GAMESTATE->m_pCurSong->Save();
		break;
	case PLAY_MODE_NONSTOP:
	case PLAY_MODE_ONI:
	case PLAY_MODE_ENDLESS:
		{
			// FIXME!!!
			//for( int i=0; i<m_apSongsQueue.size(); i++ )
			//	m_apSongsQueue[i]->Save();
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
	case PLAY_MODE_BATTLE:
	case PLAY_MODE_RAVE:
		ld.LoadFromSMFile(GAMESTATE->m_pCurSong->GetCacheFilePath(),
			*GAMESTATE->m_pCurSong);
		break;
	case PLAY_MODE_NONSTOP:
	case PLAY_MODE_ONI:
	case PLAY_MODE_ENDLESS:
		{
			// FIXME
//			for( unsigned i=0; i<m_apSongsQueue.size(); i++ )
//			{
//				Song* pSong = m_apSongsQueue[i];
//				ld.LoadFromSMFile( pSong->GetCacheFilePath(), *pSong );
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
	case PLAY_MODE_BATTLE:
	case PLAY_MODE_RAVE:
		sMessage = ssprintf( 
			"You have changed the offset or BPM of\n"
			"%s.\n"
			"Would you like to save these changes back\n"
			"to the song file?\n"
			"Choosing NO will discard your changes.",
			GAMESTATE->m_pCurSong->GetFullDisplayTitle().c_str() );
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
	switch( SM )
	{
	case SM_PlayReady:
		m_Background.FadeIn();
		m_announcerReady.PlayRandom();
		m_Ready.StartTransitioning( SM_PlayGo );
		break;

	case SM_PlayGo:
		m_announcerHereWeGo.PlayRandom();
		m_Go.StartTransitioning( SM_None );
		GAMESTATE->m_bPastHereWeGo = true;
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

			GAMESTATE->RemoveAllActiveAttacks();

			for( p=0; p<NUM_PLAYERS; p++ )
			{
				if( !GAMESTATE->IsPlayerEnabled(p) )
					continue;

				if( !GAMESTATE->m_CurStageStats.bFailed[p] )
					GAMESTATE->m_CurStageStats.iSongsPassed[p]++;
			}

			if( !IsLastSong() )
			{
				for( p=0; p<NUM_PLAYERS; p++ )
				if( GAMESTATE->IsPlayerEnabled(p) && !GAMESTATE->m_CurStageStats.bFailed[p] )
					m_pLifeMeter[p]->OnSongEnded();	// give a little life back between stages

				// HACK:  Temporarily set the song pointer to the next song so that 
				// this m_NextSongOut will show the next song banner
				Song* pCurSong = GAMESTATE->m_pCurSong;

				int iPlaySongIndex = GAMESTATE->GetCourseSongIndex()+1;
				iPlaySongIndex %= m_apSongsQueue.size();
				GAMESTATE->m_pCurSong = m_apSongsQueue[iPlaySongIndex];

				m_NextSongOut.Load( THEME->GetPathToB("ScreenGameplay next song out") );
				GAMESTATE->m_pCurSong = pCurSong;

				m_NextSongOut.StartTransitioning( SM_LoadNextSong );
			}
			else	// IsLastSong
			{
				if( m_DancingState == STATE_OUTRO )	// ScreenGameplay already ended
					return;		// ignore
				m_DancingState = STATE_OUTRO;

				GAMESTATE->RemoveAllActiveAttacks();

				if( GAMESTATE->m_SongOptions.m_FailType == SongOptions::FAIL_END_OF_SONG  &&  AllFailedEarlier() )
				{
					this->PostScreenMessage( SM_BeginFailed, 0 );
				}
				else	// ! failed
				{
					// do they deserve an extra stage?
					if( GAMESTATE->HasEarnedExtraStage() )
					{
						TweenOffScreen();
						m_Extra.StartTransitioning( SM_GoToStateAfterCleared );
						SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("gameplay extra") );
					}
					else
					{
						TweenOffScreen();
						
						switch( GAMESTATE->m_PlayMode )
						{
						case PLAY_MODE_BATTLE:
						case PLAY_MODE_RAVE:
							{
								PlayerNumber winner = GAMESTATE->GetWinner();
								switch( winner )
								{
								case PLAYER_INVALID:
									m_Draw.StartTransitioning( SM_GoToStateAfterCleared );
									break;
								default:
									m_Win[winner].StartTransitioning( SM_GoToStateAfterCleared );
									break;
								}
							}
							break;
						default:
							m_Cleared.StartTransitioning( SM_GoToStateAfterCleared );
							break;
						}
						
						SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("gameplay cleared") );
					}
				}
			}
		}
		break;

	case SM_LoadNextSong:
		LoadNextSong();
		GAMESTATE->m_bPastHereWeGo = true;
		/* We're fading in, so don't hit any notes for a few seconds; they'll be
		 * obscured by the fade. */
		StartPlayingSong( m_NextSongIn.GetLengthSeconds()+2, 0 );
		m_NextSongIn.StartTransitioning( SM_None );
		break;

	case SM_PlayToasty:
		if( PREFSMAN->m_bEasterEggs )
			if( !m_Toasty.IsTransitioning()  &&  !m_Toasty.IsFinished() )	// don't play if we've already played it once
				m_Toasty.StartTransitioning();
		break;

#define SECS_SINCE_LAST_COMMENT (SECONDS_BETWEEN_COMMENTS-m_fTimeLeftBeforeDancingComment)
	case SM_100Combo:
		if( SECS_SINCE_LAST_COMMENT > 10 )
		{
			m_announcer100Combo.PlayRandom();
			m_fTimeLeftBeforeDancingComment = SECONDS_BETWEEN_COMMENTS;
		}
		break;
	case SM_200Combo:
		if( SECS_SINCE_LAST_COMMENT > 10 )
		{
			m_announcer200Combo.PlayRandom();
			m_fTimeLeftBeforeDancingComment = SECONDS_BETWEEN_COMMENTS;
		}
		break;
	case SM_300Combo:
		if( SECS_SINCE_LAST_COMMENT > 10 )
		{
			m_announcer300Combo.PlayRandom();
			m_fTimeLeftBeforeDancingComment = SECONDS_BETWEEN_COMMENTS;
		}
		break;
	case SM_400Combo:
		if( SECS_SINCE_LAST_COMMENT > 10 )
		{
			m_announcer400Combo.PlayRandom();
			m_fTimeLeftBeforeDancingComment = SECONDS_BETWEEN_COMMENTS;
		}
		break;
	case SM_500Combo:
		if( SECS_SINCE_LAST_COMMENT > 10 )
		{
			m_announcer500Combo.PlayRandom();
			m_fTimeLeftBeforeDancingComment = SECONDS_BETWEEN_COMMENTS;
		}
		break;
	case SM_600Combo:
		if( SECS_SINCE_LAST_COMMENT > 10 )
		{
			m_announcer600Combo.PlayRandom();
			m_fTimeLeftBeforeDancingComment = SECONDS_BETWEEN_COMMENTS;
		}
		break;
	case SM_700Combo:
		if( SECS_SINCE_LAST_COMMENT > 10 )
		{
			m_announcer700Combo.PlayRandom();
			m_fTimeLeftBeforeDancingComment = SECONDS_BETWEEN_COMMENTS;
		}
		break;
	case SM_800Combo:
		if( SECS_SINCE_LAST_COMMENT > 10 )
		{
			m_announcer800Combo.PlayRandom();
			m_fTimeLeftBeforeDancingComment = SECONDS_BETWEEN_COMMENTS;
		}
		break;
	case SM_900Combo:
		if( SECS_SINCE_LAST_COMMENT > 10 )
		{
			m_announcer900Combo.PlayRandom();
			m_fTimeLeftBeforeDancingComment = SECONDS_BETWEEN_COMMENTS;
		}
		break;
	case SM_1000Combo:
		if( SECS_SINCE_LAST_COMMENT > 10 )
		{
			m_announcer1000Combo.PlayRandom();
			m_fTimeLeftBeforeDancingComment = SECONDS_BETWEEN_COMMENTS;
		}
		break;
	case SM_ComboStopped:
		if( SECS_SINCE_LAST_COMMENT > 10 )
		{
			m_announcerComboStopped.PlayRandom();
			m_fTimeLeftBeforeDancingComment = SECONDS_BETWEEN_COMMENTS;
		}
		break;
	
	case SM_BattleTrickLevel1:
		if( SECS_SINCE_LAST_COMMENT > 5 )
		{
			m_announcerBattleTrickLevel1.PlayRandom();
			m_fTimeLeftBeforeDancingComment = SECONDS_BETWEEN_COMMENTS;
		}
		m_soundBattleTrickLevel1.PlayRandom();
		m_pEnemyFace->SetFace( EnemyFace::attack );
		break;
	case SM_BattleTrickLevel2:
		if( SECS_SINCE_LAST_COMMENT > 5 )
		{
			m_announcerBattleTrickLevel2.PlayRandom();
			m_fTimeLeftBeforeDancingComment = SECONDS_BETWEEN_COMMENTS;
		}
		m_soundBattleTrickLevel2.PlayRandom();
		m_pEnemyFace->SetFace( EnemyFace::attack );
		break;
	case SM_BattleTrickLevel3:
		if( SECS_SINCE_LAST_COMMENT > 5 )
		{
			m_announcerBattleTrickLevel3.PlayRandom();
			m_fTimeLeftBeforeDancingComment = SECONDS_BETWEEN_COMMENTS;
		}
		m_soundBattleTrickLevel3.PlayRandom();
		m_pEnemyFace->SetFace( EnemyFace::attack );
		break;
	
	case SM_BattleDamageLevel1:
		if( SECS_SINCE_LAST_COMMENT > 5 )
		{
			m_announcerBattleDamageLevel1.PlayRandom();
			m_fTimeLeftBeforeDancingComment = SECONDS_BETWEEN_COMMENTS;
		}
		m_pEnemyFace->SetFace( EnemyFace::damage );
		break;
	case SM_BattleDamageLevel2:
		if( SECS_SINCE_LAST_COMMENT > 5 )
		{
			m_announcerBattleDamageLevel2.PlayRandom();
			m_fTimeLeftBeforeDancingComment = SECONDS_BETWEEN_COMMENTS;
		}
		m_pEnemyFace->SetFace( EnemyFace::damage );
		break;
	case SM_BattleDamageLevel3:
		if( SECS_SINCE_LAST_COMMENT > 5 )
		{
			m_announcerBattleDamageLevel3.PlayRandom();
			m_fTimeLeftBeforeDancingComment = SECONDS_BETWEEN_COMMENTS;
		}
		m_pEnemyFace->SetFace( EnemyFace::damage );
		break;
	
	case SM_SaveChangedBeforeGoingBack:
		if( m_bChangedOffsetOrBPM )
		{
			m_bChangedOffsetOrBPM = false;
			ShowSavePrompt( SM_GoToScreenAfterBack );
		}
		else
			HandleScreenMessage( SM_GoToScreenAfterBack );
		break;

	case SM_GoToScreenAfterBack:
		/* Reset options.  (Should this be done in ScreenSelect*?) */
		GAMESTATE->RestoreSelectedOptions();
		SCREENMAN->SetNewScreen( PREV_SCREEN(GAMESTATE->m_PlayMode) );
		break;

	case SM_GoToStateAfterCleared:
		/* Reset options.  (Should this be done in ScreenSelect*?) */
		GAMESTATE->RestoreSelectedOptions();

		if( m_bChangedOffsetOrBPM )
		{
			m_bChangedOffsetOrBPM = false;
			ShowSavePrompt( SM_GoToStateAfterCleared );
			break;
		}
		SCREENMAN->SetNewScreen( NEXT_SCREEN(GAMESTATE->m_PlayMode) );
		break;

	case SM_BeginFailed:
		m_DancingState = STATE_OUTRO;
		m_soundMusic.StopPlaying();
		TweenOffScreen();
		m_Failed.StartTransitioning( SM_GoToScreenAfterFail );

		// make the background invisible so we don't waste power drawing it
		m_Background.BeginTweening( 1 );
		m_Background.SetDiffuse( RageColor(1,1,1,0) );

		// show the survive time if extra stage
		if( GAMESTATE->IsExtraStage()  ||  GAMESTATE->IsExtraStage2() )
		{
			float fMaxSurviveSeconds = 0;
			for( int p=0; p<NUM_PLAYERS; p++ )
				if( GAMESTATE->IsPlayerEnabled(p) )
					fMaxSurviveSeconds = max( fMaxSurviveSeconds, GAMESTATE->m_CurStageStats.fAliveSeconds[p] );
			ASSERT( fMaxSurviveSeconds > 0 );
			m_textSurviveTime.SetText( "TIME: " + SecondsToTime(fMaxSurviveSeconds) );
			SET_XY( m_textSurviveTime );
		}

		SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("gameplay failed") );
		break;

	case SM_GoToScreenAfterFail:
		/* Reset options.  (Should this be done in ScreenSelect*?) */
		GAMESTATE->RestoreSelectedOptions();

		if( m_bChangedOffsetOrBPM )
		{
			m_bChangedOffsetOrBPM = false;
			ShowSavePrompt( SM_GoToScreenAfterFail );
			break;
		}

		switch( GAMESTATE->m_PlayMode )
		{
		case PLAY_MODE_ARCADE:
		case PLAY_MODE_BATTLE:
		case PLAY_MODE_RAVE:
			if( PREFSMAN->m_bEventMode )
				HandleScreenMessage( SM_GoToScreenAfterBack );
			else if( GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2() )
				SCREENMAN->SetNewScreen( "ScreenEvaluationStage" );
			else
				SCREENMAN->SetNewScreen( "ScreenGameOver" );
			break;
		case PLAY_MODE_NONSTOP:
			SCREENMAN->SetNewScreen( "ScreenEvaluationNonstop" );
			break;
		case PLAY_MODE_ONI:
			SCREENMAN->SetNewScreen( "ScreenEvaluationOni" );
			break;
		case PLAY_MODE_ENDLESS:
			SCREENMAN->SetNewScreen( "ScreenEvaluationEndless" );
			break;
		default:
			ASSERT(0);
		}
	}
}


void ScreenGameplay::TweenOnScreen()
{
	ON_COMMAND( m_sprLifeFrame );
	ON_COMMAND( m_sprStage );
	ON_COMMAND( m_textSongOptions );
	ON_COMMAND( m_sprScoreFrame );
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		ON_COMMAND( *m_pLifeMeter[p] );
		if( !GAMESTATE->IsPlayerEnabled(p) )
			continue;
		ON_COMMAND( m_textCourseSongNumber[p] );
		ON_COMMAND( *m_pScoreDisplay[p] );
		ON_COMMAND( m_textPlayerOptions[p] );
		ON_COMMAND( m_DifficultyIcon[p] );
	}
	if( m_pEnemyFace )
		ON_COMMAND( *m_pEnemyFace );
	if( m_pEnemyHealth )
		ON_COMMAND( *m_pEnemyHealth );
}

void ScreenGameplay::TweenOffScreen()
{
	OFF_COMMAND( m_sprLifeFrame );
	OFF_COMMAND( m_sprStage );
	OFF_COMMAND( m_textSongOptions );
	OFF_COMMAND( m_sprScoreFrame );
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		OFF_COMMAND( *m_pLifeMeter[p] );
		if( !GAMESTATE->IsPlayerEnabled(p) )
			continue;
		OFF_COMMAND( m_textCourseSongNumber[p] );
		OFF_COMMAND( *m_pScoreDisplay[p] );
		OFF_COMMAND( m_textPlayerOptions[p] );
		OFF_COMMAND( m_DifficultyIcon[p] );
	}
	if( m_pEnemyFace )
		OFF_COMMAND( *m_pEnemyFace );
	if( m_pEnemyHealth )
		OFF_COMMAND( *m_pEnemyHealth );

	if(m_textDebug.GetTweenTimeLeft() > 1/8.f)
	{
		m_textDebug.StopTweening();
		m_textDebug.BeginTweening( 1/8.f );
		m_textDebug.SetDiffuse( RageColor(1,1,1,1) );
	}
}

void ScreenGameplay::ShowOniGameOver( PlayerNumber pn )
{
	m_sprOniGameOver[pn].SetDiffuse( RageColor(1,1,1,1) );
	m_sprOniGameOver[pn].BeginTweening( 0.5f, Actor::TWEEN_BOUNCE_END );
	m_sprOniGameOver[pn].SetY( CENTER_Y );
	m_sprOniGameOver[pn].SetEffectBob( 4, RageVector3(0,6,0) );
}

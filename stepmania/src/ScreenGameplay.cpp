#include "global.h"
#include "ScreenGameplay.h"
#include "SongManager.h"
#include "ScreenManager.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "GameManager.h"
#include "SongManager.h"
#include "RageLog.h"
#include "LifeMeterBar.h"
#include "LifeMeterBattery.h"
#include "GameState.h"
#include "ScoreDisplayNormal.h"
#include "ScoreDisplayPercentage.h"
#include "ScoreDisplayOni.h"
#include "ScoreDisplayBattle.h"
#include "ScoreDisplayRave.h"
#include "ScreenPrompt.h"
#include "GrooveRadar.h"
#include "NotesLoaderSM.h"
#include "ThemeManager.h"
#include "RageTimer.h"
#include "ScoreKeeperMAX2.h"
#include "ScoreKeeperRave.h"
#include "NoteFieldPositioning.h"
#include "LyricsLoader.h"
#include "ActorUtil.h"
#include "NoteSkinManager.h"
#include "RageTextureManager.h"
#include "GameSoundManager.h"
#include "CombinedLifeMeterTug.h"
#include "Inventory.h"
#include "Course.h"
#include "NoteDataUtil.h"
#include "UnlockManager.h"
#include "LightsManager.h"
#include "ProfileManager.h"
#include "StatsManager.h"
#include "PlayerAI.h"	// for NUM_SKILL_LEVELS
#include "NetworkSyncManager.h"
#include "Foreach.h"
#include "DancingCharacters.h"
#include "ScreenDimensions.h"
#include "ThemeMetric.h"
#include "PlayerState.h"
#include "GameplayMessages.h"
#include "Style.h"
#include "LuaManager.h"
#include "MemoryCardManager.h"

//
// Defines
//
#define PREV_SCREEN								THEME->GetMetric (m_sName,"PrevScreen")
#define NEXT_SCREEN								THEME->GetMetric (m_sName,"NextScreen")
#define SHOW_LIFE_METER_FOR_DISABLED_PLAYERS	THEME->GetMetricB(m_sName,"ShowLifeMeterForDisabledPlayers")
#define EVAL_ON_FAIL							THEME->GetMetricB(m_sName,"ShowEvaluationOnFail")
#define SHOW_SCORE_IN_RAVE						THEME->GetMetricB(m_sName,"ShowScoreInRave")
#define SONG_POSITION_METER_WIDTH				THEME->GetMetricF(m_sName,"SongPositionMeterWidth")
#define PLAYER_X( p, styleType )				THEME->GetMetricF(m_sName,ssprintf("PlayerP%d%sX",p+1,StyleTypeToString(styleType).c_str()))

static ThemeMetric<float> INITIAL_BACKGROUND_BRIGHTNESS	("ScreenGameplay","InitialBackgroundBrightness");
static ThemeMetric<float> SECONDS_BETWEEN_COMMENTS	("ScreenGameplay","SecondsBetweenComments");
static ThemeMetric<float> TICK_EARLY_SECONDS		("ScreenGameplay","TickEarlySeconds");

/* Global, so it's accessible from ShowSavePrompt: */
static float g_fOldOffset;  // used on offset screen to calculate difference

const ScreenMessage	SM_PlayReady			= ScreenMessage(SM_User+0);
const ScreenMessage	SM_PlayGo				= ScreenMessage(SM_User+1);


// received while STATE_DANCING
const ScreenMessage	SM_LoadNextSong			= ScreenMessage(SM_User+11);

// received while STATE_OUTRO
const ScreenMessage	SM_SaveChangedBeforeGoingBack	= ScreenMessage(SM_User+20);
const ScreenMessage	SM_GoToScreenAfterBack	= ScreenMessage(SM_User+21);

const ScreenMessage	SM_BeginFailed			= ScreenMessage(SM_User+30);

// received while STATE_INTRO
const ScreenMessage	SM_StartHereWeGo		= ScreenMessage(SM_User+40);
const ScreenMessage	SM_StopHereWeGo			= ScreenMessage(SM_User+41);

static Preference<float> g_fNetStartOffset( Options, "NetworkStartOffset",	-3.0 );

REGISTER_SCREEN_CLASS( ScreenGameplay );
ScreenGameplay::ScreenGameplay( CString sName ) : Screen(sName)
{
	PLAYER_TYPE.Load( sName, "PlayerType" );
	GIVE_UP_TEXT.Load( sName, "GiveUpText" );
	GIVE_UP_ABORTED_TEXT.Load( sName, "GiveUpAbortedText" );
	START_GIVES_UP.Load( sName, "StartGivesUp" );
	BACK_GIVES_UP.Load( sName, "BackGivesUp" );
	GIVING_UP_FAILS.Load( sName, "GivingUpFails" );
}

void ScreenGameplay::Init()
{
	Screen::Init();

	/* Pause MEMCARDMAN.  If a memory card is remove, we don't want to interrupt the
	 * player by making a noise until the game finishes. */
	if( !GAMESTATE->m_bDemonstrationOrJukebox )
		MEMCARDMAN->PauseMountingThread();

	if( GAMESTATE->m_bDemonstrationOrJukebox )
		LIGHTSMAN->SetLightsMode( LIGHTSMODE_DEMONSTRATION );
	else
		LIGHTSMAN->SetLightsMode( LIGHTSMODE_GAMEPLAY );

	m_pSoundMusic = NULL;
	m_bPaused = false;

	/* We do this ourself. */
	SOUND->HandleSongTimer( false );

	//need to initialize these before checking for demonstration mode
	//otherwise destructor will try to delete possibly invalid pointers

    FOREACH_PlayerNumber(p)
	{
		m_pLifeMeter[p] = NULL;
		m_pPrimaryScoreDisplay[p] = NULL;
		m_pSecondaryScoreDisplay[p] = NULL;
		m_pPrimaryScoreKeeper[p] = NULL;
		m_pSecondaryScoreKeeper[p] = NULL;
		m_pInventory[p] = NULL ;
	}
	m_pCombinedLifeMeter = NULL;

	if( GAMESTATE->m_pCurSong == NULL && GAMESTATE->m_pCurCourse == NULL )
		return;	// ScreenDemonstration will move us to the next screen.  We just need to survive for one update without crashing.

	/* This is usually done already, but we might have come here without going through
	 * ScreenSelectMusic or the options menus at all. */
	GAMESTATE->AdjustFailType();

	/* Save selected options before we change them. */
	GAMESTATE->StoreSelectedOptions();

	/* Save settings to the profile now.  Don't do this on extra stages, since the
	 * user doesn't have full control; saving would force profiles to DIFFICULTY_HARD
	 * and save over their default modifiers every time someone got an extra stage.
	 * Do this before course modifiers are set up. */
	if( !GAMESTATE->IsExtraStage() && !GAMESTATE->IsExtraStage2() )
	{
		FOREACH_HumanPlayer( pn )
			GAMESTATE->SaveCurrentSettingsToProfile(pn);
	}

	/* Called once per stage (single song or single course). */
	GAMESTATE->BeginStage();




	// fill in difficulty of CPU players with that of the first human player
    FOREACH_PotentialCpuPlayer(p)
        GAMESTATE->m_pCurSteps[p].Set( GAMESTATE->m_pCurSteps[ GAMESTATE->GetFirstHumanPlayer() ] );

	switch( GAMESTATE->m_PlayMode )
	{
	case PLAY_MODE_BATTLE:
	case PLAY_MODE_RAVE:
		{
			// cache NoteSkin graphics
			/* XXX: is this still needed?  We explicitly preload noteskins that can be
			 * used as attacks (GameState::GetAllUsedNoteSkins). */
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
	// fill in m_apSongsQueue, m_vpStepsQueue, m_asModifiersQueue
	//
	InitSongQueues();

	/* Increment the course play count. */
	if( GAMESTATE->IsCourseMode() && !GAMESTATE->m_bDemonstrationOrJukebox )
		FOREACH_EnabledPlayer(p)
			PROFILEMAN->IncrementCoursePlayCount( GAMESTATE->m_pCurCourse, GAMESTATE->m_pCurTrail[p], p );

	STATSMAN->m_CurStageStats.playMode = GAMESTATE->m_PlayMode;
	STATSMAN->m_CurStageStats.pStyle = GAMESTATE->m_pCurStyle;

    FOREACH_EnabledPlayer(p)
	{
		ASSERT( !m_vpStepsQueue[p].empty() );

		/* Record combo rollover. */
		STATSMAN->m_CurStageStats.m_player[p].UpdateComboList( 0, true );
	}

	if( GAMESTATE->IsExtraStage() )
		STATSMAN->m_CurStageStats.StageType = StageStats::STAGE_EXTRA;
	else if( GAMESTATE->IsExtraStage2() )
		STATSMAN->m_CurStageStats.StageType = StageStats::STAGE_EXTRA2;
	else
		STATSMAN->m_CurStageStats.StageType = StageStats::STAGE_NORMAL;
	
	//
	// Init ScoreKeepers
	//

    FOREACH_EnabledPlayer(p)
	{
        switch( PREFSMAN->m_iScoringType )
		{
		case PrefsManager::SCORING_MAX2:
		case PrefsManager::SCORING_5TH:
			m_pPrimaryScoreKeeper[p] = new ScoreKeeperMAX2( 
				m_apSongsQueue, 
				m_vpStepsQueue[p], 
				m_asModifiersQueue[p], 
				GAMESTATE->m_pPlayerState[p], 
				&STATSMAN->m_CurStageStats.m_player[p] );
			break;
		default: ASSERT(0);
		}

		switch( GAMESTATE->m_PlayMode )
		{
		case PLAY_MODE_RAVE:
			m_pSecondaryScoreKeeper[p] = new ScoreKeeperRave( 
				GAMESTATE->m_pPlayerState[p],
				&STATSMAN->m_CurStageStats.m_player[p] );
			break;
		}
	}

	m_bChangedOffsetOrBPM = GAMESTATE->m_SongOptions.m_bAutoSync;

	m_DancingState = STATE_INTRO;

	// Set this in LoadNextSong()
	//m_fTimeLeftBeforeDancingComment = SECONDS_BETWEEN_COMMENTS;
	
	m_bZeroDeltaOnNextUpdate = false;
	
	// init old offset in case offset changes in song
	if( GAMESTATE->IsCourseMode() )
		g_fOldOffset = -1000;
	else
		g_fOldOffset = GAMESTATE->m_pCurSong->m_Timing.m_fBeat0OffsetInSeconds;



	m_SongBackground.SetName( "SongBackground" );
	m_SongBackground.SetDrawOrder( DRAW_ORDER_BEFORE_EVERYTHING );
	ON_COMMAND( m_SongBackground );
	this->AddChild( &m_SongBackground );

	m_SongForeground.SetName( "SongForeground" );
	m_SongForeground.SetDrawOrder( DRAW_ORDER_OVERLAY+1 );	// on top of the overlay, but under transitions
	ON_COMMAND( m_SongBackground );
	this->AddChild( &m_SongForeground );

	if( PREFSMAN->m_bShowBeginnerHelper )
	{
		m_BeginnerHelper.SetDrawOrder( DRAW_ORDER_BEFORE_EVERYTHING );
		m_BeginnerHelper.SetXY( SCREEN_CENTER_X, SCREEN_CENTER_Y );
		this->AddChild( &m_BeginnerHelper );
	}
	
	m_sprStaticBackground.SetName( "StaticBG" );
	m_sprStaticBackground.Load( THEME->GetPathG(m_sName,"Static Background") );
	SET_XY( m_sprStaticBackground );
	m_sprStaticBackground.SetDrawOrder( DRAW_ORDER_BEFORE_EVERYTHING );	// behind everything else
	this->AddChild(&m_sprStaticBackground);

	if( !GAMESTATE->m_bDemonstrationOrJukebox )	// only load if we're going to use it
	{
		m_Toasty.Load( THEME->GetPathB(m_sName,"toasty") );
		this->AddChild( &m_Toasty );
	}

    FOREACH_EnabledPlayer(p)
	{
		float fPlayerX = PLAYER_X( p, GAMESTATE->GetCurrentStyle()->m_StyleType );

		/* Perhaps this should be handled better by defining a new
		 * StyleType for ONE_PLAYER_ONE_CREDIT_AND_ONE_COMPUTER,
		 * but for now just ignore SoloSingles when it's Battle or Rave
		 * Mode.  This doesn't begin to address two-player solo (6 arrows) */
		if( PREFSMAN->m_bSoloSingle && 
			GAMESTATE->m_PlayMode != PLAY_MODE_BATTLE &&
			GAMESTATE->m_PlayMode != PLAY_MODE_RAVE &&
			GAMESTATE->GetCurrentStyle()->m_StyleType == ONE_PLAYER_ONE_SIDE )
			fPlayerX = SCREEN_CENTER_X;

		m_Player[p].SetName( ssprintf("Player%i", p+1) );
		m_Player[p].SetX( fPlayerX );
		m_Player[p].SetY( SCREEN_CENTER_Y );
		this->AddChild( &m_Player[p] );
	
		m_sprOniGameOver[p].SetName( ssprintf("OniGameOver%i", p+1) );
		m_sprOniGameOver[p].Load( THEME->GetPathG(m_sName,"oni gameover") );
		m_sprOniGameOver[p].SetX( fPlayerX );
		m_sprOniGameOver[p].SetY( SCREEN_TOP - m_sprOniGameOver[p].GetZoomedHeight()/2 );
		m_sprOniGameOver[p].SetDiffuse( RageColor(1,1,1,0) );	// 0 alpha so we don't waste time drawing while not visible
		this->AddChild( &m_sprOniGameOver[p] );
	}

	m_NextSongIn.SetDrawOrder( DRAW_ORDER_TRANSITIONS-1 );
	this->AddChild( &m_NextSongIn );

	m_NextSongOut.SetDrawOrder( DRAW_ORDER_TRANSITIONS-1 );
	this->AddChild( &m_NextSongOut );

	m_SongFinished.SetDrawOrder( DRAW_ORDER_TRANSITIONS-1 );
	this->AddChild( &m_SongFinished );


	bool bBattery = GAMESTATE->m_SongOptions.m_LifeType==SongOptions::LIFE_BATTERY;

	//
	// Add LifeFrame
	//
	m_sprLifeFrame.Load( THEME->GetPathG(m_sName,bBattery?"oni life frame":"life frame") );
	m_sprLifeFrame.SetName( "LifeFrame" );
	SET_XY( m_sprLifeFrame );
	this->AddChild( &m_sprLifeFrame );

	//
	// Add score frame
	//
	m_sprScoreFrame.Load( THEME->GetPathG(m_sName,bBattery?"oni score frame":"score frame") );
	m_sprScoreFrame.SetName( "ScoreFrame" );
	SET_XY( m_sprScoreFrame );
	this->AddChild( &m_sprScoreFrame );


	//
	// Add combined life meter
	//
	switch( GAMESTATE->m_PlayMode )
	{
	case PLAY_MODE_BATTLE:
	case PLAY_MODE_RAVE:
		m_pCombinedLifeMeter = new CombinedLifeMeterTug;
		m_pCombinedLifeMeter->SetName( "CombinedLife" );
		SET_XY( *m_pCombinedLifeMeter );
		this->AddChild( m_pCombinedLifeMeter );		
		break;
	}

	//
	// Before the lifemeter loads, if Networking is required
	// we need to wait, so that there is no Dead On Start issues.
	// if you wait too long at the second checkpoint, you will
	// appear dead when you begin your game.
	//
	NSMAN->StartRequest(0); 



	//
	// Add individual life meter
	//
	switch( GAMESTATE->m_PlayMode )
	{
	case PLAY_MODE_REGULAR:
	case PLAY_MODE_ONI:
	case PLAY_MODE_NONSTOP:
	case PLAY_MODE_ENDLESS:
        FOREACH_PlayerNumber(p)
		{
			if( !GAMESTATE->IsPlayerEnabled(p) && !SHOW_LIFE_METER_FOR_DISABLED_PLAYERS )
				continue;	// skip

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

			m_pLifeMeter[p]->Load( p );
			m_pLifeMeter[p]->SetName( ssprintf("LifeP%d",p+1) );
			SET_XY( *m_pLifeMeter[p] );
			this->AddChild( m_pLifeMeter[p] );		
		}
		break;
	case PLAY_MODE_BATTLE:
	case PLAY_MODE_RAVE:
		break;
	}

	m_ShowScoreboard=false;

	//the following is only used in SMLAN/SMOnline
	if( NSMAN->useSMserver )
	{
		PlayerNumber pn = GAMESTATE->GetFirstDisabledPlayer();
		if( pn != PLAYER_INVALID )
		{
			FOREACH_NSScoreBoardColumn( col )
			{
				m_Scoreboard[col].LoadFromFont( THEME->GetPathF(m_sName,"scoreboard") );
				m_Scoreboard[col].SetShadowLength( 0 );
				m_Scoreboard[col].SetName( ssprintf("ScoreboardC%iP%i",col+1,pn+1) );
				SET_XY( m_Scoreboard[col] );
				this->AddChild( &m_Scoreboard[col] );
				m_Scoreboard[col].SetText( NSMAN->m_Scoreboard[col] );
				m_Scoreboard[col].SetVertAlign( align_top );
				m_ShowScoreboard = true;
			}
		}
	}

	m_MaxCombo.LoadFromFont( THEME->GetPathF(m_sName,"max combo") );
	m_MaxCombo.SetName( "MaxCombo" );
	SET_XY( m_MaxCombo );
	m_MaxCombo.SetText( ssprintf("%d", STATSMAN->m_CurStageStats.m_player[GAMESTATE->m_MasterPlayerNumber].iMaxCombo) ); // TODO: Make this work for both players
	this->AddChild( &m_MaxCombo );

    FOREACH_EnabledPlayer(p)
	{
		//
		// primary score display
		//
		switch( GAMESTATE->m_PlayMode )
		{
		case PLAY_MODE_REGULAR:
		case PLAY_MODE_NONSTOP:
		case PLAY_MODE_BATTLE:
		case PLAY_MODE_RAVE:
			if( PREFSMAN->m_bPercentageScoring )
				m_pPrimaryScoreDisplay[p] = new ScoreDisplayPercentage;
			else
				m_pPrimaryScoreDisplay[p] = new ScoreDisplayNormal;
			break;
		case PLAY_MODE_ONI:
		case PLAY_MODE_ENDLESS:
			m_pPrimaryScoreDisplay[p] = new ScoreDisplayOni;
			break;
		default:
			ASSERT(0);
		}

		m_pPrimaryScoreDisplay[p]->Init( GAMESTATE->m_pPlayerState[p] );
		m_pPrimaryScoreDisplay[p]->SetName( ssprintf("ScoreP%d",p+1) );
		SET_XY( *m_pPrimaryScoreDisplay[p] );
		if( GAMESTATE->m_PlayMode != PLAY_MODE_RAVE || SHOW_SCORE_IN_RAVE ) /* XXX: ugly */
			this->AddChild( m_pPrimaryScoreDisplay[p] );

	
		//
		// secondary score display
		//
		switch( GAMESTATE->m_PlayMode )
		{
		case PLAY_MODE_RAVE:
			m_pSecondaryScoreDisplay[p] = new ScoreDisplayRave;
			break;
		}

		if( m_pSecondaryScoreDisplay[p] )
		{
			m_pSecondaryScoreDisplay[p]->Init( GAMESTATE->m_pPlayerState[p] );
			m_pSecondaryScoreDisplay[p]->SetName( ssprintf("SecondaryScoreP%d",p+1) );
			SET_XY( *m_pSecondaryScoreDisplay[p] );
			this->AddChild( m_pSecondaryScoreDisplay[p] );
		}
	}
	
	//
	// Add stage / SongNumber
	//
	m_sprStage.SetName( "Stage" );
	SET_XY( m_sprStage );

	m_sprCourseSongNumber.SetName( "CourseSongNumber" );
	SET_XY( m_sprCourseSongNumber );
	
	FOREACH_EnabledPlayer(p)
	{
		m_textCourseSongNumber[p].LoadFromFont( THEME->GetPathF(m_sName,"song num") );
		m_textCourseSongNumber[p].SetShadowLength( 0 );
		m_textCourseSongNumber[p].SetName( ssprintf("SongNumberP%d",p+1) );
		SET_XY( m_textCourseSongNumber[p] );
		m_textCourseSongNumber[p].SetText( "" );
		m_textCourseSongNumber[p].SetDiffuse( RageColor(0,0.5f,1,1) );	// light blue
	}

	FOREACH_EnabledPlayer(p)
	{
		m_textStepsDescription[p].LoadFromFont( THEME->GetPathF(m_sName,"StepsDescription") );
		m_textStepsDescription[p].SetName( ssprintf("StepsDescriptionP%i",p+1) );
		SET_XY( m_textStepsDescription[p] );
		this->AddChild( &m_textStepsDescription[p] );
	}

	switch( GAMESTATE->m_PlayMode )
	{
	case PLAY_MODE_REGULAR:
	case PLAY_MODE_BATTLE:
	case PLAY_MODE_RAVE:
		m_sprStage.Load( THEME->GetPathG(m_sName,"stage "+GAMESTATE->GetStageText()) );
		this->AddChild( &m_sprStage );
		break;
	case PLAY_MODE_NONSTOP:
	case PLAY_MODE_ONI:
	case PLAY_MODE_ENDLESS:
		this->AddChild( &m_sprCourseSongNumber );

        FOREACH_EnabledPlayer( p )
			this->AddChild( &m_textCourseSongNumber[p] );
		break;
	default:
		ASSERT(0);	// invalid GameMode
	}


	m_sprStageFrame.Load( THEME->GetPathG(m_sName,"stage frame") );
	m_sprStageFrame->SetName( "StageFrame" );
	SET_XY( m_sprStageFrame );
	this->AddChild( m_sprStageFrame );

	//
	// Player/Song options
	//
    FOREACH_EnabledPlayer(p)
	{
		m_textPlayerOptions[p].LoadFromFont( THEME->GetPathF(m_sName,"player options") );
		m_textPlayerOptions[p].SetShadowLength( 0 );
		m_textPlayerOptions[p].SetName( ssprintf("PlayerOptionsP%d",p+1) );
		SET_XY( m_textPlayerOptions[p] );
		this->AddChild( &m_textPlayerOptions[p] );
	}

	m_textSongOptions.LoadFromFont( THEME->GetPathF(m_sName,"song options") );
	m_textSongOptions.SetShadowLength( 0 );
	m_textSongOptions.SetName( "SongOptions" );
	SET_XY( m_textSongOptions );
	m_textSongOptions.SetText( GAMESTATE->m_SongOptions.GetString() );
	this->AddChild( &m_textSongOptions );

	FOREACH_EnabledPlayer( pn )
	{
		m_ActiveAttackList[pn].LoadFromFont( THEME->GetPathF(m_sName,"ActiveAttackList") );
		m_ActiveAttackList[pn].Init( GAMESTATE->m_pPlayerState[pn] );
		m_ActiveAttackList[pn].SetName( ssprintf("ActiveAttackListP%d",pn+1) );
		SET_XY( m_ActiveAttackList[pn] );
		this->AddChild( &m_ActiveAttackList[pn] );
	}



    FOREACH_EnabledPlayer(p)
	{
		m_DifficultyIcon[p].Load( THEME->GetPathG(m_sName,ssprintf("difficulty icons %dx%d",NUM_PLAYERS,NUM_DIFFICULTIES)) );
		/* Position it in LoadNextSong. */
		this->AddChild( &m_DifficultyIcon[p] );

		m_DifficultyMeter[p].Load( m_sName + ssprintf(" DifficultyMeterP%d",p+1) );
		/* Position it in LoadNextSong. */
		this->AddChild( &m_DifficultyMeter[p] );
	}


	if( PREFSMAN->m_bShowLyrics )
		this->AddChild( &m_LyricDisplay );
	

	m_textAutoPlay.LoadFromFont( THEME->GetPathF(m_sName,"autoplay") );
	m_textAutoPlay.SetName( "AutoPlay" );
	SET_XY( m_textAutoPlay );
	if( !GAMESTATE->m_bDemonstrationOrJukebox )	// only load if we're not in demonstration or jukebox
		this->AddChild( &m_textAutoPlay );
	UpdateAutoPlayText();
	

	m_BPMDisplay.SetName( "BPMDisplay" );
	m_BPMDisplay.Load();
	SET_XY( m_BPMDisplay );
	this->AddChild( &m_BPMDisplay );
	m_fLastBPS = 0;

	ZERO( m_pInventory );
    FOREACH_PlayerNumber(p)
	{
//		switch( GAMESTATE->m_PlayMode )
//		{
//		case PLAY_MODE_BATTLE:
//			m_pInventory[p] = new Inventory;
//			m_pInventory[p]->Load( p );
//			this->AddChild( m_pInventory[p] );
//			break;
//		}
	}

	m_Overlay.Load( THEME->GetPathB(m_sName,"overlay") );
	m_Overlay->SetDrawOrder( DRAW_ORDER_OVERLAY );
	this->AddChild( m_Overlay );

	
	if( !GAMESTATE->m_bDemonstrationOrJukebox )	// only load if we're going to use it
	{
		m_Ready.Load( THEME->GetPathB(m_sName,"ready") );
		this->AddChild( &m_Ready );

		m_Go.Load( THEME->GetPathB(m_sName,"go") );
		this->AddChild( &m_Go );

		m_Cleared.Load( THEME->GetPathB(m_sName,"cleared") );
		m_Cleared.SetDrawOrder( DRAW_ORDER_TRANSITIONS-1 ); // on top of everything else
		this->AddChild( &m_Cleared );

		m_Failed.Load( THEME->GetPathB(m_sName,"failed") );
		m_Failed.SetDrawOrder( DRAW_ORDER_TRANSITIONS-1 ); // on top of everything else
		this->AddChild( &m_Failed );

		if( PREFSMAN->m_bAllowExtraStage && GAMESTATE->IsFinalStage() )	// only load if we're going to use it
			m_Extra.Load( THEME->GetPathB(m_sName,"extra1") );
		if( PREFSMAN->m_bAllowExtraStage && GAMESTATE->IsExtraStage() )	// only load if we're going to use it
			m_Extra.Load( THEME->GetPathB(m_sName,"extra2") );
		this->AddChild( &m_Extra );

		// only load if we're going to use it
		switch( GAMESTATE->m_PlayMode )
		{
		case PLAY_MODE_BATTLE:
		case PLAY_MODE_RAVE:
            FOREACH_PlayerNumber(p)
			{
				m_Win[p].Load( THEME->GetPathB(m_sName,ssprintf("win p%d",p+1)) );
				this->AddChild( &m_Win[p] );
			}
			m_Draw.Load( THEME->GetPathB(m_sName,"draw") );
			this->AddChild( &m_Draw );
			break;
		}

		m_textDebug.LoadFromFont( THEME->GetPathF("Common","normal") );
		m_textDebug.SetName( "Debug" );
		SET_XY( m_textDebug );
		m_textDebug.SetDrawOrder( DRAW_ORDER_TRANSITIONS-1 );	// just under transitions, over the foreground
		this->AddChild( &m_textDebug );

		m_In.Load( THEME->GetPathB(m_sName,"in") );
		m_In.SetDrawOrder( DRAW_ORDER_TRANSITIONS );
		this->AddChild( &m_In );

		m_Back.Load( THEME->GetPathB("Common","back") );
		m_Back.SetDrawOrder( DRAW_ORDER_TRANSITIONS ); // on top of everything else
		this->AddChild( &m_Back );


		if( GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2() )	// only load if we're going to use it
		{
			m_textSurviveTime.LoadFromFont( THEME->GetPathF(m_sName,"survive time") );
			m_textSurviveTime.SetShadowLength( 0 );
			m_textSurviveTime.SetName( "SurviveTime" );
			SET_XY( m_textSurviveTime );
			m_textSurviveTime.SetDrawOrder( DRAW_ORDER_TRANSITIONS-1 );
			m_textSurviveTime.SetDiffuse( RageColor(1,1,1,0) );
			this->AddChild( &m_textSurviveTime );
		}
	}


	/* LoadNextSong first, since that positions some elements which need to be
	 * positioned before we TweenOnScreen. */
	LoadNextSong();

	TweenOnScreen();

	this->SortByDrawOrder();

	if( !GAMESTATE->m_bDemonstrationOrJukebox )	// only load if we're going to use it
	{
		m_soundAssistTick.Load(	THEME->GetPathS(m_sName,"assist tick"), true );

		switch( GAMESTATE->m_PlayMode )
		{
		case PLAY_MODE_BATTLE:
			m_soundBattleTrickLevel1.Load(	THEME->GetPathS(m_sName,"battle trick level1"), true );
			m_soundBattleTrickLevel2.Load(	THEME->GetPathS(m_sName,"battle trick level2"), true );
			m_soundBattleTrickLevel3.Load(	THEME->GetPathS(m_sName,"battle trick level3"), true );
			break;
		}
	}

	m_GiveUpTimer.SetZero();

	// Get the transitions rolling on the first update.
	// We can't do this in the constructor because ScreenGameplay is constructed 
	// in the middle of ScreenStage.
}

//
// fill in m_apSongsQueue, m_vpStepsQueue, m_asModifiersQueue
//
void ScreenGameplay::InitSongQueues()
{
	LOG->Trace("InitSongQueues");
	if( GAMESTATE->IsCourseMode() )
	{
		Course* pCourse = GAMESTATE->m_pCurCourse;
		ASSERT( pCourse );

		m_apSongsQueue.clear();
		PlayerNumber pnMaster = GAMESTATE->m_MasterPlayerNumber;
		Trail *pTrail = GAMESTATE->m_pCurTrail[pnMaster];
		ASSERT( pTrail );
		FOREACH_CONST( TrailEntry, pTrail->m_vEntries, e )
		{
			ASSERT( e->pSong );
			m_apSongsQueue.push_back( e->pSong );
		}

        FOREACH_EnabledPlayer(p)
		{
			Trail *pTrail = GAMESTATE->m_pCurTrail[p];
			ASSERT( pTrail );

			m_vpStepsQueue[p].clear();
			m_asModifiersQueue[p].clear();
			FOREACH_CONST( TrailEntry, pTrail->m_vEntries, e )
			{
				ASSERT( e->pSteps );
				m_vpStepsQueue[p].push_back( e->pSteps );
				AttackArray a;
				e->GetAttackArray( a );
				m_asModifiersQueue[p].push_back( a );
			}
		}
	}
	else
	{
		m_apSongsQueue.push_back( GAMESTATE->m_pCurSong );
        FOREACH_PlayerNumber(p)
		{
			m_vpStepsQueue[p].push_back( GAMESTATE->m_pCurSteps[p] );
			m_asModifiersQueue[p].push_back( AttackArray() );
		}
	}
}

ScreenGameplay::~ScreenGameplay()
{
	if( this->IsFirstUpdate() )
	{
		/* We never received any updates.  That means we were deleted without being
		 * used, and never actually played.  (This can happen when backing out of
		 * ScreenStage.)  Cancel the stage. */
		GAMESTATE->CancelStage();
	}

	LOG->Trace( "ScreenGameplay::~ScreenGameplay()" );
	
	if( !GAMESTATE->m_bDemonstrationOrJukebox )
		MEMCARDMAN->UnPauseMountingThread();

    FOREACH_PlayerNumber(p)
	{
		SAFE_DELETE( m_pLifeMeter[p] );
		SAFE_DELETE( m_pPrimaryScoreDisplay[p] );
		SAFE_DELETE( m_pSecondaryScoreDisplay[p] );
		SAFE_DELETE( m_pSecondaryScoreDisplay[p] );
		SAFE_DELETE( m_pPrimaryScoreKeeper[p] );
		SAFE_DELETE( m_pSecondaryScoreKeeper[p] );
		SAFE_DELETE( m_pInventory[p] );
	}
	SAFE_DELETE( m_pCombinedLifeMeter );
	if( m_pSoundMusic )
		m_pSoundMusic->StopPlaying();

	m_soundAssistTick.StopPlaying(); /* Stop any queued assist ticks. */

	NSMAN->ReportSongOver();
}

bool ScreenGameplay::IsLastSong()
{
	if( GAMESTATE->m_pCurCourse  &&  GAMESTATE->m_pCurCourse->m_bRepeat )
		return false;
	return GAMESTATE->GetCourseSongIndex()+1 == (int)m_apSongsQueue.size(); // GetCourseSongIndex() is 0-based but size() is not
}

void ScreenGameplay::SetupSong( PlayerNumber p, int iSongIndex )
{
	/* This is the first beat that can be changed without it being visible.  Until
	 * we draw for the first time, any beat can be changed. */
	GAMESTATE->m_pPlayerState[p]->m_fLastDrawnBeat = -100;
	GAMESTATE->m_pCurSteps[p].Set( m_vpStepsQueue[p][iSongIndex] );

	/* Load new NoteData into Player.  Do this before 
	 * RebuildPlayerOptionsFromActiveAttacks or else transform mods will get
	 * propogated to GAMESTATE->m_PlayerOptions too early and be double-applied
	 * to the NoteData:
	 * once in Player::Load, then again in Player::ApplyActiveAttacks.  This 
	 * is very bad for transforms like AddMines.
	 */
	NoteData originalNoteData;
	GAMESTATE->m_pCurSteps[p]->GetNoteData( originalNoteData );
	
	const Style* pStyle = GAMESTATE->GetCurrentStyle();
	NoteData ndTransformed;
	pStyle->GetTransformedNoteDataForStyle( p, originalNoteData, ndTransformed );

	// load player
	{
		NoteData nd = ndTransformed;
		NoteDataUtil::RemoveAllTapsOfType( nd, TapNote::autoKeysound );
		m_Player[p].Init( 
			PLAYER_TYPE,
			GAMESTATE->m_pPlayerState[p], 
			&STATSMAN->m_CurStageStats.m_player[p],
			m_pLifeMeter[p], 
			m_pCombinedLifeMeter, 
			m_pPrimaryScoreDisplay[p], 
			m_pSecondaryScoreDisplay[p], 
			m_pInventory[p], 
			m_pPrimaryScoreKeeper[p], 
			m_pSecondaryScoreKeeper[p] );
		m_Player[p].Load( nd );
	}

	// load auto keysounds
	{
		NoteData nd = ndTransformed;
		NoteDataUtil::RemoveAllTapsExceptForType( nd, TapNote::autoKeysound );
		m_AutoKeysounds.Load( p, nd );
	}


	// Put course options into effect.  Do this after Player::Load so
	// that mods aren't double-applied.
	GAMESTATE->m_pPlayerState[p]->m_ModsToApply.clear();
	for( unsigned i=0; i<m_asModifiersQueue[p][iSongIndex].size(); ++i )
	{
		Attack a = m_asModifiersQueue[p][iSongIndex][i];
		if( a.fStartSecond == 0 )
			a.fStartSecond = -1;	// now
		
		GAMESTATE->LaunchAttack( p, a );
		GAMESTATE->m_SongOptions.FromString( a.sModifier );
	}

	// UGLY: Force updating the BeatToNoteSkin mapping and cache NoteSkins now, or else 
	// we'll do it on the first update and skip.
	m_Player[p].ApplyWaitingTransforms();

	/* Now that course options are applied, load any needed note skins and unload old ones. */
	m_Player[p].CacheAllUsedNoteSkins( true );

	/* Update attack bOn flags. */
	GAMESTATE->Update(0);
	GAMESTATE->RebuildPlayerOptionsFromActiveAttacks( p );

	/* Hack: Course modifiers that are set to start immediately shouldn't tween on. */
	GAMESTATE->m_pPlayerState[p]->m_CurrentPlayerOptions = GAMESTATE->m_pPlayerState[p]->m_PlayerOptions;
}

void ScreenGameplay::LoadCourseSongNumber( int SongNumber )
{
	if( !GAMESTATE->IsCourseMode() )
		return;

	const CString path = THEME->GetPathG( m_sName, ssprintf("course song %i",SongNumber+1), true );
	if( path != "" )
		m_sprCourseSongNumber.Load( path );
	else
		m_sprCourseSongNumber.UnloadTexture();
}

void ScreenGameplay::LoadNextSong()
{
	GAMESTATE->ResetMusicStatistics();

	FOREACH_EnabledPlayer( p )
	{
		STATSMAN->m_CurStageStats.m_player[p].iSongsPlayed++;
		m_textCourseSongNumber[p].SetText( ssprintf("%d", STATSMAN->m_CurStageStats.m_player[p].iSongsPlayed) );
	}

	LoadCourseSongNumber( GAMESTATE->GetCourseSongIndex() );

	int iPlaySongIndex = GAMESTATE->GetCourseSongIndex();
	iPlaySongIndex %= m_apSongsQueue.size();
	GAMESTATE->m_pCurSong.Set( m_apSongsQueue[iPlaySongIndex] );
	STATSMAN->m_CurStageStats.vpSongs.push_back( GAMESTATE->m_pCurSong );

	// No need to do this here.  We do it in SongFinished().
	//GAMESTATE->RemoveAllActiveAttacks();

	// Restore the player's originally selected options.
	GAMESTATE->RestoreSelectedOptions();

	/* If we're in battery mode, force FailImmediate.  We assume in PlayerMinus::Step that
	 * failed players can't step. */
	if( GAMESTATE->m_SongOptions.m_LifeType == SongOptions::LIFE_BATTERY )
		GAMESTATE->m_SongOptions.m_FailType = SongOptions::FAIL_IMMEDIATE;

	m_textSongOptions.SetText( GAMESTATE->m_SongOptions.GetString() );

	FOREACH_EnabledPlayer( p )
	{
		SetupSong( p, iPlaySongIndex );

		Song* pSong = GAMESTATE->m_pCurSong;
		Steps* pSteps = GAMESTATE->m_pCurSteps[p];
		STATSMAN->m_CurStageStats.m_player[p].vpSteps.push_back( pSteps );

		ASSERT( GAMESTATE->m_pCurSteps[p] );
		m_textStepsDescription[p].SetText( GAMESTATE->m_pCurSteps[p]->GetDescription() );

		/* Increment the play count even if the player fails.  (It's still popular,
		 * even if the people playing it aren't good at it.) */
		if( !GAMESTATE->m_bDemonstrationOrJukebox )
			PROFILEMAN->IncrementStepsPlayCount( pSong, pSteps, p );

		m_textPlayerOptions[p].SetText( GAMESTATE->m_pPlayerState[p]->m_PlayerOptions.GetString() );
		m_ActiveAttackList[p].Refresh();

		// reset oni game over graphic
		m_sprOniGameOver[p].SetY( SCREEN_TOP - m_sprOniGameOver[p].GetZoomedHeight()/2 );
		m_sprOniGameOver[p].SetDiffuse( RageColor(1,1,1,0) );	// 0 alpha so we don't waste time drawing while not visible

		if( GAMESTATE->m_SongOptions.m_LifeType==SongOptions::LIFE_BATTERY && STATSMAN->m_CurStageStats.m_player[p].bFailed )	// already failed
			ShowOniGameOver(p);

		if( GAMESTATE->m_SongOptions.m_LifeType==SongOptions::LIFE_BAR && GAMESTATE->m_PlayMode == PLAY_MODE_REGULAR && !GAMESTATE->GetEventMode() && !GAMESTATE->m_bDemonstrationOrJukebox)
		{
			m_pLifeMeter[p]->UpdateNonstopLifebar(
				GAMESTATE->GetStageIndex(), 
				PREFSMAN->m_iNumArcadeStages, 
				PREFSMAN->m_iProgressiveStageLifebar);
		}
		if( GAMESTATE->m_SongOptions.m_LifeType==SongOptions::LIFE_BAR && GAMESTATE->m_PlayMode == PLAY_MODE_NONSTOP )
		{
			m_pLifeMeter[p]->UpdateNonstopLifebar(
				GAMESTATE->GetCourseSongIndex(), 
				GAMESTATE->m_pCurCourse->GetEstimatedNumStages(),
				PREFSMAN->m_iProgressiveNonstopLifebar);
		}

		m_DifficultyIcon[p].SetFromSteps( p, GAMESTATE->m_pCurSteps[p] );

		m_DifficultyMeter[p].SetName( m_sName + ssprintf(" DifficultyMeterP%d",p+1) );
		m_DifficultyMeter[p].SetFromSteps( GAMESTATE->m_pCurSteps[p] );

		/* The actual note data for scoring is the base class of Player.  This includes
		 * transforms, like Wide.  Otherwise, the scoring will operate on the wrong data. */
		m_pPrimaryScoreKeeper[p]->OnNextSong( GAMESTATE->GetCourseSongIndex(), GAMESTATE->m_pCurSteps[p], &m_Player[p].m_NoteData );
		if( m_pSecondaryScoreKeeper[p] )
			m_pSecondaryScoreKeeper[p]->OnNextSong( GAMESTATE->GetCourseSongIndex(), GAMESTATE->m_pCurSteps[p], &m_Player[p].m_NoteData );

		if( GAMESTATE->m_bDemonstrationOrJukebox )
		{
			GAMESTATE->m_pPlayerState[p]->m_PlayerController = PC_CPU;
			GAMESTATE->m_pPlayerState[p]->m_iCpuSkill = 5;
		}
		else if( GAMESTATE->IsCpuPlayer(p) )
		{
			GAMESTATE->m_pPlayerState[p]->m_PlayerController = PC_CPU;
			int iMeter = GAMESTATE->m_pCurSteps[p]->GetMeter();
			int iNewSkill = SCALE( iMeter, MIN_METER, MAX_METER, 0, NUM_SKILL_LEVELS-1 );
			/* Watch out: songs aren't actually bound by MAX_METER. */
			iNewSkill = clamp( iNewSkill, 0, NUM_SKILL_LEVELS-1 );
			GAMESTATE->m_pPlayerState[p]->m_iCpuSkill = iNewSkill;
		}
		else if( PREFSMAN->m_bAutoPlay )
		{
			GAMESTATE->m_pPlayerState[p]->m_PlayerController = PC_AUTOPLAY;
		}
		else
		{
			GAMESTATE->m_pPlayerState[p]->m_PlayerController = PC_HUMAN;
		}
	}

	m_AutoKeysounds.FinishLoading();
	m_pSoundMusic = m_AutoKeysounds.GetSound();

	const bool bReverse[NUM_PLAYERS] = 
	{
		GAMESTATE->m_pPlayerState[PLAYER_1]->m_PlayerOptions.m_fScrolls[PlayerOptions::SCROLL_REVERSE] == 1,
		GAMESTATE->m_pPlayerState[PLAYER_2]->m_PlayerOptions.m_fScrolls[PlayerOptions::SCROLL_REVERSE] == 1
	};

	FOREACH_EnabledPlayer( p )
	{
		m_DifficultyIcon[p].SetName( ssprintf("DifficultyP%d%s",p+1,bReverse[p]?"Reverse":"") );
		SET_XY( m_DifficultyIcon[p] );

		m_DifficultyMeter[p].SetName( ssprintf("DifficultyMeterP%d%s",p+1,bReverse[p]?"Reverse":"") );
		SET_XY( m_DifficultyMeter[p] );
	}

	const bool bBothReverse = bReverse[PLAYER_1] && bReverse[PLAYER_2];
	const bool bOneReverse = !bBothReverse && (bReverse[PLAYER_1] || bReverse[PLAYER_2]);

	/* XXX: We want to put the lyrics out of the way, but it's likely that one
	 * player is in reverse and the other isn't.  What to do? */
	m_LyricDisplay.SetName( ssprintf( "Lyrics%s", bBothReverse? "Reverse": bOneReverse? "OneReverse": "") );
	SET_XY( m_LyricDisplay );

	/* Load the Oni transitions */
	m_NextSongIn.Load( THEME->GetPathB(m_sName,"next song in") );
	// Instead, load this right before it's used
//	m_NextSongOut.Load( THEME->GetPathB(m_sName,"next song out") );

	m_SongFinished.Load( THEME->GetPathB(m_sName,"song finished") );

	// Load lyrics
	// XXX: don't load this here
	LyricsLoader LL;
	if( GAMESTATE->m_pCurSong->HasLyrics()  )
		LL.LoadFromLRCFile(GAMESTATE->m_pCurSong->GetLyricsPath(), *GAMESTATE->m_pCurSong);

	
	/* Set up song-specific graphics. */
	
	
	// Check to see if any players are in beginner mode.
	// Note: steps can be different if turn modifiers are used.
	if( PREFSMAN->m_bShowBeginnerHelper )
	{
		FOREACH_HumanPlayer( p )
		{
			if( GAMESTATE->m_pCurSteps[p]->GetDifficulty() == DIFFICULTY_BEGINNER )
				m_BeginnerHelper.AddPlayer( p, &m_Player[p].m_NoteData );
		}
	}

	m_SongBackground.Unload();

	if( !PREFSMAN->m_bShowBeginnerHelper || !m_BeginnerHelper.Initialize(2) )
	{
		m_BeginnerHelper.SetHidden( true );

		/* BeginnerHelper disabled, or failed to load. */
		m_SongBackground.LoadFromSong( GAMESTATE->m_pCurSong );

		if( !GAMESTATE->m_bDemonstrationOrJukebox )
		{
			/* This will fade from a preset brightness to the actual brightness (based
			 * on prefs and "cover").  The preset brightness may be 0 (to fade from
			 * black), or it might be 1, if the stage screen has the song BG and we're
			 * coming from it (like Pump).  This used to be done in SM_PlayReady, but
			 * that means it's impossible to snap to the new brightness immediately. */
			m_SongBackground.SetBrightness( INITIAL_BACKGROUND_BRIGHTNESS );
			m_SongBackground.FadeToActualBrightness();
		}
	}
	else
		m_BeginnerHelper.SetHidden( false );

	m_SongForeground.LoadFromSong( GAMESTATE->m_pCurSong );

	m_fTimeSinceLastDancingComment = 0;


	/* m_soundMusic and m_SongBackground take a very long time to load,
	 * so cap fDelta at 0 so m_NextSongIn will show up on screen.
	 * -Chris */
	m_bZeroDeltaOnNextUpdate = true;

	//
	// Load cabinet lights data
	//
	if( LIGHTSMAN->IsEnabled() )
	{
		m_CabinetLightsNoteData.Init();
		ASSERT( GAMESTATE->m_pCurSong );

		Steps *pSteps = GAMESTATE->m_pCurSong->GetClosestNotes( STEPS_TYPE_LIGHTS_CABINET, StringToDifficulty(PREFSMAN->m_sLightsStepsDifficulty) );
		if( pSteps != NULL )
		{
			pSteps->GetNoteData( m_CabinetLightsNoteData );
		}
		else
		{
			pSteps = GAMESTATE->m_pCurSong->GetClosestNotes( GAMESTATE->GetCurrentStyle()->m_StepsType, StringToDifficulty(PREFSMAN->m_sLightsStepsDifficulty) );
			if( pSteps )
			{
				NoteData TapNoteData;
				pSteps->GetNoteData( TapNoteData );
				NoteDataUtil::LoadTransformedLights( TapNoteData, m_CabinetLightsNoteData, GameManager::StepsTypeToNumTracks(STEPS_TYPE_LIGHTS_CABINET) );
			}
		}
	}
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
	
	RageSoundParams p;
	p.AccurateSync = true;
	p.SetPlaybackRate( GAMESTATE->m_SongOptions.m_fMusicRate );
	p.StopMode = RageSoundParams::M_CONTINUE;
	p.m_StartSecond = fStartSecond;

	ASSERT( !m_pSoundMusic->IsPlaying() );
	m_pSoundMusic->Play( &p );

	/* Make sure GAMESTATE->m_fMusicSeconds is set up. */
	GAMESTATE->m_fMusicSeconds = -5000;
	UpdateSongPosition(0);

	ASSERT( GAMESTATE->m_fMusicSeconds > -4000 ); /* make sure the "fake timer" code doesn't trigger */

	/* Return the amount of time until the first beat. */
	return fFirstSecond - fStartSecond;
}


void ScreenGameplay::PauseGame( bool bPause )
{
	if( m_bPaused == bPause )
	{
		LOG->Trace( "ScreenGameplay::PauseGame(%i) received, but already in that state; ignored", bPause );
		return;
	}

	/* Don't pause if we're already tweening out. */
	if( bPause && m_DancingState == STATE_OUTRO )
		return;

	m_bPaused = bPause;
	m_pSoundMusic->Pause( bPause );
	if( bPause )
		this->PlayCommand( "Pause" );
	else
		this->PlayCommand( "Unpause" );

	FOREACH_EnabledPlayer(p)
		m_Player[p].SetPaused( m_bPaused );
}

// play assist ticks
void ScreenGameplay::PlayTicks()
{
	if( !GAMESTATE->m_SongOptions.m_bAssistTick )
		return;

	/* Sound cards have a latency between when a sample is Play()ed and when the sound
	 * will start coming out the speaker.  Compensate for this by boosting fPositionSeconds
	 * ahead.  This is just to make sure that we request the sound early enough for it to
	 * come out on time; the actual precise timing is handled by SetStartTime. */
	float fPositionSeconds = GAMESTATE->m_fMusicSeconds;
	fPositionSeconds += SOUND->GetPlayLatency() + (float)TICK_EARLY_SECONDS + 0.250f;
	const float fSongBeat = GAMESTATE->m_pCurSong->GetBeatFromElapsedTime( fPositionSeconds );

	const int iSongRow = max( 0, BeatToNoteRowNotRounded( fSongBeat ) );
	static int iRowLastCrossed = -1;
	if( iSongRow < iRowLastCrossed )
		iRowLastCrossed = -1;

	int iTickRow = -1;
	// for each index we crossed since the last update:
	FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( m_Player[GAMESTATE->m_MasterPlayerNumber].m_NoteData, r, iRowLastCrossed+1, iSongRow+1 )
		if( m_Player[GAMESTATE->m_MasterPlayerNumber].m_NoteData.IsThereATapOrHoldHeadAtRow( r ) )
			iTickRow = r;

	iRowLastCrossed = iSongRow;

	if( iTickRow != -1 )
	{
		const float fTickBeat = NoteRowToBeat( iTickRow );
		const float fTickSecond = GAMESTATE->m_pCurSong->m_Timing.GetElapsedTimeFromBeat( fTickBeat );
		float fSecondsUntil = fTickSecond - GAMESTATE->m_fMusicSeconds;
		fSecondsUntil /= GAMESTATE->m_SongOptions.m_fMusicRate; /* 2x music rate means the time until the tick is halved */

		RageSoundParams p;
		p.StartTime = GAMESTATE->m_LastBeatUpdate + (fSecondsUntil - (float)TICK_EARLY_SECONDS);
		m_soundAssistTick.Play( &p );
	}
}

/* Play announcer "type" if it's been at least fSeconds since the last announcer. */
void ScreenGameplay::PlayAnnouncer( CString type, float fSeconds )
{
	if( GAMESTATE->m_fOpponentHealthPercent == 0 )
		return; // Shut the announcer up

	/* Don't play in demonstration. */
	if( GAMESTATE->m_bDemonstrationOrJukebox )
		return;

	/* Don't play before the first beat, or after we're finished. */
	if( m_DancingState != STATE_DANCING )
		return;
	if( GAMESTATE->m_pCurSong == NULL  ||	// this will be true on ScreenDemonstration sometimes
		GAMESTATE->m_fSongBeat < GAMESTATE->m_pCurSong->m_fFirstBeat )
		return;


	if( m_fTimeSinceLastDancingComment < fSeconds )
		return;
	m_fTimeSinceLastDancingComment = 0;

	SOUND->PlayOnceFromAnnouncer( type );

	if( m_pCombinedLifeMeter )
		m_pCombinedLifeMeter->OnTaunt();
}

void ScreenGameplay::UpdateSongPosition( float fDeltaTime )
{
	if( !m_pSoundMusic->IsPlaying() )
		return;

	RageTimer tm;
	const float fSeconds = m_pSoundMusic->GetPositionSeconds( NULL, &tm );
	const float fAdjust = SOUND->GetFrameTimingAdjustment( fDeltaTime );
	GAMESTATE->UpdateSongPosition( fSeconds+fAdjust, GAMESTATE->m_pCurSong->m_Timing, tm+fAdjust );
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
		SOUND->PlayOnceFromAnnouncer( "gameplay intro" );	// crowd cheer

		//
		// Get the transitions rolling
		//
		if( GAMESTATE->m_bDemonstrationOrJukebox )
		{
			StartPlayingSong( 0, 0 );	// *kick* (no transitions)
		}
		else if ( NSMAN->useSMserver )
		{
			//If we're using networking, we must not have any
			//delay.  If we do this can cause inconsistancy
			//on different computers and differet themes

			StartPlayingSong( 0, 0 );
			m_pSoundMusic->Stop();

			float startOffset = g_fNetStartOffset;

			NSMAN->StartRequest(1); 

			RageSoundParams p;
			p.AccurateSync = true;
			p.SetPlaybackRate( 1.0 );	//Force 1.0 playback speed
			p.StopMode = RageSoundParams::M_CONTINUE;
			p.m_StartSecond = startOffset;
			m_pSoundMusic->Play( &p );

			UpdateSongPosition(0);

			//We need to artifically trigger the sm_playeready so we can end game
			//We want to post so this happens only after we're done what we're doing.
			SCREENMAN->PostMessageToTopScreen( SM_PlayReady, 0.0 );
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


	UpdateSongPosition( fDeltaTime );

	if( m_bZeroDeltaOnNextUpdate )
	{
		Screen::Update( 0 );
		m_bZeroDeltaOnNextUpdate = false;
	}
	else
	{
		Screen::Update( fDeltaTime );
	}

	/* This happens if ScreenDemonstration::HandleScreenMessage sets a new screen when
	 * PREFSMAN->m_bDelayedScreenLoad. */
	if( GAMESTATE->m_pCurSong == NULL )
		return;
	/* This can happen if ScreenDemonstration::HandleScreenMessage sets a new screen when
	 * !PREFSMAN->m_bDelayedScreenLoad.  (The new screen was loaded when we called Screen::Update,
	 * and the ctor might set a new GAMESTATE->m_pCurSong, so the above check can fail.) */
	if( SCREENMAN->GetTopScreen() != this )
		return;

	/* Update actors when paused, but never move on to another state. */
	if( m_bPaused )
		return;

	if( GAMESTATE->m_MasterPlayerNumber != PLAYER_INVALID && !m_MaxCombo.GetHidden() )
		m_MaxCombo.SetText( ssprintf("%d", STATSMAN->m_CurStageStats.m_player[GAMESTATE->m_MasterPlayerNumber].iMaxCombo) ); /* MAKE THIS WORK FOR BOTH PLAYERS! */
	
	//LOG->Trace( "m_fOffsetInBeats = %f, m_fBeatsPerSecond = %f, m_Music.GetPositionSeconds = %f", m_fOffsetInBeats, m_fBeatsPerSecond, m_Music.GetPositionSeconds() );

	m_AutoKeysounds.Update(fDeltaTime);

	//
	// update GameState HealthState
	//
    FOREACH_EnabledPlayer(p)
	{
		if(
			(m_pLifeMeter[p] && m_pLifeMeter[p]->IsFailing()) || 
			(m_pCombinedLifeMeter && m_pCombinedLifeMeter->IsFailing(p)) )
		{
			GAMESTATE->m_pPlayerState[p]->m_HealthState = PlayerState::DEAD;
		}
		else if(
			(m_pLifeMeter[p] && m_pLifeMeter[p]->IsHot()) || 
			(m_pCombinedLifeMeter && m_pCombinedLifeMeter->IsHot(p)) )
		{
			GAMESTATE->m_pPlayerState[p]->m_HealthState = PlayerState::HOT;
		}
		else if( 
			(m_pLifeMeter[p] && m_pLifeMeter[p]->IsInDanger()) || 
			(m_pCombinedLifeMeter && m_pCombinedLifeMeter->IsInDanger(p)) )
		{
			GAMESTATE->m_pPlayerState[p]->m_HealthState = PlayerState::DANGER;
		}
		else
		{
			GAMESTATE->m_pPlayerState[p]->m_HealthState = PlayerState::ALIVE;
		}
	}


	switch( m_DancingState )
	{
	case STATE_DANCING:
		/* Set STATSMAN->m_CurStageStats.bFailed for failed players.  In, FAIL_IMMEDIATE, send
		* SM_BeginFailed if all players failed, and kill dead Oni players. */
		switch( GAMESTATE->m_SongOptions.m_FailType )
		{
		case SongOptions::FAIL_OFF:
			// don't allow fail
			break;
		default:
			// check for individual fail
			FOREACH_EnabledPlayer( pn )
			{
				if( (m_pLifeMeter[pn] && !m_pLifeMeter[pn]->IsFailing()) || 
					(m_pCombinedLifeMeter && !m_pCombinedLifeMeter->IsFailing(pn)) )
					continue; /* isn't failing */
				if( STATSMAN->m_CurStageStats.m_player[pn].bFailed )
					continue; /* failed and is already dead */
			
				/* If recovery is enabled, only set fail if both are failing.
				* There's no way to recover mid-song in battery mode. */
				if( GAMESTATE->m_SongOptions.m_LifeType != SongOptions::LIFE_BATTERY &&
					PREFSMAN->m_bTwoPlayerRecovery && !GAMESTATE->AllAreDead() )
					continue;

				LOG->Trace("Player %d failed", (int)pn);
				STATSMAN->m_CurStageStats.m_player[pn].bFailed = true;	// fail

				if( GAMESTATE->m_SongOptions.m_LifeType == SongOptions::LIFE_BATTERY &&
					GAMESTATE->m_SongOptions.m_FailType == SongOptions::FAIL_IMMEDIATE )
				{
					if( !STATSMAN->m_CurStageStats.AllFailedEarlier() )	// if not the last one to fail
					{
						// kill them!
						SOUND->PlayOnceFromDir( THEME->GetPathS(m_sName,"oni die") );
						ShowOniGameOver(pn);
						m_Player[pn].m_NoteData.Init();		// remove all notes and scoring
						m_Player[pn].FadeToFail();	// tell the NoteField to fade to white
					}
				}
			}
			break;
		}

		/* If FAIL_IMMEDIATE and everyone is failing, start SM_BeginFailed. */
		bool bBeginFailed = false;
		SongOptions::FailType ft = GAMESTATE->m_SongOptions.m_FailType;
		if( PREFSMAN->m_bMinimum1FullSongInCourses && GAMESTATE->IsCourseMode() && GAMESTATE->GetCourseSongIndex()==0 )
		{
			// take the least harsh of the two FailTypes
			ft = max( ft, SongOptions::FAIL_COMBO_OF_30_MISSES );
		}
		switch( ft )
		{
		case SongOptions::FAIL_IMMEDIATE:
			if( GAMESTATE->AllAreDead() )
				bBeginFailed = true;
			break;
		case SongOptions::FAIL_COMBO_OF_30_MISSES:
			if( GAMESTATE->AllHaveComboOf30OrMoreMisses() )
				bBeginFailed = true;
			break;
		}

		if( bBeginFailed )
			SCREENMAN->PostMessageToTopScreen( SM_BeginFailed, 0 );

		//
		// Update living players' alive time
		//
		FOREACH_EnabledPlayer(pn)
			if(!STATSMAN->m_CurStageStats.m_player[pn].bFailed)
				STATSMAN->m_CurStageStats.m_player[pn].fAliveSeconds += fDeltaTime * GAMESTATE->m_SongOptions.m_fMusicRate;

		// update fGameplaySeconds
		STATSMAN->m_CurStageStats.fGameplaySeconds += fDeltaTime;

		//
		// Check for end of song
		//
		float fSecondsToStop = GAMESTATE->m_pCurSong->GetElapsedTimeFromBeat( GAMESTATE->m_pCurSong->m_fLastBeat );

		/* Make sure we keep going long enough to register a miss for the last note. */
		fSecondsToStop += Player::GetMaxStepDistanceSeconds();

		if( GAMESTATE->m_fMusicSeconds > fSecondsToStop && !m_SongFinished.IsTransitioning() && !m_NextSongOut.IsTransitioning() )
			m_SongFinished.StartTransitioning( SM_NotesEnded );
	
		//
		// update 2d dancing characters
		//
        FOREACH_EnabledPlayer(p)
		{
			DancingCharacters *pCharacter = m_SongBackground.GetDancingCharacters();
			if( pCharacter != NULL )
			{
				TapNoteScore tns = m_Player[p].GetLastTapNoteScore();
				
				ANIM_STATES_2D StateMap[NUM_TAP_NOTE_SCORES] =
				{
					AS2D_MISS, /* TNS_NONE (shouldn't happen) */
					AS2D_MISS, /* TNS_HIT_MINE (shouldn't happen) */
					AS2D_MISS, /* TNS_MISS */
					AS2D_MISS, /* TNS_BOO */
					AS2D_GOOD, /* TNS_GOOD */
					AS2D_GOOD, /* TNS_GREAT */
					AS2D_GREAT, /* TNS_PERFECT */
					AS2D_GREAT /* TNS_MARVELOUS */
				};

				ANIM_STATES_2D state = StateMap[tns];
				if( state == AS2D_GREAT && m_pLifeMeter[p] && m_pLifeMeter[p]->GetLife() == 1.0f ) // full life
					state = AS2D_FEVER;

				pCharacter->Change2DAnimState( p, state );
			}
		}

		//
		// Check for enemy death in enemy battle
		//
		static float fLastSeenEnemyHealth = 1;
		if( fLastSeenEnemyHealth != GAMESTATE->m_fOpponentHealthPercent )
		{
			fLastSeenEnemyHealth = GAMESTATE->m_fOpponentHealthPercent;

			if( GAMESTATE->m_fOpponentHealthPercent == 0 )
			{
				// HACK:  Load incorrect directory on purpose for now.
				PlayAnnouncer( "gameplay battle damage level3", 0 );

				GAMESTATE->RemoveAllActiveAttacks();

                FOREACH_CpuPlayer(p)
				{
					SOUND->PlayOnceFromDir( THEME->GetPathS(m_sName,"oni die") );
                    ShowOniGameOver( p );
                    m_Player[p].m_NoteData.Init();		// remove all notes and scoring
                    m_Player[p].FadeToFail();	// tell the NoteField to fade to white
				}
			}
		}

		//
		// Check to see if it's time to play a ScreenGameplay comment
		//
		m_fTimeSinceLastDancingComment += fDeltaTime;

		switch( GAMESTATE->m_PlayMode )
		{
		case PLAY_MODE_REGULAR:
		case PLAY_MODE_BATTLE:
		case PLAY_MODE_RAVE:
			if( GAMESTATE->OneIsHot() )			
				PlayAnnouncer( "gameplay comment hot", SECONDS_BETWEEN_COMMENTS );
			else if( GAMESTATE->AllAreInDangerOrWorse() )	
				PlayAnnouncer( "gameplay comment danger", SECONDS_BETWEEN_COMMENTS );
			else
				PlayAnnouncer( "gameplay comment good", SECONDS_BETWEEN_COMMENTS );
			break;
		case PLAY_MODE_NONSTOP:
		case PLAY_MODE_ONI:
		case PLAY_MODE_ENDLESS:
			PlayAnnouncer( "gameplay comment oni", SECONDS_BETWEEN_COMMENTS );
			break;
		default:
			ASSERT(0);
		}
	}

	//
	// update give up timer
	//
	if( !m_GiveUpTimer.IsZero() && m_GiveUpTimer.Ago() > 4.0f )
	{
		m_GiveUpTimer.SetZero();

		if( GIVING_UP_FAILS )
		{
			/* Unless we're in FailOff, giving up means failing the song. */
			switch( GAMESTATE->m_SongOptions.m_FailType )
			{
			case SongOptions::FAIL_IMMEDIATE:
			case SongOptions::FAIL_COMBO_OF_30_MISSES:
			case SongOptions::FAIL_END_OF_SONG:
				FOREACH_EnabledPlayer(pn)
					STATSMAN->m_CurStageStats.m_player[pn].bFailed = true;	// fail
			}

			this->PostScreenMessage( SM_NotesEnded, 0 );
		}
		else
		{
			BackOutFromGameplay();
			return;
		}
	}

	//
	// update bpm display
	//
	if( m_fLastBPS != GAMESTATE->m_fCurBPS && !m_BPMDisplay.GetHidden() )
	{
		m_fLastBPS = GAMESTATE->m_fCurBPS;
		m_BPMDisplay.SetBPM( GAMESTATE->m_fCurBPS * 60.0f );
	}

	//
	// play assist ticks
	//
	PlayTicks();

	//
	// update lights
	//
	UpdateLights();

	if( NSMAN->useSMserver )
	{
		FOREACH_EnabledPlayer( pn2 )
			if( m_pLifeMeter[pn2] )
				NSMAN->m_playerLife[pn2] = int(m_pLifeMeter[pn2]->GetLife()*10000);

		FOREACH_NSScoreBoardColumn(cn)
			if( m_ShowScoreboard && NSMAN->ChangedScoreboard(cn) )
				m_Scoreboard[cn].SetText( NSMAN->m_Scoreboard[cn] );
	}
}

void ScreenGameplay::UpdateLights()
{
	if( !LIGHTSMAN->IsEnabled() )
		return;

	const Style* pStyle = GAMESTATE->GetCurrentStyle();
	bool bBlinkCabinetLight[NUM_CABINET_LIGHTS];
	bool bBlinkGameButton[MAX_GAME_CONTROLLERS][MAX_GAME_BUTTONS];
	ZERO( bBlinkCabinetLight );
	ZERO( bBlinkGameButton );
	bool bCrossedABeat = false;
	{
		float fPositionSeconds = GAMESTATE->m_fMusicSeconds + LIGHTS_FALLOFF_SECONDS/2;	// trigger the light a tiny bit early
		float fSongBeat = GAMESTATE->m_pCurSong->GetBeatFromElapsedTime( fPositionSeconds );

		int iRowNow = BeatToNoteRowNotRounded( fSongBeat );
		iRowNow = max( 0, iRowNow );
		static int iRowLastCrossed = 0;

		float fBeatLast = roundf(NoteRowToBeat(iRowLastCrossed));
		float fBeatNow = roundf(NoteRowToBeat(iRowNow));

		bCrossedABeat = fBeatLast != fBeatNow;

		FOREACH_CabinetLight( cl )
		{	
			// for each index we crossed since the last update:
			FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE( m_CabinetLightsNoteData, cl, r, iRowLastCrossed, iRowNow )
			{
				bool bBlink = (m_CabinetLightsNoteData.GetTapNote( cl, r ).type != TapNote::empty );
				bBlinkCabinetLight[cl] |= bBlink;
			}

			if( m_CabinetLightsNoteData.IsHoldNoteAtBeat( cl, iRowNow ) )
				bBlinkCabinetLight[cl] |= true;
		}

		FOREACH_EnabledPlayer( pn )
		{
			for( int t=0; t<m_Player[pn].m_NoteData.GetNumTracks(); t++ )
			{
				// for each index we crossed since the last update:
				FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE( m_Player[pn].m_NoteData, t, r, iRowLastCrossed+1, iRowNow )
				{
					TapNote tn = m_Player[pn].m_NoteData.GetTapNote(t,r);
					bool bBlink = (tn.type != TapNote::empty && tn.type != TapNote::mine);
					if( bBlink )
					{
						StyleInput si( pn, t );
						GameInput gi = pStyle->StyleInputToGameInput( si );
						bBlinkGameButton[gi.controller][gi.button] |= bBlink;
					}
				}
			}
		}

		iRowLastCrossed = iRowNow;
	}

	{
		// check for active HoldNotes
		float fPositionSeconds = GAMESTATE->m_fMusicSeconds + LIGHTS_FALLOFF_SECONDS/2;	// trigger the light a tiny bit early
		float fSongBeat = GAMESTATE->m_pCurSong->GetBeatFromElapsedTime( fPositionSeconds );
		const int iSongRow = BeatToNoteRow( fSongBeat );

		FOREACH_EnabledPlayer( pn )
		{
			// check if a hold should be active
			for( int t=0; t < m_Player[pn].m_NoteData.GetNumTracks(); ++t )
			{
				if( m_Player[pn].m_NoteData.IsHoldNoteAtBeat( t, iSongRow ) )
				{
					StyleInput si( pn, t );
					GameInput gi = pStyle->StyleInputToGameInput( si );
					bBlinkGameButton[gi.controller][gi.button] |= true;
				}
			}
		}
	}


	// Before the first beat of the song, blink all cabinet lights (except for 
	// menu buttons) on the beat.
	bool bOverrideCabinetBlink = (GAMESTATE->m_fSongBeat < GAMESTATE->m_pCurSong->m_fFirstBeat) && bCrossedABeat;
	FOREACH_CabinetLight( cl )
	{
		switch( cl )
		{
		case LIGHT_BUTTONS_LEFT:
		case LIGHT_BUTTONS_RIGHT:
			// don't blink
			break;
		default:
			bBlinkCabinetLight[cl] |= bOverrideCabinetBlink;
			break;
		}
	}

	// Send blink data.
	FOREACH_CabinetLight( cl )
	{
		if( bBlinkCabinetLight[cl] )
			LIGHTSMAN->BlinkCabinetLight( cl );
	}

	FOREACH_GameController( gc )
	{
		FOREACH_GameButton( gb )
		{
			if( bBlinkGameButton[gc][gb] )
				LIGHTSMAN->BlinkGameButton( GameInput(gc,gb) );
		}
	}
}

void ScreenGameplay::BackOutFromGameplay()
{
	m_DancingState = STATE_OUTRO;
	SCREENMAN->PlayBackSound();
	
	m_pSoundMusic->StopPlaying();
	m_soundAssistTick.StopPlaying(); /* Stop any queued assist ticks. */

	this->ClearMessageQueue();
	m_Back.StartTransitioning( SM_SaveChangedBeforeGoingBack );
}

void ScreenGameplay::AbortGiveUp()
{
	if( m_GiveUpTimer.IsZero() )
		return;

	m_textDebug.StopTweening();
	m_textDebug.SetText( GIVE_UP_ABORTED_TEXT );
	m_textDebug.BeginTweening( 1/2.f );
	m_textDebug.SetDiffuse( RageColor(1,1,1,0) );
	m_GiveUpTimer.SetZero();
}


void ScreenGameplay::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	//LOG->Trace( "ScreenGameplay::Input()" );

	if( type == IET_LEVEL_CHANGED )
		return;

	if( m_bPaused )
	{
		/* If we're paused, only accept MENU_BUTTON_START to unpause. */
		if( MenuI.IsValid() && MenuI.button == MENU_BUTTON_START && type == IET_FIRST_PRESS )
			this->PauseGame( false );
		return;
	}

	if( MenuI.IsValid()  &&  
		m_DancingState != STATE_OUTRO  &&
		!m_Back.IsTransitioning() )
	{
		/* Allow bailing out by holding the START button of all active players.  This
		 * gives a way to "give up" when a back button isn't available.  Doing this is
		 * treated as failing the song, unlike BACK, since it's always available.
		 *
		 * However, if this is also a style button, don't do this. (pump center = start) */
		bool bHoldingGiveUp = false;
		bHoldingGiveUp |= (MenuI.button == MENU_BUTTON_START && !StyleI.IsValid() && START_GIVES_UP.GetValue());
		bHoldingGiveUp |= (MenuI.button == MENU_BUTTON_BACK && !StyleI.IsValid() && BACK_GIVES_UP.GetValue());
		
		if( bHoldingGiveUp )
		{
			/* No PREFSMAN->m_bDelayedEscape; always delayed. */
			if( type==IET_RELEASE )
				AbortGiveUp();
			else if( type==IET_FIRST_PRESS && m_GiveUpTimer.IsZero() )
			{
				m_textDebug.SetText( GIVE_UP_TEXT );
				m_textDebug.StopTweening();
				m_textDebug.SetDiffuse( RageColor(1,1,1,0) );
				m_textDebug.BeginTweening( 1/8.f );
				m_textDebug.SetDiffuse( RageColor(1,1,1,1) );
				m_GiveUpTimer.Touch(); /* start the timer */
			}

			return;
		}

		/* Only handle MENU_BUTTON_BACK as a regular BACK button if BACK_GIVES_UP is
		 * disabled. */
		if( MenuI.button == MENU_BUTTON_BACK && !BACK_GIVES_UP.GetValue() )
		{
			if( ((!PREFSMAN->m_bDelayedBack && type==IET_FIRST_PRESS) ||
				(DeviceI.device==DEVICE_KEYBOARD && (type==IET_SLOW_REPEAT||type==IET_FAST_REPEAT)) ||
				(DeviceI.device!=DEVICE_KEYBOARD && type==IET_FAST_REPEAT)) )
			{
				LOG->Trace("Player %i went back", MenuI.player+1);
				BackOutFromGameplay();
			}
			else if( PREFSMAN->m_bDelayedBack && type==IET_FIRST_PRESS )
			{
				m_textDebug.SetText( "Continue holding BACK to quit" );
				m_textDebug.StopTweening();
				m_textDebug.SetDiffuse( RageColor(1,1,1,0) );
				m_textDebug.BeginTweening( 1/8.f );
				m_textDebug.SetDiffuse( RageColor(1,1,1,1) );
			}
			else if( PREFSMAN->m_bDelayedBack && type==IET_RELEASE )
			{
				m_textDebug.StopTweening();
				m_textDebug.BeginTweening( 1/8.f );
				m_textDebug.SetDiffuse( RageColor(1,1,1,0) );
			}

			return;
		}
	}

	/* Nothing below cares about releases. */
	if(type == IET_RELEASE) return;

	// Handle special keys to adjust the offset
	if( DeviceI.device == DEVICE_KEYBOARD )
	{
		switch( DeviceI.button )
		{
		case KEY_F5:
			this->HandleScreenMessage( SM_NotesEnded );
			break;
		case KEY_F6:
			m_bChangedOffsetOrBPM = true;
			GAMESTATE->m_SongOptions.m_bAutoSync = !GAMESTATE->m_SongOptions.m_bAutoSync;	// toggle
			UpdateAutoPlayText();
			break;
		case KEY_F7:
			GAMESTATE->m_SongOptions.m_bAssistTick ^= 1;

			/* Store this change, so it sticks if we change songs: */
			GAMESTATE->m_StoredSongOptions.m_bAssistTick = GAMESTATE->m_SongOptions.m_bAssistTick;
			
			m_textDebug.SetText( ssprintf("Assist Tick is %s", GAMESTATE->m_SongOptions.m_bAssistTick?"ON":"OFF") );
			m_textDebug.StopTweening();
			m_textDebug.SetDiffuse( RageColor(1,1,1,1) );
			m_textDebug.BeginTweening( 3 );		// sleep
			m_textDebug.BeginTweening( 0.5f );	// fade out
			m_textDebug.SetDiffuse( RageColor(1,1,1,0) );
			break;
		case KEY_F8:
			{
				PREFSMAN->m_bAutoPlay = !PREFSMAN->m_bAutoPlay;
				UpdateAutoPlayText();
				bool bIsHoldingShift = 
					INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, KEY_RSHIFT)) ||
					INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, KEY_LSHIFT));
                FOREACH_HumanPlayer(p)
				{
                    if( bIsHoldingShift )
                        GAMESTATE->m_pPlayerState[p]->m_PlayerController = PREFSMAN->m_bAutoPlay ? PC_CPU : PC_HUMAN;
                    else
                        GAMESTATE->m_pPlayerState[p]->m_PlayerController = PREFSMAN->m_bAutoPlay ? PC_AUTOPLAY : PC_HUMAN;
				}
			}
			break;
		case KEY_F9:
		case KEY_F10:
			{
				m_bChangedOffsetOrBPM = true;

				float fOffsetDelta;
				switch( DeviceI.button )
				{
				case KEY_F9:	fOffsetDelta = -0.020f;		break;
				case KEY_F10:	fOffsetDelta = +0.020f;		break;
				default:	ASSERT(0);						return;
				}
				if( INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, KEY_RALT)) ||
					INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, KEY_LALT)) )
					fOffsetDelta /= 2; /* .010 */
				else if( type == IET_FAST_REPEAT )
					fOffsetDelta *= 10;
				BPMSegment& seg = GAMESTATE->m_pCurSong->GetBPMSegmentAtBeat( GAMESTATE->m_fSongBeat );

				seg.m_fBPS += fOffsetDelta;

				m_textDebug.SetText( ssprintf("Cur BPM = %.2f", seg.GetBPM()) );
				m_textDebug.StopTweening();
				m_textDebug.SetDiffuse( RageColor(1,1,1,1) );
				m_textDebug.BeginTweening( 3 );		// sleep
				m_textDebug.BeginTweening( 0.5f );	// fade out
				m_textDebug.SetDiffuse( RageColor(1,1,1,0) );
			}
			break;
		case KEY_F11:
		case KEY_F12:
			{
				m_bChangedOffsetOrBPM = true;

				float fOffsetDelta;
				switch( DeviceI.button )
				{
				case KEY_F11:	fOffsetDelta = -0.02f;		break;
				case KEY_F12:	fOffsetDelta = +0.02f;		break;
				default:	ASSERT(0);						return;
				}
				if( INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, KEY_RALT)) ||
					INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, KEY_LALT)) )
					fOffsetDelta /= 20; /* 1ms */
				else switch( type )
				{
				case IET_SLOW_REPEAT:	fOffsetDelta *= 10;	break;
				case IET_FAST_REPEAT:	fOffsetDelta *= 40;	break;
				}

				GAMESTATE->m_pCurSong->m_Timing.m_fBeat0OffsetInSeconds += fOffsetDelta;

				m_textDebug.SetText( ssprintf("Offset = %.3f", GAMESTATE->m_pCurSong->m_Timing.m_fBeat0OffsetInSeconds) );
				m_textDebug.StopTweening();
				m_textDebug.SetDiffuse( RageColor(1,1,1,1) );
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
		StyleI.IsValid() &&
		GAMESTATE->IsHumanPlayer( StyleI.player ) )
	{
		AbortGiveUp();
		
		if( !PREFSMAN->m_bAutoPlay )
			m_Player[StyleI.player].Step( StyleI.col, DeviceI.ts ); 
	}
//	else if( type==IET_FIRST_PRESS && 
//		!PREFSMAN->m_bAutoPlay && 
//		MenuI.IsValifd() &&
//		GAMESTATE->IsPlayerEnabled( MenuI.player ) &&
//		GAMESTATE->IsBattleMode() )
//	{
//		int iItemSlot;
//		switch( MenuI.button )
//		{
//		case MENU_BUTTON_LEFT:	iItemSlot = 0;	break;
//		case MENU_BUTTON_START:	iItemSlot = 1;	break;
//		case MENU_BUTTON_RIGHT:	iItemSlot = 2;	break;
//		default:				iItemSlot = -1;	break;
//		}
//		
//		if( iItemSlot != -1 )
//			m_pInventory[MenuI.player]->UseItem( iItemSlot );
//	}
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

void SaveChanges( void* papSongsQueue )
{
	vector<Song*>& apSongsQueue = *(vector<Song*>*)papSongsQueue;
	for( unsigned i=0; i<apSongsQueue.size(); i++ )
		apSongsQueue[i]->Save();
}

void RevertChanges( void* papSongsQueue )
{
	vector<Song*>& apSongsQueue = *(vector<Song*>*)papSongsQueue;
	FOREACH( Song*, apSongsQueue, pSong )
	{
		SONGMAN->RevertFromDisk( *pSong );
	}
}

void ScreenGameplay::ShowSavePrompt( ScreenMessage SM_SendWhenDone )
{
	CString sMessage;
	switch( GAMESTATE->m_PlayMode )
	{
	case PLAY_MODE_REGULAR:
	case PLAY_MODE_BATTLE:
	case PLAY_MODE_RAVE:
		sMessage = ssprintf( 
			"You have changed the offset or BPM of\n"
			"%s\n", 
			GAMESTATE->m_pCurSong->GetFullDisplayTitle().c_str() );

		if( fabs(GAMESTATE->m_pCurSong->m_Timing.m_fBeat0OffsetInSeconds - g_fOldOffset) > 0.001 )
		{
			sMessage += ssprintf( 
				"\n"
				"Offset was changed from %.3f to %.3f (%.3f).\n",
				g_fOldOffset, 
				GAMESTATE->m_pCurSong->m_Timing.m_fBeat0OffsetInSeconds,
				GAMESTATE->m_pCurSong->m_Timing.m_fBeat0OffsetInSeconds - g_fOldOffset );
		}

		sMessage +=
			"\n"
			"Would you like to save these changes back\n"
			"to the song file?\n"
			"Choosing NO will discard your changes.";
			
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

	SCREENMAN->Prompt( SM_SendWhenDone, sMessage, PROMPT_YES_NO, ANSWER_NO, SaveChanges, RevertChanges, &m_apSongsQueue );
}

/*
 * Saving StageStats that are affected by the note pattern is a little tricky:
 *
 * Stats are cumulative for course play.
 *
 * For regular songs, it doesn't matter how we do it; the pattern doesn't change
 * during play.
 *
 * The pattern changes during play in battle and course mode.  We want to include these
 * changes, so run stats for a song after the song finishes.
 *
 * If we fail, be sure to include the current song in stats, with the current modifier set.
 *
 * So:
 * 
 * 1. At the end of a song in any mode, pass or fail, add stats for that song (from m_Player).
 * 2. At the end of gameplay in course mode, add stats for any songs that weren't played,
 *    applying the modifiers the song would have been played with.  This doesn't include songs
 *    that were played but failed; that was done in #1.
 */
void ScreenGameplay::SongFinished()
{
	LOG->Trace("SongFinished");
	// save any statistics
    FOREACH_EnabledPlayer(p)
	{
		/* Note that adding stats is only meaningful for the counters (eg. RADAR_NUM_JUMPS),
		 * not for the percentages (RADAR_AIR). */
		RadarValues v;
		
		NoteDataUtil::GetRadarValues( m_Player[p].m_NoteData, GAMESTATE->m_pCurSong->m_fMusicLengthSeconds, v );
		STATSMAN->m_CurStageStats.m_player[p].radarPossible += v;

		NoteDataWithScoring::GetActualRadarValues( m_Player[p].m_NoteData, p, GAMESTATE->m_pCurSong->m_fMusicLengthSeconds, v );
		STATSMAN->m_CurStageStats.m_player[p].radarActual += v;
	}

	/* Extremely important: if we don't remove attacks before moving on to the next
	 * screen, they'll still be turned on eventually. */
	GAMESTATE->RemoveAllActiveAttacks();
	FOREACH_EnabledPlayer( p )
		m_ActiveAttackList[p].Refresh();
}

void ScreenGameplay::StageFinished( bool bBackedOut )
{
	if( GAMESTATE->IsCourseMode() && GAMESTATE->m_PlayMode != PLAY_MODE_ENDLESS )
	{
		LOG->Trace("Stage finished at index %i/%i", GAMESTATE->GetCourseSongIndex(), (int) m_apSongsQueue.size() );
		/* +1 to skip the current song; that's done already. */
		for( unsigned iPlaySongIndex = GAMESTATE->GetCourseSongIndex()+1;
			 iPlaySongIndex < m_apSongsQueue.size(); ++iPlaySongIndex )
		{
			LOG->Trace("Running stats for %i", iPlaySongIndex );
			FOREACH_EnabledPlayer(p)
			{
				SetupSong( p, iPlaySongIndex );
				m_Player[p].ApplyWaitingTransforms();
				SongFinished();
			}
		}
	}

	// save current stage stats
	if( !bBackedOut )
		STATSMAN->m_vPlayedStageStats.push_back( STATSMAN->m_CurStageStats );

	/* Reset options. */
	GAMESTATE->RestoreSelectedOptions();
}

void ScreenGameplay::HandleScreenMessage( const ScreenMessage SM )
{
	CHECKPOINT_M( ssprintf("HandleScreenMessage(%i)", SM) );
	switch( SM )
	{
	case SM_PlayReady:
		SOUND->PlayOnceFromAnnouncer( "gameplay ready" );
		m_Ready.StartTransitioning( SM_PlayGo );
		break;

	case SM_PlayGo:
		if( GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2() )
			SOUND->PlayOnceFromAnnouncer( "gameplay here we go extra" );
		else if( GAMESTATE->IsFinalStage() )
			SOUND->PlayOnceFromAnnouncer( "gameplay here we go final" );
		else
			SOUND->PlayOnceFromAnnouncer( "gameplay here we go normal" );

		m_Go.StartTransitioning( SM_None );
		GAMESTATE->m_bPastHereWeGo = true;
		m_DancingState = STATE_DANCING;		// STATE CHANGE!  Now the user is allowed to press Back
		break;

	// received while STATE_DANCING
	case SM_NotesEnded:
		{
			/* Do this in LoadNextSong, so we don't tween off old attacks until
			 * m_NextSongOut finishes. */
			// GAMESTATE->RemoveAllActiveAttacks();

            FOREACH_EnabledPlayer(p)
			{
				/* If either player's passmark is enabled, check it. */
				if( GAMESTATE->m_pPlayerState[p]->m_PlayerOptions.m_fPassmark > 0 &&
					m_pLifeMeter[p] &&
					m_pLifeMeter[p]->GetLife() < GAMESTATE->m_pPlayerState[p]->m_PlayerOptions.m_fPassmark )
				{
					LOG->Trace("Player %i failed: life %f is under %f",
						p+1, m_pLifeMeter[p]->GetLife(), GAMESTATE->m_pPlayerState[p]->m_PlayerOptions.m_fPassmark );
					STATSMAN->m_CurStageStats.m_player[p].bFailed = true;
				}

				/* Mark failure.  This hasn't been done yet if m_bTwoPlayerRecovery is set. */
				if( GAMESTATE->m_SongOptions.m_FailType != SongOptions::FAIL_OFF &&
					(m_pLifeMeter[p] && m_pLifeMeter[p]->IsFailing()) || 
					(m_pCombinedLifeMeter && m_pCombinedLifeMeter->IsFailing(p)) )
					STATSMAN->m_CurStageStats.m_player[p].bFailed = true;

				if( !STATSMAN->m_CurStageStats.m_player[p].bFailed )
					STATSMAN->m_CurStageStats.m_player[p].iSongsPassed++;
			}

			/* If all players have *really* failed (bFailed, not the life meter or
			 * bFailedEarlier): */
			const bool bAllReallyFailed = STATSMAN->m_CurStageStats.AllFailed();

			if( !bAllReallyFailed && !IsLastSong() )
			{
				/* Next song. */
				FOREACH_EnabledPlayer(p)
                {
                    if( !STATSMAN->m_CurStageStats.m_player[p].bFailed )
                    {
                        // give a little life back between stages
                        if( m_pLifeMeter[p] )
                            m_pLifeMeter[p]->OnSongEnded();	
                        if( m_pCombinedLifeMeter )
                            m_pCombinedLifeMeter->OnSongEnded();	
                    }
                }

				// HACK:  Temporarily set the song pointer to the next song so that 
				// this m_NextSongOut will show the next song banner
				Song* pCurSong = GAMESTATE->m_pCurSong;

				int iPlaySongIndex = GAMESTATE->GetCourseSongIndex()+1;
				iPlaySongIndex %= m_apSongsQueue.size();
				GAMESTATE->m_pCurSong.Set( m_apSongsQueue[iPlaySongIndex] );

				m_NextSongOut.Load( THEME->GetPathB(m_sName,"next song out") );
				GAMESTATE->m_pCurSong.Set( pCurSong );

				m_NextSongOut.StartTransitioning( SM_LoadNextSong );
				LoadCourseSongNumber( GAMESTATE->GetCourseSongIndex()+1 );
				COMMAND( m_sprCourseSongNumber, "ChangeIn" );
				return;
			}
			
			// update dancing characters for win / lose
			DancingCharacters *Dancers = m_SongBackground.GetDancingCharacters();
			if( Dancers )
                FOREACH_EnabledPlayer(p)
				{
					/* XXX: In battle modes, switch( GAMESTATE->GetStageResult(p) ). */
					if( STATSMAN->m_CurStageStats.m_player[p].bFailed )
						Dancers->Change2DAnimState( p, AS2D_FAIL ); // fail anim
					else if( m_pLifeMeter[p] && m_pLifeMeter[p]->GetLife() == 1.0f ) // full life
						Dancers->Change2DAnimState( p, AS2D_WINFEVER ); // full life pass anim
					else
						Dancers->Change2DAnimState( p, AS2D_WIN ); // pass anim
				}

			/* End round. */
			if( m_DancingState == STATE_OUTRO )	// ScreenGameplay already ended
				return;		// ignore
			m_DancingState = STATE_OUTRO;

			GAMESTATE->RemoveAllActiveAttacks();
			FOREACH_EnabledPlayer( p )
				m_ActiveAttackList[p].Refresh();

			LIGHTSMAN->SetLightsMode( LIGHTSMODE_ALL_CLEARED );


			if( bAllReallyFailed )
			{
				this->PostScreenMessage( SM_BeginFailed, 0 );
				return;
			}

			// do they deserve an extra stage?
			if( GAMESTATE->HasEarnedExtraStage() )
			{
				TweenOffScreen();
				m_Extra.StartTransitioning( SM_GoToNextScreen );
				SOUND->PlayOnceFromAnnouncer( "gameplay extra" );
			}
			else
			{
				TweenOffScreen();
				
				switch( GAMESTATE->m_PlayMode )
				{
				case PLAY_MODE_BATTLE:
				case PLAY_MODE_RAVE:
					{
						PlayerNumber winner = GAMESTATE->GetBestPlayer();
						switch( winner )
						{
						case PLAYER_INVALID:
							m_Draw.StartTransitioning( SM_GoToNextScreen );
							break;
						default:
							m_Win[winner].StartTransitioning( SM_GoToNextScreen );
							break;
						}
					}
					break;
				default:
					m_Cleared.StartTransitioning( SM_GoToNextScreen );
					break;
				}
				
				SOUND->PlayOnceFromAnnouncer( "gameplay cleared" );
			}
		}

		break;

	case SM_LoadNextSong:
		SongFinished();

		COMMAND( m_sprCourseSongNumber, "ChangeOut" );

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
		PlayAnnouncer( "gameplay 100 combo", 2 );
		break;
	case SM_200Combo:
		PlayAnnouncer( "gameplay 200 combo", 2 );
		break;
	case SM_300Combo:
		PlayAnnouncer( "gameplay 300 combo", 2 );
		break;
	case SM_400Combo:
		PlayAnnouncer( "gameplay 400 combo", 2 );
		break;
	case SM_500Combo:
		PlayAnnouncer( "gameplay 500 combo", 2 );
		break;
	case SM_600Combo:
		PlayAnnouncer( "gameplay 600 combo", 2 );
		break;
	case SM_700Combo:
		PlayAnnouncer( "gameplay 700 combo", 2 );
		break;
	case SM_800Combo:
		PlayAnnouncer( "gameplay 800 combo", 2 );
		break;
	case SM_900Combo:
		PlayAnnouncer( "gameplay 900 combo", 2 );
		break;
	case SM_1000Combo:
		PlayAnnouncer( "gameplay 1000 combo", 2 );
		break;
	case SM_ComboStopped:
		PlayAnnouncer( "gameplay combo stopped", 2 );
		break;
	case SM_ComboContinuing:
		PlayAnnouncer( "gameplay combo overflow", 2 );
		break;	
	case SM_BattleTrickLevel1:
		PlayAnnouncer( "gameplay battle trick level1", 3 );
		m_soundBattleTrickLevel1.Play();
		break;
	case SM_BattleTrickLevel2:
		PlayAnnouncer( "gameplay battle trick level2", 3 );
		m_soundBattleTrickLevel2.Play();
		break;
	case SM_BattleTrickLevel3:
		PlayAnnouncer( "gameplay battle trick level3", 3 );
		m_soundBattleTrickLevel3.Play();
		break;
	
	case SM_BattleDamageLevel1:
		PlayAnnouncer( "gameplay battle damage level1", 3 );
		break;
	case SM_BattleDamageLevel2:
		PlayAnnouncer( "gameplay battle damage level2", 3 );
		break;
	case SM_BattleDamageLevel3:
		PlayAnnouncer( "gameplay battle damage level3", 3 );
		break;
	
	case SM_SaveChangedBeforeGoingBack:
		if( m_bChangedOffsetOrBPM )
		{
			m_bChangedOffsetOrBPM = false;
			ShowSavePrompt( SM_GoToScreenAfterBack );
			break;
		}

		HandleScreenMessage( SM_GoToScreenAfterBack );
		break;

	case SM_GoToScreenAfterBack:
		SongFinished();
		StageFinished( true );

		GAMESTATE->CancelStage();

		SCREENMAN->DeletePreparedScreens();
		SCREENMAN->SetNewScreen( PREV_SCREEN );
		break;

	case SM_GoToNextScreen:
		if( m_bChangedOffsetOrBPM )
		{
			m_bChangedOffsetOrBPM = false;
			ShowSavePrompt( SM_GoToNextScreen );
			break;
		}
		
		SongFinished();
		StageFinished( false );

		SCREENMAN->SetNewScreen( NEXT_SCREEN );
		break;

	case SM_LoseFocus:
		/* We might have turned the song timer off.  Be sure to turn it back on. */
		SOUND->HandleSongTimer( true );
		break;

	case SM_BeginFailed:
		m_DancingState = STATE_OUTRO;
		m_pSoundMusic->StopPlaying();
		m_soundAssistTick.StopPlaying(); /* Stop any queued assist ticks. */
		TweenOffScreen();
		m_Failed.StartTransitioning( SM_GoToNextScreen );

		// show the survive time if extra stage
		if( GAMESTATE->IsExtraStage()  ||  GAMESTATE->IsExtraStage2() )
		{
			float fMaxSurviveSeconds = 0;
            FOREACH_EnabledPlayer(p)
                fMaxSurviveSeconds = max( fMaxSurviveSeconds, STATSMAN->m_CurStageStats.m_player[p].fAliveSeconds );
			ASSERT( fMaxSurviveSeconds > 0 );
			m_textSurviveTime.SetText( "TIME: " + SecondsToMMSSMsMs(fMaxSurviveSeconds) );
			SET_XY_AND_ON_COMMAND( m_textSurviveTime );
		}
		
		if( GAMESTATE->IsCourseMode() )
			if( GAMESTATE->GetCourseSongIndex() >= int(m_apSongsQueue.size() / 2) )
				SOUND->PlayOnceFromAnnouncer( "gameplay oni failed halfway" );
			else
				SOUND->PlayOnceFromAnnouncer( "gameplay oni failed" );
		else
			SOUND->PlayOnceFromAnnouncer( "gameplay failed" );
		break;

	case SM_StopMusic:
		m_pSoundMusic->Stop();
		break;

	case SM_Pause:
		/* Ignore SM_Pause when in demonstration. */
		if( GAMESTATE->m_bDemonstrationOrJukebox )
			return;

		if( !m_bPaused )
			PauseGame( true );
		break;
	}
}


void ScreenGameplay::TweenOnScreen()
{
	ON_COMMAND( m_sprLifeFrame );
	ON_COMMAND( m_sprStage );
	ON_COMMAND( m_sprCourseSongNumber );
	ON_COMMAND( m_sprStageFrame );
	ON_COMMAND( m_textSongOptions );
	ON_COMMAND( m_sprScoreFrame );
	ON_COMMAND( m_BPMDisplay );
	ON_COMMAND( m_MaxCombo );

	if( m_pCombinedLifeMeter )
		ON_COMMAND( *m_pCombinedLifeMeter );
    FOREACH_PlayerNumber(p)
	{
		if( m_pLifeMeter[p] )
			ON_COMMAND( *m_pLifeMeter[p] );
		if( !GAMESTATE->IsPlayerEnabled(p) )
			continue;
		ON_COMMAND( m_textCourseSongNumber[p] );
		ON_COMMAND( m_textStepsDescription[p] );
		if( m_pPrimaryScoreDisplay[p] )
			ON_COMMAND( *m_pPrimaryScoreDisplay[p] );
		if( m_pSecondaryScoreDisplay[p] )
			ON_COMMAND( *m_pSecondaryScoreDisplay[p] );
		ON_COMMAND( m_textPlayerOptions[p] );
		ON_COMMAND( m_ActiveAttackList[p] );
		ON_COMMAND( m_DifficultyIcon[p] );
		ON_COMMAND( m_DifficultyMeter[p] );
	}
	m_Overlay->PlayCommand("On");

	if (m_ShowScoreboard)
		FOREACH_NSScoreBoardColumn( sc )
			ON_COMMAND( m_Scoreboard[sc] );
}

void ScreenGameplay::TweenOffScreen()
{
	OFF_COMMAND( m_sprLifeFrame );
	OFF_COMMAND( m_sprStage );
	OFF_COMMAND( m_sprCourseSongNumber );
	OFF_COMMAND( m_sprStageFrame );
	OFF_COMMAND( m_textSongOptions );
	OFF_COMMAND( m_sprScoreFrame );
	OFF_COMMAND( m_BPMDisplay );
	OFF_COMMAND( m_MaxCombo );

	if( m_pCombinedLifeMeter )
		OFF_COMMAND( *m_pCombinedLifeMeter );
    FOREACH_PlayerNumber(p)
	{
		if( m_pLifeMeter[p] )
			OFF_COMMAND( *m_pLifeMeter[p] );
		if( !GAMESTATE->IsPlayerEnabled(p) )
			continue;
		OFF_COMMAND( m_textCourseSongNumber[p] );
		OFF_COMMAND( m_textStepsDescription[p] );
		if( m_pPrimaryScoreDisplay[p] )
			OFF_COMMAND( *m_pPrimaryScoreDisplay[p] );
		if( m_pSecondaryScoreDisplay[p] )
			OFF_COMMAND( *m_pSecondaryScoreDisplay[p] );
		OFF_COMMAND( m_textPlayerOptions[p] );
		OFF_COMMAND( m_ActiveAttackList[p] );
		OFF_COMMAND( m_DifficultyIcon[p] );
		OFF_COMMAND( m_DifficultyMeter[p] );
	}
	m_Overlay->PlayCommand("Off");

	if (m_ShowScoreboard)
		FOREACH_NSScoreBoardColumn( sc )
			OFF_COMMAND( m_Scoreboard[sc] );

	m_textDebug.StopTweening();
	m_textDebug.BeginTweening( 1/8.f );
	m_textDebug.SetDiffuse( RageColor(1,1,1,0) );
}

void ScreenGameplay::ShowOniGameOver( PlayerNumber pn )
{
	m_sprOniGameOver[pn].SetDiffuse( RageColor(1,1,1,1) );
	m_sprOniGameOver[pn].BeginTweening( 0.5f, Actor::TWEEN_BOUNCE_END );
	m_sprOniGameOver[pn].SetY( SCREEN_CENTER_Y );
	m_sprOniGameOver[pn].SetEffectBob( 4, RageVector3(0,6,0) );
}

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

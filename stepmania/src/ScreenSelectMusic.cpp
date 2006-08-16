#include "global.h"
#include "ScreenSelectMusic.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "SongManager.h"
#include "GameManager.h"
#include "GameSoundManager.h"
#include "GameConstantsAndTypes.h"
#include "RageLog.h"
#include "InputMapper.h"
#include "GameState.h"
#include "CodeDetector.h"
#include "ThemeManager.h"
#include "Steps.h"
#include "ActorUtil.h"
#include "RageTextureManager.h"
#include "Course.h"
#include "ProfileManager.h"
#include "Profile.h"
#include "MenuTimer.h"
#include "LightsManager.h"
#include "StatsManager.h"
#include "StepsUtil.h"
#include "Foreach.h"
#include "Style.h"
#include "PlayerState.h"
#include "Command.h"
#include "CommonMetrics.h"
#include "BannerCache.h"
#include "song.h"
#include "InputEventPlus.h"

const int NUM_SCORE_DIGITS	=	9;

#define SCORE_SORT_CHANGE_COMMAND(i) 		THEME->GetMetricA( m_sName, ssprintf("ScoreP%iSortChangeCommand", i+1) )
#define SCORE_FRAME_SORT_CHANGE_COMMAND(i)	THEME->GetMetricA( m_sName, ssprintf("ScoreFrameP%iSortChangeCommand", i+1) )
#define METER_TYPE				THEME->GetMetric ( m_sName, "MeterType" )
#define SHOW_OPTIONS_MESSAGE_SECONDS		THEME->GetMetricF( m_sName, "ShowOptionsMessageSeconds" )

AutoScreenMessage( SM_AllowOptionsMenuRepeat )
AutoScreenMessage( SM_SongChanged )
AutoScreenMessage( SM_SortOrderChanging )
AutoScreenMessage( SM_SortOrderChanged )

static RString g_sCDTitlePath;
static bool g_bWantFallbackCdTitle;
static bool g_bCDTitleWaiting = false;
static RString g_sBannerPath;
static bool g_bBannerWaiting = false;
static bool g_bSampleMusicWaiting = false;
static RageTimer g_StartedLoadingAt(RageZeroTimer);

REGISTER_SCREEN_CLASS( ScreenSelectMusic );
ScreenSelectMusic::ScreenSelectMusic()
{
	if( PREFSMAN->m_bScreenTestMode )
	{
		GAMESTATE->m_PlayMode.Set( PLAY_MODE_REGULAR );
		GAMESTATE->m_pCurStyle.Set( GAMEMAN->GameAndStringToStyle(GAMEMAN->GetDefaultGame(),"versus") );
		GAMESTATE->m_bSideIsJoined[PLAYER_1] = true;
		GAMESTATE->m_MasterPlayerNumber = PLAYER_1;
	}
}


void ScreenSelectMusic::Init()
{
	SAMPLE_MUSIC_DELAY.Load( m_sName, "SampleMusicDelay" );
	SHOW_RADAR.Load( m_sName, "ShowRadar" );
	DO_ROULETTE_ON_MENU_TIMER.Load( m_sName, "DoRouletteOnMenuTimer" );
	ALIGN_MUSIC_BEATS.Load( m_sName, "AlignMusicBeat" );
	CODES.Load( m_sName, "Codes" );
	MUSIC_WHEEL_TYPE.Load( m_sName, "MusicWheelType" );
	SELECT_MENU_AVAILABLE.Load( m_sName, "SelectMenuAvailable" );
	MODE_MENU_AVAILABLE.Load( m_sName, "ModeMenuAvailable" );

	LIGHTSMAN->SetLightsMode( LIGHTSMODE_MENU );

	/* Finish any previous stage.  It's OK to call this when we havn't played a stage yet. 
	 * Do this before anything that might look at GAMESTATE->m_iCurrentStageIndex. */
	GAMESTATE->FinishStage();

	m_bSelectIsDown = false; // used by UpdateSelectButton

	ScreenWithMenuElements::Init();

	m_DisplayMode = GAMESTATE->IsCourseMode() ? DISPLAY_COURSES : DISPLAY_SONGS;

	/* Cache: */
	m_sSectionMusicPath = THEME->GetPathS(m_sName,"section music");
	m_sSortMusicPath = THEME->GetPathS(m_sName,"sort music");
	m_sRouletteMusicPath = THEME->GetPathS(m_sName,"roulette music");
	m_sRandomMusicPath = THEME->GetPathS(m_sName,"random music");
	m_sCourseMusicPath = THEME->GetPathS(m_sName,"course music");
	m_sFallbackCDTitlePath = THEME->GetPathG(m_sName,"fallback cdtitle");


	m_TexturePreload.Load( m_sFallbackCDTitlePath );

	if( PREFSMAN->m_BannerCache != PrefsManager::BNCACHE_OFF )
	{
		m_TexturePreload.Load( Banner::SongBannerTexture(THEME->GetPathG("Banner","all music")) );
		m_TexturePreload.Load( Banner::SongBannerTexture(THEME->GetPathG("Common","fallback banner")) );
		m_TexturePreload.Load( Banner::SongBannerTexture(THEME->GetPathG("Banner","roulette")) );
		m_TexturePreload.Load( Banner::SongBannerTexture(THEME->GetPathG("Banner","random")) );
		m_TexturePreload.Load( Banner::SongBannerTexture(THEME->GetPathG("Banner","mode")) );
	}

	if( GAMESTATE->m_pCurStyle == NULL )
		RageException::Throw( "The Style has not been set.  A theme must set the Style before loading ScreenSelectMusic." );

	if( GAMESTATE->m_PlayMode == PLAY_MODE_INVALID )
		RageException::Throw( "The PlayMode has not been set.  A theme must set the PlayMode before loading ScreenSelectMusic." );

	/* Load low-res banners, if needed. */
	BANNERCACHE->Demand();

	m_MusicWheel.SetName( "MusicWheel" );
	m_MusicWheel.Load( MUSIC_WHEEL_TYPE );
	SET_XY( m_MusicWheel );
	this->AddChild( &m_MusicWheel );

	// this is loaded SetSong and TweenToSong
	m_Banner.SetName( "Banner" );
	m_Banner.SetZTestMode( ZTEST_WRITE_ON_PASS );	// do have to pass the z test
	SET_XY( m_Banner );
	this->AddChild( &m_Banner );

	m_DifficultyDisplay.SetName( "DifficultyDisplay" );
	m_DifficultyDisplay.SetShadowLength( 0 );
	SET_XY( m_DifficultyDisplay );
	this->AddChild( &m_DifficultyDisplay );

	m_sprCDTitleFront.SetName( "CDTitle" );
	m_sprCDTitleFront.Load( THEME->GetPathG(m_sName,"fallback cdtitle") );
	SET_XY( m_sprCDTitleFront );
	COMMAND( m_sprCDTitleFront, "Front" );
	this->AddChild( &m_sprCDTitleFront );

	m_sprCDTitleBack.SetName( "CDTitle" );
	m_sprCDTitleBack.Load( THEME->GetPathG(m_sName,"fallback cdtitle") );
	SET_XY( m_sprCDTitleBack );
	COMMAND( m_sprCDTitleBack, "Back" );
	this->AddChild( &m_sprCDTitleBack );

	m_GrooveRadar.SetName( "Radar" );
	SET_XY( m_GrooveRadar );
	if( SHOW_RADAR )
		this->AddChild( &m_GrooveRadar );

	m_textNumSongs.SetName( "NumSongs" );
	m_textNumSongs.LoadFromFont( THEME->GetPathF(m_sName,"num songs") );
	SET_XY( m_textNumSongs );
	this->AddChild( &m_textNumSongs );

	m_textTotalTime.SetName( "TotalTime" );
	m_textTotalTime.LoadFromFont( THEME->GetPathF(m_sName,"total time") );
	SET_XY( m_textTotalTime );
	this->AddChild( &m_textTotalTime );

	m_MachineRank.SetName( "MachineRank" );
	m_MachineRank.LoadFromFont( THEME->GetPathF(m_sName,"rank") );
	SET_XY( m_MachineRank );
	this->AddChild( &m_MachineRank );

	FOREACH_HumanPlayer( p )
	{
		m_DifficultyMeter[p].SetName( ssprintf("MeterP%d",p+1) );
		m_DifficultyMeter[p].Load( METER_TYPE );
		SET_XY( m_DifficultyMeter[p] );
		this->AddChild( &m_DifficultyMeter[p] );

		m_sprHighScoreFrame[p].SetName( ssprintf("ScoreFrameP%d",p+1) );
		m_sprHighScoreFrame[p].Load( THEME->GetPathG(m_sName,ssprintf("score frame p%d",p+1)) );
		SET_XY( m_sprHighScoreFrame[p] );
		this->AddChild( &m_sprHighScoreFrame[p] );

		m_textHighScore[p].SetName( ssprintf("ScoreP%d",p+1) );
		m_textHighScore[p].LoadFromFont( THEME->GetPathF(m_sName,"score") );
		m_textHighScore[p].SetShadowLength( 0 );
		m_textHighScore[p].RunCommands( CommonMetrics::PLAYER_COLOR.GetValue(p) );
		SET_XY( m_textHighScore[p] );
		this->AddChild( &m_textHighScore[p] );
	}	

	m_sprLongBalloon.Load( THEME->GetPathG(m_sName,"balloon long") );
	m_sprLongBalloon->SetName( "Balloon" );
	m_sprLongBalloon->SetHidden( 1 );
	this->AddChild( m_sprLongBalloon );

	m_sprMarathonBalloon.Load( THEME->GetPathG(m_sName,"balloon marathon") );
	m_sprMarathonBalloon->SetName( "Balloon" );
	m_sprMarathonBalloon->SetHidden( 1 );
	this->AddChild( m_sprMarathonBalloon );

	m_soundDifficultyEasier.Load( THEME->GetPathS(m_sName,"difficulty easier") );
	m_soundDifficultyHarder.Load( THEME->GetPathS(m_sName,"difficulty harder") );
	m_soundOptionsChange.Load( THEME->GetPathS(m_sName,"options") );
	m_soundLocked.Load( THEME->GetPathS(m_sName,"locked") );
	m_soundSelectPressed.Load( THEME->GetPathS(m_sName,"select down"), true );

	this->SortByDrawOrder();
}

void ScreenSelectMusic::BeginScreen()
{
	ScreenWithMenuElements::BeginScreen();
	OPTIONS_MENU_AVAILABLE.Load( m_sName, "OptionsMenuAvailable" );

	// Set up extra stage mods.
	if( GAMESTATE->IsAnExtraStage() )
	{
		// make the preferred group the group of the last song played.
		if( GAMESTATE->m_sPreferredSongGroup == GROUP_ALL  &&  !PREFSMAN->m_bPickExtraStage )
		{
			ASSERT( GAMESTATE->m_pCurSong );
			GAMESTATE->m_sPreferredSongGroup.Set( GAMESTATE->m_pCurSong->m_sGroupName );
		}
		
		Song* pSong;
		Steps* pSteps;
		PlayerOptions po;
		SongOptions so;
		SONGMAN->GetExtraStageInfo( GAMESTATE->IsExtraStage2(), GAMESTATE->GetCurrentStyle(), pSong, pSteps, &po, &so );
		GAMESTATE->m_pCurSong.Set( pSong );
		GAMESTATE->m_pPreferredSong = pSong;
		FOREACH_HumanPlayer( pn )
		{
			GAMESTATE->m_pCurSteps[pn].Set( pSteps );
			GAMESTATE->m_pPlayerState[pn]->m_PlayerOptions.Assign( ModsLevel_Stage, po );
			GAMESTATE->m_PreferredDifficulty[pn].Set( pSteps->GetDifficulty() );
			MESSAGEMAN->Broadcast( ssprintf("PlayerOptionsChangedP%i", pn+1) );
		}
		GAMESTATE->m_SongOptions.Assign( ModsLevel_Stage, so );
		MESSAGEMAN->Broadcast( "SongOptionsChanged" );
	}
	
	// XXX BeginScreen?
	m_MusicWheel.BeginScreen();
		
	m_bMadeChoice = false;
	m_bGoToOptions = false;
	m_bAllowOptionsMenu = m_bAllowOptionsMenuRepeat = false;
	ZERO( m_iSelection );

	AfterMusicChange();

	SOUND->PlayOnceFromAnnouncer( "select music intro" );
}

ScreenSelectMusic::~ScreenSelectMusic()
{
	LOG->Trace( "ScreenSelectMusic::~ScreenSelectMusic()" );
	BANNERCACHE->Undemand();

}

void ScreenSelectMusic::TweenSongPartsOnScreen()
{
	m_GrooveRadar.StopTweening();
	m_GrooveRadar.TweenOnScreen();
}

void ScreenSelectMusic::TweenSongPartsOffScreen()
{
	m_GrooveRadar.TweenOffScreen();
}

void ScreenSelectMusic::SkipSongPartTweens()
{
	m_GrooveRadar.FinishTweening();
}

void ScreenSelectMusic::TweenOnScreen()
{
	ScreenWithMenuElements::TweenOnScreen();

	FOREACH_HumanPlayer( p )
	{
		ON_COMMAND( m_DifficultyMeter[p] );
	}

	TweenSongPartsOnScreen();

	switch( GAMESTATE->m_SortOrder )
	{
	case SORT_ALL_COURSES:
	case SORT_NONSTOP_COURSES:
	case SORT_ONI_COURSES:
	case SORT_ENDLESS_COURSES:
		TweenSongPartsOffScreen();
		SkipSongPartTweens();
		break;
	default:
		break;
	}

	ON_COMMAND( m_Banner );
	ON_COMMAND( m_DifficultyDisplay );
	ON_COMMAND( m_sprCDTitleFront );
	ON_COMMAND( m_sprCDTitleBack );
	ON_COMMAND( m_GrooveRadar );
	ON_COMMAND( m_textNumSongs );
	ON_COMMAND( m_textTotalTime );
	ON_COMMAND( m_MusicWheel );
	ON_COMMAND( m_sprLongBalloon );
	ON_COMMAND( m_sprMarathonBalloon );
	ON_COMMAND( m_MusicWheel );
	ON_COMMAND( m_MachineRank );

	FOREACH_HumanPlayer( p )
	{		
		ON_COMMAND( m_sprHighScoreFrame[p] );
		ON_COMMAND( m_textHighScore[p] );
	}
}

void ScreenSelectMusic::TweenOffScreen()
{
	ScreenWithMenuElements::TweenOffScreen();

	switch( GAMESTATE->m_SortOrder )
	{
	case SORT_ALL_COURSES:
	case SORT_NONSTOP_COURSES:
	case SORT_ONI_COURSES:
	case SORT_ENDLESS_COURSES:
		break;
	default:
		TweenSongPartsOffScreen();
		break;
	}

	OFF_COMMAND( m_Banner );
	OFF_COMMAND( m_DifficultyDisplay );
	OFF_COMMAND( m_sprCDTitleFront );
	OFF_COMMAND( m_sprCDTitleBack );
	OFF_COMMAND( m_GrooveRadar );
	OFF_COMMAND( m_textNumSongs );
	OFF_COMMAND( m_textTotalTime );
	OFF_COMMAND( m_MusicWheel );
	OFF_COMMAND( m_sprLongBalloon );
	OFF_COMMAND( m_sprMarathonBalloon );
	OFF_COMMAND( m_MachineRank );

	FOREACH_HumanPlayer( p )
	{		
		OFF_COMMAND( m_sprHighScoreFrame[p] );
		OFF_COMMAND( m_textHighScore[p] );
		OFF_COMMAND( m_DifficultyMeter[p] );
	}
}


/* This hides elements that are only relevant when displaying a single song,
 * and shows elements for course display.  XXX: Allow different tween commands. */
void ScreenSelectMusic::SwitchDisplayMode( DisplayMode dm )
{
	if( m_DisplayMode == dm )
		return;

	// tween off
	switch( m_DisplayMode )
	{
	case DISPLAY_SONGS:
		TweenSongPartsOffScreen();
		break;
	case DISPLAY_COURSES:
		break;
	case DISPLAY_MODES:
		break;
	}

	// tween on
	m_DisplayMode = dm;
	switch( m_DisplayMode )
	{
	case DISPLAY_SONGS:
		TweenSongPartsOnScreen();
		break;
	case DISPLAY_COURSES:
		break;
	case DISPLAY_MODES:
		break;
	}
}

void ScreenSelectMusic::TweenScoreOnAndOffAfterChangeSort()
{
	FOREACH_HumanPlayer( p )
	{
		m_textHighScore[p].RunCommands( SCORE_SORT_CHANGE_COMMAND(p) );
		m_sprHighScoreFrame[p].RunCommands( SCORE_FRAME_SORT_CHANGE_COMMAND(p) );
	}

	switch( GAMESTATE->m_SortOrder )
	{
	case SORT_ALL_COURSES:
	case SORT_NONSTOP_COURSES:
	case SORT_ONI_COURSES:
	case SORT_ENDLESS_COURSES:
		SwitchDisplayMode( DISPLAY_COURSES );
		break;
	case SORT_MODE_MENU:
		SwitchDisplayMode( DISPLAY_MODES );
		break;
	default:
		SwitchDisplayMode( DISPLAY_SONGS );
		break;
	}
}

/* If bForce is true, the next request will be started even if it might cause a skip. */
void ScreenSelectMusic::CheckBackgroundRequests( bool bForce )
{
	if( g_bCDTitleWaiting )
	{
		/* The CDTitle is normally very small, so we don't bother waiting to display it. */
		RString sPath;
		if( !m_BackgroundLoader.IsCacheFileFinished(g_sCDTitlePath, sPath) )
			return;

		g_bCDTitleWaiting = false;

		RString sCDTitlePath = sPath;

		if( sCDTitlePath.empty() || !IsAFile(sCDTitlePath) )
			sCDTitlePath = g_bWantFallbackCdTitle? m_sFallbackCDTitlePath:RString("");

		if( !sCDTitlePath.empty() )
		{
			TEXTUREMAN->DisableOddDimensionWarning();
			m_sprCDTitleFront.Load( sCDTitlePath );
			m_sprCDTitleBack.Load( sCDTitlePath );
			TEXTUREMAN->EnableOddDimensionWarning();
		}

		m_BackgroundLoader.FinishedWithCachedFile( g_sCDTitlePath );
	}

	/* Loading the rest can cause small skips, so don't do it until the wheel settles. 
	 * Do load if we're transitioning out, though, so we don't miss starting the music
	 * for the options screen if a song is selected quickly.  Also, don't do this
	 * if the wheel is locked, since we're just bouncing around after selecting TYPE_RANDOM,
	 * and it'll take a while before the wheel will settle. */
	if( !m_MusicWheel.IsSettled() && !m_MusicWheel.WheelIsLocked() && !bForce )
		return;

	if( g_bBannerWaiting )
	{
		if( m_Banner.GetTweenTimeLeft() > 0 )
			return;

		RString sPath;
		bool bFreeCache = false;
		if( TEXTUREMAN->IsTextureRegistered( Sprite::SongBannerTexture(g_sBannerPath) ) )
		{
			/* If the file is already loaded into a texture, it's finished,
			 * and we only do this to honor the HighQualTime value. */
			sPath = g_sBannerPath;
		}
		else
		{
			if( !m_BackgroundLoader.IsCacheFileFinished( g_sBannerPath, sPath ) )
				return;
			bFreeCache = true;
		}

		g_bBannerWaiting = false;
		m_Banner.Load( sPath, true );

		if( bFreeCache )
			m_BackgroundLoader.FinishedWithCachedFile( g_sBannerPath );
	}

	/* Nothing else is going.  Start the music, if we havn't yet. */
	if( g_bSampleMusicWaiting )
	{
		/* Don't start the music sample when moving fast. */
		if( g_StartedLoadingAt.Ago() < SAMPLE_MUSIC_DELAY && !bForce )
			return;

		g_bSampleMusicWaiting = false;

		SOUND->PlayMusic(
			m_sSampleMusicToPlay, m_pSampleMusicTimingData,
			true, m_fSampleStartSeconds, m_fSampleLengthSeconds,
			1.5f, /* fade out for 1.5 seconds */
			ALIGN_MUSIC_BEATS );
	}
}

void ScreenSelectMusic::Update( float fDeltaTime )
{
	Screen::Update( fDeltaTime );

	CheckBackgroundRequests( false );
}

void ScreenSelectMusic::Input( const InputEventPlus &input )
{
//	LOG->Trace( "ScreenSelectMusic::Input()" );

	// debugging?
	// I just like being able to see untransliterated titles occasionally.
	if( input.DeviceI.device == DEVICE_KEYBOARD && input.DeviceI.button == KEY_F9 )
	{
		if( input.type != IET_FIRST_PRESS ) 
			return;
		PREFSMAN->m_bShowNativeLanguage.Set( !PREFSMAN->m_bShowNativeLanguage );
		m_MusicWheel.RebuildWheelItems();
		return;
	}

	if( !input.GameI.IsValid() )
		return;		// don't care


	/* XXX: What's the difference between this and StyleI.player? */
	/* StyleI won't be valid if it's a menu button that's pressed.  
	 * There's got to be a better way of doing this.  -Chris */
	PlayerNumber pn = GAMESTATE->GetCurrentStyle()->ControllerToPlayerNumber( input.GameI.controller );
	if( !GAMESTATE->IsHumanPlayer(pn) )
		return;

	// Check for "Press START again for options" button press
	if( m_bMadeChoice  &&
	    input.MenuI.IsValid()  &&
	    input.MenuI.button == MENU_BUTTON_START  &&
	    input.type != IET_RELEASE  &&
	    input.type != IET_LEVEL_CHANGED &&
	    OPTIONS_MENU_AVAILABLE.GetValue() )
	{
		if( m_bGoToOptions )
			return; /* got it already */
		if( !m_bAllowOptionsMenu )
			return; /* not allowed */

		if( !m_bAllowOptionsMenuRepeat &&
		    (input.type == IET_SLOW_REPEAT || input.type == IET_FAST_REPEAT ))
		{
			return; /* not allowed yet */
		}
		
		m_bGoToOptions = true;
		SCREENMAN->PlayStartSound();
		this->PlayCommand( "ShowEnteringOptions" );

		// Re-queue SM_BeginFadingOut, since ShowEnteringOptions may have
		// short-circuited animations.
		this->ClearMessageQueue( SM_BeginFadingOut );
		this->PostScreenMessage( SM_BeginFadingOut, this->GetTweenTimeLeft() );

		return;
	}

	if( IsTransitioning() )
		return;		// ignore

	if( m_bMadeChoice )
		return;		// ignore

	UpdateSelectButton();

	if( input.MenuI.button == MENU_BUTTON_SELECT )
	{
		if( input.type == IET_FIRST_PRESS )
			m_MusicWheel.Move( 0 );

		return;
	}

	if( SELECT_MENU_AVAILABLE && INPUTMAPPER->IsButtonDown( MenuInput(pn, MENU_BUTTON_SELECT) ) )
	{
		if( input.type == IET_FIRST_PRESS )
		{
			switch( input.MenuI.button )
			{
			case MENU_BUTTON_LEFT:
				ChangeDifficulty( pn, -1 );
				return;
			case MENU_BUTTON_RIGHT:
				ChangeDifficulty( pn, +1 );
				return;
			case MENU_BUTTON_START:
				if( MODE_MENU_AVAILABLE )
					m_MusicWheel.ChangeSort( SORT_MODE_MENU );
				else
					m_soundLocked.Play();
				return;
			}
		}
		return;
	}

	switch( input.MenuI.button )
	{
	case MENU_BUTTON_RIGHT:
	case MENU_BUTTON_LEFT:
		{
			/* If we're rouletting, hands off. */
			if( m_MusicWheel.IsRouletting() )
				return;
			
			bool bLeftIsDown = false;
			bool bRightIsDown = false;
			FOREACH_EnabledPlayer( p )
			{
				bLeftIsDown |= INPUTMAPPER->IsButtonDown( MenuInput(p, MENU_BUTTON_LEFT) );
				bRightIsDown |= INPUTMAPPER->IsButtonDown( MenuInput(p, MENU_BUTTON_RIGHT) );
			}
			
			bool bBothDown = bLeftIsDown && bRightIsDown;
			bool bNeitherDown = !bLeftIsDown && !bRightIsDown;
			

			if( bNeitherDown )
			{
				/* Both buttons released. */
				m_MusicWheel.Move( 0 );
			}
			else if( bBothDown )
			{
				m_MusicWheel.Move( 0 );
				if( input.type == IET_FIRST_PRESS )
				{
					switch( input.MenuI.button )
					{
					case MENU_BUTTON_LEFT:
						m_MusicWheel.ChangeMusicUnlessLocked( -1 );
						break;
					case MENU_BUTTON_RIGHT:
						m_MusicWheel.ChangeMusicUnlessLocked( +1 );
						break;
					}
				}
			}
			else if( bLeftIsDown )
			{
				if( input.type != IET_RELEASE )
					m_MusicWheel.Move( -1 );
			}
			else if( bRightIsDown )
			{
				if( input.type != IET_RELEASE )
					m_MusicWheel.Move( +1 );
			}
			else
			{
				ASSERT(0);
			}
			
			
			// Reset the repeat timer when the button is released.
			// This fixes jumping when you release Left and Right after entering the sort 
			// code at the same if L & R aren't released at the exact same time.
			if( input.type == IET_RELEASE )
			{
				FOREACH_HumanPlayer( p )
				{
					INPUTMAPPER->ResetKeyRepeat( MenuInput(p, MENU_BUTTON_LEFT) );
					INPUTMAPPER->ResetKeyRepeat( MenuInput(p, MENU_BUTTON_RIGHT) );
				}
			}
		}
		break;
	}

	// TRICKY:  Do default processing of MenuLeft and MenuRight before detecting 
	// codes.  Do default processing of Start AFTER detecting codes.  This gives us a 
	// change to return if Start is part of a code because we don't want to process 
	// Start as "move to the next screen" if it was just part of a code.
	switch( input.MenuI.button )
	{
	case MENU_BUTTON_UP:	this->MenuUp( input );		break;
	case MENU_BUTTON_DOWN:	this->MenuDown( input );	break;
	case MENU_BUTTON_LEFT:	this->MenuLeft( input );	break;
	case MENU_BUTTON_RIGHT:	this->MenuRight( input );	break;
	case MENU_BUTTON_BACK:
		/* Don't make the user hold the back button if they're pressing escape and escape is the back button. */
		if( input.DeviceI.device == DEVICE_KEYBOARD  &&  input.DeviceI.button == KEY_ESC )
			this->MenuBack( input.MenuI.player );
		else
			Screen::MenuBack( input );
		break;
	// Do the default handler for Start after detecting codes.
//	case MENU_BUTTON_START:	this->MenuStart( input );	break;
	case MENU_BUTTON_COIN:	this->MenuCoin( input );	break;
	}


	if( input.type == IET_FIRST_PRESS )
	{
		if( CodeDetector::EnteredEasierDifficulty(input.GameI.controller) )
		{
			if( GAMESTATE->IsAnExtraStage() )
				m_soundLocked.Play();
			else
				ChangeDifficulty( pn, -1 );
			return;
		}
		if( CodeDetector::EnteredHarderDifficulty(input.GameI.controller) )
		{
			if( GAMESTATE->IsAnExtraStage() )
				m_soundLocked.Play();
			else
				ChangeDifficulty( pn, +1 );
			return;
		}
		if( CodeDetector::EnteredModeMenu(input.GameI.controller) )
		{
			if( MODE_MENU_AVAILABLE )
				m_MusicWheel.ChangeSort( SORT_MODE_MENU );
			else
				m_soundLocked.Play();
			return;
		}
		if( CodeDetector::EnteredNextSort(input.GameI.controller) )
		{
			if( ( GAMESTATE->IsExtraStage() && !PREFSMAN->m_bPickExtraStage ) || GAMESTATE->IsExtraStage2() )
				m_soundLocked.Play();
			else
				m_MusicWheel.NextSort();
			return;
		}
		if( !GAMESTATE->IsExtraStage() && !GAMESTATE->IsExtraStage2() && CodeDetector::DetectAndAdjustMusicOptions(input.GameI.controller) )
		{
			m_soundOptionsChange.Play();
			MESSAGEMAN->Broadcast( ssprintf("PlayerOptionsChangedP%i", pn+1) );
			MESSAGEMAN->Broadcast( "SongOptionsChanged" );
			return;
		}
	}

	switch( input.MenuI.button )
	{
	case MENU_BUTTON_START:	Screen::MenuStart( input ); break;
	}
}

void ScreenSelectMusic::UpdateSelectButton()
{
	bool bSelectIsDown = false;
	FOREACH_EnabledPlayer( p )
		bSelectIsDown |= INPUTMAPPER->IsButtonDown( MenuInput(p, MENU_BUTTON_SELECT) );
	if( !SELECT_MENU_AVAILABLE )
		bSelectIsDown = false;

	/* If m_soundSelectPressed isn't loaded yet, wait until it is before we do this. */
	if( m_bSelectIsDown != bSelectIsDown && m_soundSelectPressed.IsLoaded() )
	{
		if( bSelectIsDown )
			m_soundSelectPressed.Play();

		m_bSelectIsDown = bSelectIsDown;
		if( bSelectIsDown )
			MESSAGEMAN->Broadcast( "SelectMenuOn" );
		else
			MESSAGEMAN->Broadcast( "SelectMenuOff" );
	}
}

void ScreenSelectMusic::ChangeDifficulty( PlayerNumber pn, int dir )
{
	LOG->Trace( "ScreenSelectMusic::ChangeDifficulty( %d, %d )", pn, dir );

	ASSERT( GAMESTATE->IsHumanPlayer(pn) );

	switch( m_MusicWheel.GetSelectedType() )
	{
	case TYPE_SONG:
	case TYPE_PORTAL:
		{
			m_iSelection[pn] += dir;
			if( CLAMP(m_iSelection[pn],0,m_vpSteps.size()-1) )
				return;
			
			// the user explicity switched difficulties.  Update the preferred difficulty
			GAMESTATE->ChangePreferredDifficulty( pn, m_vpSteps[ m_iSelection[pn] ]->GetDifficulty() );

			if( dir < 0 )
				m_soundDifficultyEasier.Play();
			else
				m_soundDifficultyHarder.Play();

			vector<PlayerNumber> vpns;
			FOREACH_HumanPlayer( p )
			{
				if( pn == p || GAMESTATE->DifficultiesLocked() )
				{
					m_iSelection[p] = m_iSelection[pn];
					vpns.push_back( p );
				}
			}
			AfterStepsChange( vpns );
		}
		break;

	case TYPE_COURSE:
		{
			m_iSelection[pn] += dir;
			if( CLAMP(m_iSelection[pn],0,m_vpTrails.size()-1) )
				return;

			// the user explicity switched difficulties.  Update the preferred difficulty
			GAMESTATE->ChangePreferredCourseDifficulty( pn, m_vpTrails[ m_iSelection[pn] ]->m_CourseDifficulty );

			if( dir < 0 )
				m_soundDifficultyEasier.Play();
			else
				m_soundDifficultyHarder.Play();

			vector<PlayerNumber> vpns;
			FOREACH_HumanPlayer( p )
			{
				if( pn == p || GAMESTATE->DifficultiesLocked() )
				{
					m_iSelection[p] = m_iSelection[pn];
					vpns.push_back( p );
				}
			}
			AfterTrailChange( vpns );
		}
		break;

	case TYPE_RANDOM:
	case TYPE_ROULETTE:
	case TYPE_SECTION:
		/* XXX: We could be on a music or course sort, or even one with both; we don't
		 * really know which difficulty to change.  Maybe the two difficulties should be
		 * linked ... */
		if( GAMESTATE->ChangePreferredDifficulty( pn, dir ) )
		{
			if( dir < 0 )
				m_soundDifficultyEasier.Play();
			else
				m_soundDifficultyHarder.Play();

			vector<PlayerNumber> vpns;
			FOREACH_HumanPlayer( p )
			{
				if( pn == p || GAMESTATE->DifficultiesLocked() )
				{
					m_iSelection[p] = m_iSelection[pn];
					vpns.push_back( p );
				}
			}
			AfterStepsChange( vpns );
		}
		break;
	case TYPE_SORT:
		break;
	default:
		WARN( ssprintf("%i", m_MusicWheel.GetSelectedType()) );
		break;
	}
}


void ScreenSelectMusic::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_AllowOptionsMenuRepeat )
	{
		m_bAllowOptionsMenuRepeat = true;
	}
	else if( SM == SM_MenuTimer )
	{
		if( m_MusicWheel.IsRouletting() )
		{
			MenuStart(PLAYER_INVALID);
			m_MenuTimer->SetSeconds( 15 );
			m_MenuTimer->Start();
		}
		else if( DO_ROULETTE_ON_MENU_TIMER )
		{
			if( m_MusicWheel.GetSelectedType() != TYPE_SONG )
			{
				m_MusicWheel.StartRoulette();
				m_MenuTimer->SetSeconds( 15 );
				m_MenuTimer->Start();
			}
			else
			{
				MenuStart(PLAYER_INVALID);
			}
		}
		else
		{
			// Finish sort changing so that the wheel can respond immediately to our
			// request to choose random.
			m_MusicWheel.FinishChangingSorts();
			switch( m_MusicWheel.GetSelectedType() )
			{
			case TYPE_SONG:
			case TYPE_COURSE:
			case TYPE_RANDOM:
			case TYPE_PORTAL:
				break;
			default:
				m_MusicWheel.StartRandom();
				break;
			}
			MenuStart(PLAYER_INVALID);
		}
		return;
	}
	else if( SM == SM_GoToPrevScreen )
	{
		/* We may have stray SM_SongChanged messages from the music wheel.  We can't
		 * handle them anymore, since the title menu (and attract screens) reset
		 * the game state, so just discard them. */
		ClearMessageQueue();
	}
	else if( SM == SM_BeginFadingOut )
	{
		m_bAllowOptionsMenu = false;
		if( OPTIONS_MENU_AVAILABLE && !m_bGoToOptions )
			this->PlayCommand( "HidePressStartForOptions" );

		this->PostScreenMessage( SM_GoToNextScreen, this->GetTweenTimeLeft() );
	}
	else if( SM == SM_GoToNextScreen )
	{
		if( !m_bGoToOptions )
			SOUND->StopMusic();
	}
	else if( SM == SM_SongChanged )
	{
		AfterMusicChange();
	}
	else if( SM == SM_SortOrderChanging ) /* happens immediately */
	{
		TweenScoreOnAndOffAfterChangeSort();
	}
	else if( SM == SM_GainFocus )
	{
		CodeDetector::RefreshCacheItems( CODES );
	}
	else if( SM == SM_LoseFocus )
	{
		CodeDetector::RefreshCacheItems(); /* reset for other screens */
	}

	Screen::HandleScreenMessage( SM );
}

void ScreenSelectMusic::MenuStart( PlayerNumber pn )
{
	/* If false, we don't have a selection just yet. */
	if( !m_MusicWheel.Select() )
		return;

	// a song was selected
	switch( m_MusicWheel.GetSelectedType() )
	{
	case TYPE_SONG:
	case TYPE_PORTAL:
		{
			const bool bIsNew = PROFILEMAN->IsSongNew( m_MusicWheel.GetSelectedSong() );
			bool bIsHard = false;
			FOREACH_HumanPlayer( p )
			{
				if( GAMESTATE->m_pCurSteps[p]  &&  GAMESTATE->m_pCurSteps[p]->GetMeter() >= 10 )
					bIsHard = true;
			}

			/* See if this song is a repeat.  If we're in event mode, only check the last five songs. */
			bool bIsRepeat = false;
			int i = 0;
			if( GAMESTATE->IsEventMode() )
				i = max( 0, int(STATSMAN->m_vPlayedStageStats.size())-5 );
			for( ; i < (int)STATSMAN->m_vPlayedStageStats.size(); ++i )
				if( STATSMAN->m_vPlayedStageStats[i].vpPlayedSongs.back() == m_MusicWheel.GetSelectedSong() )
					bIsRepeat = true;

			/* Don't complain about repeats if the user didn't get to pick. */
			if( GAMESTATE->IsExtraStage() && !PREFSMAN->m_bPickExtraStage )
				bIsRepeat = false;

			if( bIsRepeat )
				SOUND->PlayOnceFromAnnouncer( "select music comment repeat" );
			else if( bIsNew )
				SOUND->PlayOnceFromAnnouncer( "select music comment new" );
			else if( bIsHard )
				SOUND->PlayOnceFromAnnouncer( "select music comment hard" );
			else
				SOUND->PlayOnceFromAnnouncer( "select music comment general" );

			m_bMadeChoice = true;

			/* If we're in event mode, we may have just played a course (putting us
			 * in course mode).  Make sure we're in a single song mode. */
			if( GAMESTATE->IsCourseMode() )
				GAMESTATE->m_PlayMode.Set( PLAY_MODE_REGULAR );
		}
		break;

	case TYPE_COURSE:
		{
			SOUND->PlayOnceFromAnnouncer( "select course comment general" );

			Course *pCourse = m_MusicWheel.GetSelectedCourse();
			ASSERT( pCourse );
			GAMESTATE->m_PlayMode.Set( pCourse->GetPlayMode() );

			// apply #LIVES
			if( pCourse->m_iLives != -1 )
			{
				SO_GROUP_ASSIGN( GAMESTATE->m_SongOptions, ModsLevel_Stage, m_LifeType, SongOptions::LIFE_BATTERY );
				SO_GROUP_ASSIGN( GAMESTATE->m_SongOptions, ModsLevel_Stage, m_iBatteryLives, pCourse->m_iLives );
			}

			m_bMadeChoice = true;
		}
		break;
	case TYPE_SECTION:
	case TYPE_ROULETTE:
	case TYPE_RANDOM:
	case TYPE_SORT:
		/* We havn't made a selection yet. */
		break;
	default:
		ASSERT(0);
	}

	if( m_bMadeChoice )
	{
		SCREENMAN->PlayStartSound();

		if( OPTIONS_MENU_AVAILABLE )
		{
			// show "hold START for options"
			this->PlayCommand( "ShowPressStartForOptions" );

			m_bAllowOptionsMenu = true;
			/* Don't accept a held START for a little while, so it's not
			 * hit accidentally.  Accept an initial START right away, though,
			 * so we don't ignore deliberate fast presses (which would be
			 * annoying). */
			this->PostScreenMessage( SM_AllowOptionsMenuRepeat, 0.5f );
		}

		/* If we're currently waiting on song assets, abort all except the music and
		 * start the music, so if we make a choice quickly before background requests
		 * come through, the music will still start. */
		g_bCDTitleWaiting = g_bBannerWaiting = false;
		m_BackgroundLoader.Abort();
		CheckBackgroundRequests( true );

		if( OPTIONS_MENU_AVAILABLE )
		{
			StartTransitioningScreen( SM_None );
			float fTime = max( SHOW_OPTIONS_MESSAGE_SECONDS, this->GetTweenTimeLeft() );
			this->PostScreenMessage( SM_BeginFadingOut, fTime );
		}
		else
		{
			StartTransitioningScreen( SM_BeginFadingOut );
		}
	}
}


void ScreenSelectMusic::MenuBack( PlayerNumber pn )
{
	m_BackgroundLoader.Abort();

	Cancel( SM_GoToPrevScreen );
}

void ScreenSelectMusic::AfterStepsChange( const vector<PlayerNumber> &vpns )
{
	FOREACH_CONST( PlayerNumber, vpns, p )
	{
		PlayerNumber pn = *p;
		ASSERT( GAMESTATE->IsHumanPlayer(pn) );
		
		CLAMP( m_iSelection[pn], 0, m_vpSteps.size()-1 );

		Song* pSong = GAMESTATE->m_pCurSong;
		Steps* pSteps = m_vpSteps.empty()? NULL: m_vpSteps[m_iSelection[pn]];

		GAMESTATE->m_pCurSteps[pn].Set( pSteps );
		GAMESTATE->m_pCurTrail[pn].Set( NULL );

		int iScore = 0;
		if( pSteps )
		{
			Profile* pProfile = PROFILEMAN->IsPersistentProfile(pn) ? PROFILEMAN->GetProfile(pn) : PROFILEMAN->GetMachineProfile();
			iScore = pProfile->GetStepsHighScoreList(pSong,pSteps).GetTopScore().GetScore();
		}

		m_textHighScore[pn].SetText( ssprintf("%*i", NUM_SCORE_DIGITS, iScore) );
		
		m_DifficultyMeter[pn].SetFromGameState( pn );
		m_GrooveRadar.SetFromSteps( pn, pSteps );
		m_MusicWheel.NotesOrTrailChanged( pn );
	}
}

void ScreenSelectMusic::AfterTrailChange( const vector<PlayerNumber> &vpns )
{
	FOREACH_CONST( PlayerNumber, vpns, p )
	{
		PlayerNumber pn = *p;
		ASSERT( GAMESTATE->IsHumanPlayer(pn) );
		
		CLAMP( m_iSelection[pn], 0, m_vpTrails.size()-1 );

		Course* pCourse = GAMESTATE->m_pCurCourse;
		Trail* pTrail = m_vpTrails.empty()? NULL: m_vpTrails[m_iSelection[pn]];

		GAMESTATE->m_pCurSteps[pn].Set( NULL );
		GAMESTATE->m_pCurTrail[pn].Set( pTrail );

		int iScore = 0;
		if( pTrail )
		{
			Profile* pProfile = PROFILEMAN->IsPersistentProfile(pn) ? PROFILEMAN->GetProfile(pn) : PROFILEMAN->GetMachineProfile();
			iScore = pProfile->GetCourseHighScoreList(pCourse,pTrail).GetTopScore().GetScore();
		}

		m_textHighScore[pn].SetText( ssprintf("%*i", NUM_SCORE_DIGITS, iScore) );
		
		m_DifficultyMeter[pn].SetFromGameState( pn );
		m_GrooveRadar.SetEmpty( pn );
		m_MusicWheel.NotesOrTrailChanged( pn );
	}
}

void ScreenSelectMusic::SwitchToPreferredDifficulty()
{
	if( !GAMESTATE->m_pCurCourse )
	{
		FOREACH_HumanPlayer( pn )
		{
			/* Find the closest match to the user's preferred difficulty. */
			int iCurDifference = -1;
			int &iSelection = m_iSelection[pn];
			for( unsigned i=0; i<m_vpSteps.size(); i++ )
			{
				/* If the current steps are listed, use them. */
				if( GAMESTATE->m_pCurSteps[pn] == m_vpSteps[i] )
				{
					iSelection = i;
					break;
				}

				if( GAMESTATE->m_PreferredDifficulty[pn] != DIFFICULTY_INVALID )
				{
					int iDiff = abs(m_vpSteps[i]->GetDifficulty() - GAMESTATE->m_PreferredDifficulty[pn]);

					if( iCurDifference == -1 || iDiff < iCurDifference )
					{
						iSelection = i;
						iCurDifference = iDiff;
					}
				}
			}

			CLAMP( iSelection, 0, m_vpSteps.size()-1 );
		}
	}
	else
	{
		FOREACH_HumanPlayer( pn )
		{
			/* Find the closest match to the user's preferred difficulty. */
			int iCurDifference = -1;
			int &iSelection = m_iSelection[pn];
			for( unsigned i=0; i<m_vpTrails.size(); i++ )
			{
				/* If the current trail is listed, use it. */
				if( GAMESTATE->m_pCurTrail[pn] == m_vpTrails[i] )
				{
					iSelection = i;
					break;
				}

				int iDiff = abs(m_vpTrails[i]->m_CourseDifficulty - GAMESTATE->m_PreferredCourseDifficulty[pn]);

				if( iCurDifference == -1 || iDiff < iCurDifference )
				{
					iSelection = i;
					iCurDifference = iDiff;
				}
			}

			CLAMP( iSelection, 0, m_vpTrails.size()-1 );
		}
	}
}

template<class T>
int FindCourseIndexOfSameMode( T begin, T end, const Course *p )
{
	const PlayMode pm = p->GetPlayMode();
	
	int n = 0;
	for( T it = begin; it != end; ++it )
	{
		if( *it == p )
			return n;

		/* If it's not playable in this mode, don't increment.  It might result in 
		 * different output in different modes, but that's better than having holes. */
		if( !(*it)->IsPlayableIn( GAMESTATE->GetCurrentStyle()->m_StepsType ) )
			continue;
		if( (*it)->GetPlayMode() != pm )
			continue;
		++n;
	}

	return -1;
}

void ScreenSelectMusic::AfterMusicChange()
{
	if( !m_MusicWheel.IsRouletting() )
		m_MenuTimer->Stall();

	// lock difficulties.  When switching from arcade to rave, we need to 
	// enforce that all players are at the same difficulty.
	if( GAMESTATE->DifficultiesLocked() )
	{
		FOREACH_HumanPlayer( p )
		{
			m_iSelection[p] = m_iSelection[0];
			GAMESTATE->m_PreferredDifficulty[p] = GAMESTATE->m_PreferredDifficulty[0];
		}
	}

	Song* pSong = m_MusicWheel.GetSelectedSong();
	GAMESTATE->m_pCurSong.Set( pSong );
	if( pSong )
		GAMESTATE->m_pPreferredSong = pSong;

	Course* pCourse = m_MusicWheel.GetSelectedCourse();
	GAMESTATE->m_pCurCourse.Set( pCourse );
	if( pCourse )
		GAMESTATE->m_pPreferredCourse = pCourse;

	m_vpSteps.clear();
	m_vpTrails.clear();

	m_Banner.SetMovingFast( !!m_MusicWheel.IsMoving() );

	vector<RString> m_Artists, m_AltArtists;

	m_MachineRank.SetText( "" );

	m_sSampleMusicToPlay = "";
	m_pSampleMusicTimingData = NULL;
	g_sCDTitlePath = "";
	g_sBannerPath = "";
	g_bWantFallbackCdTitle = false;
	bool bWantBanner = true;

	static SortOrder s_lastSortOrder = SORT_INVALID;
	if( GAMESTATE->m_SortOrder != s_lastSortOrder )
	{
		// Reload to let Lua metrics have a chance to change the help text.
		s_lastSortOrder = GAMESTATE->m_SortOrder;
	}

	switch( m_MusicWheel.GetSelectedType() )
	{
	case TYPE_SECTION:
	case TYPE_SORT:
		{	
			RString sGroup = m_MusicWheel.GetSelectedSection();
			FOREACH_PlayerNumber( p )
				m_iSelection[p] = -1;

			g_sCDTitlePath = ""; // none
			m_DifficultyDisplay.UnsetDifficulties();

			m_fSampleStartSeconds = 0;
			m_fSampleLengthSeconds = -1;

			m_textNumSongs.SetText( "" );
			m_textTotalTime.SetText( "" );
			
			switch( m_MusicWheel.GetSelectedType() )
			{
			case TYPE_SECTION:
				g_sBannerPath = SONGMAN->GetSongGroupBannerPath( sGroup );
				m_sSampleMusicToPlay = m_sSectionMusicPath;
				break;
			case TYPE_SORT:
				bWantBanner = false; /* we load it ourself */
				switch( GAMESTATE->m_SortOrder )
				{
				case SORT_MODE_MENU:
					m_Banner.LoadMode();
					break;
				}
				m_sSampleMusicToPlay = m_sSortMusicPath;
				break;
			default:
				ASSERT(0);
			}

			COMMAND( m_sprLongBalloon, "Hide" );
			COMMAND( m_sprMarathonBalloon, "Hide" );
		}
		break;
	case TYPE_SONG:
	case TYPE_PORTAL:
		{
			m_sSampleMusicToPlay = pSong->GetMusicPath();
			m_pSampleMusicTimingData = &pSong->m_Timing;
			m_fSampleStartSeconds = pSong->m_fMusicSampleStartSeconds;
			m_fSampleLengthSeconds = pSong->m_fMusicSampleLengthSeconds;

			m_textNumSongs.SetText( ssprintf("%d", SongManager::GetNumStagesForSong(pSong) ) );
			m_textTotalTime.SetText( SecondsToMMSSMsMs(pSong->m_fMusicLengthSeconds) );

			SongUtil::GetSteps( pSong, m_vpSteps, GAMESTATE->GetCurrentStyle()->m_StepsType );
			StepsUtil::RemoveLockedSteps( pSong, m_vpSteps );
			StepsUtil::SortNotesArrayByDifficulty( m_vpSteps );

			if ( PREFSMAN->m_bShowBanners )
				g_sBannerPath = pSong->GetBannerPath();

			g_sCDTitlePath = pSong->GetCDTitlePath();
			g_bWantFallbackCdTitle = true;

			const vector<Song*> best = SONGMAN->GetPopularSongs( ProfileSlot_Machine );
			const int index = FindIndex( best.begin(), best.end(), pSong );
			if( index != -1 )
				m_MachineRank.SetText( FormatNumberAndSuffix( index+1 ) );

			m_DifficultyDisplay.SetDifficulties( pSong, GAMESTATE->GetCurrentStyle()->m_StepsType );

			SwitchToPreferredDifficulty();

			/* Short delay before actually showing these, so they don't show
			 * up when scrolling fast.  It'll still show up in "slow" scrolling,
			 * but it doesn't look at weird as it does in "fast", and I don't
			 * like the effect with a lot of delay. */
			if( pSong->m_fMusicLengthSeconds > PREFSMAN->m_fMarathonVerSongSeconds )
			{
				SET_XY( m_sprMarathonBalloon );
				COMMAND( m_sprMarathonBalloon, "Show" );
				COMMAND( m_sprLongBalloon, "Hide" );
			}
			else if( pSong->m_fMusicLengthSeconds > PREFSMAN->m_fLongVerSongSeconds )
			{
				SET_XY( m_sprLongBalloon );
				COMMAND( m_sprLongBalloon, "Show" );
				COMMAND( m_sprMarathonBalloon, "Hide" );
			}
			else
			{
				COMMAND( m_sprLongBalloon, "Hide" );
				COMMAND( m_sprMarathonBalloon, "Hide" );
			}
		}
		break;
	case TYPE_ROULETTE:
	case TYPE_RANDOM:
		bWantBanner = false; /* we load it ourself */
		switch(m_MusicWheel.GetSelectedType())
		{
		case TYPE_ROULETTE:	m_Banner.LoadRoulette();	break;
		case TYPE_RANDOM: 	m_Banner.LoadRandom();		break;
		default: ASSERT(0);
		}
		g_sCDTitlePath = ""; // none
		m_DifficultyDisplay.UnsetDifficulties();

		m_fSampleStartSeconds = 0;
		m_fSampleLengthSeconds = -1;

		m_textNumSongs.SetText( "" );
		m_textTotalTime.SetText( "" );

		switch( m_MusicWheel.GetSelectedType() )
		{
		case TYPE_ROULETTE:
			m_sSampleMusicToPlay = m_sRouletteMusicPath;
			break;
		case TYPE_RANDOM:
			m_sSampleMusicToPlay = m_sRandomMusicPath;
			break;
		default:
			ASSERT(0);
		}

		COMMAND( m_sprLongBalloon, "Hide" );
		COMMAND( m_sprMarathonBalloon, "Hide" );
		
		break;
	case TYPE_COURSE:
	{
		Course* pCourse = m_MusicWheel.GetSelectedCourse();
		StepsType st = GAMESTATE->GetCurrentStyle()->m_StepsType;
		Trail *pTrail = pCourse->GetTrail( st );
		ASSERT( pTrail );

		pCourse->GetTrails( m_vpTrails, GAMESTATE->GetCurrentStyle()->m_StepsType );

		m_sSampleMusicToPlay = m_sCourseMusicPath;
		m_fSampleStartSeconds = 0;
		m_fSampleLengthSeconds = -1;

		m_textNumSongs.SetText( ssprintf("%d", pCourse->GetEstimatedNumStages()) );
		float fTotalSeconds;
		if( pCourse->GetTotalSeconds(st,fTotalSeconds) )
			m_textTotalTime.SetText( SecondsToMMSSMsMs(fTotalSeconds) );
		else
			m_textTotalTime.SetText( "xx:xx.xx" );	// The numbers format doesn't have a '?'.  Is there a better solution?

		g_sBannerPath = pCourse->m_sBannerPath;
		if( g_sBannerPath.empty() )
			m_Banner.LoadFallback();

		m_DifficultyDisplay.UnsetDifficulties();

		SwitchToPreferredDifficulty();

		CourseType ct = PlayModeToCourseType( GAMESTATE->m_PlayMode );
		const vector<Course*> best = SONGMAN->GetPopularCourses( ct, ProfileSlot_Machine );
		const int index = FindCourseIndexOfSameMode( best.begin(), best.end(), pCourse );
		if( index != -1 )
			m_MachineRank.SetText( FormatNumberAndSuffix( index+1 ) );



		COMMAND( m_sprLongBalloon, "Hide" );
		COMMAND( m_sprMarathonBalloon, "Hide" );

		break;
	}
	default:
		ASSERT(0);
	}

	m_sprCDTitleFront.UnloadTexture();
	m_sprCDTitleBack.UnloadTexture();

	/* Cancel any previous, incomplete requests for song assets, since we need new ones. */
	m_BackgroundLoader.Abort();

	g_bCDTitleWaiting = false;
	if( !g_sCDTitlePath.empty() || g_bWantFallbackCdTitle )
	{
		LOG->Trace( "cache \"%s\"", g_sCDTitlePath.c_str());
		m_BackgroundLoader.CacheFile( g_sCDTitlePath ); // empty OK
		g_bCDTitleWaiting = true;
	}

	g_bBannerWaiting = false;
	if( bWantBanner )
	{
		LOG->Trace("LoadFromCachedBanner(%s)",g_sBannerPath .c_str());
		if( m_Banner.LoadFromCachedBanner( g_sBannerPath ) )
		{
			/* If the high-res banner is already loaded, just
			 * delay before loading it, so the low-res one has
			 * time to fade in. */
			if( !TEXTUREMAN->IsTextureRegistered( Sprite::SongBannerTexture(g_sBannerPath) ) )
				m_BackgroundLoader.CacheFile( g_sBannerPath );

			g_bBannerWaiting = true;
		}
	}

	// Don't stop music if it's already playing the right file.
	g_bSampleMusicWaiting = false;
	if( !m_MusicWheel.IsRouletting() && SOUND->GetMusicPath() != m_sSampleMusicToPlay )
	{
		SOUND->StopMusic();
		if( !m_sSampleMusicToPlay.empty() )
			g_bSampleMusicWaiting = true;
	}

	g_StartedLoadingAt.Touch();

	vector<PlayerNumber> vpns;
	FOREACH_HumanPlayer( p )
		vpns.push_back( p );

	if( GAMESTATE->m_pCurCourse )
		AfterTrailChange( vpns );
	else
		AfterStepsChange( vpns );
}

// lua start
#include "LuaBinding.h"

class LunaScreenSelectMusic: public Luna<ScreenSelectMusic>
{
public:
	LunaScreenSelectMusic() { LUA->Register( Register ); }

	static int GetGoToOptions( T* p, lua_State *L ) { lua_pushboolean( L, p->GetGoToOptions() ); return 1; }
	static void Register( Lua *L )
	{
  		ADD_METHOD( GetGoToOptions );

		Luna<T>::Register( L );
	}
};

LUA_REGISTER_DERIVED_CLASS( ScreenSelectMusic, ScreenWithMenuElements )
// lua end

/*
 * (c) 2001-2004 Chris Danford
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

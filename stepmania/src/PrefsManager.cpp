#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: PrefsManager

 Desc: See Header.

 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Chris Gomez
-----------------------------------------------------------------------------
*/

#include "PrefsManager.h"
#include "IniFile.h"
#include "GameState.h"
#include "RageDisplay.h"
#include "RageUtil.h"
#include "arch/arch.h" /* for default driver specs */
#include "RageSoundReader_Resample.h" /* for ResampleQuality */
#include "RageFile.h"
#include "ProductInfo.h"

#define STEPMANIA_INI_PATH "Data/StepMania.ini"
#define STATIC_INI_PATH "Data/Static.ini"

PrefsManager*	PREFSMAN = NULL;	// global and accessable from anywhere in our program

const float DEFAULT_SOUND_VOLUME = 1.00f;
const CString DEFAULT_LIGHTS_DRIVER = "Null";

bool g_bAutoRestart = false;

PrefsManager::PrefsManager()
{
#ifdef DEBUG
	m_bWindowed = true;
#else
	m_bWindowed = false;
#endif
	m_iDisplayWidth = 640;
	m_iDisplayHeight = 480;
	m_iDisplayColorDepth = 16;
	m_iTextureColorDepth = 16;		// default to 16 for better preformance on slower cards
	m_iMovieColorDepth = 16;
	m_iMaxTextureResolution = 2048;
	m_iRefreshRate = REFRESH_DEFAULT;
	m_bOnlyDedicatedMenuButtons = false;
#ifdef DEBUG
	m_bShowStats = true;
#else
	m_bShowStats = false;
#endif
	m_bShowBanners = true ;
	m_BackgroundMode = BGMODE_ANIMATIONS;
	m_iNumBackgrounds = 8;
	m_bShowDanger = true;
	m_fBGBrightness = 0.8f;
	m_bMenuTimer = true;
	m_iNumArcadeStages = 3;
	m_bEventMode = false;
	m_bAutoPlay = false;
	m_fJudgeWindowScale = 1.0f;
	m_fLifeDifficultyScale = 1.0f;
	m_fJudgeWindowMarvelousSeconds = 0.0225f;
	m_fJudgeWindowPerfectSeconds = 0.045f;
	m_fJudgeWindowGreatSeconds = 0.090f;
	m_fJudgeWindowGoodSeconds = 0.135f;
	m_fJudgeWindowBooSeconds = 0.180f;
	m_fJudgeWindowOKSeconds = 0.250f;	// allow enough time to take foot off and put back on
	m_fJudgeWindowMineSeconds = 0.135f;
	m_fJudgeWindowAttackSeconds = 0.135f;
	m_fLifeDeltaMarvelousPercentChange = +0.008f;
	m_fLifeDeltaPerfectPercentChange = +0.008f;
	m_fLifeDeltaGreatPercentChange = +0.004f;
	m_fLifeDeltaGoodPercentChange = +0.000f;
	m_fLifeDeltaBooPercentChange = -0.040f;
	m_fLifeDeltaMissPercentChange = -0.080f;
	m_fLifeDeltaOKPercentChange = +0.008f;
	m_fLifeDeltaNGPercentChange = -0.080f;
	m_fLifeDeltaMinePercentChange = -0.160f;
	m_iRegenComboAfterFail = 10; // cumulative
	m_iRegenComboAfterMiss = 5; // cumulative
	m_iMaxRegenComboAfterFail = 10;
	m_iMaxRegenComboAfterMiss = 10;
	m_bTwoPlayerRecovery = true;
	m_bDelayedEscape = true;
	m_bInstructions = true;
	m_bShowDontDie = true;
	m_bShowSelectGroup = true;
	m_bShowNative = true;
	m_bArcadeOptionsNavigation = false;
	m_bSoloSingle = false;
	m_bDelayedTextureDelete = true;
	m_bTexturePreload = false;
	m_bDelayedScreenLoad = false;
	m_BannerCache = BNCACHE_LOW_RES;
	m_MusicWheelUsesSections = ALWAYS;
	m_iMusicWheelSwitchSpeed = 10;
	m_bEasterEggs = true;
	m_iMarvelousTiming = 2;
	m_iCoinMode = COIN_HOME;
	m_iCoinsPerCredit = 1;
	m_Premium = NO_PREMIUM;
	m_iBoostAppPriority = -1;
	m_bSmoothLines = false;
	m_ShowSongOptions = YES;
	m_bDancePointsForOni = false;
	m_bPercentageScoring = false;
	m_fMinPercentageForHighScore = 0.5f;
	m_bShowLyrics = true;
	m_bAutogenMissingTypes = true;
	m_bAutogenGroupCourses = true;
	m_bBreakComboToGetItem = false;
	m_ShowDancingCharacters = CO_OFF;
	m_bUseUnlockSystem = false;
	m_bFirstRun = true;
	m_bAutoMapOnJoyChange = true;
	m_fGlobalOffsetSeconds = 0;
	m_bShowBeginnerHelper = false;
	m_bEndlessBreakEnabled = true;
	m_iEndlessNumStagesUntilBreak = 5;
	m_iEndlessBreakLength = 5;

	// set to 0 so people aren't shocked at first
	m_iProgressiveLifebar = 0;
	m_iProgressiveNonstopLifebar = 0;
	m_iProgressiveStageLifebar = 0;

	/* DDR Extreme-style extra stage support.
	 * Default off so people used to the current behavior (or those with extra
	 * stage CRS files) don't get it changed around on them. */
	m_bPickExtraStage = false;

	m_bComboContinuesBetweenSongs = false;

	// default to old sort order
	m_iCourseSortOrder = COURSE_SORT_SONGS;
	m_bMoveRandomToEnd = false;
	m_iScoringType = SCORING_MAX2;

	m_iGetRankingName = RANKING_ON;

	m_fLongVerSongSeconds = 60*2.5f;	// Dynamite Rave is 2:55
	m_fMarathonVerSongSeconds = 60*5.f;

	/* I'd rather get occasional people asking for support for this even though it's
	 * already here than lots of people asking why songs aren't being displayed. */
	m_bHiddenSongs = false;
	m_bVsync = true;
	m_sLanguage = "";	// ThemeManager will deal with this invalid language

	m_iCenterImageTranslateX = 0;
	m_iCenterImageTranslateY = 0;
	m_fCenterImageScaleX = 1;
	m_fCenterImageScaleY = 1;

	m_bAttractSound = true;
	m_bDemonstrationSound = true;
	m_bAllowExtraStage = true;
	g_bAutoRestart = false;

	/* XXX: Set these defaults for individual consoles using VideoCardDefaults.ini. */
#ifdef _XBOX
	m_bInterlaced = true;
	m_bPAL = false;
	m_fScreenPosX = 0 ;
	m_fScreenPosY = 0 ;
	m_fScreenWidth = 640 ;
	m_fScreenHeight = 480 ;
#else
	m_bInterlaced = false;
#endif
	m_sSoundDrivers = DEFAULT_SOUND_DRIVER_LIST;
	/* Number of frames to write ahead; usually 44100 frames per second.
	 * (Number of millisec would be more flexible, but it's more useful to
	 * specify numbers directly.) This is purely a troubleshooting option
	 * and is not honored by all sound drivers. */
	m_iSoundWriteAhead = 0;
	m_sMovieDrivers = DEFAULT_MOVIE_DRIVER_LIST;

	// StepMania.cpp sets these on first run:
	m_sVideoRenderers = "";
#if defined(WIN32)
	m_iLastSeenMemory = 0;
#endif

	m_fSoundVolume = DEFAULT_SOUND_VOLUME;
	m_sLightsDriver = DEFAULT_LIGHTS_DRIVER;
	/* This is experimental: let's see if preloading helps people's skipping.
	 * If it doesn't do anything useful, it'll be removed. */
	m_bSoundPreloadAll = false;
	m_iSoundResampleQuality = RageSoundReader_Resample::RESAMP_NORMAL;

	m_bAllowUnacceleratedRenderer = false;
	m_bThreadedInput = true;
	m_bScreenTestMode = false;
	m_sIgnoredMessageWindows = "";

	m_sCoursesToShowRanking = "";

	m_bLogging = true;
	m_bForceLogFlush = false;
#ifdef DEBUG
	m_bShowLogOutput = true;
#else
	m_bShowLogOutput = false;
#endif
	m_bTimestamping = false;
	m_bLogCheckpoints = false;

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		m_iMemoryCardUsbBus[p] = -1;
		m_iMemoryCardUsbPort[p] = -1;
	}

	m_sMemoryCardProfileSubdir = PRODUCT_NAME;

	ReadGlobalPrefsFromDisk();
}

PrefsManager::~PrefsManager()
{
}

void PrefsManager::ReadGlobalPrefsFromDisk()
{
	IniFile ini;
	ini.SetPath( STEPMANIA_INI_PATH );
	ini.ReadFile();

	/* Load this on top of the regular INI; if it exists, any settings listed
	 * in it will override user settings. */
	ini.SetPath( STATIC_INI_PATH );
	ini.ReadFile();

	ini.GetValue( "Options", "Windowed",						m_bWindowed );
	ini.GetValue( "Options", "Interlaced",						m_bInterlaced );
#ifdef _XBOX
	ini.GetValue( "Options", "PAL",								m_bPAL );
	ini.GetValue( "Options", "ScreenPosX",						m_fScreenPosX );
	ini.GetValue( "Options", "ScreenPosY",						m_fScreenPosY );
	ini.GetValue( "Options", "ScreenWidth",						m_fScreenWidth );
	ini.GetValue( "Options", "ScreenHeight",					m_fScreenHeight );
#endif
	ini.GetValue( "Options", "DisplayWidth",					m_iDisplayWidth );
	ini.GetValue( "Options", "DisplayHeight",					m_iDisplayHeight );
	ini.GetValue( "Options", "DisplayColorDepth",				m_iDisplayColorDepth );
	ini.GetValue( "Options", "TextureColorDepth",				m_iTextureColorDepth );
	ini.GetValue( "Options", "MovieColorDepth",					m_iMovieColorDepth );
	ini.GetValue( "Options", "MaxTextureResolution",			m_iMaxTextureResolution );
	ini.GetValue( "Options", "RefreshRate",						m_iRefreshRate );
	ini.GetValue( "Options", "UseDedicatedMenuButtons",			m_bOnlyDedicatedMenuButtons );
	ini.GetValue( "Options", "ShowStats",						m_bShowStats );
	ini.GetValue( "Options", "ShowBanners",						m_bShowBanners );
	ini.GetValue( "Options", "BackgroundMode",					(int&)m_BackgroundMode );
	ini.GetValue( "Options", "NumBackgrounds",					m_iNumBackgrounds);
	ini.GetValue( "Options", "ShowDanger",						m_bShowDanger );
	ini.GetValue( "Options", "BGBrightness",					m_fBGBrightness );
	ini.GetValue( "Options", "MenuTimer",						m_bMenuTimer );
	ini.GetValue( "Options", "NumArcadeStages",					m_iNumArcadeStages );
	ini.GetValue( "Options", "EventMode",						m_bEventMode );
	ini.GetValue( "Options", "AutoPlay",						m_bAutoPlay );
	ini.GetValue( "Options", "JudgeWindowScale",				m_fJudgeWindowScale );
	ini.GetValue( "Options", "LifeDifficultyScale",				m_fLifeDifficultyScale );
	ini.GetValue( "Options", "JudgeWindowMarvelousSeconds",		m_fJudgeWindowMarvelousSeconds );
	ini.GetValue( "Options", "JudgeWindowPerfectSeconds",		m_fJudgeWindowPerfectSeconds );
	ini.GetValue( "Options", "JudgeWindowGreatSeconds",			m_fJudgeWindowGreatSeconds );
	ini.GetValue( "Options", "JudgeWindowGoodSeconds",			m_fJudgeWindowGoodSeconds );
	ini.GetValue( "Options", "JudgeWindowBooSeconds",			m_fJudgeWindowBooSeconds );
	ini.GetValue( "Options", "JudgeWindowOKSeconds",			m_fJudgeWindowOKSeconds );
	ini.GetValue( "Options", "JudgeWindowMineSeconds",			m_fJudgeWindowMineSeconds );
	ini.GetValue( "Options", "JudgeWindowAttackSeconds",		m_fJudgeWindowAttackSeconds );
	ini.GetValue( "Options", "LifeDeltaMarvelousPercentChange",	m_fLifeDeltaMarvelousPercentChange );
	ini.GetValue( "Options", "LifeDeltaPerfectPercentChange",	m_fLifeDeltaPerfectPercentChange );
	ini.GetValue( "Options", "LifeDeltaGreatPercentChange",		m_fLifeDeltaGreatPercentChange );
	ini.GetValue( "Options", "LifeDeltaGoodPercentChange",		m_fLifeDeltaGoodPercentChange );
	ini.GetValue( "Options", "LifeDeltaBooPercentChange",		m_fLifeDeltaBooPercentChange );
	ini.GetValue( "Options", "LifeDeltaMissPercentChange",		m_fLifeDeltaMissPercentChange );
	ini.GetValue( "Options", "LifeDeltaOKPercentChange",		m_fLifeDeltaOKPercentChange );
	ini.GetValue( "Options", "LifeDeltaNGPercentChange",		m_fLifeDeltaNGPercentChange );
	ini.GetValue( "Options", "LifeDeltaMinePercentChange",		m_fLifeDeltaMinePercentChange );
	ini.GetValue( "Options", "RegenComboAfterFail",				m_iRegenComboAfterFail );
	ini.GetValue( "Options", "RegenComboAfterMiss",				m_iRegenComboAfterMiss );
	ini.GetValue( "Options", "MaxRegenComboAfterFail",			m_iMaxRegenComboAfterFail );
	ini.GetValue( "Options", "MaxRegenComboAfterMiss",			m_iMaxRegenComboAfterMiss );
	ini.GetValue( "Options", "TwoPlayerRecovery",				m_bTwoPlayerRecovery );
	ini.GetValue( "Options", "DelayedEscape",					m_bDelayedEscape );
	ini.GetValue( "Options", "HiddenSongs",						m_bHiddenSongs );
	ini.GetValue( "Options", "Vsync",							m_bVsync );
	ini.GetValue( "Options", "HowToPlay",						m_bInstructions );
	ini.GetValue( "Options", "Caution",							m_bShowDontDie );
	ini.GetValue( "Options", "ShowSelectGroup",					m_bShowSelectGroup );
	ini.GetValue( "Options", "ShowNative",						m_bShowNative );
	ini.GetValue( "Options", "ArcadeOptionsNavigation",			m_bArcadeOptionsNavigation );
	ini.GetValue( "Options", "DWIPath",							m_DWIPath );
	ini.GetValue( "Options", "DelayedTextureDelete",			m_bDelayedTextureDelete );
	ini.GetValue( "Options", "TexturePreload",					m_bTexturePreload );
	ini.GetValue( "Options", "DelayedScreenLoad",				m_bDelayedScreenLoad );
	ini.GetValue( "Options", "BannerCache",						(int&)m_BannerCache );
	ini.GetValue( "Options", "MusicWheelUsesSections",			(int&)m_MusicWheelUsesSections );
	ini.GetValue( "Options", "MusicWheelSwitchSpeed",			m_iMusicWheelSwitchSpeed );
	ini.GetValue( "Options", "SoundDrivers",					m_sSoundDrivers );
	ini.GetValue( "Options", "SoundWriteAhead",					m_iSoundWriteAhead );
	ini.GetValue( "Options", "MovieDrivers",					m_sMovieDrivers );
	ini.GetValue( "Options", "EasterEggs",						m_bEasterEggs );
	ini.GetValue( "Options", "MarvelousTiming",					(int&)m_iMarvelousTiming );
	ini.GetValue( "Options", "SoundVolume",						m_fSoundVolume );
	ini.GetValue( "Options", "LightsDriver",					m_sLightsDriver );
	ini.GetValue( "Options", "SoundPreloadAll",					m_bSoundPreloadAll );
	ini.GetValue( "Options", "SoundResampleQuality",			m_iSoundResampleQuality );
	ini.GetValue( "Options", "CoinMode",						m_iCoinMode );
	ini.GetValue( "Options", "CoinsPerCredit",					m_iCoinsPerCredit );
	ini.GetValue( "Options", "Premium",							(int&)m_Premium );
	ini.GetValue( "Options", "BoostAppPriority",				m_iBoostAppPriority );
	ini.GetValue( "Options", "PickExtraStage",					m_bPickExtraStage );
	ini.GetValue( "Options", "ComboContinuesBetweenSongs",		m_bComboContinuesBetweenSongs );
	ini.GetValue( "Options", "LongVerSeconds",					m_fLongVerSongSeconds );
	ini.GetValue( "Options", "MarathonVerSeconds",				m_fMarathonVerSongSeconds );
	ini.GetValue( "Options", "ShowSongOptions",					(int&)m_ShowSongOptions );
	ini.GetValue( "Options", "AllowUnacceleratedRenderer",		m_bAllowUnacceleratedRenderer );
	ini.GetValue( "Options", "ThreadedInput",					m_bThreadedInput );
	ini.GetValue( "Options", "ScreenTestMode",					m_bScreenTestMode );
	ini.GetValue( "Options", "IgnoredMessageWindows",			m_sIgnoredMessageWindows );
	ini.GetValue( "Options", "SoloSingle",						m_bSoloSingle );
	ini.GetValue( "Options", "DancePointsForOni",				m_bDancePointsForOni );
	ini.GetValue( "Options", "PercentageScoring",				m_bPercentageScoring );
	ini.GetValue( "Options", "MinPercentageForHighScore",		m_fMinPercentageForHighScore );
	ini.GetValue( "Options", "ShowLyrics",						m_bShowLyrics );
	ini.GetValue( "Options", "AutogenMissingTypes",				m_bAutogenMissingTypes );
	ini.GetValue( "Options", "AutogenGroupCourses",				m_bAutogenGroupCourses );
	ini.GetValue( "Options", "BreakComboToGetItem",				m_bBreakComboToGetItem );
	ini.GetValue( "Options", "ShowDancingCharacters",			(int&)m_ShowDancingCharacters );

	ini.GetValue( "Options", "CourseSortOrder",					(int&)m_iCourseSortOrder );
	ini.GetValue( "Options", "MoveRandomToEnd",					m_bMoveRandomToEnd );

	ini.GetValue( "Options", "ScoringType",						(int&)m_iScoringType );

	ini.GetValue( "Options", "ProgressiveLifebar",				m_iProgressiveLifebar );
	ini.GetValue( "Options", "ProgressiveNonstopLifebar", 		m_iProgressiveNonstopLifebar );
	ini.GetValue( "Options", "ProgressiveStageLifebar",			m_iProgressiveStageLifebar );

	ini.GetValue( "Options", "UseUnlockSystem",					m_bUseUnlockSystem );

	ini.GetValue( "Options", "FirstRun",						m_bFirstRun );
	ini.GetValue( "Options", "AutoMapJoysticks",				m_bAutoMapOnJoyChange );
	ini.GetValue( "Options", "VideoRenderers",					m_sVideoRenderers );
	ini.GetValue( "Options", "LastSeenVideoDriver",				m_sLastSeenVideoDriver );
	ini.GetValue( "Options", "LastSeenInputDevices",			m_sLastSeenInputDevices );
#if defined(WIN32)
	ini.GetValue( "Options", "LastSeenMemory",					m_iLastSeenMemory );
#endif
	ini.GetValue( "Options", "CoursesToShowRanking",			m_sCoursesToShowRanking );
	ini.GetValue( "Options", "GetRankingName",					(int&)m_iGetRankingName);
	ini.GetValue( "Options", "SmoothLines",					m_bSmoothLines );
	ini.GetValue( "Options", "GlobalOffsetSeconds",				m_fGlobalOffsetSeconds );
	ini.GetValue( "Options", "ShowBeginnerHelper",				m_bShowBeginnerHelper );
	ini.GetValue( "Options", "Language",						m_sLanguage );
	ini.GetValue( "Options", "EndlessBreakEnabled",				m_bEndlessBreakEnabled );
	ini.GetValue( "Options", "EndlessStagesUntilBreak",			m_iEndlessNumStagesUntilBreak );
	ini.GetValue( "Options", "EndlessBreakLength",				m_iEndlessBreakLength );

	ini.GetValue( "Options", "MemoryCardProfileSubdir",			m_sMemoryCardProfileSubdir );
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		ini.GetValue( "Options", ssprintf("DefaultLocalProfileIDP%d",p+1),	m_sDefaultLocalProfileID[p] );
		ini.GetValue( "Options", ssprintf("MemoryCardOsMountPointP%d",p+1),	m_sMemoryCardOsMountPoint[p] );
		FixSlashesInPlace( m_sMemoryCardOsMountPoint[p] );
		ini.GetValue( "Options", ssprintf("MemoryCardUsbBusP%d",p+1),		m_iMemoryCardUsbBus[p] );
		ini.GetValue( "Options", ssprintf("MemoryCardUsbPortP%d",p+1),		m_iMemoryCardUsbPort[p] );
	}

	ini.GetValue( "Options", "CenterImageTranslateX",			m_iCenterImageTranslateX );
	ini.GetValue( "Options", "CenterImageTranslateY",			m_iCenterImageTranslateY );
	ini.GetValue( "Options", "CenterImageScaleX",				m_fCenterImageScaleX );
	ini.GetValue( "Options", "CenterImageScaleY",				m_fCenterImageScaleY );
	ini.GetValue( "Options", "AttractSound",					m_bAttractSound );
	ini.GetValue( "Options", "DemonstrationSound",				m_bDemonstrationSound );
	ini.GetValue( "Options", "AllowExtraStage",					m_bAllowExtraStage );
	ini.GetValue( "Options", "AutoRestart",						g_bAutoRestart );

	CString sAdditionalSongFolders;
	if( ini.GetValue( "Options", "AdditionalSongFolders",			sAdditionalSongFolders ) )
	{
		FixSlashesInPlace( sAdditionalSongFolders );
		m_asAdditionalSongFolders.clear();
		split( sAdditionalSongFolders, ",", m_asAdditionalSongFolders, true );
	}

	ini.GetValue( "Debug", "Logging",							m_bLogging );
	ini.GetValue( "Debug", "ForceLogFlush",						m_bForceLogFlush );
	ini.GetValue( "Debug", "ShowLogOutput",						m_bShowLogOutput );
	ini.GetValue( "Debug", "Timestamping",						m_bTimestamping );
	ini.GetValue( "Debug", "LogCheckpoints",					m_bLogCheckpoints );
}

void PrefsManager::SaveGlobalPrefsToDisk() const
{
	IniFile ini;
	ini.SetPath( STEPMANIA_INI_PATH );

	ini.SetValue( "Options", "Windowed",						m_bWindowed );
	ini.SetValue( "Options", "DisplayWidth",					m_iDisplayWidth );
	ini.SetValue( "Options", "DisplayHeight",					m_iDisplayHeight );
	ini.SetValue( "Options", "DisplayColorDepth",				m_iDisplayColorDepth );
	ini.SetValue( "Options", "TextureColorDepth",				m_iTextureColorDepth );
	ini.SetValue( "Options", "MovieColorDepth",					m_iMovieColorDepth );
	ini.SetValue( "Options", "MaxTextureResolution",			m_iMaxTextureResolution );
	ini.SetValue( "Options", "RefreshRate",						m_iRefreshRate );
	ini.SetValue( "Options", "UseDedicatedMenuButtons",			m_bOnlyDedicatedMenuButtons );
	ini.SetValue( "Options", "ShowStats",						m_bShowStats );
	ini.SetValue( "Options", "ShowBanners",						m_bShowBanners );
	ini.SetValue( "Options", "BackgroundMode",					m_BackgroundMode);
	ini.SetValue( "Options", "NumBackgrounds",					m_iNumBackgrounds);
	ini.SetValue( "Options", "ShowDanger",						m_bShowDanger );
	ini.SetValue( "Options", "BGBrightness",					m_fBGBrightness );
	ini.SetValue( "Options", "MenuTimer",						m_bMenuTimer );
	ini.SetValue( "Options", "NumArcadeStages",					m_iNumArcadeStages );
	ini.SetValue( "Options", "EventMode",						m_bEventMode );
	ini.SetValue( "Options", "AutoPlay",						m_bAutoPlay );
	ini.SetValue( "Options", "JudgeWindowScale",				m_fJudgeWindowScale );
	ini.SetValue( "Options", "LifeDifficultyScale",				m_fLifeDifficultyScale );
	ini.SetValue( "Options", "JudgeWindowMarvelousSeconds",		m_fJudgeWindowMarvelousSeconds );
	ini.SetValue( "Options", "JudgeWindowPerfectSeconds",		m_fJudgeWindowPerfectSeconds );
	ini.SetValue( "Options", "JudgeWindowGreatSeconds",			m_fJudgeWindowGreatSeconds );
	ini.SetValue( "Options", "JudgeWindowGoodSeconds",			m_fJudgeWindowGoodSeconds );
	ini.SetValue( "Options", "JudgeWindowBooSeconds",			m_fJudgeWindowBooSeconds );
	ini.SetValue( "Options", "JudgeWindowOKSeconds",			m_fJudgeWindowOKSeconds );
	ini.SetValue( "Options", "JudgeWindowMineSeconds",			m_fJudgeWindowMineSeconds );
	ini.SetValue( "Options", "JudgeWindowAttackSeconds",		m_fJudgeWindowAttackSeconds );
	ini.SetValue( "Options", "LifeDeltaMarvelousPercentChange",	m_fLifeDeltaMarvelousPercentChange );
	ini.SetValue( "Options", "LifeDeltaPerfectPercentChange",	m_fLifeDeltaPerfectPercentChange );
	ini.SetValue( "Options", "LifeDeltaGreatPercentChange",		m_fLifeDeltaGreatPercentChange );
	ini.SetValue( "Options", "LifeDeltaGoodPercentChange",		m_fLifeDeltaGoodPercentChange );
	ini.SetValue( "Options", "LifeDeltaBooPercentChange",		m_fLifeDeltaBooPercentChange );
	ini.SetValue( "Options", "LifeDeltaMissPercentChange",		m_fLifeDeltaMissPercentChange );
	ini.SetValue( "Options", "LifeDeltaOKPercentChange",		m_fLifeDeltaOKPercentChange );
	ini.SetValue( "Options", "LifeDeltaNGPercentChange",		m_fLifeDeltaNGPercentChange );
	ini.SetValue( "Options", "LifeDeltaMinePercentChange",		m_fLifeDeltaMinePercentChange );
	ini.SetValue( "Options", "RegenComboAfterFail",				m_iRegenComboAfterFail );
	ini.SetValue( "Options", "RegenComboAfterMiss",				m_iRegenComboAfterMiss );
	ini.SetValue( "Options", "MaxRegenComboAfterFail",			m_iMaxRegenComboAfterFail );
	ini.SetValue( "Options", "MaxRegenComboAfterMiss",			m_iMaxRegenComboAfterMiss );
	ini.SetValue( "Options", "TwoPlayerRecovery",				m_bTwoPlayerRecovery );
	ini.SetValue( "Options", "DelayedEscape",					m_bDelayedEscape );
	ini.SetValue( "Options", "HiddenSongs",						m_bHiddenSongs );
	ini.SetValue( "Options", "Vsync",							m_bVsync );
	ini.SetValue( "Options", "Interlaced",						m_bInterlaced );
#ifdef _XBOX
	ini.SetValue( "Options", "PAL",								m_bPAL );
	ini.SetValue( "Options", "ScreenPosX",						m_fScreenPosX );
	ini.SetValue( "Options", "ScreenPosY",						m_fScreenPosY );
	ini.SetValue( "Options", "ScreenWidth",						m_fScreenWidth );
	ini.SetValue( "Options", "ScreenHeight",					m_fScreenHeight );
#endif
	ini.SetValue( "Options", "HowToPlay",						m_bInstructions );
	ini.SetValue( "Options", "Caution",							m_bShowDontDie );
	ini.SetValue( "Options", "ShowSelectGroup",					m_bShowSelectGroup );
	ini.SetValue( "Options", "ShowNative",						m_bShowNative );
	ini.SetValue( "Options", "ArcadeOptionsNavigation",			m_bArcadeOptionsNavigation );
	ini.SetValue( "Options", "DWIPath",							m_DWIPath );
	ini.SetValue( "Options", "DelayedTextureDelete",			m_bDelayedTextureDelete );
	ini.SetValue( "Options", "TexturePreload",					m_bTexturePreload );
	ini.SetValue( "Options", "DelayedScreenLoad",				m_bDelayedScreenLoad );
	ini.SetValue( "Options", "BannerCache",						m_BannerCache );
	ini.SetValue( "Options", "MusicWheelUsesSections",			m_MusicWheelUsesSections );
	ini.SetValue( "Options", "MusicWheelSwitchSpeed",			m_iMusicWheelSwitchSpeed );
	ini.SetValue( "Options", "EasterEggs",						m_bEasterEggs );
	ini.SetValue( "Options", "MarvelousTiming",					m_iMarvelousTiming );
	ini.SetValue( "Options", "SoundPreloadAll",					m_bSoundPreloadAll );
	ini.SetValue( "Options", "SoundResampleQuality",			m_iSoundResampleQuality );
	ini.SetValue( "Options", "CoinMode",						m_iCoinMode );
	ini.SetValue( "Options", "CoinsPerCredit",					m_iCoinsPerCredit );
	ini.SetValue( "Options", "Premium",							m_Premium );
	ini.SetValue( "Options", "BoostAppPriority",				m_iBoostAppPriority );
	ini.SetValue( "Options", "PickExtraStage",					m_bPickExtraStage );
	ini.SetValue( "Options", "ComboContinuesBetweenSongs",		m_bComboContinuesBetweenSongs );
	ini.SetValue( "Options", "LongVerSeconds",					m_fLongVerSongSeconds );
	ini.SetValue( "Options", "MarathonVerSeconds",				m_fMarathonVerSongSeconds );
	ini.SetValue( "Options", "ShowSongOptions",					m_ShowSongOptions );
	ini.SetValue( "Options", "AllowUnacceleratedRenderer",		m_bAllowUnacceleratedRenderer );
	ini.SetValue( "Options", "ThreadedInput",					m_bThreadedInput );
	ini.SetValue( "Options", "ScreenTestMode",					m_bScreenTestMode );
	ini.SetValue( "Options", "IgnoredMessageWindows",			m_sIgnoredMessageWindows );
	ini.SetValue( "Options", "SoloSingle",						m_bSoloSingle );
	ini.SetValue( "Options", "DancePointsForOni",				m_bDancePointsForOni );
	ini.SetValue( "Options", "PercentageScoring",				m_bPercentageScoring );
	ini.SetValue( "Options", "MinPercentageForHighScore",		m_fMinPercentageForHighScore );
	ini.SetValue( "Options", "ShowLyrics",						m_bShowLyrics );
	ini.SetValue( "Options", "AutogenMissingTypes",				m_bAutogenMissingTypes );
	ini.SetValue( "Options", "AutogenGroupCourses",				m_bAutogenGroupCourses );
	ini.SetValue( "Options", "BreakComboToGetItem",				m_bBreakComboToGetItem );
	ini.SetValue( "Options", "ShowDancingCharacters",			m_ShowDancingCharacters );
	ini.SetValue( "Options", "UseUnlockSystem",					m_bUseUnlockSystem );
	ini.SetValue( "Options", "FirstRun",						m_bFirstRun );
	ini.SetValue( "Options", "AutoMapJoysticks",				m_bAutoMapOnJoyChange );
	ini.SetValue( "Options", "VideoRenderers",					m_sVideoRenderers );
	ini.SetValue( "Options", "LastSeenVideoDriver",				m_sLastSeenVideoDriver );
	ini.SetValue( "Options", "LastSeenInputDevices",			m_sLastSeenInputDevices );
#if defined(WIN32)
	ini.SetValue( "Options", "LastSeenMemory",					m_iLastSeenMemory );
#endif
	ini.SetValue( "Options", "CoursesToShowRanking",			m_sCoursesToShowRanking );
	ini.SetValue( "Options", "GetRankingName",					m_iGetRankingName);
	ini.SetValue( "Options", "SmoothLines",					m_bSmoothLines );
	ini.SetValue( "Options", "GlobalOffsetSeconds",				m_fGlobalOffsetSeconds );

	ini.SetValue( "Options", "CourseSortOrder",					m_iCourseSortOrder );
	ini.SetValue( "Options", "MoveRandomToEnd",					m_bMoveRandomToEnd );

	ini.SetValue( "Options", "ScoringType",						m_iScoringType );

	ini.SetValue( "Options", "ProgressiveLifebar",				m_iProgressiveLifebar );
	ini.SetValue( "Options", "ProgressiveStageLifebar",			m_iProgressiveStageLifebar );
	ini.SetValue( "Options", "ProgressiveNonstopLifebar",		m_iProgressiveNonstopLifebar );
	ini.SetValue( "Options", "ShowBeginnerHelper",				m_bShowBeginnerHelper );
	ini.SetValue( "Options", "Language",						m_sLanguage );
	ini.SetValue( "Options", "EndlessBreakEnabled",				m_bEndlessBreakEnabled );
	ini.SetValue( "Options", "EndlessStagesUntilBreak",			m_iEndlessNumStagesUntilBreak );
	ini.SetValue( "Options", "EndlessBreakLength",				m_iEndlessBreakLength );

	ini.SetValue( "Options", "MemoryCardProfileSubdir",			m_sMemoryCardProfileSubdir );
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		ini.SetValue( "Options", ssprintf("DefaultLocalProfileIDP%d",p+1),	m_sDefaultLocalProfileID[p] );
		ini.SetValue( "Options", ssprintf("MemoryCardOsMountPointP%d",p+1),				m_sMemoryCardOsMountPoint[p] );
		ini.SetValue( "Options", ssprintf("MemoryCardUsbBusP%d",p+1),		m_iMemoryCardUsbBus[p] );
		ini.SetValue( "Options", ssprintf("MemoryCardUsbPortP%d",p+1),		m_iMemoryCardUsbPort[p] );
	}

	ini.SetValue( "Options", "CenterImageTranslateX",			m_iCenterImageTranslateX );
	ini.SetValue( "Options", "CenterImageTranslateY",			m_iCenterImageTranslateY );
	ini.SetValue( "Options", "CenterImageScaleX",				m_fCenterImageScaleX );
	ini.SetValue( "Options", "CenterImageScaleY",				m_fCenterImageScaleY );
	ini.SetValue( "Options", "AttractSound",					m_bAttractSound );
	ini.SetValue( "Options", "DemonstrationSound",				m_bDemonstrationSound );
	ini.SetValue( "Options", "AllowExtraStage",					m_bAllowExtraStage );
	ini.SetValue( "Options", "AutoRestart",						g_bAutoRestart );
	ini.SetValue( "Options", "SoundWriteAhead",					m_iSoundWriteAhead );

	/* Only write these if they aren't the default.  This ensures that we can change
	 * the default and have it take effect for everyone (except people who
	 * tweaked this value). */
	if(m_sSoundDrivers != DEFAULT_SOUND_DRIVER_LIST)
		ini.SetValue ( "Options", "SoundDrivers",				m_sSoundDrivers );
	if(m_fSoundVolume != DEFAULT_SOUND_VOLUME)
		ini.SetValue( "Options", "SoundVolume",					m_fSoundVolume );
	if(m_sLightsDriver != DEFAULT_LIGHTS_DRIVER)
		ini.SetValue( "Options", "LightsDriver",				m_sLightsDriver );
	if(m_sMovieDrivers != DEFAULT_MOVIE_DRIVER_LIST)
		ini.SetValue ( "Options", "MovieDrivers",				m_sMovieDrivers );

	ini.SetValue( "Options", "AdditionalSongFolders", 			join(",", m_asAdditionalSongFolders) );

	ini.SetValue( "Debug", "Logging",							m_bLogging );
	ini.SetValue( "Debug", "ForceLogFlush",						m_bForceLogFlush );
	ini.SetValue( "Debug", "ShowLogOutput",						m_bShowLogOutput );
	ini.SetValue( "Debug", "Timestamping",						m_bTimestamping );
	ini.SetValue( "Debug", "LogCheckpoints",					m_bLogCheckpoints );

	ini.WriteFile();
}

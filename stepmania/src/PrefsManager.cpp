#include "global.h"
#include "PrefsManager.h"
#include "IniFile.h"
#include "GameState.h"
#include "RageDisplay.h"
#include "RageUtil.h"
#include "arch/arch.h" /* for default driver specs */
#include "RageSoundReader_Resample.h" /* for ResampleQuality */
#include "RageFile.h"
#include "ProductInfo.h"
#include "GameConstantsAndTypes.h"
#include "Foreach.h"
#include "Preference.h"

#define DEFAULTS_INI_PATH	"Data/Defaults.ini"		// these can be overridden
#define STEPMANIA_INI_PATH	"Data/StepMania.ini"	// overlay on Defaults.ini, contains the user's choices
#define STATIC_INI_PATH		"Data/Static.ini"		// overlay on the 2 above, can't be overridden

PrefsManager*	PREFSMAN = NULL;	// global and accessable from anywhere in our program

const float DEFAULT_SOUND_VOLUME = 1.00f;
const CString DEFAULT_LIGHTS_DRIVER = "Null";


//
// For self-registering prefs
//
vector<IPreference*> *g_pvpSubscribers = NULL;

void Subscribe( IPreference *p )
{
	// TRICKY: If we make this a global vector instead of a global pointer,
	// then we'd have to be careful that the static constructors of all
	// Preferences are called before the vector constructor.  It's
	// too tricky to enfore that, so we'll allocate the vector ourself
	// so that the compiler can't possibly call the vector constructor
	// after we've already added to the vector.
	if( g_pvpSubscribers == NULL )
		g_pvpSubscribers = new vector<IPreference*>;
	g_pvpSubscribers->push_back( p );
}



bool g_bAutoRestart = false;

PrefsManager::PrefsManager()
{
	Init();
	ReadGlobalPrefsFromDisk();
}

void PrefsManager::Init()
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
	m_bCelShadeModels = false;		// Work-In-Progress.. disable by default.
	m_fConstantUpdateDeltaSeconds = 0;
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
	m_fJudgeWindowAdd = 0;
	m_fLifeDifficultyScale = 1.0f;
	m_fJudgeWindowSecondsMarvelous =	0.0225f;
	m_fJudgeWindowSecondsPerfect =		0.045f;
	m_fJudgeWindowSecondsGreat =		0.090f;
	m_fJudgeWindowSecondsGood =			0.135f;
	m_fJudgeWindowSecondsBoo =			0.180f;
	m_fJudgeWindowSecondsOK =			0.250f;	// allow enough time to take foot off and put back on
	m_fJudgeWindowSecondsMine =			0.090f;	// same as great
	m_fJudgeWindowSecondsAttack =		0.135f;
	m_fLifeDeltaPercentChangeMarvelous =	+0.008f;
	m_fLifeDeltaPercentChangePerfect =		+0.008f;
	m_fLifeDeltaPercentChangeGreat =		+0.004f;
	m_fLifeDeltaPercentChangeGood =			+0.000f;
	m_fLifeDeltaPercentChangeBoo =			-0.040f;
	m_fLifeDeltaPercentChangeMiss =			-0.080f;
	m_fLifeDeltaPercentChangeHitMine =		-0.160f;
	m_fLifeDeltaPercentChangeOK =			+0.008f;
	m_fLifeDeltaPercentChangeNG =			-0.080f;

	m_fTugMeterPercentChangeMarvelous =		+0.010f;
	m_fTugMeterPercentChangePerfect =		+0.008f;
	m_fTugMeterPercentChangeGreat =			+0.004f;
	m_fTugMeterPercentChangeGood =			+0.000f;
	m_fTugMeterPercentChangeBoo =			-0.010f;
	m_fTugMeterPercentChangeMiss =			-0.020f;
	m_fTugMeterPercentChangeHitMine =		-0.040f;
	m_fTugMeterPercentChangeOK =			+0.008f;
	m_fTugMeterPercentChangeNG =			-0.020f;

	m_iRegenComboAfterFail = 10; // cumulative
	m_iRegenComboAfterMiss = 5; // cumulative
	m_iMaxRegenComboAfterFail = 10;
	m_iMaxRegenComboAfterMiss = 10;
	m_bTwoPlayerRecovery = true;
	m_bMercifulDrain = true;
	m_bMinimum1FullSongInCourses = false;
	
	m_iPercentScoreWeightMarvelous = 3;
	m_iPercentScoreWeightPerfect = 2;
	m_iPercentScoreWeightGreat = 1;
	m_iPercentScoreWeightGood = 0;
	m_iPercentScoreWeightBoo = 0;
	m_iPercentScoreWeightMiss = 0;
	m_iPercentScoreWeightOK = 3;
	m_iPercentScoreWeightNG = 0;
	m_iPercentScoreWeightHitMine = -2;
	m_iGradeWeightMarvelous = 2;
	m_iGradeWeightPerfect = 2;
	m_iGradeWeightGreat = 1;
	m_iGradeWeightGood = 0;
	m_iGradeWeightBoo = -4;
	m_iGradeWeightMiss = -8;
	m_iGradeWeightHitMine = -8;
	m_iGradeWeightOK = 6;
	m_iGradeWeightNG = 0;
	m_iNumGradeTiersUsed = 7;
	for( int i=0; i<NUM_GRADE_TIERS; i++ )
		m_fGradePercent[i] = 0;
	m_fGradePercent[GRADE_TIER_1] = 1.0f;
	m_fGradePercent[GRADE_TIER_2] = 1.0f;
	m_fGradePercent[GRADE_TIER_3] = 0.93f;	// AA
	m_fGradePercent[GRADE_TIER_4] = 0.80f;	// A
	m_fGradePercent[GRADE_TIER_5] = 0.65f;	// B
	m_fGradePercent[GRADE_TIER_6] = 0.45f;	// C
	m_fGradePercent[GRADE_TIER_7] = -99999;	// D
	m_bGradeTier02IsAllPerfects = true;
	
	m_fSuperMeterPercentChangeMarvelous =	+0.05f;
	m_fSuperMeterPercentChangePerfect =		+0.04f;
	m_fSuperMeterPercentChangeGreat =		+0.02f;
	m_fSuperMeterPercentChangeGood =		+0.00f;
	m_fSuperMeterPercentChangeBoo =			-0.00f;
	m_fSuperMeterPercentChangeMiss =		-0.20f;
	m_fSuperMeterPercentChangeHitMine =		-0.40f;
	m_fSuperMeterPercentChangeOK =			+0.04f;
	m_fSuperMeterPercentChangeNG =			-0.20f;
	m_bMercifulSuperMeter = true;

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
	m_bDelayedModelDelete = false;
	m_BannerCache = BNCACHE_LOW_RES;
	m_bPalettedBannerCache = false;
	m_bFastLoad = true;
	m_MusicWheelUsesSections = ALWAYS;
	m_iMusicWheelSwitchSpeed = 10;
	m_bEasterEggs = true;
	m_iMarvelousTiming = 2;
	m_iCoinMode = COIN_HOME;
	m_iCoinsPerCredit = 1;
	m_Premium = NO_PREMIUM;
	m_bDelayedCreditsReconcile = false;
	m_iBoostAppPriority = -1;
	m_bSmoothLines = false;
	m_ShowSongOptions = YES;
	m_bDancePointsForOni = false;
	m_bPercentageScoring = false;
	m_fMinPercentageForMachineSongHighScore = 0.5f;
	m_fMinPercentageForMachineCourseHighScore = 0.001f;	// don't save course scores with 0 percentage
	m_bDisqualification = false;
	m_bShowLyrics = true;
	m_bAutogenSteps = true;
	m_bAutogenGroupCourses = true;
	m_bBreakComboToGetItem = false;
	m_bLockCourseDifficulties = true;
	m_ShowDancingCharacters = CO_RANDOM;
	m_bUseUnlockSystem = false;
	m_bFirstRun = true;
	m_bAutoMapOnJoyChange = true;
	m_fGlobalOffsetSeconds = 0;
	m_bShowBeginnerHelper = false;
	m_bEndlessBreakEnabled = true;
	m_iEndlessNumStagesUntilBreak = 5;
	m_iEndlessBreakLength = 5;
	m_bDisableScreenSaver = true;

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
	m_bSubSortByNumSteps = false;
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

	m_iAttractSoundFrequency = 1;
	m_bAllowExtraStage = true;
	m_bHideDefaultNoteSkin = false;
	m_iMaxHighScoresPerListForMachine = 10;
	m_iMaxHighScoresPerListForPlayer = 3;
	m_fPadStickSeconds = 0;
	m_bForceMipMaps = false;
	m_bTrilinearFiltering = false;
	m_bAnisotropicFiltering = false;
	g_bAutoRestart = false;
	m_bSignProfileData = false;

	m_bEditorShowBGChangesPlay = false;

	/* XXX: Set these defaults for individual consoles using VideoCardDefaults.ini. */
	m_bPAL = false;
#ifndef _XBOX
	m_bInterlaced = false;
#endif

	m_sSoundDrivers = "";	// default
	/* Number of frames to write ahead; usually 44100 frames per second.
	 * (Number of millisec would be more flexible, but it's more useful to
	 * specify numbers directly.) This is purely a troubleshooting option
	 * and is not honored by all sound drivers. */
	m_iSoundWriteAhead = 0;
	m_iSoundDevice = "";
	m_fSoundVolume = DEFAULT_SOUND_VOLUME;
	m_iSoundResampleQuality = RageSoundReader_Resample::RESAMP_NORMAL;

	m_sMovieDrivers = DEFAULT_MOVIE_DRIVER_LIST;

	// StepMania.cpp sets these on first run:
	m_sVideoRenderers = "";
#if defined(WIN32)
	m_iLastSeenMemory = 0;
#endif

	m_sLightsDriver = DEFAULT_LIGHTS_DRIVER;
	m_sLightsStepsDifficulty = "medium";
	m_bBlinkGameplayButtonLightsOnNote = false;
	m_bAllowUnacceleratedRenderer = false;
	m_bThreadedInput = true;
	m_bThreadedMovieDecode = true;
	m_bScreenTestMode = false;
	m_sMachineName = "NoName";
	m_sIgnoredMessageWindows = "";

	m_sCoursesToShowRanking = "";

	m_bLogToDisk = true;
	m_bForceLogFlush = false;
#ifdef DEBUG
	m_bShowLogOutput = true;
#else
	m_bShowLogOutput = false;
#endif
	m_bTimestamping = false;
	m_bLogSkips = false;
	m_bLogCheckpoints = false;
	m_bShowLoadingWindow = true;

	m_bMemoryCards = false;

	FOREACH_PlayerNumber( p )
	{
		m_iMemoryCardUsbBus[p] = -1;
		m_iMemoryCardUsbPort[p] = -1;
		m_iMemoryCardUsbLevel[p] = -1;
	}

	m_sMemoryCardProfileSubdir = PRODUCT_NAME;
	m_iProductID = 1;


	FOREACH_CONST( IPreference*, *g_pvpSubscribers, p ) (*p)->LoadDefault();
}

PrefsManager::~PrefsManager()
{
}

void PrefsManager::ReadGlobalPrefsFromDisk()
{
	ReadPrefsFromFile( DEFAULTS_INI_PATH );
	ReadPrefsFromFile( STEPMANIA_INI_PATH );
	ReadPrefsFromFile( STATIC_INI_PATH );
}

void PrefsManager::ResetToFactoryDefaults()
{
	// clobber the users prefs by initing then applying defaults
	Init();
	m_bFirstRun = false;
	ReadPrefsFromFile( DEFAULTS_INI_PATH );
	ReadPrefsFromFile( STATIC_INI_PATH );
	
	SaveGlobalPrefsToDisk();
}

void PrefsManager::ReadPrefsFromFile( CString sIni )
{
	IniFile ini;
	if( !ini.ReadFile(sIni) )
		return;

	ini.GetValue( "Options", "Windowed",						m_bWindowed );
	ini.GetValue( "Options", "Interlaced",						m_bInterlaced );
	ini.GetValue( "Options", "PAL",								m_bPAL );
	ini.GetValue( "Options", "CelShadeModels",					m_bCelShadeModels );
	ini.GetValue( "Options", "ConstantUpdateDeltaSeconds",		m_fConstantUpdateDeltaSeconds );
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
	ini.GetValue( "Options", "JudgeWindowAdd",					m_fJudgeWindowAdd );
	ini.GetValue( "Options", "JudgeWindowSecondsMarvelous",		m_fJudgeWindowSecondsMarvelous );
	ini.GetValue( "Options", "JudgeWindowSecondsPerfect",		m_fJudgeWindowSecondsPerfect );
	ini.GetValue( "Options", "JudgeWindowSecondsGreat",			m_fJudgeWindowSecondsGreat );
	ini.GetValue( "Options", "JudgeWindowSecondsGood",			m_fJudgeWindowSecondsGood );
	ini.GetValue( "Options", "JudgeWindowSecondsBoo",			m_fJudgeWindowSecondsBoo );
	ini.GetValue( "Options", "JudgeWindowSecondsOK",			m_fJudgeWindowSecondsOK );
	ini.GetValue( "Options", "JudgeWindowSecondsMine",			m_fJudgeWindowSecondsMine );
	ini.GetValue( "Options", "JudgeWindowSecondsAttack",		m_fJudgeWindowSecondsAttack );
	ini.GetValue( "Options", "LifeDifficultyScale",				m_fLifeDifficultyScale );
	ini.GetValue( "Options", "LifeDeltaPercentChangeMarvelous",	m_fLifeDeltaPercentChangeMarvelous );
	ini.GetValue( "Options", "LifeDeltaPercentChangePerfect",	m_fLifeDeltaPercentChangePerfect );
	ini.GetValue( "Options", "LifeDeltaPercentChangeGreat",		m_fLifeDeltaPercentChangeGreat );
	ini.GetValue( "Options", "LifeDeltaPercentChangeGood",		m_fLifeDeltaPercentChangeGood );
	ini.GetValue( "Options", "LifeDeltaPercentChangeBoo",		m_fLifeDeltaPercentChangeBoo );
	ini.GetValue( "Options", "LifeDeltaPercentChangeMiss",		m_fLifeDeltaPercentChangeMiss );
	ini.GetValue( "Options", "LifeDeltaPercentChangeHitMine",	m_fLifeDeltaPercentChangeHitMine );
	ini.GetValue( "Options", "LifeDeltaPercentChangeOK",		m_fLifeDeltaPercentChangeOK );
	ini.GetValue( "Options", "LifeDeltaPercentChangeNG",		m_fLifeDeltaPercentChangeNG );
	ini.GetValue( "Options", "TugMeterPercentChangeMarvelous",	m_fTugMeterPercentChangeMarvelous );
	ini.GetValue( "Options", "TugMeterPercentChangePerfect",	m_fTugMeterPercentChangePerfect );
	ini.GetValue( "Options", "TugMeterPercentChangeGreat",		m_fTugMeterPercentChangeGreat );
	ini.GetValue( "Options", "TugMeterPercentChangeGood",		m_fTugMeterPercentChangeGood );
	ini.GetValue( "Options", "TugMeterPercentChangeBoo",		m_fTugMeterPercentChangeBoo );
	ini.GetValue( "Options", "TugMeterPercentChangeMiss",		m_fTugMeterPercentChangeMiss );
	ini.GetValue( "Options", "TugMeterPercentChangeHitMine",	m_fTugMeterPercentChangeHitMine );
	ini.GetValue( "Options", "TugMeterPercentChangeOK",			m_fTugMeterPercentChangeOK );
	ini.GetValue( "Options", "TugMeterPercentChangeNG",			m_fTugMeterPercentChangeNG );
	ini.GetValue( "Options", "RegenComboAfterFail",				m_iRegenComboAfterFail );
	ini.GetValue( "Options", "RegenComboAfterMiss",				m_iRegenComboAfterMiss );
	ini.GetValue( "Options", "MaxRegenComboAfterFail",			m_iMaxRegenComboAfterFail );
	ini.GetValue( "Options", "MaxRegenComboAfterMiss",			m_iMaxRegenComboAfterMiss );
	ini.GetValue( "Options", "TwoPlayerRecovery",				m_bTwoPlayerRecovery );
	ini.GetValue( "Options", "MercifulDrain",					m_bMercifulDrain );
	ini.GetValue( "Options", "Minimum1FullSongInCourses",			m_bMinimum1FullSongInCourses );

	ini.GetValue( "Options", "PercentScoreWeightMarvelous",		m_iPercentScoreWeightMarvelous );
	ini.GetValue( "Options", "PercentScoreWeightPerfect",		m_iPercentScoreWeightPerfect );
	ini.GetValue( "Options", "PercentScoreWeightGreat",			m_iPercentScoreWeightGreat );
	ini.GetValue( "Options", "PercentScoreWeightGood",			m_iPercentScoreWeightGood );
	ini.GetValue( "Options", "PercentScoreWeightBoo",			m_iPercentScoreWeightBoo );
	ini.GetValue( "Options", "PercentScoreWeightMiss",			m_iPercentScoreWeightMiss );
	ini.GetValue( "Options", "PercentScoreWeightOK",			m_iPercentScoreWeightOK );
	ini.GetValue( "Options", "PercentScoreWeightNG",			m_iPercentScoreWeightNG );
	ini.GetValue( "Options", "PercentScoreWeightHitMine",		m_iPercentScoreWeightHitMine );
	ini.GetValue( "Options", "GradeWeightMarvelous",			m_iGradeWeightMarvelous );
	ini.GetValue( "Options", "GradeWeightPerfect",				m_iGradeWeightPerfect );
	ini.GetValue( "Options", "GradeWeightGreat",				m_iGradeWeightGreat );
	ini.GetValue( "Options", "GradeWeightGood",					m_iGradeWeightGood );
	ini.GetValue( "Options", "GradeWeightBoo",					m_iGradeWeightBoo );
	ini.GetValue( "Options", "GradeWeightMiss",					m_iGradeWeightMiss );
	ini.GetValue( "Options", "GradeWeightHitMine",				m_iGradeWeightHitMine );
	ini.GetValue( "Options", "GradeWeightOK",					m_iGradeWeightOK );
	ini.GetValue( "Options", "GradeWeightNG",					m_iGradeWeightNG );

	ini.GetValue( "Options", "NumGradeTiersUsed",				m_iNumGradeTiersUsed );
	for( int g=0; g<NUM_GRADE_TIERS; g++ )
	{
		Grade grade = (Grade)g;
		CString s = GradeToString( grade );
		ini.GetValue( "Options", "GradePercent"+s,				m_fGradePercent[g] );
	}
	ini.GetValue( "Options", "GradeTier02IsAllPerfects",		m_bGradeTier02IsAllPerfects );

	ini.GetValue( "Options", "SuperMeterPercentChangeMarvelous",m_fSuperMeterPercentChangeMarvelous );
	ini.GetValue( "Options", "SuperMeterPercentChangePerfect",	m_fSuperMeterPercentChangePerfect );
	ini.GetValue( "Options", "SuperMeterPercentChangeGreat",	m_fSuperMeterPercentChangeGreat );
	ini.GetValue( "Options", "SuperMeterPercentChangeGood",		m_fSuperMeterPercentChangeGood );
	ini.GetValue( "Options", "SuperMeterPercentChangeBoo",		m_fSuperMeterPercentChangeBoo );
	ini.GetValue( "Options", "SuperMeterPercentChangeMiss",		m_fSuperMeterPercentChangeMiss );
	ini.GetValue( "Options", "SuperMeterPercentChangeHitMine",	m_fSuperMeterPercentChangeHitMine );
	ini.GetValue( "Options", "SuperMeterPercentChangeOK",		m_fSuperMeterPercentChangeOK );
	ini.GetValue( "Options", "SuperMeterPercentChangeNG",		m_fSuperMeterPercentChangeNG );
	ini.GetValue( "Options", "MercifulSuperMeter",				m_bMercifulSuperMeter );

	ini.GetValue( "Options", "DelayedEscape",					m_bDelayedEscape );
	ini.GetValue( "Options", "HiddenSongs",						m_bHiddenSongs );
	ini.GetValue( "Options", "Vsync",							m_bVsync );
	ini.GetValue( "Options", "HowToPlay",						m_bInstructions );
	ini.GetValue( "Options", "Caution",							m_bShowDontDie );
	ini.GetValue( "Options", "ShowSelectGroup",					m_bShowSelectGroup );
	ini.GetValue( "Options", "ShowNative",						m_bShowNative );
	ini.GetValue( "Options", "ArcadeOptionsNavigation",			m_bArcadeOptionsNavigation );
	ini.GetValue( "Options", "DelayedTextureDelete",			m_bDelayedTextureDelete );
	ini.GetValue( "Options", "TexturePreload",					m_bTexturePreload );
	ini.GetValue( "Options", "DelayedScreenLoad",				m_bDelayedScreenLoad );
	ini.GetValue( "Options", "DelayedModelDelete",				m_bDelayedModelDelete );
	ini.GetValue( "Options", "BannerCache",						(int&)m_BannerCache );
	ini.GetValue( "Options", "PalettedBannerCache",				m_bPalettedBannerCache );
	ini.GetValue( "Options", "FastLoad",						m_bFastLoad );
	ini.GetValue( "Options", "MusicWheelUsesSections",			(int&)m_MusicWheelUsesSections );
	ini.GetValue( "Options", "MusicWheelSwitchSpeed",			m_iMusicWheelSwitchSpeed );
	ini.GetValue( "Options", "SoundDrivers",					m_sSoundDrivers );
	ini.GetValue( "Options", "SoundWriteAhead",					m_iSoundWriteAhead );
	ini.GetValue( "Options", "SoundDevice",						m_iSoundDevice );
	ini.GetValue( "Options", "MovieDrivers",					m_sMovieDrivers );
	ini.GetValue( "Options", "EasterEggs",						m_bEasterEggs );
	ini.GetValue( "Options", "MarvelousTiming",					(int&)m_iMarvelousTiming );
	ini.GetValue( "Options", "SoundVolume",						m_fSoundVolume );
	ini.GetValue( "Options", "LightsDriver",					m_sLightsDriver );
	ini.GetValue( "Options", "SoundResampleQuality",			m_iSoundResampleQuality );
	ini.GetValue( "Options", "CoinMode",						m_iCoinMode );
	ini.GetValue( "Options", "CoinsPerCredit",					m_iCoinsPerCredit );
	m_iCoinsPerCredit = max(m_iCoinsPerCredit, 1);
	ini.GetValue( "Options", "Premium",							(int&)m_Premium );
	ini.GetValue( "Options", "DelayedCreditsReconcile",			m_bDelayedCreditsReconcile );
	ini.GetValue( "Options", "BoostAppPriority",				m_iBoostAppPriority );
	ini.GetValue( "Options", "PickExtraStage",					m_bPickExtraStage );
	ini.GetValue( "Options", "ComboContinuesBetweenSongs",		m_bComboContinuesBetweenSongs );
	ini.GetValue( "Options", "LongVerSeconds",					m_fLongVerSongSeconds );
	ini.GetValue( "Options", "MarathonVerSeconds",				m_fMarathonVerSongSeconds );
	ini.GetValue( "Options", "ShowSongOptions",					(int&)m_ShowSongOptions );
	ini.GetValue( "Options", "LightsStepsDifficulty",			m_sLightsStepsDifficulty );
	ini.GetValue( "Options", "BlinkGameplayButtonLightsOnNote",	m_bBlinkGameplayButtonLightsOnNote );
	ini.GetValue( "Options", "AllowUnacceleratedRenderer",		m_bAllowUnacceleratedRenderer );
	ini.GetValue( "Options", "ThreadedInput",					m_bThreadedInput );
	ini.GetValue( "Options", "ThreadedMovieDecode",				m_bThreadedMovieDecode );
	ini.GetValue( "Options", "ScreenTestMode",					m_bScreenTestMode );
	ini.GetValue( "Options", "MachineName",						m_sMachineName );
	ini.GetValue( "Options", "IgnoredMessageWindows",			m_sIgnoredMessageWindows );
	ini.GetValue( "Options", "SoloSingle",						m_bSoloSingle );
	ini.GetValue( "Options", "DancePointsForOni",				m_bDancePointsForOni );
	ini.GetValue( "Options", "PercentageScoring",				m_bPercentageScoring );
	ini.GetValue( "Options", "MinPercentageForMachineSongHighScore",	m_fMinPercentageForMachineSongHighScore );
	ini.GetValue( "Options", "MinPercentageForMachineCourseHighScore",	m_fMinPercentageForMachineCourseHighScore );
	ini.GetValue( "Options", "Disqualification",				m_bDisqualification );
	ini.GetValue( "Options", "ShowLyrics",						m_bShowLyrics );
	ini.GetValue( "Options", "AutogenSteps",					m_bAutogenSteps );
	ini.GetValue( "Options", "AutogenGroupCourses",				m_bAutogenGroupCourses );
	ini.GetValue( "Options", "BreakComboToGetItem",				m_bBreakComboToGetItem );
	ini.GetValue( "Options", "LockCourseDifficulties",			m_bLockCourseDifficulties );
	ini.GetValue( "Options", "ShowDancingCharacters",			(int&)m_ShowDancingCharacters );

	ini.GetValue( "Options", "CourseSortOrder",					(int&)m_iCourseSortOrder );
	ini.GetValue( "Options", "MoveRandomToEnd",					m_bMoveRandomToEnd );
	ini.GetValue( "Options", "SubSortByNumSteps",				m_bSubSortByNumSteps );

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
	ini.GetValue( "Options", "SmoothLines",						m_bSmoothLines );
	ini.GetValue( "Options", "GlobalOffsetSeconds",				m_fGlobalOffsetSeconds );
	ini.GetValue( "Options", "ShowBeginnerHelper",				m_bShowBeginnerHelper );
	ini.GetValue( "Options", "Language",						m_sLanguage );
	ini.GetValue( "Options", "EndlessBreakEnabled",				m_bEndlessBreakEnabled );
	ini.GetValue( "Options", "EndlessStagesUntilBreak",			m_iEndlessNumStagesUntilBreak );
	ini.GetValue( "Options", "EndlessBreakLength",				m_iEndlessBreakLength );
	ini.GetValue( "Options", "DisableScreenSaver",				m_bDisableScreenSaver );

	ini.GetValue( "Options", "MemoryCardProfileSubdir",			m_sMemoryCardProfileSubdir );
	ini.GetValue( "Options", "ProductID",						m_iProductID );
	ini.GetValue( "Options", "MemoryCards",						m_bMemoryCards );
	FOREACH_PlayerNumber( p )
	{
		ini.GetValue( "Options", ssprintf("DefaultLocalProfileIDP%d",p+1),	m_sDefaultLocalProfileID[p] );
		ini.GetValue( "Options", ssprintf("MemoryCardOsMountPointP%d",p+1),	m_sMemoryCardOsMountPoint[p] );
		FixSlashesInPlace( m_sMemoryCardOsMountPoint[p] );
		ini.GetValue( "Options", ssprintf("MemoryCardUsbBusP%d",p+1),		m_iMemoryCardUsbBus[p] );
		ini.GetValue( "Options", ssprintf("MemoryCardUsbPortP%d",p+1),		m_iMemoryCardUsbPort[p] );
		ini.GetValue( "Options", ssprintf("MemoryCardUsbLevelP%d",p+1),		m_iMemoryCardUsbLevel[p] );
	}

	ini.GetValue( "Options", "CenterImageTranslateX",			m_iCenterImageTranslateX );
	ini.GetValue( "Options", "CenterImageTranslateY",			m_iCenterImageTranslateY );
	ini.GetValue( "Options", "CenterImageScaleX",				m_fCenterImageScaleX );
	ini.GetValue( "Options", "CenterImageScaleY",				m_fCenterImageScaleY );
	ini.GetValue( "Options", "AttractSoundFrequency",			m_iAttractSoundFrequency );
	ini.GetValue( "Options", "AllowExtraStage",					m_bAllowExtraStage );
	ini.GetValue( "Options", "HideDefaultNoteSkin",				m_bHideDefaultNoteSkin );
	ini.GetValue( "Options", "MaxHighScoresPerListForMachine",	m_iMaxHighScoresPerListForMachine );
	ini.GetValue( "Options", "MaxHighScoresPerListForPlayer",	m_iMaxHighScoresPerListForPlayer );
	ini.GetValue( "Options", "PadStickSeconds",					m_fPadStickSeconds );
	ini.GetValue( "Options", "ForceMipMaps",					m_bForceMipMaps );
	ini.GetValue( "Options", "TrilinearFiltering",				m_bTrilinearFiltering );
	ini.GetValue( "Options", "AnisotropicFiltering",			m_bAnisotropicFiltering );
	ini.GetValue( "Options", "AutoRestart",						g_bAutoRestart );
	ini.GetValue( "Options", "SignProfileData",					m_bSignProfileData );

	ini.GetValue( "Editor", "ShowBGChangesPlay",				m_bEditorShowBGChangesPlay );

	ini.GetValue( "Options", "AdditionalSongFolders",			m_sAdditionalSongFolders );
	ini.GetValue( "Options", "AdditionalFolders",				m_sAdditionalFolders );
	FixSlashesInPlace(m_sAdditionalSongFolders);
	FixSlashesInPlace(m_sAdditionalFolders);

	ini.GetValue( "Debug", "LogToDisk",							m_bLogToDisk );
	ini.GetValue( "Debug", "ForceLogFlush",						m_bForceLogFlush );
	ini.GetValue( "Debug", "ShowLogOutput",						m_bShowLogOutput );
	ini.GetValue( "Debug", "Timestamping",						m_bTimestamping );
	ini.GetValue( "Debug", "LogSkips",							m_bLogSkips );
	ini.GetValue( "Debug", "LogCheckpoints",					m_bLogCheckpoints );
	ini.GetValue( "Debug", "ShowLoadingWindow",					m_bShowLoadingWindow );


	FOREACH( IPreference*, *g_pvpSubscribers, p ) (*p)->ReadFrom( ini );
}

void PrefsManager::SaveGlobalPrefsToDisk() const
{
	IniFile ini;

	ini.SetValue( "Options", "Windowed",						m_bWindowed );
	ini.SetValue( "Options", "CelShadeModels",					m_bCelShadeModels );
	ini.SetValue( "Options", "ConstantUpdateDeltaSeconds",		m_fConstantUpdateDeltaSeconds );
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
	ini.SetValue( "Options", "JudgeWindowAdd",					m_fJudgeWindowAdd );
	ini.SetValue( "Options", "JudgeWindowSecondsMarvelous",		m_fJudgeWindowSecondsMarvelous );
	ini.SetValue( "Options", "JudgeWindowSecondsPerfect",		m_fJudgeWindowSecondsPerfect );
	ini.SetValue( "Options", "JudgeWindowSecondsGreat",			m_fJudgeWindowSecondsGreat );
	ini.SetValue( "Options", "JudgeWindowSecondsGood",			m_fJudgeWindowSecondsGood );
	ini.SetValue( "Options", "JudgeWindowSecondsBoo",			m_fJudgeWindowSecondsBoo );
	ini.SetValue( "Options", "JudgeWindowSecondsOK",			m_fJudgeWindowSecondsOK );
	ini.SetValue( "Options", "JudgeWindowSecondsMine",			m_fJudgeWindowSecondsMine );
	ini.SetValue( "Options", "JudgeWindowSecondsAttack",		m_fJudgeWindowSecondsAttack );
	ini.SetValue( "Options", "LifeDifficultyScale",				m_fLifeDifficultyScale );
	ini.SetValue( "Options", "LifeDeltaPercentChangeMarvelous",	m_fLifeDeltaPercentChangeMarvelous );
	ini.SetValue( "Options", "LifeDeltaPercentChangePerfect",	m_fLifeDeltaPercentChangePerfect );
	ini.SetValue( "Options", "LifeDeltaPercentChangeGreat",		m_fLifeDeltaPercentChangeGreat );
	ini.SetValue( "Options", "LifeDeltaPercentChangeGood",		m_fLifeDeltaPercentChangeGood );
	ini.SetValue( "Options", "LifeDeltaPercentChangeBoo",		m_fLifeDeltaPercentChangeBoo );
	ini.SetValue( "Options", "LifeDeltaPercentChangeMiss",		m_fLifeDeltaPercentChangeMiss );
	ini.SetValue( "Options", "LifeDeltaPercentChangeHitMine",	m_fLifeDeltaPercentChangeHitMine );
	ini.SetValue( "Options", "LifeDeltaPercentChangeOK",		m_fLifeDeltaPercentChangeOK );
	ini.SetValue( "Options", "LifeDeltaPercentChangeNG",		m_fLifeDeltaPercentChangeNG );
	ini.SetValue( "Options", "TugMeterPercentChangeMarvelous",	m_fTugMeterPercentChangeMarvelous );
	ini.SetValue( "Options", "TugMeterPercentChangePerfect",	m_fTugMeterPercentChangePerfect );
	ini.SetValue( "Options", "TugMeterPercentChangeGreat",		m_fTugMeterPercentChangeGreat );
	ini.SetValue( "Options", "TugMeterPercentChangeGood",		m_fTugMeterPercentChangeGood );
	ini.SetValue( "Options", "TugMeterPercentChangeBoo",		m_fTugMeterPercentChangeBoo );
	ini.SetValue( "Options", "TugMeterPercentChangeMiss",		m_fTugMeterPercentChangeMiss );
	ini.SetValue( "Options", "TugMeterPercentChangeHitMine",	m_fTugMeterPercentChangeHitMine );
	ini.SetValue( "Options", "TugMeterPercentChangeOK",			m_fTugMeterPercentChangeOK );
	ini.SetValue( "Options", "TugMeterPercentChangeNG",			m_fTugMeterPercentChangeNG );
	ini.SetValue( "Options", "RegenComboAfterFail",				m_iRegenComboAfterFail );
	ini.SetValue( "Options", "RegenComboAfterMiss",				m_iRegenComboAfterMiss );
	ini.SetValue( "Options", "MaxRegenComboAfterFail",			m_iMaxRegenComboAfterFail );
	ini.SetValue( "Options", "MaxRegenComboAfterMiss",			m_iMaxRegenComboAfterMiss );
	ini.SetValue( "Options", "TwoPlayerRecovery",				m_bTwoPlayerRecovery );
	ini.SetValue( "Options", "MercifulDrain",					m_bMercifulDrain );
	ini.SetValue( "Options", "Minimum1FullSongInCourses",			m_bMinimum1FullSongInCourses );

	ini.SetValue( "Options", "PercentScoreWeightMarvelous",		m_iPercentScoreWeightMarvelous );
	ini.SetValue( "Options", "PercentScoreWeightPerfect",		m_iPercentScoreWeightPerfect );
	ini.SetValue( "Options", "PercentScoreWeightGreat",			m_iPercentScoreWeightGreat );
	ini.SetValue( "Options", "PercentScoreWeightGood",			m_iPercentScoreWeightGood );
	ini.SetValue( "Options", "PercentScoreWeightBoo",			m_iPercentScoreWeightBoo );
	ini.SetValue( "Options", "PercentScoreWeightMiss",			m_iPercentScoreWeightMiss );
	ini.SetValue( "Options", "PercentScoreWeightOK",			m_iPercentScoreWeightOK );
	ini.SetValue( "Options", "PercentScoreWeightNG",			m_iPercentScoreWeightNG );
	ini.SetValue( "Options", "PercentScoreWeightHitMine",		m_iPercentScoreWeightHitMine );
	ini.SetValue( "Options", "GradeWeightMarvelous",			m_iGradeWeightMarvelous );
	ini.SetValue( "Options", "GradeWeightPerfect",				m_iGradeWeightPerfect );
	ini.SetValue( "Options", "GradeWeightGreat",				m_iGradeWeightGreat );
	ini.SetValue( "Options", "GradeWeightGood",					m_iGradeWeightGood );
	ini.SetValue( "Options", "GradeWeightBoo",					m_iGradeWeightBoo );
	ini.SetValue( "Options", "GradeWeightMiss",					m_iGradeWeightMiss );
	ini.SetValue( "Options", "GradeWeightHitMine",				m_iGradeWeightHitMine );
	ini.SetValue( "Options", "GradeWeightOK",					m_iGradeWeightOK );
	ini.SetValue( "Options", "GradeWeightNG",					m_iGradeWeightNG );
	
	ini.SetValue( "Options", "NumGradeTiersUsed",				m_iNumGradeTiersUsed );
	for( int g=0; g<NUM_GRADE_TIERS; g++ )
	{
		Grade grade = (Grade)g;
		CString s = GradeToString( grade );
		ini.SetValue( "Options", "GradePercent"+s,			m_fGradePercent[g] );
	}
	ini.SetValue( "Options", "GradeTier02IsAllPerfects",		m_bGradeTier02IsAllPerfects );

	ini.SetValue( "Options", "SuperMeterPercentChangeMarvelous",m_fSuperMeterPercentChangeMarvelous );
	ini.SetValue( "Options", "SuperMeterPercentChangePerfect",	m_fSuperMeterPercentChangePerfect );
	ini.SetValue( "Options", "SuperMeterPercentChangeGreat",	m_fSuperMeterPercentChangeGreat );
	ini.SetValue( "Options", "SuperMeterPercentChangeGood",		m_fSuperMeterPercentChangeGood );
	ini.SetValue( "Options", "SuperMeterPercentChangeBoo",		m_fSuperMeterPercentChangeBoo );
	ini.SetValue( "Options", "SuperMeterPercentChangeMiss",		m_fSuperMeterPercentChangeMiss );
	ini.SetValue( "Options", "SuperMeterPercentChangeHitMine",	m_fSuperMeterPercentChangeHitMine );
	ini.SetValue( "Options", "SuperMeterPercentChangeOK",		m_fSuperMeterPercentChangeOK );
	ini.SetValue( "Options", "SuperMeterPercentChangeNG",		m_fSuperMeterPercentChangeNG );
	ini.SetValue( "Options", "MercifulSuperMeter",				m_bMercifulSuperMeter );

	ini.SetValue( "Options", "DelayedEscape",					m_bDelayedEscape );
	ini.SetValue( "Options", "HiddenSongs",						m_bHiddenSongs );
	ini.SetValue( "Options", "Vsync",							m_bVsync );
	ini.SetValue( "Options", "Interlaced",						m_bInterlaced );
	ini.SetValue( "Options", "PAL",								m_bPAL );
	ini.SetValue( "Options", "HowToPlay",						m_bInstructions );
	ini.SetValue( "Options", "Caution",							m_bShowDontDie );
	ini.SetValue( "Options", "ShowSelectGroup",					m_bShowSelectGroup );
	ini.SetValue( "Options", "ShowNative",						m_bShowNative );
	ini.SetValue( "Options", "ArcadeOptionsNavigation",			m_bArcadeOptionsNavigation );
	ini.SetValue( "Options", "DelayedTextureDelete",			m_bDelayedTextureDelete );
	ini.SetValue( "Options", "TexturePreload",					m_bTexturePreload );
	ini.SetValue( "Options", "DelayedScreenLoad",				m_bDelayedScreenLoad );
	ini.SetValue( "Options", "DelayedModelDelete",				m_bDelayedModelDelete );
	ini.SetValue( "Options", "BannerCache",						m_BannerCache );
	ini.SetValue( "Options", "PalettedBannerCache",				m_bPalettedBannerCache );
	ini.SetValue( "Options", "FastLoad",						m_bFastLoad );
	ini.SetValue( "Options", "MusicWheelUsesSections",			m_MusicWheelUsesSections );
	ini.SetValue( "Options", "MusicWheelSwitchSpeed",			m_iMusicWheelSwitchSpeed );
	ini.SetValue( "Options", "EasterEggs",						m_bEasterEggs );
	ini.SetValue( "Options", "MarvelousTiming",					m_iMarvelousTiming );
	ini.SetValue( "Options", "SoundResampleQuality",			m_iSoundResampleQuality );
	ini.SetValue( "Options", "CoinMode",						m_iCoinMode );
	ini.SetValue( "Options", "CoinsPerCredit",					m_iCoinsPerCredit );
	ini.SetValue( "Options", "Premium",							m_Premium );
	ini.SetValue( "Options", "DelayedCreditsReconcile",			m_bDelayedCreditsReconcile );
	ini.SetValue( "Options", "BoostAppPriority",				m_iBoostAppPriority );
	ini.SetValue( "Options", "PickExtraStage",					m_bPickExtraStage );
	ini.SetValue( "Options", "ComboContinuesBetweenSongs",		m_bComboContinuesBetweenSongs );
	ini.SetValue( "Options", "LongVerSeconds",					m_fLongVerSongSeconds );
	ini.SetValue( "Options", "MarathonVerSeconds",				m_fMarathonVerSongSeconds );
	ini.SetValue( "Options", "ShowSongOptions",					m_ShowSongOptions );
	ini.SetValue( "Options", "LightsStepsDifficulty",			m_sLightsStepsDifficulty );
	ini.SetValue( "Options", "BlinkGameplayButtonLightsOnNote",	m_bBlinkGameplayButtonLightsOnNote );
	ini.SetValue( "Options", "AllowUnacceleratedRenderer",		m_bAllowUnacceleratedRenderer );
	ini.SetValue( "Options", "ThreadedInput",					m_bThreadedInput );
	ini.SetValue( "Options", "ThreadedMovieDecode",				m_bThreadedMovieDecode );
	ini.SetValue( "Options", "ScreenTestMode",					m_bScreenTestMode );
	ini.SetValue( "Options", "MachineName",						m_sMachineName );
	ini.SetValue( "Options", "IgnoredMessageWindows",			m_sIgnoredMessageWindows );
	ini.SetValue( "Options", "SoloSingle",						m_bSoloSingle );
	ini.SetValue( "Options", "DancePointsForOni",				m_bDancePointsForOni );
	ini.SetValue( "Options", "PercentageScoring",				m_bPercentageScoring );
	ini.SetValue( "Options", "MinPercentageForMachineSongHighScore",	m_fMinPercentageForMachineSongHighScore );
	ini.SetValue( "Options", "MinPercentageForMachineCourseHighScore",	m_fMinPercentageForMachineCourseHighScore );
	ini.SetValue( "Options", "Disqualification",				m_bDisqualification );
	ini.SetValue( "Options", "ShowLyrics",						m_bShowLyrics );
	ini.SetValue( "Options", "AutogenSteps",					m_bAutogenSteps );
	ini.SetValue( "Options", "AutogenGroupCourses",				m_bAutogenGroupCourses );
	ini.SetValue( "Options", "BreakComboToGetItem",				m_bBreakComboToGetItem );
	ini.SetValue( "Options", "LockCourseDifficulties",			m_bLockCourseDifficulties );
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
	ini.SetValue( "Options", "SmoothLines",						m_bSmoothLines );
	ini.SetValue( "Options", "GlobalOffsetSeconds",				m_fGlobalOffsetSeconds );

	ini.SetValue( "Options", "CourseSortOrder",					m_iCourseSortOrder );
	ini.SetValue( "Options", "MoveRandomToEnd",					m_bMoveRandomToEnd );
	ini.SetValue( "Options", "SubSortByNumSteps",				m_bSubSortByNumSteps );

	ini.SetValue( "Options", "ScoringType",						m_iScoringType );

	ini.SetValue( "Options", "ProgressiveLifebar",				m_iProgressiveLifebar );
	ini.SetValue( "Options", "ProgressiveStageLifebar",			m_iProgressiveStageLifebar );
	ini.SetValue( "Options", "ProgressiveNonstopLifebar",		m_iProgressiveNonstopLifebar );
	ini.SetValue( "Options", "ShowBeginnerHelper",				m_bShowBeginnerHelper );
	ini.SetValue( "Options", "Language",						m_sLanguage );
	ini.SetValue( "Options", "EndlessBreakEnabled",				m_bEndlessBreakEnabled );
	ini.SetValue( "Options", "EndlessStagesUntilBreak",			m_iEndlessNumStagesUntilBreak );
	ini.SetValue( "Options", "EndlessBreakLength",				m_iEndlessBreakLength );
	ini.SetValue( "Options", "DisableScreenSaver",				m_bDisableScreenSaver );

	ini.SetValue( "Options", "MemoryCardProfileSubdir",			m_sMemoryCardProfileSubdir );
	ini.SetValue( "Options", "ProductID",						m_iProductID );
	ini.SetValue( "Options", "MemoryCards",						m_bMemoryCards );
	FOREACH_PlayerNumber( p )
	{
		ini.SetValue( "Options", ssprintf("DefaultLocalProfileIDP%d",p+1),	m_sDefaultLocalProfileID[p] );
		ini.SetValue( "Options", ssprintf("MemoryCardOsMountPointP%d",p+1),				m_sMemoryCardOsMountPoint[p] );
		ini.SetValue( "Options", ssprintf("MemoryCardUsbBusP%d",p+1),		m_iMemoryCardUsbBus[p] );
		ini.SetValue( "Options", ssprintf("MemoryCardUsbPortP%d",p+1),		m_iMemoryCardUsbPort[p] );
		ini.SetValue( "Options", ssprintf("MemoryCardUsbLevelP%d",p+1),		m_iMemoryCardUsbLevel[p] );
	}

	ini.SetValue( "Options", "CenterImageTranslateX",			m_iCenterImageTranslateX );
	ini.SetValue( "Options", "CenterImageTranslateY",			m_iCenterImageTranslateY );
	ini.SetValue( "Options", "CenterImageScaleX",				m_fCenterImageScaleX );
	ini.SetValue( "Options", "CenterImageScaleY",				m_fCenterImageScaleY );
	ini.SetValue( "Options", "AttractSoundFrequency",			m_iAttractSoundFrequency );
	ini.SetValue( "Options", "AllowExtraStage",					m_bAllowExtraStage );
	ini.SetValue( "Options", "HideDefaultNoteSkin",				m_bHideDefaultNoteSkin );
	ini.SetValue( "Options", "MaxHighScoresPerListForMachine",	m_iMaxHighScoresPerListForMachine );
	ini.SetValue( "Options", "MaxHighScoresPerListForPlayer",	m_iMaxHighScoresPerListForPlayer );
	ini.SetValue( "Options", "PadStickSeconds",					m_fPadStickSeconds );
	ini.SetValue( "Options", "ForceMipMaps",					m_bForceMipMaps );
	ini.SetValue( "Options", "TrilinearFiltering",				m_bTrilinearFiltering );
	ini.SetValue( "Options", "AnisotropicFiltering",			m_bAnisotropicFiltering );
	ini.SetValue( "Options", "AutoRestart",						g_bAutoRestart );
	ini.SetValue( "Options", "SignProfileData",					m_bSignProfileData );
	
	ini.SetValue( "Options", "SoundWriteAhead",					m_iSoundWriteAhead );
	ini.SetValue( "Options", "SoundDevice",						m_iSoundDevice );

	ini.SetValue( "Editor", "ShowBGChangesPlay",				m_bEditorShowBGChangesPlay );

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

	ini.SetValue( "Options", "AdditionalSongFolders", 			m_sAdditionalSongFolders);
	ini.SetValue( "Options", "AdditionalFolders", 				m_sAdditionalFolders);

	ini.SetValue( "Debug", "LogToDisk",							m_bLogToDisk );
	ini.SetValue( "Debug", "ForceLogFlush",						m_bForceLogFlush );
	ini.SetValue( "Debug", "ShowLogOutput",						m_bShowLogOutput );
	ini.SetValue( "Debug", "Timestamping",						m_bTimestamping );
	ini.SetValue( "Debug", "LogSkips",							m_bLogSkips );
	ini.SetValue( "Debug", "LogCheckpoints",					m_bLogCheckpoints );
	ini.SetValue( "Debug", "ShowLoadingWindow",					m_bShowLoadingWindow );

	FOREACH_CONST( IPreference*, *g_pvpSubscribers, p ) (*p)->WriteTo( ini );

	ini.WriteFile( STEPMANIA_INI_PATH );
}

CString PrefsManager::GetSoundDrivers()
{
	if( m_sSoundDrivers.empty() )
		return (CString)DEFAULT_SOUND_DRIVER_LIST;
	else
		return m_sSoundDrivers;
}

int PrefsManager::GetCoinMode()
{
	if( m_bEventMode && m_iCoinMode == COIN_PAY )
		return COIN_FREE; 
	else 
		return m_iCoinMode; 
}

PrefsManager::Premium	PrefsManager::GetPremium() 
{ 
	if(m_bEventMode) 
		return NO_PREMIUM; 
	else 
		return m_Premium; 
}


/*
 * (c) 2001-2004 Chris Danford, Chris Gomez
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

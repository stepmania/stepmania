#include "global.h"
#include "PrefsManager.h"
#include "IniFile.h"
#include "GameState.h"
#include "RageDisplay.h"
#include "RageUtil.h"
#include "arch/arch_default.h"
#include "RageSoundReader_Resample.h" /* for ResampleQuality */
#include "RageFile.h"
#include "ProductInfo.h"
#include "Foreach.h"
#include "Preference.h"
#include "RageLog.h"

#define DEFAULTS_INI_PATH	"Data/Defaults.ini"		// these can be overridden
#define STEPMANIA_INI_PATH	"Data/StepMania.ini"	// overlay on Defaults.ini, contains the user's choices
#define STATIC_INI_PATH		"Data/Static.ini"		// overlay on the 2 above, can't be overridden

PrefsManager*	PREFSMAN = NULL;	// global and accessable from anywhere in our program

const float DEFAULT_SOUND_VOLUME = 1.00f;
const CString DEFAULT_LIGHTS_DRIVER = "Null";


//
// For self-registering prefs
//
#include "SubscriptionManager.h"
template<>
set<IPreference*>* SubscriptionManager<IPreference>::s_pSubscribers = NULL;

void PrefsManager::Subscribe( IPreference *p )
{
	SubscriptionManager<IPreference>::Subscribe( p );
}

void PrefsManager::Unsubscribe( IPreference *p )
{
	SubscriptionManager<IPreference>::Unsubscribe( p );
}

IPreference *PrefsManager::GetPreferenceByName( const CString &sName )
{
	FOREACHS( IPreference*, *SubscriptionManager<IPreference>::s_pSubscribers, p )
	{
		if( !(*p)->GetName().CompareNoCase( sName ) )
			return *p;
	}

	return NULL;
}

bool g_bAutoRestart = false;

PrefsManager::PrefsManager() :
	m_bWindowed				( Options, "Windowed",				
#ifdef DEBUG
	true
#else
	false
#endif
		),
	m_iDisplayWidth			( Options, "DisplayWidth",			640 ),
	m_iDisplayHeight		( Options, "DisplayHeight",			480 ),
	m_iDisplayColorDepth	( Options, "DisplayColorDepth",		16 ),
	m_iTextureColorDepth	( Options, "TextureColorDepth",		16 ),
	m_iMovieColorDepth		( Options, "MovieColorDepth",		16 ),
	m_iMaxTextureResolution	( Options, "MaxTextureResolution",	2048 ),
	m_iRefreshRate			( Options, "RefreshRate",			REFRESH_DEFAULT ),
	m_fDisplayAspectRatio	( Options, "DisplayAspectRatio",	4/3.0f ),
	m_bShowStats			( Options, "ShowStats",				false ),
	m_bShowBanners			( Options, "ShowBanners",			true ),

	m_BackgroundMode		( Options, "BackgroundMode",		BGMODE_ANIMATIONS ),
	m_iNumBackgrounds		( Options, "NumBackgrounds",		8 ),
	m_fBGBrightness			( Options, "BGBrightness",			0.8f ),
	m_bHiddenSongs			( Options, "HiddenSongs",			false ),	/* I'd rather get occasional people asking for support for this even though it's already here than lots of people asking why songs aren't being displayed. */
	m_bVsync				( Options, "Vsync",					true ),
	m_bInterlaced			( Options, "Interlaced",			false ),
	/* XXX: Set these defaults for individual consoles using VideoCardDefaults.ini. */
	m_bPAL					( Options, "PAL",					false ),
	m_bDelayedTextureDelete	( Options, "DelayedTextureDelete",	true ),
	m_bTexturePreload		( Options, "TexturePreload",		false ),
	m_bDelayedScreenLoad	( Options, "DelayedScreenLoad",		false ),
	m_bDelayedModelDelete	( Options, "DelayedModelDelete",	false ),
	m_BannerCache			( Options, "BannerCache",			BNCACHE_LOW_RES_PRELOAD ),
	m_bPalettedBannerCache	( Options, "PalettedBannerCache",	false ),
	m_bFastLoad				( Options, "FastLoad",				true ),

	m_bOnlyDedicatedMenuButtons	( Options, "OnlyDedicatedMenuButtons",	false ),
	m_bMenuTimer				( Options, "MenuTimer",					true ),
	m_bShowDanger				( Options, "ShowDanger",					true ),

	m_fJudgeWindowScale				( Options, "JudgeWindowScale",				1.0f ),
	m_fJudgeWindowAdd				( Options, "JudgeWindowAdd",				0 ),
	m_fJudgeWindowSecondsMarvelous	( Options, "JudgeWindowSecondsMarvelous",	0.0225f ),
	m_fJudgeWindowSecondsPerfect	( Options, "JudgeWindowSecondsPerfect",		0.045f ),
	m_fJudgeWindowSecondsGreat		( Options, "JudgeWindowSecondsGreat",		0.090f ),
	m_fJudgeWindowSecondsGood		( Options, "JudgeWindowSecondsGood",		0.135f ),
	m_fJudgeWindowSecondsBoo		( Options, "JudgeWindowSecondsBoo",			0.180f ),
	m_fJudgeWindowSecondsOK			( Options, "JudgeWindowSecondsOK",			0.250f ),	// allow enough time to take foot off and put back on
	m_fJudgeWindowSecondsRoll		( Options, "JudgeWindowSecondsRoll",		0.350f ),
	m_fJudgeWindowSecondsMine		( Options, "JudgeWindowSecondsMine",		0.090f ),	// same as great
	m_fJudgeWindowSecondsAttack		( Options, "JudgeWindowSecondsAttack",		0.135f ),

	m_fLifeDifficultyScale				( Options, "LifeDifficultyScale",				1.0f ),
	m_fLifeDeltaPercentChangeMarvelous	( Options, "LifeDeltaPercentChangeMarvelous",	+0.008f ),
	m_fLifeDeltaPercentChangePerfect	( Options, "LifeDeltaPercentChangePerfect",		+0.008f ),
	m_fLifeDeltaPercentChangeGreat		( Options, "LifeDeltaPercentChangeGreat",		+0.004f ),
	m_fLifeDeltaPercentChangeGood		( Options, "LifeDeltaPercentChangeGood",		+0.000f ),
	m_fLifeDeltaPercentChangeBoo		( Options, "LifeDeltaPercentChangeBoo",			-0.040f ),
	m_fLifeDeltaPercentChangeMiss		( Options, "LifeDeltaPercentChangeMiss",		-0.080f ),
	m_fLifeDeltaPercentChangeHitMine	( Options, "LifeDeltaPercentChangeHitMine",		-0.160f ),
	m_fLifeDeltaPercentChangeOK			( Options, "LifeDeltaPercentChangeOK",			+0.008f ),
	m_fLifeDeltaPercentChangeNG			( Options, "LifeDeltaPercentChangeNG",			-0.080f ),

	m_fTugMeterPercentChangeMarvelous	( Options, "TugMeterPercentChangeMarvelous",	+0.010f ),
	m_fTugMeterPercentChangePerfect		( Options, "TugMeterPercentChangePerfect",		+0.008f ),
	m_fTugMeterPercentChangeGreat		( Options, "TugMeterPercentChangeGreat",		+0.004f ),
	m_fTugMeterPercentChangeGood		( Options, "TugMeterPercentChangeGood",			+0.000f ),
	m_fTugMeterPercentChangeBoo			( Options, "TugMeterPercentChangeBoo",			-0.010f ),
	m_fTugMeterPercentChangeMiss		( Options, "TugMeterPercentChangeMiss",			-0.020f ),
	m_fTugMeterPercentChangeHitMine		( Options, "TugMeterPercentChangeHitMine",		-0.040f ),
	m_fTugMeterPercentChangeOK			( Options, "TugMeterPercentChangeOK",			+0.008f ),
	m_fTugMeterPercentChangeNG			( Options, "TugMeterPercentChangeNG",			-0.020f ),

	m_iRegenComboAfterFail			( Options, "RegenComboAfterFail",			10 ),
	m_iRegenComboAfterMiss			( Options, "RegenComboAfterMiss",			5 ),
	m_iMaxRegenComboAfterFail		( Options, "MaxRegenComboAfterFail",		10 ),
	m_iMaxRegenComboAfterMiss		( Options, "MaxRegenComboAfterMiss",		10 ),
	m_bTwoPlayerRecovery			( Options, "TwoPlayerRecovery",				true ),
	m_bMercifulDrain				( Options, "MercifulDrain",					true ),	// negative life deltas are scaled by the players life percentage
	m_bMinimum1FullSongInCourses	( Options, "Minimum1FullSongInCourses",		false ),	// FEoS for 1st song, FailImmediate thereafter
	m_bFailOffInBeginner			( Options, "FailOffInBeginner",				false ),
	m_bFailOffForFirstStageEasy		( Options, "FailOffForFirstStageEasy",		false ),
	m_bMercifulBeginner				( Options, "MercifulBeginner",				false ),

	m_iPercentScoreWeightMarvelous	( Options, "PercentScoreWeightMarvelous",	3 ),
	m_iPercentScoreWeightPerfect	( Options, "PercentScoreWeightPerfect",		2 ),
	m_iPercentScoreWeightGreat		( Options, "PercentScoreWeightGreat",		1 ),
	m_iPercentScoreWeightGood		( Options, "PercentScoreWeightGood",		0 ),
	m_iPercentScoreWeightBoo		( Options, "PercentScoreWeightBoo",			0 ),
	m_iPercentScoreWeightMiss		( Options, "PercentScoreWeightMiss",		0 ),
	m_iPercentScoreWeightHitMine	( Options, "PercentScoreWeightHitMine",		-2 ),
	m_iPercentScoreWeightOK			( Options, "PercentScoreWeightOK",			3 ),
	m_iPercentScoreWeightNG			( Options, "PercentScoreWeightNG",			0 ),
	
	m_iGradeWeightMarvelous		( Options, "GradeWeightMarvelous",	2 ),
	m_iGradeWeightPerfect		( Options, "GradeWeightPerfect",	2 ),
	m_iGradeWeightGreat			( Options, "GradeWeightGreat",		1 ),
	m_iGradeWeightGood			( Options, "GradeWeightGood",		0 ),
	m_iGradeWeightBoo			( Options, "GradeWeightBoo",		-4 ),
	m_iGradeWeightMiss			( Options, "GradeWeightMiss",		-8 ),
	m_iGradeWeightHitMine		( Options, "GradeWeightHitMine",	-8 ),
	m_iGradeWeightOK			( Options, "GradeWeightOK",			6 ),
	m_iGradeWeightNG			( Options, "GradeWeightNG",			0 ),

	m_fSuperMeterPercentChangeMarvelous	( Options, "SuperMeterPercentChangeMarvelous",	+0.05f ),
	m_fSuperMeterPercentChangePerfect	( Options, "SuperMeterPercentChangePerfect",	+0.04f ),
	m_fSuperMeterPercentChangeGreat		( Options, "SuperMeterPercentChangeGreat",		+0.02f ),
	m_fSuperMeterPercentChangeGood		( Options, "SuperMeterPercentChangeGood",		+0.00f ),
	m_fSuperMeterPercentChangeBoo		( Options, "SuperMeterPercentChangeBoo",		-0.00f ),
	m_fSuperMeterPercentChangeMiss		( Options, "SuperMeterPercentChangeMiss",		-0.20f ),
	m_fSuperMeterPercentChangeHitMine	( Options, "SuperMeterPercentChangeHitMine",	-0.40f ),
	m_fSuperMeterPercentChangeOK		( Options, "SuperMeterPercentChangeOK",			+0.04f ),
	m_fSuperMeterPercentChangeNG		( Options, "SuperMeterPercentChangeNG",			-0.20f ),
	m_bMercifulSuperMeter				( Options, "MercifulSuperMeter",				true ),
	
	m_fTimeMeterSecondsChangeMarvelous	( Options, "TimeMeterSecondsChangeMarvelous",	+0.1f ),
	m_fTimeMeterSecondsChangePerfect	( Options, "TimeMeterSecondsChangePerfect",		 0.0f ),
	m_fTimeMeterSecondsChangeGreat		( Options, "TimeMeterSecondsChangeGreat",		-0.5f ),
	m_fTimeMeterSecondsChangeGood		( Options, "TimeMeterSecondsChangeGood",		-1.0f ),
	m_fTimeMeterSecondsChangeBoo		( Options, "TimeMeterSecondsChangeBoo",			-2.0f ),
	m_fTimeMeterSecondsChangeMiss		( Options, "TimeMeterSecondsChangeMiss",		-4.0f ),
	m_fTimeMeterSecondsChangeHitMine	( Options, "TimeMeterSecondsChangeHitMine",		-2.0f ),
	m_fTimeMeterSecondsChangeOK			( Options, "TimeMeterSecondsChangeOK",			-0.0f ),
	m_fTimeMeterSecondsChangeNG			( Options, "TimeMeterSecondsChangeNG",			-4.0f ),

	m_bAutoPlay					( Options, "AutoPlay",					false ),
	m_bDelayedBack				( Options, "DelayedBack",				true ),
	m_bShowInstructions			( Options, "ShowInstructions",			true ),
	m_bShowSelectGroup			( Options, "ShowSelectGroup",			true ),
	m_bShowCaution				( Options, "ShowCaution",				true ),
	m_bShowNativeLanguage		( Options, "ShowNativeLanguage",		true ),
	m_bArcadeOptionsNavigation	( Options, "ArcadeOptionsNavigation",	false ),
	m_MusicWheelUsesSections	( Options, "MusicWheelUsesSections",	ALWAYS ),
	m_iMusicWheelSwitchSpeed	( Options, "MusicWheelSwitchSpeed",		10 ),
	m_bEasterEggs				( Options, "EasterEggs",				true ),
	m_MarvelousTiming			( Options, "MarvelousTiming",			MARVELOUS_EVERYWHERE ),
	m_bEventMode				( Options, "EventMode",					false ),
	m_iCoinsPerCredit			( Options, "CoinsPerCredit",			1 ),
	m_iSongsPerPlay				( Options, "SongsPerPlay",				3 ),

	m_CoinMode						( Options, "CoinMode",					COIN_HOME ),
	m_Premium						( Options, "Premium",					PREMIUM_NONE ),
	m_bDelayedCreditsReconcile		( Options, "DelayedCreditsReconcile",	false ),
	m_bPickExtraStage				( Options, "PickExtraStage",			false ),

	m_bComboContinuesBetweenSongs	( Options, "ComboContinuesBetweenSongs",false ),
	m_fLongVerSongSeconds			( Options, "LongVerSongSeconds",		60*2.5f ),	// Dynamite Rave is 2:55
	m_fMarathonVerSongSeconds		( Options, "MarathonVerSongSeconds",	60*5.f ),
	m_ShowSongOptions				( Options, "ShowSongOptions",			YES ),
	m_bSoloSingle					( Options, "SoloSingle",				false ),
	m_bDancePointsForOni			( Options, "DancePointsForOni",			false ),
	m_bPercentageScoring			( Options, "PercentageScoring",		false ),
	m_fMinPercentageForMachineSongHighScore		( Options, "MinPercentageForMachineSongHighScore",		0.5f ),
	m_fMinPercentageForMachineCourseHighScore	( Options, "MinPercentageForMachineCourseHighScore",	0.0001f ),	// don't save course scores with 0 percentage
	m_bDisqualification				( Options, "Disqualification",			false ),
	m_bShowLyrics					( Options, "ShowLyrics",				true ),
	m_bAutogenSteps					( Options, "AutogenSteps",				true ),
	m_bAutogenGroupCourses			( Options, "AutogenGroupCourses",		true ),
	m_bBreakComboToGetItem			( Options, "BreakComboToGetItem",		false ),
	m_bLockCourseDifficulties		( Options, "LockCourseDifficulties",	true ),
	m_ShowDancingCharacters			( Options, "ShowDancingCharacters",		CO_OFF ),
	m_bUseUnlockSystem				( Options, "UseUnlockSystem",			false ),
	m_bFirstRun						( Options, "FirstRun",					true ),
	m_bAutoMapOnJoyChange			( Options, "AutoMapOnJoyChange",		true ),
	m_fGlobalOffsetSeconds			( Options, "GlobalOffsetSeconds",		0 ),
	m_iProgressiveLifebar			( Options, "ProgressiveLifebar",		0 ),
	m_iProgressiveStageLifebar		( Options, "ProgressiveStageLifebar",	0 ),
	m_iProgressiveNonstopLifebar	( Options, "ProgressiveNonstopLifebar",	0 ),
	m_bShowBeginnerHelper			( Options, "ShowBeginnerHelper",		false ),
	m_bEndlessBreakEnabled			( Options, "EndlessBreakEnabled",		true ),
	m_iEndlessNumStagesUntilBreak	( Options, "EndlessNumStagesUntilBreak",5 ),
	m_iEndlessBreakLength			( Options, "EndlessBreakLength",		5 ),
	m_bDisableScreenSaver			( Options, "DisableScreenSaver",		true ),
	m_sLanguage						( Options, "Language",					"" ),	// ThemeManager will deal with this invalid language
	m_sMemoryCardProfileSubdir		( Options, "MemoryCardProfileSubdir",	PRODUCT_NAME ),
	m_iProductID					( Options, "ProductID",					1 ),
	
	m_bMemoryCards					( Options, "MemoryCards",		false ),
	
	m_iCenterImageTranslateX		( Options, "CenterImageTranslateX",	0 ),
	m_iCenterImageTranslateY		( Options, "CenterImageTranslateY",	0 ),
	m_fCenterImageAddWidth			( Options, "CenterImageAddWidth",	0 ),
	m_fCenterImageAddHeight			( Options, "CenterImageAddHeight",	0 ),
	m_iAttractSoundFrequency		( Options, "AttractSoundFrequency",	1 ),
	m_bAllowExtraStage				( Options, "AllowExtraStage",		true ),
	m_bHideDefaultNoteSkin			( Options, "HideDefaultNoteSkin",	false ),
	m_iMaxHighScoresPerListForMachine		( Options, "MaxHighScoresPerListForMachine",		10 ),
	m_iMaxHighScoresPerListForPlayer		( Options, "MaxHighScoresPerListForPlayer",			3 ),
	m_iMaxRecentScoresForMachine			( Options, "MaxRecentScoresForMachine",				100 ),
	m_iMaxRecentScoresForPlayer				( Options, "MaxRecentScoresForPlayer",				20 ),
	m_bAllowMultipleHighScoreWithSameName	( Options, "AllowMultipleHighScoreWithSameName",	true ),
	m_bCelShadeModels						( Options, "CelShadeModels",						false )	// Work-In-Progress.. disable by default.
{
	Init();
	ReadGlobalPrefsFromDisk();
}

void PrefsManager::Init()
{
	m_fConstantUpdateDeltaSeconds = 0;	
	
	m_iBoostAppPriority = -1;
	m_bSmoothLines = false;

	// default to old sort order
	m_iCourseSortOrder = COURSE_SORT_SONGS;
	m_bMoveRandomToEnd = false;
	m_bSubSortByNumSteps = false;
	m_iScoringType = SCORING_MAX2;

	m_iGetRankingName = RANKING_ON;


	m_fPadStickSeconds = 0;
	m_bForceMipMaps = false;
	m_bTrilinearFiltering = false;
	m_bAnisotropicFiltering = false;
	g_bAutoRestart = false;
	m_bSignProfileData = false;

	m_bEditorShowBGChangesPlay = false;

	m_sSoundDrivers = DEFAULT_SOUND_DRIVER_LIST;
	/* Number of frames to write ahead; usually 44100 frames per second.
	 * (Number of millisec would be more flexible, but it's more useful to
	 * specify numbers directly.) This is purely a troubleshooting option
	 * and is not honored by all sound drivers. */
	m_iSoundWriteAhead = 0;
	m_iSoundDevice = "";
	m_fSoundVolume = DEFAULT_SOUND_VOLUME;
	m_iSoundResampleQuality = RageSoundReader_Resample::RESAMP_NORMAL;

	m_sInputDrivers = DEFAULT_INPUT_DRIVER_LIST;

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

	FOREACH_PlayerNumber( p )
	{
		m_iMemoryCardUsbBus[p] = -1;
		m_iMemoryCardUsbPort[p] = -1;
		m_iMemoryCardUsbLevel[p] = -1;
	}
	
#if defined(XBOX)
	m_bEnableVirtualMemory = true;
	m_iPageFileSize = 384;
	m_iPageSize = 16;
	m_iPageThreshold = 8;
	m_bLogVirtualMemory = false;
#endif

	FOREACHS_CONST( IPreference*, *SubscriptionManager<IPreference>::s_pSubscribers, p )
		(*p)->LoadDefault();
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

	ReadGlobalPrefsFromIni( ini );
}

void PrefsManager::ReadGlobalPrefsFromIni( const IniFile &ini )
{
	ini.GetValue( "Options", "ConstantUpdateDeltaSeconds",		m_fConstantUpdateDeltaSeconds );
	
	ini.GetValue( "Options", "SoundDrivers",					m_sSoundDrivers );
	ini.GetValue( "Options", "SoundWriteAhead",					m_iSoundWriteAhead );
	ini.GetValue( "Options", "SoundDevice",						m_iSoundDevice );
	ini.GetValue( "Options", "InputDrivers",					m_sInputDrivers );
	ini.GetValue( "Options", "MovieDrivers",					m_sMovieDrivers );
	ini.GetValue( "Options", "SoundVolume",						m_fSoundVolume );
	ini.GetValue( "Options", "LightsDriver",					m_sLightsDriver );
	ini.GetValue( "Options", "SoundResampleQuality",			m_iSoundResampleQuality );
	
	m_iCoinsPerCredit = max( (int)m_iCoinsPerCredit, 1);

	ini.GetValue( "Options", "BoostAppPriority",				m_iBoostAppPriority );
	ini.GetValue( "Options", "LightsStepsDifficulty",			m_sLightsStepsDifficulty );
	ini.GetValue( "Options", "BlinkGameplayButtonLightsOnNote",	m_bBlinkGameplayButtonLightsOnNote );
	ini.GetValue( "Options", "AllowUnacceleratedRenderer",		m_bAllowUnacceleratedRenderer );
	ini.GetValue( "Options", "ThreadedInput",					m_bThreadedInput );
	ini.GetValue( "Options", "ThreadedMovieDecode",				m_bThreadedMovieDecode );
	ini.GetValue( "Options", "ScreenTestMode",					m_bScreenTestMode );
	ini.GetValue( "Options", "MachineName",						m_sMachineName );
	ini.GetValue( "Options", "IgnoredMessageWindows",			m_sIgnoredMessageWindows );

	ini.GetValue( "Options", "CourseSortOrder",					(int&)m_iCourseSortOrder );
	ini.GetValue( "Options", "MoveRandomToEnd",					m_bMoveRandomToEnd );
	ini.GetValue( "Options", "SubSortByNumSteps",				m_bSubSortByNumSteps );

	ini.GetValue( "Options", "ScoringType",						(int&)m_iScoringType );

	ini.GetValue( "Options", "VideoRenderers",					m_sVideoRenderers );
	ini.GetValue( "Options", "LastSeenVideoDriver",				m_sLastSeenVideoDriver );
	ini.GetValue( "Options", "LastSeenInputDevices",			m_sLastSeenInputDevices );
#if defined(WIN32)
	ini.GetValue( "Options", "LastSeenMemory",					m_iLastSeenMemory );
#endif
	ini.GetValue( "Options", "CoursesToShowRanking",			m_sCoursesToShowRanking );
	ini.GetValue( "Options", "GetRankingName",					(int&)m_iGetRankingName);
	ini.GetValue( "Options", "SmoothLines",						m_bSmoothLines );

	FOREACH_PlayerNumber( p )
	{
		ini.GetValue( "Options", ssprintf("DefaultLocalProfileIDP%d",p+1),	m_sDefaultLocalProfileID[p] );
		ini.GetValue( "Options", ssprintf("MemoryCardOsMountPointP%d",p+1),	m_sMemoryCardOsMountPoint[p] );
		FixSlashesInPlace( m_sMemoryCardOsMountPoint[p] );
		ini.GetValue( "Options", ssprintf("MemoryCardUsbBusP%d",p+1),		m_iMemoryCardUsbBus[p] );
		ini.GetValue( "Options", ssprintf("MemoryCardUsbPortP%d",p+1),		m_iMemoryCardUsbPort[p] );
		ini.GetValue( "Options", ssprintf("MemoryCardUsbLevelP%d",p+1),		m_iMemoryCardUsbLevel[p] );
	}

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

#if defined(XBOX)
	ini.GetValue( "Options", "EnableVirtualMemory",				m_bEnableVirtualMemory );
	ini.GetValue( "Options", "PageFileSize",					m_iPageFileSize );
	ini.GetValue( "Options", "PageSize",						m_iPageSize );
	ini.GetValue( "Options", "PageThreshold",					m_iPageThreshold );
	ini.GetValue( "Debug", "LogVirtualMemory",					m_bLogVirtualMemory );
#endif

	ini.GetValue( "Debug", "LogToDisk",							m_bLogToDisk );
	ini.GetValue( "Debug", "ForceLogFlush",						m_bForceLogFlush );
	ini.GetValue( "Debug", "ShowLogOutput",						m_bShowLogOutput );
	ini.GetValue( "Debug", "Timestamping",						m_bTimestamping );
	ini.GetValue( "Debug", "LogSkips",							m_bLogSkips );
	ini.GetValue( "Debug", "LogCheckpoints",					m_bLogCheckpoints );
	ini.GetValue( "Debug", "ShowLoadingWindow",					m_bShowLoadingWindow );

	FOREACHS_CONST( IPreference*, *SubscriptionManager<IPreference>::s_pSubscribers, p )
		(*p)->ReadFrom( ini );
}

void PrefsManager::SaveGlobalPrefsToDisk() const
{
	IniFile ini;
	SaveGlobalPrefsToIni( ini );
	ini.WriteFile( STEPMANIA_INI_PATH );
}

void PrefsManager::SaveGlobalPrefsToIni( IniFile &ini ) const
{
	ini.SetValue( "Options", "ConstantUpdateDeltaSeconds",		m_fConstantUpdateDeltaSeconds );
	ini.SetValue( "Options", "BackgroundMode",					m_BackgroundMode);
	ini.SetValue( "Options", "NumBackgrounds",					m_iNumBackgrounds);
	ini.SetValue( "Options", "BGBrightness",					m_fBGBrightness );

	ini.SetValue( "Options", "JudgeWindowScale",				m_fJudgeWindowScale );
	ini.SetValue( "Options", "JudgeWindowAdd",					m_fJudgeWindowAdd );
	ini.SetValue( "Options", "JudgeWindowSecondsMarvelous",		m_fJudgeWindowSecondsMarvelous );
	ini.SetValue( "Options", "JudgeWindowSecondsPerfect",		m_fJudgeWindowSecondsPerfect );
	ini.SetValue( "Options", "JudgeWindowSecondsGreat",			m_fJudgeWindowSecondsGreat );
	ini.SetValue( "Options", "JudgeWindowSecondsGood",			m_fJudgeWindowSecondsGood );
	ini.SetValue( "Options", "JudgeWindowSecondsBoo",			m_fJudgeWindowSecondsBoo );
	ini.SetValue( "Options", "JudgeWindowSecondsOK",			m_fJudgeWindowSecondsOK );
	ini.SetValue( "Options", "JudgeWindowSecondsRoll",			m_fJudgeWindowSecondsRoll );
	ini.SetValue( "Options", "JudgeWindowSecondsMine",			m_fJudgeWindowSecondsMine );
	ini.SetValue( "Options", "JudgeWindowSecondsAttack",		m_fJudgeWindowSecondsAttack );
	ini.SetValue( "Options", "LifeDifficultyScale",				m_fLifeDifficultyScale );

	ini.SetValue( "Options", "HiddenSongs",						m_bHiddenSongs );
	ini.SetValue( "Options", "Vsync",							m_bVsync );
	ini.SetValue( "Options", "Interlaced",						m_bInterlaced );
	ini.SetValue( "Options", "PAL",								m_bPAL );

	ini.SetValue( "Options", "DelayedTextureDelete",			m_bDelayedTextureDelete );
	ini.SetValue( "Options", "TexturePreload",					m_bTexturePreload );
	ini.SetValue( "Options", "DelayedScreenLoad",				m_bDelayedScreenLoad );
	ini.SetValue( "Options", "DelayedModelDelete",				m_bDelayedModelDelete );
	ini.SetValue( "Options", "FastLoad",						m_bFastLoad );
	ini.SetValue( "Options", "SoundResampleQuality",			m_iSoundResampleQuality );
	ini.SetValue( "Options", "BoostAppPriority",				m_iBoostAppPriority );
	ini.SetValue( "Options", "LightsStepsDifficulty",			m_sLightsStepsDifficulty );
	ini.SetValue( "Options", "BlinkGameplayButtonLightsOnNote",	m_bBlinkGameplayButtonLightsOnNote );
	ini.SetValue( "Options", "AllowUnacceleratedRenderer",		m_bAllowUnacceleratedRenderer );
	ini.SetValue( "Options", "ThreadedInput",					m_bThreadedInput );
	ini.SetValue( "Options", "ThreadedMovieDecode",				m_bThreadedMovieDecode );
	ini.SetValue( "Options", "ScreenTestMode",					m_bScreenTestMode );
	ini.SetValue( "Options", "MachineName",						m_sMachineName );
	ini.SetValue( "Options", "IgnoredMessageWindows",			m_sIgnoredMessageWindows );
	ini.SetValue( "Options", "VideoRenderers",					m_sVideoRenderers );
	ini.SetValue( "Options", "LastSeenVideoDriver",				m_sLastSeenVideoDriver );
	ini.SetValue( "Options", "LastSeenInputDevices",			m_sLastSeenInputDevices );
#if defined(WIN32)
	ini.SetValue( "Options", "LastSeenMemory",					m_iLastSeenMemory );
#endif
	ini.SetValue( "Options", "CoursesToShowRanking",			m_sCoursesToShowRanking );
	ini.SetValue( "Options", "GetRankingName",					m_iGetRankingName);
	ini.SetValue( "Options", "SmoothLines",						m_bSmoothLines );

	ini.SetValue( "Options", "CourseSortOrder",					m_iCourseSortOrder );
	ini.SetValue( "Options", "MoveRandomToEnd",					m_bMoveRandomToEnd );
	ini.SetValue( "Options", "SubSortByNumSteps",				m_bSubSortByNumSteps );

	ini.SetValue( "Options", "ScoringType",						m_iScoringType );

	FOREACH_PlayerNumber( p )
	{
		ini.SetValue( "Options", ssprintf("DefaultLocalProfileIDP%d",p+1),	m_sDefaultLocalProfileID[p] );
		ini.SetValue( "Options", ssprintf("MemoryCardOsMountPointP%d",p+1),				m_sMemoryCardOsMountPoint[p] );
		ini.SetValue( "Options", ssprintf("MemoryCardUsbBusP%d",p+1),		m_iMemoryCardUsbBus[p] );
		ini.SetValue( "Options", ssprintf("MemoryCardUsbPortP%d",p+1),		m_iMemoryCardUsbPort[p] );
		ini.SetValue( "Options", ssprintf("MemoryCardUsbLevelP%d",p+1),		m_iMemoryCardUsbLevel[p] );
	}

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
	if(m_sInputDrivers != DEFAULT_INPUT_DRIVER_LIST)
		ini.SetValue ( "Options", "InputDrivers",				m_sInputDrivers );
	if(m_sMovieDrivers != DEFAULT_MOVIE_DRIVER_LIST)
		ini.SetValue ( "Options", "MovieDrivers",				m_sMovieDrivers );
	if(m_sLightsDriver != DEFAULT_LIGHTS_DRIVER)
		ini.SetValue( "Options", "LightsDriver",				m_sLightsDriver );

	ini.SetValue( "Options", "AdditionalSongFolders", 			m_sAdditionalSongFolders);
	ini.SetValue( "Options", "AdditionalFolders", 				m_sAdditionalFolders);

#if defined(XBOX)
	ini.SetValue( "Options", "EnableVirtualMemory",				m_bEnableVirtualMemory );
	ini.SetValue( "Options", "PageFileSize",					m_iPageFileSize );
	ini.SetValue( "Options", "PageSize",						m_iPageSize );
	ini.SetValue( "Options", "PageThreshold",					m_iPageThreshold );
#endif

	ini.SetValue( "Debug", "LogToDisk",							m_bLogToDisk );
	ini.SetValue( "Debug", "ForceLogFlush",						m_bForceLogFlush );
	ini.SetValue( "Debug", "ShowLogOutput",						m_bShowLogOutput );
	ini.SetValue( "Debug", "Timestamping",						m_bTimestamping );
	ini.SetValue( "Debug", "LogSkips",							m_bLogSkips );
	ini.SetValue( "Debug", "LogCheckpoints",					m_bLogCheckpoints );
	ini.SetValue( "Debug", "ShowLoadingWindow",					m_bShowLoadingWindow );
#if defined(XBOX)
	ini.SetValue( "Debug", "LogVirtualMemory",					m_bLogVirtualMemory );
#endif

	FOREACHS_CONST( IPreference*, *SubscriptionManager<IPreference>::s_pSubscribers, p )
		(*p)->WriteTo( ini );
}


// lua start
#include "LuaBinding.h"

template<class T>
class LunaPrefsManager : public Luna<T>
{
public:
	LunaPrefsManager() { LUA->Register( Register ); }

	static int GetPreference( T* p, lua_State *L )
	{
		CString sName = SArg(1);
		IPreference *pPref = PREFSMAN->GetPreferenceByName( sName );
		if( pPref == NULL )
		{
			LOG->Warn( "GetPreference: unknown preference \"%s\"", sName.c_str() );
			lua_pushnil( L );
		}
		else
		{
			pPref->PushValue( L );
		}
		return 1;
	}
	static int SetPreference( T* p, lua_State *L )
	{
		CString sName = SArg(1);

		IPreference *pPref = PREFSMAN->GetPreferenceByName( sName );
		if( pPref == NULL )
			LOG->Warn( "GetPreference: unknown preference \"%s\"", sName.c_str() );
		else
		{
			lua_pushvalue( L, 2 );
			pPref->SetFromStack( L );
		}

		return 0;
	}

	static void Register(lua_State *L)
	{
		ADD_METHOD( GetPreference )
		ADD_METHOD( SetPreference )
		Luna<T>::Register( L );

		// Add global singleton if constructed already.  If it's not constructed yet,
		// then we'll register it later when we reinit Lua just before 
		// initializing the display.
		if( PREFSMAN )
		{
			lua_pushstring(L, "PREFSMAN");
			PREFSMAN->PushSelf( LUA->L );
			lua_settable(L, LUA_GLOBALSINDEX);
		}
	}
};

LUA_REGISTER_CLASS( PrefsManager )
// lua end

#include "LuaFunctions.h"


int LuaFunc_GetPreference( lua_State *L )
{
	REQ_ARGS( "GetPreference", 1 );
	REQ_ARG( "GetPreference", 1, string );

	CString sName;
	LuaHelpers::PopStack( sName, NULL );

	IPreference *pPref = PREFSMAN->GetPreferenceByName( sName );
	if( pPref == NULL )
	{
		LOG->Warn( "GetPreference: unknown preference \"%s\"", sName.c_str() );
		lua_pushnil( L );
	}
	else
		pPref->PushValue( L );

	return 1;
}
LuaFunction( GetPreference ); /* register it */

LuaFunction_NoArgs( EventMode,		PREFSMAN->m_bEventMode )
LuaFunction_NoArgs( ShowCaution,	PREFSMAN->m_bShowCaution )


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

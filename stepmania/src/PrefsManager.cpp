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
	m_bMinimum1FullSongInNonstop	( Options, "Minimum1FullSongInNonstop",		false ),	// FEoS for 1st song, FailImmediate thereafter
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

	m_AutoPlay					( Options, "AutoPlay",					PC_HUMAN ),
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
	m_sDefaultLocalProfileIDP1		( Options, "DefaultLocalProfileIDP1",	"" ),
	m_sDefaultLocalProfileIDP2		( Options, "DefaultLocalProfileIDP2",	"" ),
	m_bMemoryCards					( Options, "MemoryCards",				false ),
	m_sMemoryCardOsMountPointP1		( Options, "MemoryCardOsMountPointP1",	"" ),
	m_sMemoryCardOsMountPointP2		( Options, "MemoryCardOsMountPointP2",	"" ),
	m_iMemoryCardUsbBusP1			( Options, "MemoryCardUsbBusP1",	-1 ),
	m_iMemoryCardUsbBusP2			( Options, "MemoryCardUsbBusP2",	-1 ),
	m_iMemoryCardUsbPortP1			( Options, "MemoryCardUsbPortP1",	-1 ),
	m_iMemoryCardUsbPortP2			( Options, "MemoryCardUsbPortP2",	-1 ),
	m_iMemoryCardUsbLevelP1			( Options, "MemoryCardUsbLevelP1",	-1 ),
	m_iMemoryCardUsbLevelP2			( Options, "MemoryCardUsbLevelP2",	-1 ),
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
	m_bCelShadeModels						( Options, "CelShadeModels",						false ),	// Work-In-Progress.. disable by default.
	m_bPreferredSortUsesGroups				( Options, "PreferredSortUsesGroups",				true ),

	m_fConstantUpdateDeltaSeconds	( Options, "ConstantUpdateDeltaSeconds",0 ),
	m_fPadStickSeconds				( Options, "PadStickSeconds",			0 ),
	m_bForceMipMaps					( Options, "ForceMipMaps",				0 ),
	m_bTrilinearFiltering			( Options, "TrilinearFiltering",		0 ),
	m_bAnisotropicFiltering			( Options, "AnisotropicFiltering",		0 ),

	m_bSignProfileData				( Options, "SignProfileData",			false ),
	m_bEditorShowBGChangesPlay		( Options, "EditorShowBGChangesPlay",	false ),
	m_CourseSortOrder				( Options, "CourseSortOrder",			COURSE_SORT_SONGS ),
	m_bMoveRandomToEnd				( Options, "MoveRandomToEnd",			false ),
	m_bSubSortByNumSteps			( Options, "SubSortByNumSteps",			false ),
	m_GetRankingName				( Options, "GetRankingName",			RANKING_ON ),
	m_ScoringType					( Options, "ScoringType",				SCORING_MAX2 ),
	m_BoostAppPriority				( Options, "BoostAppPriority",			BOOST_AUTO ),
	m_sAdditionalSongFolders		( Options, "AdditionalSongFolders",		"" ),
	m_sAdditionalFolders			( Options, "AdditionalFolders",			"" ),
	m_sLastSeenVideoDriver			( Options, "LastSeenVideoDriver",		"" ),
	m_sLastSeenInputDevices			( Options, "LastSeenInputDevices",		"" ),
#if defined(WIN32)
	m_iLastSeenMemory				( Options, "LastSeenMemory",			0 ),
#endif
	m_sVideoRenderers				( Options, "VideoRenderers",			"" ),	// StepMania.cpp sets these on first run:
	m_bSmoothLines					( Options, "SmoothLines",				false ),
	m_sSoundDrivers					( Options, "SoundDrivers",				"" ),
	m_iSoundWriteAhead				( Options, "SoundWriteAhead",			0 ),
	m_iSoundDevice					( Options, "SoundDevice",				"" ),
	m_fSoundVolume					( Options, "SoundVolume",				-1 ),	// default
	m_SoundResampleQuality			( Options, "SoundResampleQuality",		RageSoundReader_Resample::RESAMP_NORMAL ),
	m_sInputDrivers					( Options, "InputDrivers",				"" ),
	m_sMovieDrivers					( Options, "MovieDrivers",				"" ),
	m_sLightsDriver					( Options, "LightsDriver",				"" ),
	m_sLightsStepsDifficulty		( Options, "LightsStepsDifficulty",		"medium" ),
	m_bBlinkGameplayButtonLightsOnNote	( Options, "BlinkGameplayButtonLightsOnNote",false ),
	m_bAllowUnacceleratedRenderer	( Options, "AllowUnacceleratedRenderer",false ),
	m_bThreadedInput				( Options, "ThreadedInput",				true ),
	m_bThreadedMovieDecode			( Options, "ThreadedMovieDecode",		true ),
	m_bScreenTestMode				( Options, "ScreenTestMode",			false ),
	m_bDebugLights					( Options, "DebugLights",				false ),
	m_bMonkeyInput					( Options, "MonkeyInput",				false ),
	m_sMachineName					( Options, "MachineName",				"" ),
	m_sIgnoredMessageWindows		( Options, "IgnoredMessageWindows",		"" ),
	m_sCoursesToShowRanking			( Options, "CoursesToShowRanking",		"" ),

	/* Debug: */
	m_bLogToDisk					( Options, "LogToDisk",					true ),
	m_bForceLogFlush				( Options, "ForceLogFlush",				false ),
	m_bShowLogOutput				( Options, "ShowLogOutput",				false ),
	m_bTimestamping					( Options, "Timestamping",				false ),
	m_bLogSkips						( Options, "LogSkips",					false ),
	m_bLogCheckpoints				( Options, "LogCheckpoints",			false ),
	m_bShowLoadingWindow			( Options, "ShowLoadingWindow",			true ),

	/* Game-specific prefs: */
	m_sDefaultModifiers				( Options, "DefaultModifiers",			"" )

#if defined(XBOX)
	,
	m_bEnableVirtualMemory		( Options, "EnableVirtualMemory",			true ),
	m_iPageFileSize				( Options, "PageFileSize",					384 ),
	m_iPageSize					( Options, "PageSize",						16 ),
	m_iPageThreshold			( Options, "PageThreshold",					8 ),
	m_bLogVirtualMemory 		( Options, "LogVirtualMemory",				false )
#endif
{
	Init();
	ReadGlobalPrefsFromDisk();
}

void PrefsManager::Init()
{
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
	m_bFirstRun.Set( false );
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
	FOREACHS_CONST( IPreference*, *SubscriptionManager<IPreference>::s_pSubscribers, p )
		(*p)->ReadFrom( ini );

	// validate
	m_iSongsPerPlay.Set( clamp(m_iSongsPerPlay.Get(),0,MAX_SONGS_PER_PLAY) );
	FOREACH_PlayerNumber( pn )
		GetMemoryCardOsMountPoint(pn).Set( FixSlashes(GetMemoryCardOsMountPoint(pn)) );
}

void PrefsManager::SaveGlobalPrefsToDisk() const
{
	IniFile ini;
	SaveGlobalPrefsToIni( ini );
	ini.WriteFile( STEPMANIA_INI_PATH );
}

void PrefsManager::SaveGlobalPrefsToIni( IniFile &ini ) const
{
	FOREACHS_CONST( IPreference*, *SubscriptionManager<IPreference>::s_pSubscribers, p )
		(*p)->WriteTo( ini );
}

// wrappers
CString PrefsManager::GetSoundDrivers()	
{
	if ( m_sSoundDrivers.Get().empty() )
		return (CString)DEFAULT_SOUND_DRIVER_LIST;
	else
		return m_sSoundDrivers;
}

float PrefsManager::GetSoundVolume()
{
	if ( m_fSoundVolume==-1 )
		return DEFAULT_SOUND_VOLUME;
	else
		return m_fSoundVolume; 
}

CString PrefsManager::GetInputDrivers()	{
	if( m_sInputDrivers.Get().empty() )
		return (CString)DEFAULT_INPUT_DRIVER_LIST;
	else
		return m_sInputDrivers;
}

CString PrefsManager::GetMovieDrivers()
{ 
	if ( m_sMovieDrivers.Get().empty() )
		return (CString)DEFAULT_MOVIE_DRIVER_LIST;
	else
		return m_sMovieDrivers; 
}

CString PrefsManager::GetLightsDriver()	{ 
	if ( m_sLightsDriver.Get().empty() )
		return (CString)DEFAULT_LIGHTS_DRIVER;
	else
		return m_sLightsDriver;
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

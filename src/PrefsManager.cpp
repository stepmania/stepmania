#include "global.h"
#include "PrefsManager.h"
#include "IniFile.h"
#include "LuaManager.h"
#include "Preference.h"
#include "ProductInfo.h"
#include "RageDisplay.h"
#include "RageFile.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "SpecialFiles.h"

#if !defined(WITHOUT_NETWORKING)
#include "ver.h"
#endif

//DEFAULTS_INI_PATH	= "Data/Defaults.ini";		// these can be overridden
//PREFERENCES_INI_PATH	// overlay on Defaults.ini, contains the user's choices
//STATIC_INI_PATH	= "Data/Static.ini";		// overlay on the 2 above, can't be overridden
//TYPE_TXT_FILE	= "Data/Type.txt";

PrefsManager*	PREFSMAN = nullptr;	// global and accessible from anywhere in our program

static const char *MusicWheelUsesSectionsNames[] = {
	"Never",
	"Always",
	"ABCOnly",
};
XToString( MusicWheelUsesSections );
StringToX( MusicWheelUsesSections );
LuaXType( MusicWheelUsesSections );

static const char *AllowW1Names[] = {
	"Never",
	"CoursesOnly",
	"Everywhere",
};
XToString( AllowW1 );
StringToX( AllowW1 );
LuaXType( AllowW1 );

static const char *MaybeNames[] = {
	"Ask",
	"No",
	"Yes",
};
XToString( Maybe );
StringToX( Maybe );
LuaXType( Maybe );

static const char *GetRankingNameNames[] = {
	"Off",
	"On",
	"List",
};
XToString( GetRankingName );
StringToX( GetRankingName );
LuaXType( GetRankingName );


static const char *RandomBackgroundModeNames[] = {
	"Off",
	"Animations",
	"RandomMovies",
};
XToString( RandomBackgroundMode );
StringToX( RandomBackgroundMode );
LuaXType( RandomBackgroundMode );

static const char *ShowDancingCharactersNames[] = {
	"Off",
	"Random",
	"Select",
};
XToString( ShowDancingCharacters );
StringToX( ShowDancingCharacters );
LuaXType( ShowDancingCharacters );

static const char *ImageCacheModeNames[] = {
	"Off",
	"LowResPreload",
	"LowResLoadOnDemand",
	"Full"
};
XToString( ImageCacheMode );
StringToX( ImageCacheMode );
LuaXType( ImageCacheMode );

static const char *HighResolutionTexturesNames[] = {
	"Auto",
	"ForceOff",
	"ForceOn",
};
XToString( HighResolutionTextures );
StringToX( HighResolutionTextures );
LuaXType( HighResolutionTextures );

static const char *AttractSoundFrequencyNames[] = {
	"Never",
	"EveryTime",
	"Every2Times",
	"Every3Times",
	"Every4Times",
	"Every5Times",
};
XToString( AttractSoundFrequency );
StringToX( AttractSoundFrequency );
LuaXType( AttractSoundFrequency );

static const char *CourseSortOrdersNames[] = {
	"Preferred",
	"Songs",
	"Meter",
	"MeterSum",
	"MeterRank",
};
XToString( CourseSortOrders );
StringToX( CourseSortOrders );
LuaXType( CourseSortOrders );

static const char *BackgroundFitModeNames[] = {
	"CoverDistort",
	"CoverPreserve",
	"FitInside",
	"FitInsideAvoidLetter",
	"FitInsideAvoidPillar",
};
XToString( BackgroundFitMode );
StringToX( BackgroundFitMode );
LuaXType( BackgroundFitMode );

bool g_bAutoRestart = false;
#ifdef DEBUG
# define TRUE_IF_DEBUG true
#else
# define TRUE_IF_DEBUG false
#endif

void ValidateDisplayAspectRatio( float &val )
{
	if( val < 0 )
		val = 16/9.f;
}

void ValidateSongsPerPlay( int &val )
{
	CLAMP(val,0,MAX_SONGS_PER_PLAY);
}

PrefsManager::PrefsManager() :
	m_sCurrentGame			( "CurrentGame",		"" ),

	m_sAnnouncer			( "Announcer",			"" ),
	m_sTheme			( "Theme",			SpecialFiles::BASE_THEME_NAME ),
	m_sDefaultModifiers		( "DefaultModifiers",		"" ),

	m_bWindowed			( "Windowed",			true ),
	m_sDisplayId			( "DisplayId", "" ),
	m_iDisplayWidth			( "DisplayWidth",		854 ),
	m_iDisplayHeight		( "DisplayHeight",		480 ),
	m_fDisplayAspectRatio		( "DisplayAspectRatio",		16/9.f, ValidateDisplayAspectRatio ),
	m_iDisplayColorDepth		( "DisplayColorDepth",		32 ),
	m_iTextureColorDepth		( "TextureColorDepth",		32 ),
	m_iMovieColorDepth		( "MovieColorDepth",		32 ),
	m_bStretchBackgrounds		( "StretchBackgrounds",		false ),
	m_BGFitMode			( "BackgroundFitMode",		BFM_CoverPreserve),
	m_HighResolutionTextures	( "HighResolutionTextures",	HighResolutionTextures_Auto ),
	m_iMaxTextureResolution		( "MaxTextureResolution",	2048 ),
	m_iRefreshRate			( "RefreshRate",		REFRESH_DEFAULT ),
	m_bAllowMultitexture		( "AllowMultitexture",		true ),
	m_bFullscreenIsBorderlessWindow( "FullscreenIsBorderlessWindow", true ),
	m_bShowStats			( "ShowStats",			TRUE_IF_DEBUG),
	m_bShowBanners			( "ShowBanners",		true ),
	m_bShowMouseCursor		( "ShowMouseCursor",		false ),

	m_bHiddenSongs			( "HiddenSongs",		false ),
	m_bVsync			( "Vsync",			true ),
	m_FastNoteRendering		( "FastNoteRendering",		false),
	m_bInterlaced			( "Interlaced",			false ),
	m_bPAL				( "PAL",			false ),
	m_bDelayedTextureDelete		( "DelayedTextureDelete",	false ),
	m_bDelayedModelDelete		( "DelayedModelDelete",		false ),
	m_ImageCache			( "ImageCache",		IMGCACHE_LOW_RES_PRELOAD ),
	m_bFastLoad			( "FastLoad",			true ),
	m_bFastLoadAdditionalSongs      ( "FastLoadAdditionalSongs",    true ),
	m_NeverCacheList		( "NeverCacheList", ""),

	m_bOnlyDedicatedMenuButtons	( "OnlyDedicatedMenuButtons",	false ),
	m_bMenuTimer			( "MenuTimer",			false ),

	m_fLifeDifficultyScale		( "LifeDifficultyScale",	1.0f ),

	m_bRateModsAffectTweens		( "RateModsAffectFGChanges",	false ),

	m_iRegenComboAfterMiss		( "RegenComboAfterMiss",	5 ),
	m_iMaxRegenComboAfterMiss	( "MaxRegenComboAfterMiss",	5 ), // this was 10 by default in SM3.95 -dguzek
	m_bMercifulDrain		( "MercifulDrain",		false ),	// negative life deltas are scaled by the players life percentage
	m_HarshHotLifePenalty		( "HarshHotLifePenalty",	true ),
	m_bMinimum1FullSongInCourses	( "Minimum1FullSongInCourses",	false ),	// FEoS for 1st song, FailImmediate thereafter
	m_bFailOffInBeginner		( "FailOffInBeginner",		false ),
	m_bFailOffForFirstStageEasy	( "FailOffForFirstStageEasy",	false ),
	m_bMercifulBeginner		( "MercifulBeginner",		false ),
	m_bMercifulSuperMeter		( "MercifulSuperMeter",		true ),
	m_bDelayedBack			( "DelayedBack",		true ),
	m_AllowHoldForOptions("AllowHoldForOptions", true),
	m_bShowInstructions		( "ShowInstructions",		true ),
	m_bShowCaution			( "ShowCaution",		true ),
	m_bShowNativeLanguage		( "ShowNativeLanguage",		true ),
	m_iArcadeOptionsNavigation	( "ArcadeOptionsNavigation",	0 ),
	m_ThreeKeyNavigation		( "ThreeKeyNavigation",		false ),
	m_MusicWheelUsesSections	( "MusicWheelUsesSections",	MusicWheelUsesSections_ALWAYS ),
	m_iMusicWheelSwitchSpeed	( "MusicWheelSwitchSpeed",	15 ),
	m_AllowW1			( "AllowW1",			ALLOW_W1_EVERYWHERE ),
	m_bEventMode			( "EventMode",			true ),
	m_iCoinsPerCredit		( "CoinsPerCredit",		1 ),
	m_iSongsPerPlay			( "SongsPerPlay",		3, ValidateSongsPerPlay ),
	m_bDelayedCreditsReconcile	( "DelayedCreditsReconcile",	false ),
	m_bComboContinuesBetweenSongs	( "ComboContinuesBetweenSongs",	false ),
	m_AllowMultipleToasties		("AllowMultipleToasties",	true ),
	m_MinTNSToHideNotes		("MinTNSToHideNotes",		TNS_W3 ),
	m_ShowSongOptions		( "ShowSongOptions",		Maybe_NO ),
	m_bPercentageScoring		( "PercentageScoring",		false ),
	// Wow, these preference names are *seriously* long -Colby
	m_fMinPercentageForMachineSongHighScore		( "MinPercentageForMachineSongHighScore",	0.0001f ), // This is for home, who cares how bad you do?
	m_fMinPercentageForMachineCourseHighScore	( "MinPercentageForMachineCourseHighScore",	0.0001f ), // don't save course scores with 0 percentage
	m_bDisqualification		( "Disqualification",			false ),
	m_bAutogenSteps			( "AutogenSteps",			false ),
	m_bAutogenGroupCourses		( "AutogenGroupCourses",		true ),
	m_bOnlyPreferredDifficulties	( "OnlyPreferredDifficulties",		false ),
	m_bBreakComboToGetItem		( "BreakComboToGetItem",		false ),
	m_bLockCourseDifficulties	( "LockCourseDifficulties",		true ),
	m_ShowDancingCharacters		( "ShowDancingCharacters",		SDC_Random ),
	m_bUseUnlockSystem		( "UseUnlockSystem",			false ),
	m_fGlobalOffsetSeconds		( "GlobalOffsetSeconds",		-0.008f ),
	m_iProgressiveLifebar		( "ProgressiveLifebar",			0 ),
	m_iProgressiveStageLifebar	( "ProgressiveStageLifebar",		0 ),
	m_iProgressiveNonstopLifebar	( "ProgressiveNonstopLifebar",		0 ),
	m_bShowBeginnerHelper		( "ShowBeginnerHelper",			false ),
	m_bDisableScreenSaver		( "DisableScreenSaver",			true ),
	m_sLanguage			( "Language",				"" ),	// ThemeManager will deal with this invalid language
	m_sMemoryCardProfileSubdir	( "MemoryCardProfileSubdir",		PRODUCT_ID ),
	m_iProductID			( "ProductID",				1 ),
	m_iCenterImageTranslateX	( "CenterImageTranslateX",		0 ),
	m_iCenterImageTranslateY	( "CenterImageTranslateY",		0 ),
	m_fCenterImageAddWidth		( "CenterImageAddWidth",		0 ),
	m_fCenterImageAddHeight		( "CenterImageAddHeight",		0 ),
	m_AttractSoundFrequency		( "AttractSoundFrequency",		ASF_EVERY_TIME ),
	m_bAllowExtraStage		( "AllowExtraStage",			false ),
	m_iMaxHighScoresPerListForMachine	( "MaxHighScoresPerListForMachine",	10 ),
	m_iMaxHighScoresPerListForPlayer	( "MaxHighScoresPerListForPlayer",	3 ),
	m_bAllowMultipleHighScoreWithSameName	( "AllowMultipleHighScoreWithSameName",	true ),
	m_DisableUploadDir("DisableUploadDir", false),
	m_bCelShadeModels		( "CelShadeModels",			false ),	// Work-In-Progress.. disable by default.
	m_bPreferredSortUsesGroups	( "PreferredSortUsesGroups",		true ),
	m_fDebounceCoinInputTime	( "DebounceCoinInputTime",		0 ),

	m_fPadStickSeconds		( "PadStickSeconds",			0 ),
	m_EditRecordModeLeadIn		("EditRecordModeLeadIn",		1.0f ),
	m_EditClearPromptThreshold	("EditClearPromptThreshold",		50),
	m_bForceMipMaps			( "ForceMipMaps",			false ),
	m_bTrilinearFiltering		( "TrilinearFiltering",			false ),
	m_bAnisotropicFiltering		( "AnisotropicFiltering",		false ),

	m_bSignProfileData		( "SignProfileData",			false ),
	m_CourseSortOrder		( "CourseSortOrder",			COURSE_SORT_SONGS ),
	m_bSubSortByNumSteps		( "SubSortByNumSteps",			false ),
	m_GetRankingName		( "GetRankingName",			RANKING_ON ),
	m_sAdditionalSongFolders	( "AdditionalSongFolders",		"" ),
	m_sAdditionalCourseFolders	( "AdditionalCourseFolders",		"" ),
	m_sAdditionalFolders		( "AdditionalFolders",			"" ),
	m_sDefaultTheme			( "DefaultTheme",			"default" ),
	m_sLastSeenVideoDriver		( "LastSeenVideoDriver",		"" ),
	m_sVideoRenderers		( "VideoRenderers",			"" ),	// StepMania.cpp sets these on first run:
	m_bSmoothLines			( "SmoothLines",			false ),
	m_iSoundWriteAhead		( "SoundWriteAhead",			0 ),
	m_iSoundDevice			( "SoundDevice",			"" ),
	m_iRageSoundSampleCountClamp	("RageSoundSampleCountClamp", 0), //some sound drivers mask the sample location number, the most popular number for this is 2^27, this causes lockup after ~50 minutes at 44.1khz sample rate
	m_iSoundPreferredSampleRate	( "SoundPreferredSampleRate",		0 ),
	m_sLightsStepsDifficulty	( "LightsStepsDifficulty",		"hard,medium" ),
	m_bAllowUnacceleratedRenderer	( "AllowUnacceleratedRenderer",		false ),
	m_bThreadedInput		( "ThreadedInput",			true ),
	m_bThreadedMovieDecode		( "ThreadedMovieDecode",		true ),
	m_sTestInitialScreen		( "TestInitialScreen",			"" ),
	m_bDebugLights			( "DebugLights",			false ),
	m_bMonkeyInput			( "MonkeyInput",			false ),
	m_sMachineName			( "MachineName",			"" ),
	m_sCoursesToShowRanking		( "CoursesToShowRanking",		"" ),
	m_MuteActions			( "MuteActions",			false ),
	m_bAllowSongDeletion		( "AllowSongDeletion",			false ),

	m_bQuirksMode			( "QuirksMode",		false ),

	m_custom_songs_enable("CustomSongsEnable", false),
	m_custom_songs_max_count("CustomSongsMaxCount", 1000), // No limit. -- 2 Unlimited
	m_custom_songs_load_timeout("CustomSongsLoadTimeout", 5.f),
	m_custom_songs_max_seconds("CustomSongsMaxSeconds", 120.f),
	m_custom_songs_max_megabytes("CustomSongsMaxMegabytes", 5.f),

	/* Debug: */
	m_bLogToDisk			( "LogToDisk",		true ),
#if defined(DEBUG)
	m_bForceLogFlush		( "ForceLogFlush",	true ),
	m_bShowLogOutput		( "ShowLogOutput",	true ),
#else
	m_bForceLogFlush		( "ForceLogFlush",	false ),
	m_bShowLogOutput		( "ShowLogOutput",	false ),
#endif
	m_bLogSkips			( "LogSkips",		false ),
	m_bLogCheckpoints		( "LogCheckpoints",	false ),
	m_bShowLoadingWindow		( "ShowLoadingWindow",	true ),
	m_bPseudoLocalize		( "PseudoLocalize",	false ),
	m_show_theme_errors("ShowThemeErrors", false)

#if !defined(WITHOUT_NETWORKING)
	,
	m_bEnableScoreboard		( "EnableScoreboard",	true )
	,
	m_bUpdateCheckEnable			( "UpdateCheckEnable",				true )
	// TODO - Aldo_MX: Use PREFSMAN->m_iUpdateCheckIntervalSeconds & PREFSMAN->m_iUpdateCheckLastCheckedSecond
	//,
	//m_iUpdateCheckIntervalSeconds	( "UpdateCheckIntervalSeconds",		86400 ),	// 24 hours
	//m_iUpdateCheckLastCheckedSecond	( "UpdateCheckLastCheckSecond",		0 )
	// TODO - Aldo_MX: Write helpers in LuaManager.cpp to treat unsigned int/long like LUA Numbers
	//,
	//m_uUpdateCheckLastCheckedBuild	( "UpdateCheckLastCheckedBuild",	version_num )
#endif

{
	Init();
	ReadPrefsFromDisk();

	// Register with Lua.
	{
		Lua *L = LUA->Get();
		lua_pushstring( L, "PREFSMAN" );
		this->PushSelf( L );
		lua_settable( L, LUA_GLOBALSINDEX );
		LUA->Release( L );
	}
}
#undef TRUE_IF_DEBUG

void PrefsManager::Init()
{
	IPreference::LoadAllDefaults();

	m_mapGameNameToGamePrefs.clear();
}

PrefsManager::~PrefsManager()
{
	// Unregister with Lua.
	LUA->UnsetGlobal( "PREFSMAN" );
}

void PrefsManager::SetCurrentGame( const RString &sGame )
{
	if( m_sCurrentGame.Get() == sGame )
		return;	// redundant

	if( !m_sCurrentGame.Get().empty() )
		StoreGamePrefs();

	m_sCurrentGame.Set( sGame );

	RestoreGamePrefs();
}

void PrefsManager::StoreGamePrefs()
{
	ASSERT( !m_sCurrentGame.Get().empty() );

	// save off old values
	GamePrefs &gp = m_mapGameNameToGamePrefs[m_sCurrentGame.Get()];
	gp.m_sAnnouncer = m_sAnnouncer;
	gp.m_sTheme = m_sTheme;
	gp.m_sDefaultModifiers = m_sDefaultModifiers;
}

void PrefsManager::RestoreGamePrefs()
{
	ASSERT( !m_sCurrentGame.Get().empty() );

	// load prefs
	GamePrefs gp;
	map<RString, GamePrefs>::const_iterator iter = m_mapGameNameToGamePrefs.find( m_sCurrentGame );
	if( iter != m_mapGameNameToGamePrefs.end() )
		gp = iter->second;

	m_sAnnouncer		.Set( gp.m_sAnnouncer );
	m_sTheme			.Set( gp.m_sTheme );
	m_sDefaultModifiers	.Set( gp.m_sDefaultModifiers );

	// give Static.ini a chance to clobber the saved game prefs
	ReadPrefsFromFile( SpecialFiles::STATIC_INI_PATH, GetPreferencesSection(), true );
}

PrefsManager::GamePrefs::GamePrefs() : m_sAnnouncer(""), m_sTheme(SpecialFiles::BASE_THEME_NAME), m_sDefaultModifiers("") {}

void PrefsManager::ReadPrefsFromDisk()
{
	ReadDefaultsFromFile( SpecialFiles::DEFAULTS_INI_PATH, GetPreferencesSection() );
	IPreference::LoadAllDefaults();

	ReadPrefsFromFile( SpecialFiles::PREFERENCES_INI_PATH, "Options", false );
	ReadGamePrefsFromIni( SpecialFiles::PREFERENCES_INI_PATH );
	ReadPrefsFromFile( SpecialFiles::STATIC_INI_PATH, GetPreferencesSection(), true );

	if( !m_sCurrentGame.Get().empty() )
		RestoreGamePrefs();
}

void PrefsManager::ResetToFactoryDefaults()
{
	// clobber the users prefs by initing then applying defaults
	Init();
	IPreference::LoadAllDefaults();
	ReadPrefsFromFile( SpecialFiles::STATIC_INI_PATH, GetPreferencesSection(), true );

	SavePrefsToDisk();
}

void PrefsManager::ReadPrefsFromFile( const RString &sIni, const RString &sSection, bool bIsStatic )
{
	IniFile ini;
	if( !ini.ReadFile(sIni) )
		return;

	ReadPrefsFromIni( ini, sSection, bIsStatic );
}

static const RString GAME_SECTION_PREFIX = "Game-";

void PrefsManager::ReadPrefsFromIni( const IniFile &ini, const RString &sSection, bool bIsStatic )
{
	// Apply our fallback recursively (if any) before applying ourself.
	static int s_iDepth = 0;
	s_iDepth++;
	ASSERT( s_iDepth < 100 );
	RString sFallback;
	if( ini.GetValue(sSection,"Fallback",sFallback) )
	{
		ReadPrefsFromIni( ini, sFallback, bIsStatic );
	}
	s_iDepth--;

	/*
	IPreference *pPref = PREFSMAN->GetPreferenceByName( *sName );
	if( pPref == nullptr )
	{
		LOG->Warn( "Unknown preference in [%s]: %s", sClassName.c_str(), sName->c_str() );
		continue;
	}
	pPref->FromString( sVal );
	*/

	const XNode *pChild = ini.GetChild(sSection);
	if( pChild )
		IPreference::ReadAllPrefsFromNode( pChild, bIsStatic );
}

void PrefsManager::ReadGamePrefsFromIni( const RString &sIni )
{
	IniFile ini;
	if( !ini.ReadFile(sIni) )
		return;

	FOREACH_CONST_Child( &ini, section )
	{
		RString section_name= section->GetName();
		if( !BeginsWith(section_name, GAME_SECTION_PREFIX) )
			continue;

		RString sGame = section_name.Right( section_name.length() - GAME_SECTION_PREFIX.length() );
		GamePrefs &gp = m_mapGameNameToGamePrefs[ sGame ];

		// todo: read more prefs here? -aj
		ini.GetValue(section_name, "Announcer",		gp.m_sAnnouncer);
		ini.GetValue(section_name, "Theme",			gp.m_sTheme);
		ini.GetValue(section_name, "DefaultModifiers",	gp.m_sDefaultModifiers);
	}
}

void PrefsManager::ReadDefaultsFromFile( const RString &sIni, const RString &sSection )
{
	IniFile ini;
	if( !ini.ReadFile(sIni) )
		return;

	ReadDefaultsFromIni( ini, sSection );
}

void PrefsManager::ReadDefaultsFromIni( const IniFile &ini, const RString &sSection )
{
	// Apply our fallback recursively (if any) before applying ourself.
	// TODO: detect circular?
	RString sFallback;
	if( ini.GetValue(sSection,"Fallback",sFallback) )
		ReadDefaultsFromIni( ini, sFallback );

	IPreference::ReadAllDefaultsFromNode( ini.GetChild(sSection) );
}

void PrefsManager::SavePrefsToDisk()
{
	IniFile ini;
	SavePrefsToIni( ini );
	ini.WriteFile( SpecialFiles::PREFERENCES_INI_PATH );
}

void PrefsManager::SavePrefsToIni( IniFile &ini )
{
	if( !m_sCurrentGame.Get().empty() )
		StoreGamePrefs();

	XNode* pNode = ini.GetChild( "Options" );
	if( pNode == nullptr )
		pNode = ini.AppendChild( "Options" );
	IPreference::SavePrefsToNode( pNode );

	for (auto const &iter : m_mapGameNameToGamePrefs)
	{
		RString sSection = "Game-" + RString( iter.first );

		// todo: write more values here? -aj
		ini.SetValue( sSection, "Announcer",		iter.second.m_sAnnouncer );
		ini.SetValue( sSection, "Theme",		iter.second.m_sTheme );
		ini.SetValue( sSection, "DefaultModifiers",	iter.second.m_sDefaultModifiers );
	}
}


RString PrefsManager::GetPreferencesSection() const
{
	RString sSection = "Options";

	// OK if this fails
	GetFileContents( SpecialFiles::TYPE_TXT_FILE, sSection, true );

	// OK if this fails
	GetCommandlineArgument( "Type", &sSection );

	return sSection;
}


// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the PrefsManager. */
class LunaPrefsManager: public Luna<PrefsManager>
{
public:
	static int GetPreference( T* p, lua_State *L )
	{
		RString sName = SArg(1);
		IPreference *pPref = IPreference::GetPreferenceByName( sName );
		if( pPref == nullptr )
		{
			LuaHelpers::ReportScriptErrorFmt( "GetPreference: unknown preference \"%s\"", sName.c_str() );
			lua_pushnil( L );
			return 1;
		}

		pPref->PushValue( L );
		return 1;
	}
	static int SetPreference( T* p, lua_State *L )
	{
		RString sName = SArg(1);

		IPreference *pPref = IPreference::GetPreferenceByName( sName );
		if( pPref == nullptr )
		{
			LuaHelpers::ReportScriptErrorFmt( "SetPreference: unknown preference \"%s\"", sName.c_str() );
			COMMON_RETURN_SELF;
		}

		lua_pushvalue( L, 2 );
		pPref->SetFromStack( L );
		COMMON_RETURN_SELF;
	}
	static int SetPreferenceToDefault( T* p, lua_State *L )
	{
		RString sName = SArg(1);

		IPreference *pPref = IPreference::GetPreferenceByName( sName );
		if( pPref == nullptr )
		{
			LuaHelpers::ReportScriptErrorFmt( "SetPreferenceToDefault: unknown preference \"%s\"", sName.c_str() );
			COMMON_RETURN_SELF;
		}

		pPref->LoadDefault();
		LOG->Trace( "Restored preference \"%s\" to default \"%s\"", sName.c_str(), pPref->ToString().c_str() );
		COMMON_RETURN_SELF;
	}
	static int PreferenceExists( T* p, lua_State *L )
	{
		RString sName = SArg(1);

		IPreference *pPref = IPreference::GetPreferenceByName( sName );
		if( pPref == nullptr )
		{
			lua_pushboolean( L, false );
			return 1;
		}
		lua_pushboolean( L, true );
		return 1;
	}

	static int SavePreferences( T* p, lua_State *L ) { p->SavePrefsToDisk(); COMMON_RETURN_SELF; }

	LunaPrefsManager()
	{
		ADD_METHOD( GetPreference );
		ADD_METHOD( SetPreference );
		ADD_METHOD( SetPreferenceToDefault );
		ADD_METHOD( PreferenceExists );
		ADD_METHOD( SavePreferences );
	}
};

LUA_REGISTER_CLASS( PrefsManager )
// lua end

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

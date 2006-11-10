#include "global.h"
#include "PrefsManager.h"
#include "IniFile.h"
#include "RageDisplay.h"
#include "RageUtil.h"
#include "RageFile.h"
#include "ProductInfo.h"
#include "Foreach.h"
#include "Preference.h"
#include "RageLog.h"
#include "SpecialFiles.h"
#include "LuaManager.h"

//DEFAULTS_INI_PATH	= "Data/Defaults.ini";		// these can be overridden
//PREFERENCES_INI_PATH	// overlay on Defaults.ini, contains the user's choices
//STATIC_INI_PATH	= "Data/Static.ini";		// overlay on the 2 above, can't be overridden
//TYPE_TXT_FILE	= "Data/Type.txt";

PrefsManager*	PREFSMAN = NULL;	// global and accessable from anywhere in our program

static const char *TimingWindowNames[] = {
	"W1",
	"W2",
	"W3",
	"W4",
	"W5",
	"Mine",
	"Attack",
	"Hold",
	"Roll",
};
XToString( TimingWindow );


static const char *ScoreEventNames[] = {
	"W1",
	"W2",
	"W3",
	"W4",
	"W5",
	"Miss",
	"HitMine",
	"Held",
	"LetGo",
};
XToString( ScoreEvent );


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

static const char *BannerCacheModeNames[] = {
	"Off",
	"LowResPreload",
	"LowResLoadOnDemand",
	"Full"
};
XToString( BannerCacheMode );
StringToX( BannerCacheMode );
LuaXType( BannerCacheMode );

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

static const char *ScoringTypeNames[] = {
	"New",
	"Old",
};
XToString( ScoringType );
StringToX( ScoringType );
LuaXType( ScoringType );

bool g_bAutoRestart = false;
#ifdef DEBUG
# define TRUE_IF_DEBUG true
#else
# define TRUE_IF_DEBUG false
#endif

void TimingWindowSecondsInit( size_t /*TimingWindow*/ i, RString &sNameOut, float &defaultValueOut )
{
	sNameOut = "TimingWindowSeconds" + TimingWindowToString( (TimingWindow)i );
	switch( i )
	{
	default:	ASSERT(0);
	case TW_W1:	defaultValueOut = 0.0225f;	break;
	case TW_W2:	defaultValueOut = 0.045f;	break;
	case TW_W3:	defaultValueOut = 0.090f;	break;
	case TW_W4:	defaultValueOut = 0.135f;	break;
	case TW_W5:	defaultValueOut = 0.180f;	break;
	case TW_Mine:	defaultValueOut = 0.090f;	break;	// same as great
	case TW_Hold:	defaultValueOut = 0.250f;	break;	// allow enough time to take foot off and put back on
	case TW_Roll:	defaultValueOut = 0.350f;	break;
	case TW_Attack:	defaultValueOut = 0.135f;	break;
	}
}

void PercentScoreWeightInit( size_t /*ScoreEvent*/ i, RString &sNameOut, int &defaultValueOut )
{
	sNameOut = "PercentScoreWeight" + ScoreEventToString( (ScoreEvent)i );
	switch( i )
	{
	default:		ASSERT(0);
	case SE_W1:		defaultValueOut = 3;	break;
	case SE_W2:		defaultValueOut = 2;	break;
	case SE_W3:		defaultValueOut = 1;	break;
	case SE_W4:		defaultValueOut = 0;	break;
	case SE_W5:		defaultValueOut = 0;	break;
	case SE_Miss:		defaultValueOut = 0;	break;
	case SE_HitMine:	defaultValueOut = -2;	break;
	case SE_Held:		defaultValueOut = 3;	break;
	case SE_LetGo:		defaultValueOut = 0;	break;
	}
}

void GradeWeightInit( size_t /*ScoreEvent*/ i, RString &sNameOut, int &defaultValueOut )
{
	sNameOut = "GradeWeight" + ScoreEventToString( (ScoreEvent)i );
	switch( i )
	{
	default:		ASSERT(0);
	case SE_W1:		defaultValueOut = 2;	break;
	case SE_W2:		defaultValueOut = 2;	break;
	case SE_W3:		defaultValueOut = 1;	break;
	case SE_W4:		defaultValueOut = 0;	break;
	case SE_W5:		defaultValueOut = -4;	break;
	case SE_Miss:		defaultValueOut = -8;	break;
	case SE_HitMine:	defaultValueOut = -8;	break;
	case SE_Held:		defaultValueOut = 6;	break;
	case SE_LetGo:		defaultValueOut = 0;	break;
	}
}

void SuperMeterPercentChangeInit( size_t /*ScoreEvent*/ i, RString &sNameOut, float &defaultValueOut )
{
	sNameOut = "SuperMeterPercentChange" + ScoreEventToString( (ScoreEvent)i );
	switch( i )
	{
	default:		ASSERT(0);
	case SE_W1:		defaultValueOut = +0.05f;	break;
	case SE_W2:		defaultValueOut = +0.04f;	break;
	case SE_W3:		defaultValueOut = +0.02f;	break;
	case SE_W4:		defaultValueOut = +0.00f;	break;
	case SE_W5:		defaultValueOut = -0.00f;	break;
	case SE_Miss:		defaultValueOut = -0.20f;	break;
	case SE_HitMine:	defaultValueOut = -0.40f;	break;
	case SE_Held:		defaultValueOut = +0.04f;	break;
	case SE_LetGo:		defaultValueOut = -0.20f;	break;
	}
}

void TimeMeterSecondsChangeInit( size_t /*ScoreEvent*/ i, RString &sNameOut, float &defaultValueOut )
{
	sNameOut = "TimeMeterSecondsChange" + ScoreEventToString( (ScoreEvent)i );
	switch( i )
	{
	default:		ASSERT(0);
	case SE_W1:		defaultValueOut = +0.1f;	break;
	case SE_W2:		defaultValueOut = +0.0f;	break;
	case SE_W3:		defaultValueOut = -0.5f;	break;
	case SE_W4:		defaultValueOut = -1.0f;	break;
	case SE_W5:		defaultValueOut = -2.0f;	break;
	case SE_Miss:		defaultValueOut = -4.0f;	break;
	case SE_HitMine:	defaultValueOut = -2.0f;	break;
	case SE_Held:		defaultValueOut = -0.0f;	break;
	case SE_LetGo:		defaultValueOut = -4.0f;	break;
	}
}


void ValidateDisplayAspectRatio( float &val )
{
	if( val < 0 )
		val = 4/3.f;
}

void ValidateSongsPerPlay( int &val )
{
	CLAMP(val,0,MAX_SONGS_PER_PLAY);
}

PrefsManager::PrefsManager() :
	m_sCurrentGame		( "CurrentGame",		"" ),

	m_sAnnouncer		( "Announcer",			"" ),
	m_sTheme		( "Theme",			"" ),
	m_sDefaultModifiers	( "DefaultModifiers",		"" ),

	m_bWindowed		( "Windowed",			TRUE_IF_DEBUG ),
	m_iDisplayWidth		( "DisplayWidth",		640 ),
	m_iDisplayHeight	( "DisplayHeight",		480 ),
	m_fDisplayAspectRatio	( "DisplayAspectRatio",		4/3.f, ValidateDisplayAspectRatio ),
	m_iDisplayColorDepth	( "DisplayColorDepth",		16 ),
	m_iTextureColorDepth	( "TextureColorDepth",		16 ),
	m_iMovieColorDepth	( "MovieColorDepth",		16 ),
	m_iMaxTextureResolution	( "MaxTextureResolution",	2048 ),
	m_iRefreshRate		( "RefreshRate",		REFRESH_DEFAULT ),
	m_bAllowMultitexture	( "AllowMultitexture",		true ),
	m_bShowStats		( "ShowStats",			TRUE_IF_DEBUG),
	m_bShowBanners		( "ShowBanners",		true ),

	m_bSongBackgrounds	( "SongBackgrounds",		true ),
	m_RandomBackgroundMode	( "RandomBackgroundMode",	BGMODE_ANIMATIONS ),
	m_iNumBackgrounds	( "NumBackgrounds",		8 ),
	m_fBGBrightness		( "BGBrightness",		0.7f ),
	m_bHiddenSongs		( "HiddenSongs",		false ),
	m_bVsync		( "Vsync",			true ),
	m_bInterlaced		( "Interlaced",			false ),
	m_bPAL			( "PAL",			false ),
	m_bDelayedTextureDelete	( "DelayedTextureDelete",	true ),
	m_bDelayedModelDelete	( "DelayedModelDelete",		false ),
	m_BannerCache		( "BannerCache",		BNCACHE_LOW_RES_PRELOAD ),
	m_bPalettedBannerCache	( "PalettedBannerCache",	false ),
	m_bFastLoad		( "FastLoad",			false ),
	m_bFastLoadAdditionalSongs      ( "FastLoadAdditionalSongs",    false ),

	m_bOnlyDedicatedMenuButtons	( "OnlyDedicatedMenuButtons",	false ),
	m_bMenuTimer		( "MenuTimer",			true ),

	m_fTimingWindowScale	( "TimingWindowScale",		1.0f ),
	m_fTimingWindowAdd	( "TimingWindowAdd",		0 ),
	m_fTimingWindowSeconds	( TimingWindowSecondsInit,	NUM_TimingWindow ),

	m_fLifeDifficultyScale	( "LifeDifficultyScale",	1.0f ),


	m_iRegenComboAfterMiss		( "RegenComboAfterMiss",	5 ),
	m_bMercifulDrain		( "MercifulDrain",		true ),	// negative life deltas are scaled by the players life percentage
	m_bMinimum1FullSongInCourses	( "Minimum1FullSongInCourses",	false ),	// FEoS for 1st song, FailImmediate thereafter
	m_bFailOffInBeginner		( "FailOffInBeginner",		false ),
	m_bFailOffForFirstStageEasy	( "FailOffForFirstStageEasy",	false ),
	m_bMercifulBeginner		( "MercifulBeginner",		false ),

	m_iPercentScoreWeight		( PercentScoreWeightInit,	NUM_ScoreEvent ),
	
	m_iGradeWeight			( GradeWeightInit,		NUM_ScoreEvent ),

	m_fSuperMeterPercentChange	( SuperMeterPercentChangeInit,	NUM_ScoreEvent ),
	m_bMercifulSuperMeter		( "MercifulSuperMeter",		true ),
	
	m_fTimeMeterSecondsChange	( TimeMeterSecondsChangeInit,	NUM_ScoreEvent ),

	m_AutoPlay			( "AutoPlay",			PC_HUMAN ),
	m_bDelayedBack			( "DelayedBack",		true ),
	m_bShowInstructions		( "ShowInstructions",		true ),
	m_bShowSelectGroup		( "ShowSelectGroup",		false ),
	m_bShowCaution			( "ShowCaution",		true ),
	m_bShowNativeLanguage		( "ShowNativeLanguage",		true ),
	m_bArcadeOptionsNavigation	( "ArcadeOptionsNavigation",	false ),
	m_MusicWheelUsesSections	( "MusicWheelUsesSections",	MusicWheelUsesSections_ALWAYS ),
	m_iMusicWheelSwitchSpeed	( "MusicWheelSwitchSpeed",	10 ),
	m_AllowW1			( "AllowW1",			ALLOW_W1_EVERYWHERE ),
	m_bEventMode			( "EventMode",			false ),
	m_iCoinsPerCredit		( "CoinsPerCredit",		1 ),
	m_iSongsPerPlay			( "SongsPerPlay",		3, ValidateSongsPerPlay ),

	m_CoinMode			( "CoinMode",			COIN_MODE_HOME ),
	m_Premium			( "Premium",			PREMIUM_NONE ),
	m_bDelayedCreditsReconcile	( "DelayedCreditsReconcile",	false ),
	m_bPickExtraStage		( "PickExtraStage",		false ),

	m_bComboContinuesBetweenSongs	( "ComboContinuesBetweenSongs",	false ),
	m_ShowSongOptions		( "ShowSongOptions",		Maybe_YES ),
	m_bDancePointsForOni		( "DancePointsForOni",		false ),
	m_bPercentageScoring		( "PercentageScoring",		false ),
	m_fMinPercentageForMachineSongHighScore		( "MinPercentageForMachineSongHighScore",	0.5f ),
	m_fMinPercentageForMachineCourseHighScore	( "MinPercentageForMachineCourseHighScore",	0.0001f ),	// don't save course scores with 0 percentage
	m_bDisqualification		( "Disqualification",			false ),
	m_bAutogenSteps			( "AutogenSteps",			true ),
	m_bAutogenGroupCourses		( "AutogenGroupCourses",		true ),
	m_bBreakComboToGetItem		( "BreakComboToGetItem",		false ),
	m_bLockCourseDifficulties	( "LockCourseDifficulties",		true ),
	m_ShowDancingCharacters		( "ShowDancingCharacters",		SDC_Random ),
	m_bUseUnlockSystem		( "UseUnlockSystem",			false ),
	m_fGlobalOffsetSeconds		( "GlobalOffsetSeconds",		0 ),
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
	m_bAllowExtraStage		( "AllowExtraStage",			true ),
	m_bHideDefaultNoteSkin		( "HideDefaultNoteSkin",		false ),
	m_iMaxHighScoresPerListForMachine	( "MaxHighScoresPerListForMachine",	10 ),
	m_iMaxHighScoresPerListForPlayer	( "MaxHighScoresPerListForPlayer",	3 ),
	m_iMaxRecentScoresForMachine		( "MaxRecentScoresForMachine",		100 ),
	m_iMaxRecentScoresForPlayer		( "MaxRecentScoresForPlayer",		20 ),
	m_bAllowMultipleHighScoreWithSameName	( "AllowMultipleHighScoreWithSameName",	true ),
	m_bCelShadeModels		( "CelShadeModels",			false ),	// Work-In-Progress.. disable by default.
	m_bPreferredSortUsesGroups	( "PreferredSortUsesGroups",		true ),

	m_fPadStickSeconds		( "PadStickSeconds",			0 ),
	m_bForceMipMaps			( "ForceMipMaps",			false ),
	m_bTrilinearFiltering		( "TrilinearFiltering",			false ),
	m_bAnisotropicFiltering		( "AnisotropicFiltering",		false ),

	m_bSignProfileData		( "SignProfileData",			false ),
	m_CourseSortOrder		( "CourseSortOrder",			COURSE_SORT_SONGS ),
	m_bSubSortByNumSteps		( "SubSortByNumSteps",			false ),
	m_GetRankingName		( "GetRankingName",			RANKING_ON ),
	m_ScoringType			( "ScoringType",			SCORING_NEW ),
	m_sAdditionalSongFolders	( "AdditionalSongFolders",		"" ),
	m_sAdditionalCourseFolders	( "AdditionalCourseFolders",		"" ),
	m_sAdditionalFolders		( "AdditionalFolders",			"" ),
	m_sLastSeenVideoDriver		( "LastSeenVideoDriver",		"" ),
	m_sVideoRenderers		( "VideoRenderers",			"" ),	// StepMania.cpp sets these on first run:
	m_bSmoothLines			( "SmoothLines",			false ),
	m_fSoundVolume			( "SoundVolume",			1.0f ),
	m_iSoundWriteAhead		( "SoundWriteAhead",			0 ),
	m_iSoundDevice			( "SoundDevice",			"" ),
	m_iSoundPreferredSampleRate	( "SoundPreferredSampleRate",		44100 ),
	m_sLightsStepsDifficulty	( "LightsStepsDifficulty",		"medium" ),
	m_bAllowUnacceleratedRenderer	( "AllowUnacceleratedRenderer",		false ),
	m_bThreadedInput		( "ThreadedInput",			true ),
	m_bThreadedMovieDecode		( "ThreadedMovieDecode",		true ),
	m_bScreenTestMode		( "ScreenTestMode",			false ),
	m_bDebugLights			( "DebugLights",			false ),
	m_bMonkeyInput			( "MonkeyInput",			false ),
	m_sMachineName			( "MachineName",			"" ),
	m_sCoursesToShowRanking		( "CoursesToShowRanking",		"" ),

	/* Debug: */
	m_bLogToDisk			( "LogToDisk",		true ),
	m_bForceLogFlush		( "ForceLogFlush",	false ),
	m_bShowLogOutput		( "ShowLogOutput",	false ),
	m_bLogSkips			( "LogSkips",		false ),
	m_bLogCheckpoints		( "LogCheckpoints",	false ),
	m_bShowLoadingWindow		( "ShowLoadingWindow",	true ),
	m_bPseudoLocalize		( "PseudoLocalize",	false )

#if !defined(WITHOUT_NETWORKING)
	,
	m_bEnableScoreboard		( "EnableScoreboard",	true )
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
	GamePrefs &gp = m_mapGameNameToGamePrefs[m_sCurrentGame];
	gp.m_sAnnouncer			= m_sAnnouncer;
	gp.m_sTheme				= m_sTheme;
	gp.m_sDefaultModifiers	= m_sDefaultModifiers;
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

	//IPreference *pPref = PREFSMAN->GetPreferenceByName( *sName );
	//	if( pPref == NULL )
	//	{
	//		LOG->Warn( "Unknown preference in [%s]: %s", sClassName.c_str(), sName->c_str() );
	//		continue;
	//	}
	//	pPref->FromString( sVal );

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
		if( !BeginsWith(section->GetName(), GAME_SECTION_PREFIX) )
			continue;

		RString sGame = section->GetName().Right( section->GetName().length() - GAME_SECTION_PREFIX.length() );
		GamePrefs &gp = m_mapGameNameToGamePrefs[ sGame ];

		ini.GetValue( section->GetName(), "Announcer",		gp.m_sAnnouncer );
		ini.GetValue( section->GetName(), "Theme",			gp.m_sTheme );
		ini.GetValue( section->GetName(), "DefaultModifiers",	gp.m_sDefaultModifiers );
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
	if( pNode == NULL )
		pNode = ini.AppendChild( "Options" );
	IPreference::SavePrefsToNode( pNode );

	FOREACHM_CONST( RString, GamePrefs, m_mapGameNameToGamePrefs, iter )
	{
		RString sSection = "Game-" + RString( iter->first );

		ini.SetValue( sSection, "Announcer",		iter->second.m_sAnnouncer );
		ini.SetValue( sSection, "Theme",		iter->second.m_sTheme );
		ini.SetValue( sSection, "DefaultModifiers",	iter->second.m_sDefaultModifiers );
	}
}


RString PrefsManager::GetPreferencesSection() const
{
	RString sSection = "Preferences";

	// OK if this fails
	GetFileContents( SpecialFiles::TYPE_TXT_FILE, sSection, true );
	
	// OK if this fails
	GetCommandlineArgument( "Type", &sSection );

	return sSection;
}


// wrappers
float PrefsManager::GetSoundVolume()
{
	// return a resonable volume to that users of this method don't have to handle invalid values
	return clamp(m_fSoundVolume.Get(),0.0f,1.0f);
}

// lua start
#include "LuaBinding.h"

class LunaPrefsManager: public Luna<PrefsManager>
{
public:
	static int GetPreference( T* p, lua_State *L )
	{
		RString sName = SArg(1);
		IPreference *pPref = IPreference::GetPreferenceByName( sName );
		if( pPref == NULL )
		{
			LOG->Warn( "GetPreference: unknown preference \"%s\"", sName.c_str() );
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
		if( pPref == NULL )
		{
			LOG->Warn( "SetPreference: unknown preference \"%s\"", sName.c_str() );
			return 0;
		}

		lua_pushvalue( L, 2 );
		pPref->SetFromStack( L );
		return 0;
	}
	static int SetPreferenceToDefault( T* p, lua_State *L )
	{
		RString sName = SArg(1);

		IPreference *pPref = IPreference::GetPreferenceByName( sName );
		if( pPref == NULL )
		{
			LOG->Warn( "SetPreferenceToDefault: unknown preference \"%s\"", sName.c_str() );
			return 0;
		}

		pPref->LoadDefault();
		LOG->Trace( "Restored preference \"%s\" to default \"%s\"", sName.c_str(), pPref->ToString().c_str() );
		return 0;
	}

	LunaPrefsManager()
	{
		ADD_METHOD( GetPreference );
		ADD_METHOD( SetPreference );
		ADD_METHOD( SetPreferenceToDefault );
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

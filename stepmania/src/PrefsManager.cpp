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
#include "NoteSkinManager.h"
#include "GameState.h"
#include "RageException.h"
#include "RageDisplay.h"
#include "RageUtil.h"
#include "GameDef.h"
#include "AnnouncerManager.h"
#include "ThemeManager.h"
#include "GameConstantsAndTypes.h"
#include "arch/arch.h" /* for default driver specs */


PrefsManager*	PREFSMAN = NULL;	// global and accessable from anywhere in our program

const float DEFAULT_SOUND_VOLUME = 0.50;

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
	m_iMaxTextureResolution = 2048;
	m_iRefreshRate = REFRESH_DEFAULT;
	m_bIgnoreJoyAxes = true;		// ON by default because all USB convertors that are compatible with pads map to buttons
	m_bOnlyDedicatedMenuButtons = false;
#ifdef DEBUG
	m_bShowStats = true;
#else
	m_bShowStats = false;
#endif
	m_BackgroundMode = BGMODE_ANIMATIONS;
	m_bShowDanger = true;
	m_fBGBrightness = 0.8f;
	m_bMenuTimer = true;
	m_iNumArcadeStages = 3;
	m_bEventMode = false;
	m_bAutoPlay = false;
	m_fJudgeWindowScale = 1.0f;
	m_fJudgeWindowMarvelousSeconds = 0.0225f;
	m_fJudgeWindowPerfectSeconds = 0.045f;
	m_fJudgeWindowGreatSeconds = 0.090f;
	m_fJudgeWindowGoodSeconds = 0.135f;
	m_fJudgeWindowBooSeconds = 0.180f;
	m_fJudgeWindowOKSeconds = 0.140f;
	m_fLifeDifficultyScale = 1.0f;
	m_iMovieDecodeMS = 2;
	m_bUseBGIfNoBanner = false;
	m_bDelayedEscape = true;
	m_bInstructions = true;
	m_bShowDontDie = true;
	m_bShowSelectGroup = true;
	m_bShowNative = true;
	m_bArcadeOptionsNavigation = false;
	m_bSoloSingle = false;
	m_bDelayedTextureDelete = true;
	m_bDelayedScreenLoad = false;
	m_MusicWheelUsesSections = ALWAYS;
	m_iMusicWheelSwitchSpeed = 10;
	m_bEasterEggs = true;
	m_bMarvelousTiming = true;
	m_iCoinMode = COIN_HOME;
	m_iCoinsPerCredit = 1;
	m_bJointPremium = false;
	m_iBoostAppPriority = -1;
	m_iPolygonRadar = -1;
	m_ShowSongOptions = YES;
	m_bDancePointsForOni = false;
	m_bTimestamping = false;
	m_bShowLyrics = true;
	m_bAutogenMissingTypes = true;
	m_bAutogenGroupCourses = true;
	m_bBreakComboToGetItem = false;
	m_bShowDancingCharacters = false;
	m_BannerCacheType = preload_none;
	m_fDancePointsAccumulated = 0;
	m_bUseUnlockSystem = false;
	m_bFirstRun = true;
	m_bAutoMapJoysticks = true;
	
	/* DDR Extreme-style extra stage support.
	 * Default off so people used to the current behavior (or those with extra
	 * stage CRS files) don't get it changed around on them. */
	m_bPickExtraStage = false;

	m_fLongVerSongSeconds = 60*2.5f;	// Dynamite Rave is 2:55
	m_fMarathonVerSongSeconds = 60*5.f;

	/* I'd rather get occasional people asking for support for this even though it's
	 * already here than lots of people asking why songs aren't being displayed. */
	m_bHiddenSongs = false;
	m_bVsync = true;
	
	m_sRenderer = "";
	m_sSoundDrivers = DEFAULT_SOUND_DRIVER_LIST;
	m_fSoundVolume = DEFAULT_SOUND_VOLUME;
	/* This is experimental: let's see if preloading helps people's skipping.
	 * If it doesn't do anything useful, it'll be removed. */
	m_bSoundPreloadAll = false;
	
	m_bAllowUnacceleratedRenderer = false;

	m_sDefaultNoteSkin = "default";

	ReadGlobalPrefsFromDisk( true );
}

PrefsManager::~PrefsManager()
{
}

void PrefsManager::ReadGlobalPrefsFromDisk( bool bSwitchToLastPlayedGame )
{
	IniFile ini;
	ini.SetPath( "Data/StepMania.ini" );
	if( !ini.ReadFile() )
		return;		// could not read config file, load nothing

	ini.GetValueB( "Options", "Windowed",					m_bWindowed );
	ini.GetValueI( "Options", "DisplayWidth",				m_iDisplayWidth );
	ini.GetValueI( "Options", "DisplayHeight",				m_iDisplayHeight );
	ini.GetValueI( "Options", "DisplayColorDepth",			m_iDisplayColorDepth );
	ini.GetValueI( "Options", "TextureColorDepth",			m_iTextureColorDepth );
	ini.GetValueI( "Options", "MaxTextureResolution",		m_iMaxTextureResolution );
	ini.GetValueI( "Options", "RefreshRate",				m_iRefreshRate );
	ini.GetValueB( "Options", "IgnoreJoyAxes",				m_bIgnoreJoyAxes );
	ini.GetValueB( "Options", "UseDedicatedMenuButtons",	m_bOnlyDedicatedMenuButtons );
	ini.GetValueB( "Options", "ShowStats",					m_bShowStats );
	ini.GetValueI( "Options", "BackgroundMode",				(int&)m_BackgroundMode );
	ini.GetValueB( "Options", "ShowDanger",					m_bShowDanger );
	ini.GetValueF( "Options", "BGBrightness",				m_fBGBrightness );
	ini.GetValueB( "Options", "MenuTimer",					m_bMenuTimer );
	ini.GetValueI( "Options", "NumArcadeStages",			m_iNumArcadeStages );
	ini.GetValueB( "Options", "EventMode",					m_bEventMode );
	ini.GetValueB( "Options", "AutoPlay",					m_bAutoPlay );
	ini.GetValueF( "Options", "JudgeWindowScale",			m_fJudgeWindowScale );
	ini.GetValueF( "Options", "JudgeWindowMarvelousSeconds",m_fJudgeWindowMarvelousSeconds );
	ini.GetValueF( "Options", "JudgeWindowPerfectSeconds",	m_fJudgeWindowPerfectSeconds );
	ini.GetValueF( "Options", "JudgeWindowGreatSeconds",	m_fJudgeWindowGreatSeconds );
	ini.GetValueF( "Options", "JudgeWindowGoodSeconds",		m_fJudgeWindowGoodSeconds );
	ini.GetValueF( "Options", "JudgeWindowBooSeconds",		m_fJudgeWindowBooSeconds );
	ini.GetValueF( "Options", "JudgeWindowOKSeconds",		m_fJudgeWindowOKSeconds );
	ini.GetValueF( "Options", "LifeDifficultyScale",		m_fLifeDifficultyScale );
	ini.GetValueI( "Options", "MovieDecodeMS",				m_iMovieDecodeMS );
	ini.GetValueB( "Options", "UseBGIfNoBanner",			m_bUseBGIfNoBanner );
	ini.GetValueB( "Options", "DelayedEscape",				m_bDelayedEscape );
	ini.GetValueB( "Options", "HiddenSongs",				m_bHiddenSongs );
	ini.GetValueB( "Options", "Vsync",						m_bVsync );
	ini.GetValueB( "Options", "HowToPlay",					m_bInstructions );
	ini.GetValueB( "Options", "Caution",					m_bShowDontDie );
	ini.GetValueB( "Options", "SelectGroup",				m_bShowSelectGroup );
	ini.GetValueB( "Options", "ShowNative",					m_bShowNative );
	ini.GetValueB( "Options", "ArcadeOptionsNavigation",	m_bArcadeOptionsNavigation );
	ini.GetValue ( "Options", "DWIPath",					m_DWIPath );
	ini.GetValueB( "Options", "DelayedTextureDelete",		m_bDelayedTextureDelete );
	ini.GetValueB( "Options", "DelayedScreenLoad",			m_bDelayedScreenLoad );
	ini.GetValueI( "Options", "MusicWheelUsesSections",		(int&)m_MusicWheelUsesSections );
	ini.GetValueI( "Options", "MusicWheelSwitchSpeed",		m_iMusicWheelSwitchSpeed );
	ini.GetValue ( "Options", "Renderer",					m_sRenderer );
	ini.GetValue ( "Options", "SoundDrivers",				m_sSoundDrivers );
	ini.GetValueB( "Options", "EasterEggs",					m_bEasterEggs );
	ini.GetValueB( "Options", "MarvelousTiming",			m_bMarvelousTiming );
	ini.GetValueF( "Options", "SoundVolume",				m_fSoundVolume );
	ini.GetValueB( "Options", "SoundPreloadAll",			m_bSoundPreloadAll );
	ini.GetValueI( "Options", "CoinMode",					m_iCoinMode );
	ini.GetValueI( "Options", "CoinsPerCredit",				m_iCoinsPerCredit );
	ini.GetValueB( "Options", "JointPremium",				m_bJointPremium );
	ini.GetValueI( "Options", "BoostAppPriority",			m_iBoostAppPriority );
	ini.GetValueI( "Options", "PolygonRadar",				m_iPolygonRadar );
	ini.GetValueB( "Options", "PickExtraStage",				m_bPickExtraStage );
	ini.GetValueF( "Options", "LongVerSeconds",				m_fLongVerSongSeconds );
	ini.GetValueF( "Options", "MarathonVerSeconds",			m_fMarathonVerSongSeconds );
	ini.GetValueI( "Options", "ShowSongOptions",			(int&)m_ShowSongOptions );
	ini.GetValueB( "Options", "AllowUnacceleratedRenderer",	m_bAllowUnacceleratedRenderer );
	ini.GetValueB( "Options", "SoloSingle",					m_bSoloSingle );
	ini.GetValueB( "Options", "DancePointsForOni",			m_bDancePointsForOni );
	ini.GetValueB( "Options", "ShowLyrics",					m_bShowLyrics );
	ini.GetValueB( "Options", "AutogenMissingTypes",		m_bAutogenMissingTypes );
	ini.GetValueB( "Options", "AutogenGroupCourses",		m_bAutogenGroupCourses );
	ini.GetValueB( "Options", "Timestamping",				m_bTimestamping );
	ini.GetValue ( "Options", "DefaultModifiers",			m_sDefaultModifiers );
	ini.GetValueB( "Options", "BreakComboToGetItem",		m_bBreakComboToGetItem );
	ini.GetValueB( "Options", "ShowDancingCharacters",		m_bShowDancingCharacters );
	ini.GetValueI( "Options", "BannerCacheType",			(int&)m_BannerCacheType );
	/* XXX: This belongs in the memcard code, not prefs. */
	ini.GetValueF( "Options", "DancePointsAccumulated",		m_fDancePointsAccumulated );
	ini.GetValueB( "Options", "UseUnlockSystem",			m_bUseUnlockSystem );
	ini.GetValueB( "Options", "FirstRun",					m_bFirstRun );
	ini.GetValueB( "Options", "AutoMapJoysticks",			m_bAutoMapJoysticks );

	m_asAdditionalSongFolders.clear();
	CString sAdditionalSongFolders;
	ini.GetValue( "Options", "AdditionalSongFolders", sAdditionalSongFolders );
	split( sAdditionalSongFolders, ",", m_asAdditionalSongFolders, true );

	if( bSwitchToLastPlayedGame )
	{
		Game game;
		if( ini.GetValueI("Options", "Game", (int&)game) )
			GAMESTATE->m_CurGame = game;
	}
}


void PrefsManager::SaveGlobalPrefsToDisk()
{
	IniFile ini;
	ini.SetPath( "Data/StepMania.ini" );

	ini.SetValueB( "Options", "Windowed",					m_bWindowed );
	ini.SetValueI( "Options", "DisplayWidth",				m_iDisplayWidth );
	ini.SetValueI( "Options", "DisplayHeight",				m_iDisplayHeight );
	ini.SetValueI( "Options", "DisplayColorDepth",			m_iDisplayColorDepth );
	ini.SetValueI( "Options", "TextureColorDepth",			m_iTextureColorDepth );
	ini.SetValueI( "Options", "MaxTextureResolution",		m_iMaxTextureResolution );
	ini.SetValueI( "Options", "RefreshRate",				m_iRefreshRate );
	ini.SetValueB( "Options", "IgnoreJoyAxes",				m_bIgnoreJoyAxes );
	ini.SetValueB( "Options", "UseDedicatedMenuButtons",	m_bOnlyDedicatedMenuButtons );
	ini.SetValueB( "Options", "ShowStats",					m_bShowStats );
	ini.SetValueI( "Options", "BackgroundMode",				m_BackgroundMode);
	ini.SetValueB( "Options", "ShowDanger",					m_bShowDanger );
	ini.SetValueF( "Options", "BGBrightness",				m_fBGBrightness );
	ini.SetValueB( "Options", "MenuTimer",					m_bMenuTimer );
	ini.SetValueI( "Options", "NumArcadeStages",			m_iNumArcadeStages );
	ini.SetValueB( "Options", "EventMode",					m_bEventMode );
	ini.SetValueB( "Options", "AutoPlay",					m_bAutoPlay );
	ini.SetValueF( "Options", "JudgeWindowScale",			m_fJudgeWindowScale );
	ini.SetValueF( "Options", "JudgeWindowMarvelousSeconds",m_fJudgeWindowMarvelousSeconds );
	ini.SetValueF( "Options", "JudgeWindowPerfectSeconds",	m_fJudgeWindowPerfectSeconds );
	ini.SetValueF( "Options", "JudgeWindowGreatSeconds",	m_fJudgeWindowGreatSeconds );
	ini.SetValueF( "Options", "JudgeWindowGoodSeconds",		m_fJudgeWindowGoodSeconds );
	ini.SetValueF( "Options", "JudgeWindowBooSeconds",		m_fJudgeWindowBooSeconds );
	ini.SetValueF( "Options", "JudgeWindowOKSeconds",		m_fJudgeWindowOKSeconds );
	ini.SetValueF( "Options", "LifeDifficultyScale",		m_fLifeDifficultyScale );
	ini.SetValueI( "Options", "MovieDecodeMS",				m_iMovieDecodeMS );
	ini.SetValueB( "Options", "UseBGIfNoBanner",			m_bUseBGIfNoBanner );
	ini.SetValueB( "Options", "DelayedEscape",				m_bDelayedEscape );
	ini.SetValueB( "Options", "HiddenSongs",				m_bHiddenSongs );
	ini.SetValueB( "Options", "Vsync",						m_bVsync );
	ini.SetValueB( "Options", "HowToPlay",					m_bInstructions );
	ini.SetValueB( "Options", "Caution",					m_bShowDontDie );
	ini.SetValueB( "Options", "SelectGroup",				m_bShowSelectGroup );
	ini.SetValueB( "Options", "ShowNative",					m_bShowNative );
	ini.SetValueB( "Options", "ArcadeOptionsNavigation",	m_bArcadeOptionsNavigation );
	ini.SetValue ( "Options", "DWIPath",					m_DWIPath );
	ini.SetValueB( "Options", "DelayedTextureDelete",		m_bDelayedTextureDelete );
	ini.SetValueB( "Options", "DelayedScreenLoad",			m_bDelayedScreenLoad );
	ini.SetValueI( "Options", "MusicWheelUsesSections",		m_MusicWheelUsesSections );
	ini.SetValueI( "Options", "MusicWheelSwitchSpeed",		m_iMusicWheelSwitchSpeed );
	ini.SetValueB( "Options", "EasterEggs",					m_bEasterEggs );
	ini.SetValueB( "Options", "MarvelousTiming",			m_bMarvelousTiming );
	ini.SetValueB( "Options", "SoundPreloadAll",			m_bSoundPreloadAll );
	ini.SetValueI( "Options", "CoinMode",					m_iCoinMode );
	ini.SetValueI( "Options", "CoinsPerCredit",				m_iCoinsPerCredit );
	ini.SetValueB( "Options", "JointPremium",				m_bJointPremium );
	ini.SetValueI( "Options", "BoostAppPriority",			m_iBoostAppPriority );
	ini.SetValueI( "Options", "PolygonRadar",				m_iPolygonRadar );
	ini.SetValueB( "Options", "PickExtraStage",				m_bPickExtraStage );
	ini.SetValueF( "Options", "LongVerSeconds",				m_fLongVerSongSeconds );
	ini.SetValueF( "Options", "MarathonVerSeconds",			m_fMarathonVerSongSeconds );
	ini.SetValueI( "Options", "ShowSongOptions",			m_ShowSongOptions );
	ini.SetValueB( "Options", "AllowUnacceleratedRenderer",	m_bAllowUnacceleratedRenderer );
	ini.SetValueB( "Options", "SoloSingle",					m_bSoloSingle );
	ini.SetValueB( "Options", "DancePointsForOni",			m_bDancePointsForOni );
	ini.SetValueB( "Options", "ShowLyrics",					m_bShowLyrics );
	ini.SetValueB( "Options", "AutogenMissingTypes",		m_bAutogenMissingTypes );
	ini.SetValueB( "Options", "AutogenGroupCourses",		m_bAutogenGroupCourses );
	ini.SetValueB( "Options", "Timestamping",				m_bTimestamping );
	ini.SetValue ( "Options", "DefaultModifiers",			m_sDefaultModifiers );
	ini.SetValueB( "Options", "BreakComboToGetItem",		m_bBreakComboToGetItem );
	ini.SetValueB( "Options", "ShowDancingCharacters",		m_bShowDancingCharacters );
	ini.SetValueI( "Options", "BannerCacheType",			m_BannerCacheType );
	ini.SetValueF( "Options", "DancePointsAccumulated",		m_fDancePointsAccumulated );
	ini.SetValueB( "Options", "UseUnlockSystem",			m_bUseUnlockSystem );
	ini.SetValueB( "Options", "FirstRun",					m_bFirstRun );
	ini.SetValueB( "Options", "AutoMapJoysticks",			m_bAutoMapJoysticks );


	/* Only write these if they aren't the default.  This ensures that we can change
	 * the default and have it take effect for everyone (except people who
	 * tweaked this value). */
	if(m_sSoundDrivers != DEFAULT_SOUND_DRIVER_LIST)
		ini.SetValue ( "Options", "SoundDrivers",				m_sSoundDrivers );
	if(m_fSoundVolume != DEFAULT_SOUND_VOLUME)
		ini.SetValueF( "Options", "SoundVolume",			m_fSoundVolume );
	if(m_sRenderer != "")
		ini.SetValue ( "Options", "Renderer",				m_sRenderer );


	ini.SetValue( "Options", "AdditionalSongFolders", join(",", m_asAdditionalSongFolders) );

	ini.SetValueI( "Options", "Game",	GAMESTATE->m_CurGame );

	ini.WriteFile();
}

void PrefsManager::ReadGamePrefsFromDisk()
{
	if( !GAMESTATE )
		return;

	CString sGameName = GAMESTATE->GetCurrentGameDef()->m_szName;
	IniFile ini;
	ini.SetPath( "Data/GamePrefs.ini" );
	ini.ReadFile();	// it's OK if this fails

	CString sAnnouncer = sGameName, sTheme = sGameName, sNoteSkin = sGameName;

	// if these calls fail, the three strings will keep the initial values set above.
	ini.GetValue( sGameName, "Announcer",		sAnnouncer );
	ini.GetValue( sGameName, "Theme",			sTheme );
	ini.GetValue( sGameName, "NoteSkin",		sNoteSkin );

	// it's OK to call these functions with names that don't exist.
	ANNOUNCER->SwitchAnnouncer( sAnnouncer );
	THEME->SwitchTheme( sTheme );

	if(NOTESKIN->DoesNoteSkinExist(sNoteSkin))
		m_sDefaultNoteSkin = sNoteSkin;
	else
		m_sDefaultNoteSkin = "default";

//	NOTESKIN->SwitchNoteSkin( sNoteSkin );
}

void PrefsManager::SaveGamePrefsToDisk()
{
	if( !GAMESTATE )
		return;

	CString sGameName = GAMESTATE->GetCurrentGameDef()->m_szName;
	IniFile ini;
	ini.SetPath( "Data/GamePrefs.ini" );
	ini.ReadFile();	// it's OK if this fails

	ini.SetValue( sGameName, "Announcer",		ANNOUNCER->GetCurAnnouncerName() );
	ini.SetValue( sGameName, "Theme",			THEME->GetCurThemeName() );
//	ini.SetValue( sGameName, "NoteSkin",		NOTESKIN->GetCurNoteSkinName() );
	ini.SetValue( sGameName, "NoteSkin",		m_sDefaultNoteSkin );

	ini.WriteFile();
}


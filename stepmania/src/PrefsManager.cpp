#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: PrefsManager

 Desc: See Header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "PrefsManager.h"
#include "IniFile.h"
#include "GameManager.h"
#include "GameState.h"
#include "RageException.h"
#include "RageDisplay.h"
#include "RageUtil.h"
#include "AnnouncerManager.h"
#include "ThemeManager.h"
#include "arch/arch.h" /* for default driver specs */


PrefsManager*	PREFSMAN = NULL;	// global and accessable from anywhere in our program


PrefsManager::PrefsManager()
{
#ifdef _DEBUG
	m_bWindowed = true;
#else
	m_bWindowed = false;
#endif
	m_iDisplayWidth = 640;
	m_iDisplayHeight = 480;
	m_iDisplayColorDepth = 16;
	m_iTextureColorDepth = 16;
	m_iRefreshRate = REFRESH_DEFAULT;
	m_bIgnoreJoyAxes = true;
	m_bOnlyDedicatedMenuButtons = false;
#ifdef _DEBUG
	m_bShowStats = true;
#else
	m_bShowStats = false;
#endif
	m_BackgroundMode = BGMODE_ANIMATIONS;
	m_bShowDanger = true;
	m_fBGBrightness = 0.8f;
	m_bMenuTimer = true;
	m_bEventMode = false;
	m_iNumArcadeStages = 3;
	m_bAutoPlay = false;
	m_fJudgeWindowSeconds = 0.17f;
	m_fJudgeWindowPerfectPercent = 0.25f;
	m_fJudgeWindowGreatPercent = 0.50f;
	m_fJudgeWindowGoodPercent = 0.75f;
	m_fLifeDifficultyScale = 1.0f;
	m_iMovieDecodeMS = 2;
	m_bUseBGIfNoBanner = false;
	m_bDelayedEscape = true;
	m_bHowToPlay = true;
	m_bShowDontDie = true;
	m_bShowSelectGroup = true;
	m_bArcadeOptionsNavigation = false;
	m_iUnloadTextureDelaySeconds = 0; // disabled 60*30;	// 30 mins
	m_bCoinOpMode = false;

	/* I'd rather get occasional people asking for support for this even though it's
	 * already here than lots of people asking why songs aren't being displayed. */
	m_bHiddenSongs = false;
	m_bVsync = true;

	m_bSoundDrivers = DEFAULT_SOUND_DRIVER_LIST;

	ReadGlobalPrefsFromDisk( true );
}

PrefsManager::~PrefsManager()
{
}

 void PrefsManager::ReadGlobalPrefsFromDisk( bool bSwitchToLastPlayedGame )
{
	IniFile ini;
	ini.SetPath( "StepMania.ini" );
	if( !ini.ReadFile() )
		return;		// could not read config file, load nothing

	ini.GetValueB( "Options", "Windowed",					m_bWindowed );
	ini.GetValueI( "Options", "DisplayWidth",				m_iDisplayWidth );
	ini.GetValueI( "Options", "DisplayHeight",				m_iDisplayHeight );
	ini.GetValueI( "Options", "DisplayColorDepth",			m_iDisplayColorDepth );
	ini.GetValueI( "Options", "TextureColorDepth",			m_iTextureColorDepth );
	ini.GetValueI( "Options", "RefreshRate",				m_iRefreshRate );
	ini.GetValueB( "Options", "IgnoreJoyAxes",				m_bIgnoreJoyAxes );
	ini.GetValueB( "Options", "UseDedicatedMenuButtons",	m_bOnlyDedicatedMenuButtons );
	ini.GetValueB( "Options", "ShowStats",					m_bShowStats );
	ini.GetValueI( "Options", "BackgroundMode",				(int&)m_BackgroundMode );
	ini.GetValueB( "Options", "ShowDanger",					m_bShowDanger );
	ini.GetValueF( "Options", "BGBrightness",				m_fBGBrightness );
	ini.GetValueB( "Options", "MenuTimer",					m_bMenuTimer );
	ini.GetValueB( "Options", "EventMode",					m_bEventMode );
	ini.GetValueI( "Options", "NumArcadeStages",			m_iNumArcadeStages );
	ini.GetValueB( "Options", "AutoPlay",					m_bAutoPlay );
	ini.GetValueF( "Options", "JudgeWindowSeconds",			m_fJudgeWindowSeconds );
	ini.GetValueF( "Options", "JudgeWindowPerfectPercent",	m_fJudgeWindowPerfectPercent );
	ini.GetValueF( "Options", "JudgeWindowGreatPercent",	m_fJudgeWindowGreatPercent );
	ini.GetValueF( "Options", "JudgeWindowGoodPercent",		m_fJudgeWindowGoodPercent );
	ini.GetValueF( "Options", "LifeDifficultyScale",		m_fLifeDifficultyScale );
	ini.GetValueI( "Options", "MovieDecodeMS",				m_iMovieDecodeMS );
	ini.GetValueB( "Options", "UseBGIfNoBanner",			m_bUseBGIfNoBanner );
	ini.GetValueB( "Options", "DelayedEscape",				m_bDelayedEscape );
	ini.GetValueB( "Options", "HiddenSongs",				m_bHiddenSongs );
	ini.GetValueB( "Options", "Vsync",						m_bVsync );
	ini.GetValueB( "Options", "HowToPlay",					m_bHowToPlay );
	ini.GetValueB( "Options", "Caution",					m_bShowDontDie );
	ini.GetValueB( "Options", "SelectGroup",				m_bShowSelectGroup );
	ini.GetValueB( "Options", "ArcadeOptionsNavigation",	m_bArcadeOptionsNavigation );
	ini.GetValue ( "Options", "DWIPath",					m_DWIPath );
	ini.GetValueI( "Options", "UnloadTextureDelaySeconds",	m_iUnloadTextureDelaySeconds );
	ini.GetValueB( "Options", "CoinOpMode",					m_bCoinOpMode );
	ini.GetValue ( "Options", "SoundDrivers",				m_bSoundDrivers );

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
	ini.SetPath( "StepMania.ini" );

	ini.SetValueB( "Options", "Windowed",					m_bWindowed );
	ini.SetValueI( "Options", "DisplayWidth",				m_iDisplayWidth );
	ini.SetValueI( "Options", "DisplayHeight",				m_iDisplayHeight );
	ini.SetValueI( "Options", "DisplayColorDepth",			m_iDisplayColorDepth );
	ini.SetValueI( "Options", "TextureColorDepth",			m_iTextureColorDepth );
	ini.SetValueI( "Options", "RefreshRate",				m_iRefreshRate );
	ini.SetValueB( "Options", "IgnoreJoyAxes",				m_bIgnoreJoyAxes );
	ini.SetValueB( "Options", "UseDedicatedMenuButtons",	m_bOnlyDedicatedMenuButtons );
	ini.SetValueB( "Options", "ShowStats",					m_bShowStats );
	ini.SetValueI( "Options", "BackgroundMode",				m_BackgroundMode);
	ini.SetValueB( "Options", "ShowDanger",					m_bShowDanger );
	ini.SetValueF( "Options", "BGBrightness",				m_fBGBrightness );
	ini.SetValueB( "Options", "EventMode",					m_bEventMode );
	ini.SetValueB( "Options", "MenuTimer",					m_bMenuTimer );
	ini.SetValueI( "Options", "NumArcadeStages",			m_iNumArcadeStages );
	ini.SetValueB( "Options", "AutoPlay",					m_bAutoPlay );
	ini.SetValueF( "Options", "JudgeWindowSeconds",			m_fJudgeWindowSeconds );
	ini.SetValueF( "Options", "JudgeWindowPerfectPercent",	m_fJudgeWindowPerfectPercent );
	ini.SetValueF( "Options", "JudgeWindowGreatPercent",	m_fJudgeWindowGreatPercent );
	ini.SetValueF( "Options", "JudgeWindowGoodPercent",		m_fJudgeWindowGoodPercent );
	ini.SetValueF( "Options", "LifeDifficultyScale",		m_fLifeDifficultyScale );
	ini.SetValueI( "Options", "MovieDecodeMS",				m_iMovieDecodeMS );
	ini.SetValueB( "Options", "UseBGIfNoBanner",			m_bUseBGIfNoBanner );
	ini.SetValueB( "Options", "DelayedEscape",				m_bDelayedEscape );
	ini.SetValueB( "Options", "HiddenSongs",				m_bHiddenSongs );
	ini.SetValueB( "Options", "Vsync",						m_bVsync );
	ini.SetValueB( "Options", "HowToPlay",					m_bHowToPlay );
	ini.SetValueB( "Options", "Caution",					m_bShowDontDie );
	ini.SetValueB( "Options", "SelectGroup",				m_bShowSelectGroup );
	ini.SetValueB( "Options", "ArcadeOptionsNavigation",	m_bArcadeOptionsNavigation );
	ini.SetValue ( "Options", "DWIPath",					m_DWIPath );
	ini.SetValueI( "Options", "UnloadTextureDelaySeconds",	m_iUnloadTextureDelaySeconds );
	ini.SetValueB( "Options", "CoinOpMode",					m_bCoinOpMode );
	ini.SetValue ( "Options", "SoundDrivers",				m_bSoundDrivers );

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
	ini.SetPath( sGameName+"Prefs.ini" );
	ini.ReadFile();	// it's OK if this fails

	CString sAnnouncer = sGameName, sTheme = sGameName, sNoteSkin = sGameName;

	// if these calls fail, the three strings will keep the initial values set above.
	ini.GetValue( "Options", "Announcer",		sAnnouncer );
	ini.GetValue( "Options", "Theme",			sTheme );
	ini.GetValue( "Options", "NoteSkin",		sNoteSkin );

	// it's OK to call these functions with names that don't exist.
	ANNOUNCER->SwitchAnnouncer( sAnnouncer );
	THEME->SwitchTheme( sTheme );
	GAMEMAN->SwitchNoteSkin( sNoteSkin );
}

void PrefsManager::SaveGamePrefsToDisk()
{
	if( !GAMESTATE )
		return;

	CString sGameName = GAMESTATE->GetCurrentGameDef()->m_szName;
	IniFile ini;
	ini.SetPath( sGameName+"Prefs.ini" );

	ini.SetValue( "Options", "Announcer",		ANNOUNCER->GetCurAnnouncerName() );
	ini.SetValue( "Options", "Theme",			THEME->GetCurThemeName() );
	ini.SetValue( "Options", "NoteSkin",		GAMEMAN->GetCurNoteSkin() );

	ini.WriteFile();
}


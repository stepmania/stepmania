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


PrefsManager*	PREFSMAN = NULL;	// global and accessable from anywhere in our program


PrefsManager::PrefsManager()
{
#ifdef _DEBUG
	m_bWindowed = true;
#else
	m_bWindowed = false;
#endif
	m_iDisplayResolution = 640;
	m_iTextureResolution = 1024;
	m_iRefreshRate = 0;
	m_bIgnoreJoyAxes = false;
	m_bOnlyDedicatedMenuButtons = false;
	m_bShowFPS = false;
	m_BackgroundMode = BGMODE_ANIMATIONS;
	m_bShowDanger = true;
	m_fBGBrightness = 0.8f;
	m_bMenuTimer = true;
	m_bEventMode = false;
	m_iNumArcadeStages = 3;
	m_bAutoPlay = false;
	m_fJudgeWindow = 0.18f;
	m_fLifeDifficultyScale = 1.0f;
	m_iMovieDecodeMS = 2;

	ReadGlobalPrefsFromDisk( true );
}

PrefsManager::~PrefsManager()
{
	SaveGlobalPrefsToDisk();
	SaveGamePrefsToDisk();
}

void PrefsManager::ReadGlobalPrefsFromDisk( bool bSwitchToLastPlayedGame )
{
	IniFile ini;
	ini.SetPath( "StepMania.ini" );
	if( !ini.ReadFile() )
		return;		// could not read config file, load nothing

	ini.GetValueB( "Options", "Windowed",				m_bWindowed );
	ini.GetValueI( "Options", "DisplayResolution",		m_iDisplayResolution );
	ini.GetValueI( "Options", "TextureResolution",		m_iTextureResolution );
	ini.GetValueI( "Options", "RefreshRate",			m_iRefreshRate );
	ini.GetValueB( "Options", "IgnoreJoyAxes",			m_bIgnoreJoyAxes );
	ini.GetValueB( "Options", "UseDedicatedMenuButtons",m_bOnlyDedicatedMenuButtons );
	ini.GetValueB( "Options", "ShowFPS",				m_bShowFPS );
	ini.GetValueI( "Options", "BackgroundMode",			(int&)m_BackgroundMode );
	ini.GetValueB( "Options", "ShowDanger",				m_bShowDanger );
	ini.GetValueF( "Options", "BGBrightness",			m_fBGBrightness );
	ini.GetValueB( "Options", "MenuTimer",				m_bMenuTimer );
	ini.GetValueB( "Options", "EventMode",				m_bEventMode );
	ini.GetValueI( "Options", "NumArcadeStages",		m_iNumArcadeStages );
	ini.GetValueB( "Options", "AutoPlay",				m_bAutoPlay );
	ini.GetValueF( "Options", "JudgeWindow",			m_fJudgeWindow );
	ini.GetValueF( "Options", "LifeDifficultyScale",	m_fLifeDifficultyScale );
	ini.GetValueI( "Options", "MovieDecodeMS",			m_iMovieDecodeMS );

	m_asAdditionalSongFolders.RemoveAll();
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

	ini.SetValueB( "Options", "Windowed",				m_bWindowed );
	ini.SetValueI( "Options", "DisplayResolution",		m_iDisplayResolution );
	ini.SetValueI( "Options", "TextureResolution",		m_iTextureResolution );
	ini.SetValueI( "Options", "RefreshRate",			m_iRefreshRate );
	ini.SetValueB( "Options", "IgnoreJoyAxes",			m_bIgnoreJoyAxes );
	ini.GetValueB( "Options", "UseDedicatedMenuButtons",m_bOnlyDedicatedMenuButtons );
	ini.SetValueB( "Options", "ShowFPS",				m_bShowFPS );
	ini.SetValueI( "Options", "BackgroundMode",			m_BackgroundMode);
	ini.SetValueB( "Options", "ShowDanger",				m_bShowDanger );
	ini.SetValueF( "Options", "BGBrightness",			m_fBGBrightness );
	ini.SetValueB( "Options", "EventMode",				m_bEventMode );
	ini.SetValueB( "Options", "MenuTimer",				m_bMenuTimer );
	ini.SetValueI( "Options", "NumArcadeStages",		m_iNumArcadeStages );
	ini.SetValueB( "Options", "AutoPlay",				m_bAutoPlay );
	ini.SetValueF( "Options", "JudgeWindow",			m_fJudgeWindow );
	ini.SetValueF( "Options", "LifeDifficultyScale",	m_fLifeDifficultyScale );
	ini.SetValueI( "Options", "MovieDecodeMS",			m_iMovieDecodeMS );

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
	ini.SetPath( sGameName + "Prefs.ini" );
	if( !ini.ReadFile() )
		return;		// could not read config file, load nothing

	CString sAnnouncer, sTheme, sNoteSkin;

	ini.GetValue( "Options", "Announcer",			sAnnouncer );
	ini.GetValue( "Options", "Theme",				sTheme );
	ini.GetValue( "Options", "NoteSkin",			sNoteSkin );


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
	ini.SetPath( sGameName + "Prefs.ini" );

	ini.SetValue( "Options", "Announcer",			ANNOUNCER->GetCurAnnouncerName() );
	ini.SetValue( "Options", "Theme",				THEME->GetCurThemeName() );
	ini.SetValue( "Options", "NoteSkin",			GAMEMAN->GetCurNoteSkin() );

	ini.WriteFile();
}


int PrefsManager::GetDisplayHeight()
{
	switch( m_iDisplayResolution )
	{
	case 1280:	return 1024;	break;
	case 1024:	return 768;	break;
	case 800:	return 600;	break;
	case 640:	return 480;	break;
	case 512:	return 384;	break;
	case 400:	return 300;	break;
	case 320:	return 240;	break;
	default:	throw RageException( "Invalid DisplayWidth '%d'", m_iDisplayResolution );	return 480;
	}
}

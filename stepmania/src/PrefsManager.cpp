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


PrefsManager*	PREFS = NULL;	// global and accessable from anywhere in our program

GraphicProfileOptions GRAPHIC_OPTIONS[NUM_GRAPHIC_PROFILES] =
{
	{	// PROFILE_SUPER_LOW
		"Super Low",
		320,
		240,
		256,
		16,
		16,
		false,
	},
	{	// PROFILE_LOW
		"Low",
		400,
		300,
		256,
		16,
		16,
		true,
	},
	{	// PROFILE_MEDIUM
		"Medium",
		640,
		480,
		512,
		16,
		16,
		true,
	},
	{	// PROFILE_HIGH
		"High",
		640,
		480,
		1024,
		16,
		32,
		true,
	},
	{	// PROFILE_CUSTOM
		"Custom",
		640,
		480,
		512,
		16,
		16,
		true,
	},
};

PrefsManager::PrefsManager()
{
	m_SongSortOrder = SORT_GROUP;
	m_iCurrentStage = 1;
	m_GameMode = GAME_MODE_ARCADE;

	m_bWindowed = false;
	m_GraphicProfile = PROFILE_MEDIUM;

	for( int p=0; p<NUM_PLAYERS; p++ )
		m_PreferredDifficultyClass[p] = CLASS_EASY;

	ReadPrefsFromDisk();
}


PrefsManager::~PrefsManager()
{
	// don't worry about releasing the Song array.  Let the OS do it :-)
	SavePrefsToDisk();
}


GraphicProfileOptions*	PrefsManager::GetCustomGraphicProfileOptions()
{
	return &GRAPHIC_OPTIONS[PROFILE_CUSTOM];
}


GraphicProfileOptions*	PrefsManager::GetCurrentGraphicProfileOptions()
{
	return &GRAPHIC_OPTIONS[m_GraphicProfile];
}


void PrefsManager::ReadPrefsFromDisk()
{
	IniFile ini;
	ini.SetPath( "StepMania.ini" );
	if( !ini.ReadFile() ) {
		return;		// could not read config file, load nothing
	}

	GraphicProfileOptions* pGPO = GetCustomGraphicProfileOptions();

	ini.GetValueB( "GraphicOptions", "Windowed",		m_bWindowed );
	ini.GetValueI( "GraphicOptions", "Profile",			(int&)m_GraphicProfile );
	ini.GetValueI( "GraphicOptions", "Width",			pGPO->m_iWidth );
	ini.GetValueI( "GraphicOptions", "Height",			pGPO->m_iHeight );
	ini.GetValueI( "GraphicOptions", "MaxTextureSize",	pGPO->m_iMaxTextureSize );
	ini.GetValueI( "GraphicOptions", "DisplayColor",	pGPO->m_iDisplayColor );
	ini.GetValueI( "GraphicOptions", "TextureColor",	pGPO->m_iTextureColor );

	ini.GetValueB( "GameOptions", "IgnoreJoyAxes",		m_GameOptions.m_bIgnoreJoyAxes );
	ini.GetValueB( "GameOptions", "ShowFPS",			m_GameOptions.m_bShowFPS );
	ini.GetValueB( "GameOptions", "UseRandomVis",		m_GameOptions.m_bUseRandomVis );
	ini.GetValueB( "GameOptions", "Announcer",			m_GameOptions.m_bAnnouncer );
	ini.GetValueB( "GameOptions", "EventMode",			m_GameOptions.m_bEventMode );
	ini.GetValueB( "GameOptions", "ShowSelectDifficulty", m_GameOptions.m_bShowSelectDifficulty );
	ini.GetValueB( "GameOptions", "ShowSelectGroup",	m_GameOptions.m_bShowSelectGroup );
	ini.GetValueI( "GameOptions", "NumArcadeStages",	m_GameOptions.m_iNumArcadeStages );
	ini.GetValueI( "GameOptions", "Difficulty",			m_GameOptions.m_iDifficulty );
}


void PrefsManager::SavePrefsToDisk()
{
	IniFile ini;
	ini.SetPath( "StepMania.ini" );


	GraphicProfileOptions* pGPO = GetCustomGraphicProfileOptions();

	ini.SetValueB( "GraphicOptions", "Windowed",		m_bWindowed );
	ini.SetValueI( "GraphicOptions", "Profile",			m_GraphicProfile );
	ini.SetValueI( "GraphicOptions", "Width",			pGPO->m_iWidth );
	ini.SetValueI( "GraphicOptions", "Height",			pGPO->m_iHeight );
	ini.SetValueI( "GraphicOptions", "MaxTextureSize",	pGPO->m_iMaxTextureSize );
	ini.SetValueI( "GraphicOptions", "DisplayColor",	pGPO->m_iDisplayColor );
	ini.SetValueI( "GraphicOptions", "TextureColor",	pGPO->m_iTextureColor );

	ini.SetValueB( "GameOptions", "IgnoreJoyAxes",		m_GameOptions.m_bIgnoreJoyAxes );
	ini.SetValueB( "GameOptions", "ShowFPS",			m_GameOptions.m_bShowFPS );
	ini.SetValueB( "GameOptions", "UseRandomVis",		m_GameOptions.m_bUseRandomVis );
	ini.SetValueB( "GameOptions", "Announcer",			m_GameOptions.m_bAnnouncer );
	ini.SetValueB( "GameOptions", "EventMode",			m_GameOptions.m_bEventMode );
	ini.SetValueB( "GameOptions", "ShowSelectDifficulty", m_GameOptions.m_bShowSelectDifficulty );
	ini.SetValueB( "GameOptions", "ShowSelectGroup",	m_GameOptions.m_bShowSelectGroup );
	ini.SetValueI( "GameOptions", "NumArcadeStages",	m_GameOptions.m_iNumArcadeStages );
	ini.SetValueI( "GameOptions", "Difficulty",			m_GameOptions.m_iDifficulty );

	ini.WriteFile();
}

int PrefsManager::GetStageNumber()
{
	return m_iCurrentStage;
}

bool PrefsManager::IsFinalStage()
{
	return m_GameOptions.m_iNumArcadeStages == m_iCurrentStage;
}

CString PrefsManager::GetStageText()
{
	if( m_iCurrentStage == 3 )
		return "Final";

	CString sNumberSuffix;
	if( ( (m_iCurrentStage/10) % 10 ) == 1 )	// in the teens (e.g. 19, 213)
	{
		sNumberSuffix = "th";
	}
	else	// not in the teens
	{
		const int iLastDigit = m_iCurrentStage%10;
		switch( iLastDigit )
		{
		case 1:	sNumberSuffix = "st";	break;
		case 2:	sNumberSuffix = "nd";	break;
		case 3:	sNumberSuffix = "rd";	break;
		default:sNumberSuffix = "th";	break;
		}
	}
	return ssprintf( "%d%s", PREFS->m_iCurrentStage, sNumberSuffix );
}


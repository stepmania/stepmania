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


PrefsManager*	PREFSMAN = NULL;	// global and accessable from anywhere in our program


PrefsManager::PrefsManager()
{
	m_bWindowed = false;
	m_bHighDetail = true;
	m_bHighTextureDetail = true;
	m_bIgnoreJoyAxes = false;
	m_bShowFPS = false;
	m_bUseRandomVis = false;
	m_bAnnouncer = true;
	m_bEventMode = true;
	m_iNumArcadeStages = 3;
	m_iDifficulty = 4;

	for( int p=0; p<NUM_PLAYERS; p++ )
		m_PreferredDifficultyClass[p] = CLASS_EASY;
	m_SongSortOrder = SORT_GROUP;
	m_PlayMode = PLAY_MODE_ARCADE;
	m_iCurrentStageIndex = 0;

	ReadPrefsFromDisk();
}

PrefsManager::~PrefsManager()
{
	SavePrefsToDisk();
}

void PrefsManager::ReadPrefsFromDisk()
{
	IniFile ini;
	ini.SetPath( "StepMania.ini" );
	if( !ini.ReadFile() )
		return;		// could not read config file, load nothing

	ini.GetValueB( "Options", "Windowed",			m_bWindowed );
	ini.GetValueB( "Options", "HighDetail",			m_bHighDetail );
	ini.GetValueB( "Options", "HighTextureDetail",	m_bHighTextureDetail );
	ini.GetValueB( "Options", "IgnoreJoyAxes",		m_bIgnoreJoyAxes );
	ini.GetValueB( "Options", "ShowFPS",			m_bShowFPS );
	ini.GetValueB( "Options", "UseRandomVis",		m_bUseRandomVis );
	ini.GetValueB( "Options", "Announcer",			m_bAnnouncer );
	ini.GetValueB( "Options", "EventMode",			m_bEventMode );
	ini.GetValueI( "Options", "NumArcadeStages",	m_iNumArcadeStages );
	ini.GetValueI( "Options", "Difficulty",			m_iDifficulty );
}


void PrefsManager::SavePrefsToDisk()
{
	IniFile ini;
	ini.SetPath( "StepMania.ini" );

	ini.SetValueB( "Options", "Windowed",			m_bWindowed );
	ini.SetValueB( "Options", "HighDetail",			m_bHighDetail );
	ini.GetValueB( "Options", "HighTextureDetail",	m_bHighTextureDetail );
	ini.SetValueB( "Options", "IgnoreJoyAxes",		m_bIgnoreJoyAxes );
	ini.SetValueB( "Options", "ShowFPS",			m_bShowFPS );
	ini.SetValueB( "Options", "UseRandomVis",		m_bUseRandomVis );
	ini.SetValueB( "Options", "Announcer",			m_bAnnouncer );
	ini.SetValueB( "Options", "EventMode",			m_bEventMode );
	ini.SetValueI( "Options", "NumArcadeStages",	m_iNumArcadeStages );
	ini.SetValueI( "Options", "Difficulty",			m_iDifficulty );

	ini.WriteFile();
}

int PrefsManager::GetStageIndex()
{
	return m_iCurrentStageIndex;
}

int PrefsManager::GetStageNumber()
{
	return m_iCurrentStageIndex+1;
}

bool PrefsManager::IsFinalStage()
{
	return m_iCurrentStageIndex == m_iNumArcadeStages-1;
}

CString PrefsManager::GetStageText()
{
	if( m_iCurrentStageIndex == m_iNumArcadeStages-1 )
		return "Final";

	int iStageNo = m_iCurrentStageIndex+1;

	CString sNumberSuffix;
	if( ( (iStageNo/10) % 10 ) == 1 )	// in the teens (e.g. 19, 213)
	{
		sNumberSuffix = "th";
	}
	else	// not in the teens
	{
		const int iLastDigit = iStageNo%10;
		switch( iLastDigit )
		{
		case 1:	sNumberSuffix = "st";	break;
		case 2:	sNumberSuffix = "nd";	break;
		case 3:	sNumberSuffix = "rd";	break;
		default:sNumberSuffix = "th";	break;
		}
	}
	return ssprintf( "%d%s", this->GetStageNumber(), sNumberSuffix );
}


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
	m_bHighTextureDetail = false;
	m_bIgnoreJoyAxes = false;
	m_bShowFPS = false;
	m_visMode = VIS_MODE_ANIMATION;
	m_bAnnouncer = true;
	m_bEventMode = false;
	m_iNumArcadeStages = 3;
	m_bAutoPlay = false;
	m_fJudgeWindow = 0.12f;

	for( int p=0; p<NUM_PLAYERS; p++ )
		m_PreferredDifficultyClass[p] = CLASS_EASY;
	m_SongSortOrder = SORT_GROUP;
	m_PlayMode = PLAY_MODE_INVALID;
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
	ini.GetValueI( "Options", "VisMode",			m_visMode );
	ini.GetValueB( "Options", "Announcer",			m_bAnnouncer );
	ini.GetValueB( "Options", "EventMode",			m_bEventMode );
	ini.GetValueI( "Options", "NumArcadeStages",	m_iNumArcadeStages );
	ini.GetValueB( "Options", "AutoPlay",			m_bAutoPlay );
	ini.GetValueF( "Options", "JudgeWindow",		m_fJudgeWindow );
	
	CString sAdditionalSongFolders;
	ini.GetValue( "Options", "SongFolders", sAdditionalSongFolders );
	split( sAdditionalSongFolders, ",", m_asSongFolders, true );
}


void PrefsManager::SavePrefsToDisk()
{
	IniFile ini;
	ini.SetPath( "StepMania.ini" );

	ini.SetValueB( "Options", "Windowed",			m_bWindowed );
	ini.SetValueB( "Options", "HighDetail",			m_bHighDetail );
	ini.SetValueB( "Options", "HighTextureDetail",	m_bHighTextureDetail );
	ini.SetValueB( "Options", "IgnoreJoyAxes",		m_bIgnoreJoyAxes );
	ini.SetValueB( "Options", "ShowFPS",			m_bShowFPS );
	ini.SetValueI( "Options", "VisMode",			m_visMode );
	ini.SetValueB( "Options", "Announcer",			m_bAnnouncer );
	ini.SetValueB( "Options", "EventMode",			m_bEventMode );
	ini.SetValueI( "Options", "NumArcadeStages",	m_iNumArcadeStages );
	ini.SetValueB( "Options", "AutoPlay",			m_bAutoPlay );
	ini.SetValueF( "Options", "JudgeWindow",		m_fJudgeWindow );

	ini.SetValue( "Options", "SongFolders", join(",", m_asSongFolders) );

	ini.WriteFile();
}

int PrefsManager::GetStageIndex()
{
	return m_iCurrentStageIndex;
}

bool PrefsManager::IsFinalStage()
{
	if( PREFSMAN->m_bEventMode )
		return false;
	return m_iCurrentStageIndex == m_iNumArcadeStages-1;
}

bool PrefsManager::IsExtraStage()
{
	if( PREFSMAN->m_bEventMode )
		return false;
	return m_iCurrentStageIndex == m_iNumArcadeStages;
}

bool PrefsManager::IsExtraStage2()
{
	if( PREFSMAN->m_bEventMode )
		return false;
	return m_iCurrentStageIndex == m_iNumArcadeStages+1;
}

CString PrefsManager::GetStageText()
{
	if( IsFinalStage() )
		return "Final";
	else if( IsExtraStage() )
		return "Extra";
	else if( IsExtraStage2() )
		return "Extra 2";


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
	return ssprintf( "%d%s", iStageNo, sNumberSuffix );
}

D3DXCOLOR PrefsManager::GetStageColor()
{
	if( IsFinalStage() )
		return D3DXCOLOR(1,0.1f,0.1f,1);	// red
	else if( IsExtraStage() || IsExtraStage2() )
		return D3DXCOLOR(1,1,0.3f,1);		// yellow
	else
		return D3DXCOLOR(0.3f,1,0.3f,1);	// green
}


#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: PrefsManager

 Desc: See Header.

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
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
		false,
	},
	{	// PROFILE_LOW
		"Low",
		400,
		300,
		256,
		16,
		16,
		false,
		true,
	},
	{	// PROFILE_MEDIUM
		"Medium",
		640,
		480,
		512,
		16,
		16,
		false,
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
		true,
	},
	{	// PROFILE_CUSTOM
		"Custom",
		640,
		480,
		512,
		16,
		16,
		false,
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
		return;		// load nothing
		//FatalError( "could not read config file" );
	}

	CMapStringToString* pKey;
	CString name_string, value_string;
	

	pKey = ini.GetKeyPointer( "GameOptions" );
	if( pKey )
	{
		for( POSITION pos = pKey->GetStartPosition(); pos != NULL; )
		{
			pKey->GetNextAssoc( pos, name_string, value_string );

			if( name_string == "IgnoreJoyAxes" )		m_GameOptions.m_bIgnoreJoyAxes	= ( value_string == "1" );
			if( name_string == "ShowFPS" )				m_GameOptions.m_bShowFPS		= ( value_string == "1" );
			if( name_string == "UseRandomVis" )			m_GameOptions.m_bUseRandomVis	= ( value_string == "1" );
			if( name_string == "SkipCaution" )			m_GameOptions.m_bSkipCaution	= ( value_string == "1" );
			if( name_string == "Announcer" )			m_GameOptions.m_bAnnouncer		= ( value_string == "1" );
		}
	}

	pKey = ini.GetKeyPointer( "GraphicOptions" );
	if( pKey )
	{
		for( POSITION pos = pKey->GetStartPosition(); pos != NULL; )
		{
			pKey->GetNextAssoc( pos, name_string, value_string );

			if( name_string == "Windowed" )			m_bWindowed		= ( value_string == "1" );
			if( name_string == "Profile" )
			{
				if( value_string == "super low" )	m_GraphicProfile = PROFILE_SUPER_LOW;
				else if( value_string == "low" )	m_GraphicProfile = PROFILE_LOW;
				else if( value_string == "medium" )	m_GraphicProfile = PROFILE_MEDIUM;
				else if( value_string == "high" )	m_GraphicProfile = PROFILE_HIGH;
				else if( value_string == "custom" )	m_GraphicProfile = PROFILE_CUSTOM;
				else								m_GraphicProfile = PROFILE_MEDIUM;
			}

			GraphicProfileOptions* pGPO = GetCustomGraphicProfileOptions();
			if( name_string == "Width" )			pGPO->m_dwWidth				= atoi( value_string );
			if( name_string == "Height" )			pGPO->m_dwHeight			= atoi( value_string );
			if( name_string == "MaxTextureSize" )	pGPO->m_dwMaxTextureSize	= atoi( value_string );
			if( name_string == "DisplayColor" )		pGPO->m_dwDisplayColor		= atoi( value_string );
			if( name_string == "TextureColor" )		pGPO->m_dwTextureColor		= atoi( value_string );
			if( name_string == "Shadows" )			pGPO->m_bShadows			= ( value_string == "1" );
			if( name_string == "Backgrounds" )		pGPO->m_bBackgrounds		= ( value_string == "1" );
		}
	}

}


void PrefsManager::SavePrefsToDisk()
{
	IniFile ini;
	ini.SetPath( "StepMania.ini" );
//	ini.ReadFile();		// don't read the file so that we overwrite everything there


	// save the GameOptions
	ini.SetValue( "GraphicOptions", "Windowed",	m_bWindowed ? "1":"0" );
	switch( m_GraphicProfile )
	{
	case PROFILE_SUPER_LOW:	ini.SetValue( "GraphicOptions", "Profile",	"super low" );	break;
	case PROFILE_LOW:		ini.SetValue( "GraphicOptions", "Profile",	"low" );		break;
	case PROFILE_MEDIUM:	ini.SetValue( "GraphicOptions", "Profile",	"medium" );		break;
	case PROFILE_HIGH:		ini.SetValue( "GraphicOptions", "Profile",	"high" );		break;
	case PROFILE_CUSTOM:	ini.SetValue( "GraphicOptions", "Profile",	"custom" );		break;
	default:	ASSERT( false );
	}


	GraphicProfileOptions* pGPO = GetCustomGraphicProfileOptions();

	ini.SetValue( "GraphicOptions", "Width",			ssprintf("%d", pGPO->m_dwWidth) );
	ini.SetValue( "GraphicOptions", "Height",			ssprintf("%d", pGPO->m_dwWidth) );
	ini.SetValue( "GraphicOptions", "MaxTextureSize",	ssprintf("%d", pGPO->m_dwMaxTextureSize) );
	ini.SetValue( "GraphicOptions", "DisplayColor",		ssprintf("%d", pGPO->m_dwDisplayColor) );
	ini.SetValue( "GraphicOptions", "TextureColor",		ssprintf("%d", pGPO->m_dwTextureColor) );
	ini.SetValue( "GraphicOptions", "Shadows",			pGPO->m_bShadows ? "1":"0" );
	ini.SetValue( "GraphicOptions", "Backgrounds",		pGPO->m_bBackgrounds ? "1":"0" );


	ini.SetValue( "GameOptions", "IgnoreJoyAxes",		m_GameOptions.m_bIgnoreJoyAxes ? "1":"0" );
	ini.SetValue( "GameOptions", "ShowFPS",				m_GameOptions.m_bShowFPS ? "1":"0" );
	ini.SetValue( "GameOptions", "UseRandomVis",		m_GameOptions.m_bUseRandomVis ? "1":"0" );
	ini.SetValue( "GameOptions", "SkipCaution",			m_GameOptions.m_bSkipCaution ? "1":"0" );
	ini.SetValue( "GameOptions", "Announcer",			m_GameOptions.m_bAnnouncer ? "1":"0" );



	ini.WriteFile();
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


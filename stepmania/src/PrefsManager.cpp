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




PrefsManager::PrefsManager()
{
	m_SongSortOrder = SORT_GROUP;
	m_iCurrentStage = 1;

	for( int p=0; p<NUM_PLAYERS; p++ )
		m_PreferredDifficultyClass[p] = CLASS_EASY;

	ReadPrefsFromDisk();
}


PrefsManager::~PrefsManager()
{
	// don't worry about releasing the Song array.  Let the OS do it :-)
	SavePrefsToDisk();
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

			if( name_string == "Windowed" )				m_GraphicOptions.m_bWindowed		= ( value_string == "1" );
			if( name_string == "Profile" )
			{
				if( value_string == "super low" )	m_GraphicOptions.m_Profile = PROFILE_SUPER_LOW;
				else if( value_string == "low" )	m_GraphicOptions.m_Profile = PROFILE_LOW;
				else if( value_string == "medium" )	m_GraphicOptions.m_Profile = PROFILE_MEDIUM;
				else if( value_string == "high" )	m_GraphicOptions.m_Profile = PROFILE_HIGH;
				else if( value_string == "custom" )	m_GraphicOptions.m_Profile = PROFILE_CUSTOM;
				else								m_GraphicOptions.m_Profile = PROFILE_MEDIUM;
			}
			if( name_string == "Resolution" )			m_GraphicOptions.m_iResolution		= atoi( value_string );
			if( name_string == "MaxTextureSize" )		m_GraphicOptions.m_iMaxTextureSize	= atoi( value_string );
			if( name_string == "DisplayColor" )			m_GraphicOptions.m_iDisplayColor	= atoi( value_string );
			if( name_string == "TextureColor" )			m_GraphicOptions.m_iTextureColor	= atoi( value_string );
			if( name_string == "Shadows" )				m_GraphicOptions.m_bShadows			= ( value_string == "1" );
			if( name_string == "30fpsLock" )			m_GraphicOptions.m_b30fpsLock		= ( value_string == "1" );
			if( name_string == "Backgrounds" )			m_GraphicOptions.m_bBackgrounds		= ( value_string == "1" );
		}
	}

}


void PrefsManager::SavePrefsToDisk()
{
	IniFile ini;
	ini.SetPath( "StepMania.ini" );
//	ini.ReadFile();		// don't read the file so that we overwrite everything there


	// save the GameOptions
	ini.SetValue( "GraphicOptions", "Windowed",			m_GraphicOptions.m_bWindowed ? "1":"0" );
	switch( m_GraphicOptions.m_Profile )
	{
	case PROFILE_SUPER_LOW:	ini.SetValue( "GraphicOptions", "Profile",	"super low" );	break;
	case PROFILE_LOW:		ini.SetValue( "GraphicOptions", "Profile",	"low" );		break;
	case PROFILE_MEDIUM:	ini.SetValue( "GraphicOptions", "Profile",	"medium" );		break;
	case PROFILE_HIGH:		ini.SetValue( "GraphicOptions", "Profile",	"high" );		break;
	case PROFILE_CUSTOM:	ini.SetValue( "GraphicOptions", "Profile",	"custom" );		break;
	default:	ASSERT( false );
	}
	ini.SetValue( "GraphicOptions", "Resolution",		ssprintf("%d", m_GraphicOptions.m_iResolution) );
	ini.SetValue( "GraphicOptions", "MaxTextureSize",	ssprintf("%d", m_GraphicOptions.m_iMaxTextureSize) );
	ini.SetValue( "GraphicOptions", "DisplayColor",		ssprintf("%d", m_GraphicOptions.m_iDisplayColor) );
	ini.SetValue( "GraphicOptions", "TextureColor",		ssprintf("%d", m_GraphicOptions.m_iTextureColor) );
	ini.SetValue( "GraphicOptions", "Shadows",			m_GraphicOptions.m_bShadows ? "1":"0" );
	ini.SetValue( "GraphicOptions", "30fpsLock",		m_GraphicOptions.m_b30fpsLock ? "1":"0" );
	ini.SetValue( "GraphicOptions", "Backgrounds",		m_GraphicOptions.m_bBackgrounds ? "1":"0" );


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


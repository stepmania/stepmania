#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenAppearanceOptions

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenAppearanceOptions.h"
#include "RageTextureManager.h"
#include "RageUtil.h"
#include "RageMusic.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "GameConstantsAndTypes.h"
#include "StepMania.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "AnnouncerManager.h"
#include "GameManager.h"
#include "GameState.h"



enum {
	AO_ANNOUNCER = 0,
	AO_THEME,
	AO_SKIN,
	AO_HOWTOPLAY,
	NUM_APPEARANCE_OPTIONS_LINES
};

OptionRowData g_AppearanceOptionsLines[NUM_APPEARANCE_OPTIONS_LINES] = {
	{ "Announcer",		1, {"OFF"} },	// fill this in on ImportOptions()
	{ "Theme",			0, {""} },		// fill this in on ImportOptions()
	{ "Note\nSkin",		0, {""} },		// fill this in on ImportOptions()
	{ "How To\nPlay",	2, {"SKIP","SHOW"} },
};

ScreenAppearanceOptions::ScreenAppearanceOptions() :
	ScreenOptions(
		THEME->GetPathTo("BGAnimations","appearance options"),
		THEME->GetPathTo("Graphics","appearance options page"),
		THEME->GetPathTo("Graphics","appearance options top edge")
		)
{
	LOG->Trace( "ScreenAppearanceOptions::ScreenAppearanceOptions()" );

	// fill g_InputOptionsLines with explanation text
	for( int i=0; i<NUM_APPEARANCE_OPTIONS_LINES; i++ )
	{
		CString sLineName = g_AppearanceOptionsLines[i].szTitle;
		sLineName.Replace("\n","");
		sLineName.Replace(" ","");
		strcpy( g_AppearanceOptionsLines[i].szExplanation, THEME->GetMetric("ScreenAppearanceOptions",sLineName) );
	}

	Init( 
		INPUTMODE_BOTH, 
		g_AppearanceOptionsLines, 
		NUM_APPEARANCE_OPTIONS_LINES,
		false );
	m_Menu.SetTimer( 99 );
	m_Menu.StopTimer();

	MUSIC->LoadAndPlayIfNotAlready( THEME->GetPathTo("Sounds","appearance options music") );
}

void ScreenAppearanceOptions::ImportOptions()
{
	//
	// fill in announcer names
	//
	CStringArray arrayAnnouncerNames;
	ANNOUNCER->GetAnnouncerNames( arrayAnnouncerNames );

	m_OptionRowData[AO_ANNOUNCER].iNumOptions	=	arrayAnnouncerNames.size() + 1; 
	unsigned i;
	for( i=0; i<arrayAnnouncerNames.size(); i++ )
		strcpy( m_OptionRowData[AO_ANNOUNCER].szOptionsText[i+1], arrayAnnouncerNames[i] ); 


	// highlight currently selected announcer
	m_iSelectedOption[0][AO_ANNOUNCER] = -1;
	for( i=1; i<m_OptionRowData[AO_ANNOUNCER].iNumOptions; i++ )
	{
		if( 0==stricmp(m_OptionRowData[AO_ANNOUNCER].szOptionsText[i], ANNOUNCER->GetCurAnnouncerName()) )
		{
			m_iSelectedOption[0][AO_ANNOUNCER] = i;
			break;
		}
	}
	if( m_iSelectedOption[0][AO_ANNOUNCER] == -1 )
		m_iSelectedOption[0][AO_ANNOUNCER] = 0;


	//
	// fill in theme names
	//
	CStringArray arrayThemeNames;
	THEME->GetThemeNamesForCurGame( arrayThemeNames );

	m_OptionRowData[AO_THEME].iNumOptions	=	arrayThemeNames.size(); 
	
	for( i=0; i<arrayThemeNames.size(); i++ )
		strcpy( m_OptionRowData[AO_THEME].szOptionsText[i], arrayThemeNames[i] ); 


	// highlight currently selected theme
	m_iSelectedOption[0][AO_THEME] = -1;
	for( i=0; i<m_OptionRowData[AO_THEME].iNumOptions; i++ )
	{
		if( 0==stricmp(m_OptionRowData[AO_THEME].szOptionsText[i], THEME->GetCurThemeName()) )
		{
			m_iSelectedOption[0][AO_THEME] = i;
			break;
		}
	}
	if( m_iSelectedOption[0][AO_THEME] == -1 )
		m_iSelectedOption[0][AO_THEME] = 0;


	//
	// fill in skin names
	//
	CStringArray arraySkinNames;
	GAMEMAN->GetNoteSkinNames( arraySkinNames );

	m_OptionRowData[AO_SKIN].iNumOptions	=	arraySkinNames.size(); 
	
	for( i=0; i<arraySkinNames.size(); i++ )
		strcpy( m_OptionRowData[AO_SKIN].szOptionsText[i], arraySkinNames[i] ); 

	// highlight currently selected skin
	m_iSelectedOption[0][AO_SKIN] = -1;
	for( i=0; i<m_OptionRowData[AO_SKIN].iNumOptions; i++ )
	{
		if( 0==stricmp(m_OptionRowData[AO_SKIN].szOptionsText[i], GAMEMAN->GetCurNoteSkin()) )
		{
			m_iSelectedOption[0][AO_SKIN] = i;
			break;
		}
	}
	if( m_iSelectedOption[0][AO_SKIN] == -1 )
		m_iSelectedOption[0][AO_SKIN] = 0;

	m_iSelectedOption[0][AO_HOWTOPLAY]					= PREFSMAN->m_bHowToPlay? 1:0;
}

void ScreenAppearanceOptions::ExportOptions()
{
	PREFSMAN->SaveGamePrefsToDisk();
	PREFSMAN->SaveGlobalPrefsToDisk();

	int iSelectedAnnouncer = m_iSelectedOption[0][AO_ANNOUNCER];
	CString sNewAnnouncer = m_OptionRowData[AO_ANNOUNCER].szOptionsText[iSelectedAnnouncer];
	if( iSelectedAnnouncer == 0 )
		sNewAnnouncer = "";
	ANNOUNCER->SwitchAnnouncer( sNewAnnouncer );

	int iSelectedTheme = m_iSelectedOption[0][AO_THEME];
	CString sNewTheme = m_OptionRowData[AO_THEME].szOptionsText[iSelectedTheme];
	THEME->SwitchTheme( sNewTheme );

	int iSelectedSkin = m_iSelectedOption[0][AO_SKIN];
	CString sNewSkin = m_OptionRowData[AO_SKIN].szOptionsText[iSelectedSkin];
	GAMEMAN->SwitchNoteSkin( sNewSkin );

	PREFSMAN->m_bHowToPlay				= !!m_iSelectedOption[0][AO_HOWTOPLAY];

	PREFSMAN->SaveGamePrefsToDisk();
	PREFSMAN->SaveGlobalPrefsToDisk();
}

void ScreenAppearanceOptions::GoToPrevState()
{
	GoToNextState();
}

void ScreenAppearanceOptions::GoToNextState()
{
	PREFSMAN->SaveGamePrefsToDisk();
	SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
}






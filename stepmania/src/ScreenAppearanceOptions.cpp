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
#include <assert.h>
#include "RageTextureManager.h"
#include "RageUtil.h"
#include "RageMusic.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "ScreenOptions.h"
#include "ScreenTitleMenu.h"
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
	NUM_APPEARANCE_OPTIONS_LINES
};

OptionLineData g_AppearanceOptionsLines[NUM_APPEARANCE_OPTIONS_LINES] = {
	{ "Announcer",	1, {"OFF"} },	// fill this in on ImportOptions()
	{ "Theme",		0, {""} },		// fill this in on ImportOptions()
	{ "Note Skin",	0, {""} },		// fill this in on ImportOptions()
};

ScreenAppearanceOptions::ScreenAppearanceOptions() :
	ScreenOptions(
		THEME->GetPathTo("Graphics","appearance options background"),
		THEME->GetPathTo("Graphics","appearance options page"),
		THEME->GetPathTo("Graphics","appearance options top edge")
		)
{
	LOG->Trace( "ScreenAppearanceOptions::ScreenAppearanceOptions()" );

	Init( 
		INPUTMODE_BOTH, 
		g_AppearanceOptionsLines, 
		NUM_APPEARANCE_OPTIONS_LINES
		);
	m_Menu.StopTimer();
}

void ScreenAppearanceOptions::ImportOptions()
{
	//
	// fill in announcer names
	//
	CStringArray arrayAnnouncerNames;
	ANNOUNCER->GetAnnouncerNames( arrayAnnouncerNames );

	m_OptionLineData[AO_ANNOUNCER].iNumOptions	=	arrayAnnouncerNames.GetSize() + 1; 
	
	for( int i=0; i<arrayAnnouncerNames.GetSize(); i++ )
		strcpy( m_OptionLineData[AO_ANNOUNCER].szOptionsText[i+1], arrayAnnouncerNames[i] ); 


	// highlight currently selected announcer
	m_iSelectedOption[0][AO_ANNOUNCER] = -1;
	for( i=1; i<m_OptionLineData[AO_ANNOUNCER].iNumOptions; i++ )
	{
		if( 0==stricmp(m_OptionLineData[AO_ANNOUNCER].szOptionsText[i], ANNOUNCER->GetCurAnnouncerName()) )
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

	m_OptionLineData[AO_THEME].iNumOptions	=	arrayThemeNames.GetSize(); 
	
	for( i=0; i<arrayThemeNames.GetSize(); i++ )
		strcpy( m_OptionLineData[AO_THEME].szOptionsText[i], arrayThemeNames[i] ); 


	// highlight currently selected theme
	m_iSelectedOption[0][AO_THEME] = -1;
	for( i=0; i<m_OptionLineData[AO_THEME].iNumOptions; i++ )
	{
		if( 0==stricmp(m_OptionLineData[AO_THEME].szOptionsText[i], THEME->GetCurThemeName()) )
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

	m_OptionLineData[AO_SKIN].iNumOptions	=	arraySkinNames.GetSize(); 
	
	for( i=0; i<arraySkinNames.GetSize(); i++ )
		strcpy( m_OptionLineData[AO_SKIN].szOptionsText[i], arraySkinNames[i] ); 

	// highlight currently selected skin
	m_iSelectedOption[0][AO_SKIN] = -1;
	for( i=0; i<m_OptionLineData[AO_SKIN].iNumOptions; i++ )
	{
		if( 0==stricmp(m_OptionLineData[AO_SKIN].szOptionsText[i], GAMEMAN->GetCurNoteSkin()) )
		{
			m_iSelectedOption[0][AO_SKIN] = i;
			break;
		}
	}
	if( m_iSelectedOption[0][AO_SKIN] == -1 )
		m_iSelectedOption[0][AO_SKIN] = 0;
}

void ScreenAppearanceOptions::ExportOptions()
{
	int iSelectedAnnouncer = m_iSelectedOption[0][AO_ANNOUNCER];
	CString sNewAnnouncer = m_OptionLineData[AO_ANNOUNCER].szOptionsText[iSelectedAnnouncer];
	if( iSelectedAnnouncer == 0 )
		sNewAnnouncer = "";
	ANNOUNCER->SwitchAnnouncer( sNewAnnouncer );

	int iSelectedTheme = m_iSelectedOption[0][AO_THEME];
	CString sNewTheme = m_OptionLineData[AO_THEME].szOptionsText[iSelectedTheme];
	THEME->SwitchTheme( sNewTheme );

	int iSelectedSkin = m_iSelectedOption[0][AO_SKIN];
	CString sNewSkin = m_OptionLineData[AO_SKIN].szOptionsText[iSelectedSkin];
	GAMEMAN->SwitchNoteSkin( sNewSkin );

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
	SCREENMAN->SetNewScreen( new ScreenTitleMenu );
}






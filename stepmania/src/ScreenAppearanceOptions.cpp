#include "global.h"
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
#include "RageSoundManager.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "GameConstantsAndTypes.h"
#include "StepMania.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "AnnouncerManager.h"
#include "NoteSkinManager.h"
#include "GameState.h"
#include "ThemeManager.h"


enum {
	AO_ANNOUNCER = 0,
	AO_THEME,
//	AO_DIFF_SELECT,
	AO_SKIN,
	AO_INSTRUCTIONS,
	AO_CAUTION,
	AO_SELECT_GROUP,
	AO_WHEEL_SECTIONS,
	AO_SHOW_TRANSLATIONS,
	NUM_APPEARANCE_OPTIONS_LINES
};

OptionRowData g_AppearanceOptionsLines[NUM_APPEARANCE_OPTIONS_LINES] = {
	{ "Announcer",		    1, {"OFF"} },	// fill this in on ImportOptions()
	{ "Theme",			    0, {""} },		// fill this in on ImportOptions()
//	{ "Difficulty\nSelect", 2, {"DDR Extreme", "Normal"} },
	{ "Note\nSkin",		    0, {""} },		// fill this in on ImportOptions()
	{ "How To\nPlay",	    2, {"SKIP","SHOW"} },
	{ "Caution",		    2, {"SKIP","SHOW"} },
	{ "Song\nGroup",	    2, {"ALL MUSIC","CHOOSE"} },
	{ "Wheel\nSections",    3, {"NEVER","ALWAYS", "ABC ONLY"} },
	{ "Translations",	    2, {"NATIVE","TRANSLITERATE"} },
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

	SOUNDMAN->PlayMusic( THEME->GetPathTo("Sounds","appearance options music") );
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
	NOTESKIN->GetNoteSkinNames( arraySkinNames );

	m_OptionRowData[AO_SKIN].iNumOptions    =       arraySkinNames.size();

	for( i=0; i<arraySkinNames.size(); i++ )
	strcpy( m_OptionRowData[AO_SKIN].szOptionsText[i], arraySkinNames[i] );

	// highlight currently selected skin
	m_iSelectedOption[0][AO_SKIN] = -1;
	for( i=0; i<m_OptionRowData[AO_SKIN].iNumOptions; i++ )
	{
		if( 0==stricmp(m_OptionRowData[AO_SKIN].szOptionsText[i], PREFSMAN->m_sDefaultNoteSkin) )
		{
			m_iSelectedOption[0][AO_SKIN] = i;
			break;
		}
	}
	if( m_iSelectedOption[0][AO_SKIN] == -1 )
		m_iSelectedOption[0][AO_SKIN] = 0;

	m_iSelectedOption[0][AO_INSTRUCTIONS]				= PREFSMAN->m_bInstructions? 1:0;
	m_iSelectedOption[0][AO_CAUTION]					= PREFSMAN->m_bShowDontDie? 1:0;
//	m_iSelectedOption[0][AO_DIFF_SELECT]				= PREFSMAN->m_bDDRExtremeDifficultySelect? 1:0;
	m_iSelectedOption[0][AO_SELECT_GROUP]				= PREFSMAN->m_bShowSelectGroup? 1:0;
	m_iSelectedOption[0][AO_WHEEL_SECTIONS]				= (int)PREFSMAN->m_MusicWheelUsesSections;
	m_iSelectedOption[0][AO_SHOW_TRANSLATIONS]			= PREFSMAN->m_bShowTranslations;
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

	PREFSMAN->m_sDefaultNoteSkin				= sNewSkin;
//	PREFSMAN->m_bDDRExtremeDifficultySelect		= !!m_iSelectedOption[0][AO_DIFF_SELECT];
	PREFSMAN->m_bInstructions					= !!m_iSelectedOption[0][AO_INSTRUCTIONS];
	PREFSMAN->m_bShowDontDie					= !!m_iSelectedOption[0][AO_CAUTION];
	PREFSMAN->m_bShowSelectGroup				= !!m_iSelectedOption[0][AO_SELECT_GROUP];
	(int&)PREFSMAN->m_MusicWheelUsesSections	= m_iSelectedOption[0][AO_WHEEL_SECTIONS];
	PREFSMAN->m_bShowTranslations				= !!m_iSelectedOption[0][AO_SHOW_TRANSLATIONS];

	PREFSMAN->SaveGamePrefsToDisk();
	PREFSMAN->SaveGlobalPrefsToDisk();
}

void ScreenAppearanceOptions::GoToPrevState()
{
	SCREENMAN->SetNewScreen( "ScreenOptionsMenu" );
}

void ScreenAppearanceOptions::GoToNextState()
{
	PREFSMAN->SaveGamePrefsToDisk();
	GoToPrevState();
}






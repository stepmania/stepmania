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
	AO_SKIN,
	AO_INSTRUCTIONS,
	AO_CAUTION,
	AO_DANCE_POINTS_FOR_ONI,
	AO_SELECT_GROUP,
	AO_WHEEL_SECTIONS,
	AO_SHOW_TRANSLATIONS,
	AO_SHOW_LYRICS,
	NUM_APPEARANCE_OPTIONS_LINES
};


OptionRow g_AppearanceOptionsLines[NUM_APPEARANCE_OPTIONS_LINES] = {
	OptionRow( "Announcer"			 ),
	OptionRow( "Theme"				 ),
	OptionRow( "Default\nNoteSkin"	 ),
	OptionRow( "How To\nPlay",		"SKIP","SHOW"),
	OptionRow( "Caution",			"SKIP","SHOW"),
	OptionRow( "Oni Score\nDisplay","PERCENT","DANCE POINTS"),
	OptionRow( "Song\nGroup",		"ALL MUSIC","CHOOSE"),
	OptionRow( "Wheel\nSections",	"NEVER","ALWAYS", "ABC ONLY"),
	OptionRow( "Translations",		"NATIVE","TRANSLITERATE"),
	OptionRow( "Lyrics",			"HIDE","SHOW"),
};

ScreenAppearanceOptions::ScreenAppearanceOptions() :
	ScreenOptions("ScreenAppearanceOptions",false)
{
	LOG->Trace( "ScreenAppearanceOptions::ScreenAppearanceOptions()" );

	Init( 
		INPUTMODE_BOTH, 
		g_AppearanceOptionsLines, 
		NUM_APPEARANCE_OPTIONS_LINES,
		false, true );
	m_Menu.m_MenuTimer.Disable();

	SOUNDMAN->PlayMusic( THEME->GetPathTo("Sounds","ScreenAppearanceOptions music") );
}

void ScreenAppearanceOptions::ImportOptions()
{
	unsigned i;

	//
	// fill in announcer names
	//
	CStringArray arrayAnnouncerNames;
	ANNOUNCER->GetAnnouncerNames( arrayAnnouncerNames );

	m_OptionRow[AO_ANNOUNCER].choices.clear();
	m_OptionRow[AO_ANNOUNCER].choices.push_back( "OFF" );
	for( i=0; i<arrayAnnouncerNames.size() && i<MAX_OPTIONS_PER_LINE; i++ )
	{
		m_OptionRow[AO_ANNOUNCER].choices.push_back( arrayAnnouncerNames[i] ); 
	}

	// highlight currently selected announcer
	m_iSelectedOption[0][AO_ANNOUNCER] = 0;
	for( i=1; i<m_OptionRow[AO_ANNOUNCER].choices.size(); i++ )
		if( 0==stricmp(m_OptionRow[AO_ANNOUNCER].choices[i], ANNOUNCER->GetCurAnnouncerName()) )
			m_iSelectedOption[0][AO_ANNOUNCER] = i;


	//
	// fill in theme names
	//
	CStringArray arrayThemeNames;
	THEME->GetThemeNamesForCurGame( arrayThemeNames );

	m_OptionRow[AO_THEME].choices.clear();
	
	for( i=0; i<arrayThemeNames.size() && i<MAX_OPTIONS_PER_LINE; i++ )
	{
		m_OptionRow[AO_THEME].choices.push_back( arrayThemeNames[i] ); 
	}

	// highlight currently selected theme
	m_iSelectedOption[0][AO_THEME] = 0;
	for( i=0; i<m_OptionRow[AO_THEME].choices.size(); i++ )
		if( 0==stricmp(m_OptionRow[AO_THEME].choices[i], THEME->GetCurThemeName()) )
			m_iSelectedOption[0][AO_THEME] = i;


	//
	// fill in skin names
	//
	CStringArray arraySkinNames;
	NOTESKIN->GetNoteSkinNames( arraySkinNames );

	m_OptionRow[AO_SKIN].choices.clear();

	for( i=0; i<arraySkinNames.size() && i<MAX_OPTIONS_PER_LINE; i++ )
	{
		arraySkinNames[i].MakeUpper();
		m_OptionRow[AO_SKIN].choices.push_back( arraySkinNames[i] ); 
	}

	// highlight currently selected skin
	m_iSelectedOption[0][AO_SKIN] = 0;
	for( i=0; i<m_OptionRow[AO_SKIN].choices.size(); i++ )
		if( 0==stricmp(m_OptionRow[AO_SKIN].choices[i], PREFSMAN->m_sDefaultNoteSkin) )
			m_iSelectedOption[0][AO_SKIN] = i;


	m_iSelectedOption[0][AO_INSTRUCTIONS]				= PREFSMAN->m_bInstructions? 1:0;
	m_iSelectedOption[0][AO_CAUTION]					= PREFSMAN->m_bShowDontDie? 1:0;
	m_iSelectedOption[0][AO_DANCE_POINTS_FOR_ONI]		= PREFSMAN->m_bDancePointsForOni? 1:0;
	m_iSelectedOption[0][AO_SELECT_GROUP]				= PREFSMAN->m_bShowSelectGroup? 1:0;
	m_iSelectedOption[0][AO_WHEEL_SECTIONS]				= (int)PREFSMAN->m_MusicWheelUsesSections;
	m_iSelectedOption[0][AO_SHOW_TRANSLATIONS]			= PREFSMAN->m_bShowTranslations;
	m_iSelectedOption[0][AO_SHOW_LYRICS]				= PREFSMAN->m_bShowLyrics;
}

void ScreenAppearanceOptions::ExportOptions()
{
	PREFSMAN->SaveGamePrefsToDisk();
	PREFSMAN->SaveGlobalPrefsToDisk();

	int iSelectedAnnouncer = m_iSelectedOption[0][AO_ANNOUNCER];
	CString sNewAnnouncer = m_OptionRow[AO_ANNOUNCER].choices[iSelectedAnnouncer];
	if( iSelectedAnnouncer == 0 )
		sNewAnnouncer = "";
	ANNOUNCER->SwitchAnnouncer( sNewAnnouncer );

	int iSelectedTheme = m_iSelectedOption[0][AO_THEME];
	CString sNewTheme = m_OptionRow[AO_THEME].choices[iSelectedTheme];
	THEME->SwitchTheme( sNewTheme );

    int iSelectedSkin = m_iSelectedOption[0][AO_SKIN];
    CString sNewSkin = m_OptionRow[AO_SKIN].choices[iSelectedSkin];

	PREFSMAN->m_sDefaultNoteSkin				= sNewSkin;
	PREFSMAN->m_bInstructions					= !!m_iSelectedOption[0][AO_INSTRUCTIONS];
	PREFSMAN->m_bShowDontDie					= !!m_iSelectedOption[0][AO_CAUTION];
	PREFSMAN->m_bShowSelectGroup				= !!m_iSelectedOption[0][AO_SELECT_GROUP];
	(int&)PREFSMAN->m_MusicWheelUsesSections	= m_iSelectedOption[0][AO_WHEEL_SECTIONS];
	PREFSMAN->m_bShowTranslations				= !!m_iSelectedOption[0][AO_SHOW_TRANSLATIONS];
	PREFSMAN->m_bShowLyrics						= !!m_iSelectedOption[0][AO_SHOW_LYRICS];
	PREFSMAN->m_bDancePointsForOni				= !!m_iSelectedOption[0][AO_DANCE_POINTS_FOR_ONI];

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






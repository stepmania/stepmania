#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenAutogenOptions

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenAutogenOptions.h"
#include "RageUtil.h"
#include "RageSounds.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "GameConstantsAndTypes.h"
#include "StepMania.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "ThemeManager.h"


enum {
	AO_AUTOGEN_MISSING_TYPES,
	AO_AUTOGEN_GROUP_COURSES,
	NUM_AUTOGEN_OPTIONS_LINES
};

OptionRow g_AutogenOptionsLines[NUM_AUTOGEN_OPTIONS_LINES] = {
	OptionRow( "Autogen\nMissing Types",	"OFF","ON" ),
	OptionRow( "Autogen\nGroup Courses",	"OFF","ON" ),
};

ScreenAutogenOptions::ScreenAutogenOptions() :
	ScreenOptions("ScreenAutogenOptions",false)
{
	LOG->Trace( "ScreenAutogenOptions::ScreenAutogenOptions()" );

	Init( 
		INPUTMODE_BOTH, 
		g_AutogenOptionsLines, 
		NUM_AUTOGEN_OPTIONS_LINES,
		false, true );
	m_Menu.m_MenuTimer.Disable();

	SOUND->PlayMusic( THEME->GetPathToS("ScreenAutogenOptions music") );
}

void ScreenAutogenOptions::ImportOptions()
{
	m_iSelectedOption[0][AO_AUTOGEN_MISSING_TYPES]	= PREFSMAN->m_bAutogenMissingTypes ? 1:0;
	m_iSelectedOption[0][AO_AUTOGEN_GROUP_COURSES]	= PREFSMAN->m_bAutogenGroupCourses ? 1 : 0;
}

void ScreenAutogenOptions::ExportOptions()
{
	PREFSMAN->m_bAutogenMissingTypes	= m_iSelectedOption[0][AO_AUTOGEN_MISSING_TYPES] == 1;
	PREFSMAN->m_bAutogenGroupCourses	= m_iSelectedOption[0][AO_AUTOGEN_GROUP_COURSES] == 1;
}

void ScreenAutogenOptions::GoToPrevState()
{
	SCREENMAN->SetNewScreen( "ScreenOptionsMenu" );
}

void ScreenAutogenOptions::GoToNextState()
{
	PREFSMAN->SaveGlobalPrefsToDisk();
	GoToPrevState();
}


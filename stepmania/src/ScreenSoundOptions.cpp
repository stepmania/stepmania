#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenSoundOptions

 Desc: See header.

 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Kevin Slaughter
-----------------------------------------------------------------------------
*/

#include "ScreenSoundOptions.h"
#include "RageTextureManager.h"
#include "RageUtil.h"
#include "RageSoundManager.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "GameConstantsAndTypes.h"
#include "StepMania.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "ThemeManager.h"


enum {
	SO_MASTER_VOLUME,
	NUM_SOUND_OPTIONS_LINES
};
OptionRowData g_SoundOptionsLines[NUM_SOUND_OPTIONS_LINES] = {
	{ "Master\nVolume",		10, {"0","1","2","3","4","5","6","7","8","9"} },
};

ScreenSoundOptions::ScreenSoundOptions() :
	ScreenOptions(	THEME->GetPathTo("BGAnimations","sound options"),
					THEME->GetPathTo("Graphics","sound options page"),
					THEME->GetPathTo("Graphics","sound options top edge") )
{
	LOG->Trace( "ScreenSoundOptions::ScreenSoundOptions()" );

	// fill g_InputOptionsLines with explanation text
	for( int i=0; i<NUM_SOUND_OPTIONS_LINES; i++ )
	{
		CString sLineName = g_SoundOptionsLines[i].szTitle;
		sLineName.Replace("\n","");
		sLineName.Replace(" ","");
		strcpy( g_SoundOptionsLines[i].szExplanation, THEME->GetMetric("ScreenSoundOptions",sLineName) );
	}

	Init( INPUTMODE_BOTH, g_SoundOptionsLines, NUM_SOUND_OPTIONS_LINES, false );
	m_Menu.StopTimer();

	SOUNDMAN->PlayMusic( THEME->GetPathTo("Sounds","sound options music") );
}

void ScreenSoundOptions::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	this->ExportOptions();
	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenSoundOptions::ImportOptions()
{
	m_iSelectedOption[0][SO_MASTER_VOLUME] = PREFSMAN->m_fSoundVolume;
}

void ScreenSoundOptions::ExportOptions()
{
	switch( m_iSelectedOption[0][SO_MASTER_VOLUME] )
	{
		case 0:SOUNDMAN->SetPrefs(0.10000f);PREFSMAN->m_fSoundVolume = 0.10000f;break;
		case 1:SOUNDMAN->SetPrefs(0.20000f);PREFSMAN->m_fSoundVolume = 0.20000f;break;
		case 2:SOUNDMAN->SetPrefs(0.30000f);PREFSMAN->m_fSoundVolume = 0.30000f;break;
		case 3:SOUNDMAN->SetPrefs(0.40000f);PREFSMAN->m_fSoundVolume = 0.40000f;break;
		case 4:SOUNDMAN->SetPrefs(0.50000f);PREFSMAN->m_fSoundVolume = 0.50000f;break;
		case 5:SOUNDMAN->SetPrefs(0.60000f);PREFSMAN->m_fSoundVolume = 0.60000f;break;
		case 6:SOUNDMAN->SetPrefs(0.70000f);PREFSMAN->m_fSoundVolume = 0.70000f;break;
		case 7:SOUNDMAN->SetPrefs(0.80000f);PREFSMAN->m_fSoundVolume = 0.80000f;break;
		case 8:SOUNDMAN->SetPrefs(0.90000f);PREFSMAN->m_fSoundVolume = 0.90000f;break;
		case 9:SOUNDMAN->SetPrefs(1.00000f);PREFSMAN->m_fSoundVolume = 1.00000f;break;
	}
}

void ScreenSoundOptions::GoToPrevState()
{
	SCREENMAN->SetNewScreen( "ScreenOptionsMenu" );
}

void ScreenSoundOptions::GoToNextState()
{
	PREFSMAN->SaveGlobalPrefsToDisk();
	GoToPrevState();
}
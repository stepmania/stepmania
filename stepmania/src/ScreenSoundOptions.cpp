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
	{ "Master\nVolume",		6, {"MUTE","20%","40%","60%","80%","100%"} },
};

ScreenSoundOptions::ScreenSoundOptions() :
	ScreenOptions("ScreenSoundOptions",false)
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
	m_Menu.m_MenuTimer.Disable();

	SOUNDMAN->PlayMusic( THEME->GetPathTo("Sounds","ScreenSoundOptions music") );
}

void ScreenSoundOptions::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	this->ExportOptions();
	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenSoundOptions::ImportOptions()
{
	float fVolPercent = PREFSMAN->m_fSoundVolume;
	m_iSelectedOption[0][SO_MASTER_VOLUME] = (int)(fVolPercent*5);
}

void ScreenSoundOptions::ExportOptions()
{
	float fVolPercent = m_iSelectedOption[0][SO_MASTER_VOLUME] / 5.f;
	SOUNDMAN->SetPrefs(fVolPercent);
	PREFSMAN->m_fSoundVolume = fVolPercent;
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


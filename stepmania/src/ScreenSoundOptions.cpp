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
OptionRow g_SoundOptionsLines[NUM_SOUND_OPTIONS_LINES] = {
	OptionRow( "Master\nVolume",		"MUTE","20%","40%","60%","80%","100%" ),
};

ScreenSoundOptions::ScreenSoundOptions() :
	ScreenOptions("ScreenSoundOptions",false)
{
	LOG->Trace( "ScreenSoundOptions::ScreenSoundOptions()" );

	Init( INPUTMODE_BOTH, g_SoundOptionsLines, NUM_SOUND_OPTIONS_LINES, false, true );
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


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
//	SO_MASTER_VOLUME,
	SO_PRELOAD_SOUND,
	NUM_SOUND_OPTIONS_LINES
};
OptionRow g_SoundOptionsLines[NUM_SOUND_OPTIONS_LINES] = {
	/* Err.  This shouldn't be here.  m_fSoundVolume is not the master volume,
	 * it's the internal attenuation before mixing; we don't have control over
	 * the master volume. m_fSoundVolume was never intended to be changed by
	 * users, except to troubleshoot clipping; that's why I didn't put it in
	 * the options to begin with. */
//	OptionRow( "Master\nVolume",		"MUTE","20%","40%","60%","80%","100%" ),
	OptionRow( "Preload\nSounds",		"NO","YES" ),
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
//	float fVolPercent = PREFSMAN->m_fSoundVolume;
//	m_iSelectedOption[0][SO_MASTER_VOLUME] = (int)(PREFSMAN->m_fSoundVolume*5);
	m_iSelectedOption[0][SO_PRELOAD_SOUND] = PREFSMAN->m_bSoundPreloadAll;
}

void ScreenSoundOptions::ExportOptions()
{
//	float fVolPercent = m_iSelectedOption[0][SO_MASTER_VOLUME] / 5.f;
//	SOUNDMAN->SetPrefs(fVolPercent);
//	PREFSMAN->m_fSoundVolume = fVolPercent;
	PREFSMAN->m_bSoundPreloadAll = !!m_iSelectedOption[0][SO_PRELOAD_SOUND];
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


#pragma once
/*
-----------------------------------------------------------------------------
 Class: PrefsManager

 Desc: Holds user-chosen preferences that are saved between sessions.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GameConstantsAndTypes.h"


#include "AnnouncerManager.h"
#include "ThemeManager.h"
#include "GameManager.h"


class PrefsManager
{
public:
	PrefsManager();
	~PrefsManager();

	enum BackgroundMode { BGMODE_OFF, BGMODE_ANIMATIONS, BGMODE_MOVIEVIS };

	// GameOptions (ARE saved between sessions)
	bool	m_bWindowed;
	int		m_iDisplayResolution;
	int		m_iTextureResolution;
	int		m_iRefreshRate;		// 0 means 'default'
	bool	m_bIgnoreJoyAxes;
	bool	m_bShowFPS;
	BackgroundMode	m_BackgroundMode;
	float	m_fBGBrightness;
	bool	m_bMenuTimer;
	bool	m_bEventMode;
	int		m_iNumArcadeStages;
	bool	m_bAutoPlay;
	float	m_fJudgeWindow;

	CStringArray m_asSongFolders;

	int GetDisplayHeight();

	void ReadGlobalPrefsFromDisk( bool bSwitchToLastPlayedGame );
	void SaveGlobalPrefsToDisk();


	// AppearanceOptions (ARE saved between sessions, and saved per game)
//	CString m_sAnnouncer;	// need to make sure to call ANNOUNCER->SwitchAnnouncer() when this changes
//	CString m_sTheme;		// need to make sure to call THEME->SwitchTheme() when this changes
//	CString m_sNoteSkin;	

	void ReadGamePrefsFromDisk();
	void SaveGamePrefsToDisk();
};


extern PrefsManager*	PREFSMAN;	// global and accessable from anywhere in our program

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

	enum BackgroundMode { BGMODE_OFF, BGMODE_ANIMATIONS, BGMODE_MOVIEVIS, BGMODE_RANDOMMOVIES };

	// GameOptions (ARE saved between sessions)
	bool	m_bWindowed;
	int		m_iDisplayResolution;
	int		m_iTextureResolution;
	int		m_iRefreshRate;		// 0 means 'default'
	bool	m_bShowStats;
	BackgroundMode	m_BackgroundMode;
	float	m_fBGBrightness;
	int 	m_iMovieDecodeMS;
	bool	m_bUseBGIfNoBanner;
	bool	m_bHiddenSongs;

	bool	m_bIgnoreJoyAxes;	
	bool	m_bOnlyDedicatedMenuButtons;
	bool	m_bMenuTimer;
	bool	m_bShowDanger;
	int		m_iNumArcadeStages;
	bool	m_bEventMode;
	float	m_fJudgeWindow;
	float	m_fLifeDifficultyScale;
	bool	m_bAutoPlay;
	bool	m_bDelayedEscape;
	
	CStringArray m_asAdditionalSongFolders;

	int GetDisplayHeight();

	void ReadGlobalPrefsFromDisk( bool bSwitchToLastPlayedGame );
	void SaveGlobalPrefsToDisk();


	void ReadGamePrefsFromDisk();
	void SaveGamePrefsToDisk();
};


extern PrefsManager*	PREFSMAN;	// global and accessable from anywhere in our program

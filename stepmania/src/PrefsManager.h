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
	bool			m_bWindowed;
	int				m_iDisplayResolution;
	int				m_iTextureResolution;
	int				m_iRefreshRate;		// 0 means 'default'
	bool			m_bShowStats;
	BackgroundMode	m_BackgroundMode;
	float			m_fBGBrightness;
	int 			m_iMovieDecodeMS;
	bool			m_bUseBGIfNoBanner;
	bool			m_bHiddenSongs;
	bool			m_bVsync;
	int				m_iUnloadTextureDelaySeconds;

	bool			m_bIgnoreJoyAxes;	
	bool			m_bOnlyDedicatedMenuButtons;
	bool			m_bMenuTimer;
	bool			m_bShowDanger;
	int				m_iNumArcadeStages;
	bool			m_bEventMode;
	float			m_fJudgeWindowSeconds;
	float			m_fLifeDifficultyScale;
	float			m_fJudgeWindowPerfectPercent;
	float			m_fJudgeWindowGreatPercent;
	float			m_fJudgeWindowGoodPercent;
	bool			m_bAutoPlay;
	bool			m_bDelayedEscape;
	bool			m_bHowToPlay;
	bool			m_bArcadeOptionsNavigation;
	bool			m_bCoinOpMode;
	
	CStringArray m_asAdditionalSongFolders;
	CString m_DWIPath;

	int GetDisplayHeight();

	void ReadGlobalPrefsFromDisk( bool bSwitchToLastPlayedGame );
	void SaveGlobalPrefsToDisk();


	void ReadGamePrefsFromDisk();
	void SaveGamePrefsToDisk();
};


extern PrefsManager*	PREFSMAN;	// global and accessable from anywhere in our program

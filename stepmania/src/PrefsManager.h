#ifndef PREFSMANAGER_H
#define PREFSMANAGER_H
/*
-----------------------------------------------------------------------------
 Class: PrefsManager

 Desc: Holds user-chosen preferences that are saved between sessions.

 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Chris Gomez
-----------------------------------------------------------------------------
*/

class PrefsManager
{
public:
	PrefsManager();
	~PrefsManager();

	// GameOptions (ARE saved between sessions)
	bool			m_bWindowed;
	int				m_iDisplayWidth;
	int				m_iDisplayHeight;
	int				m_iDisplayColorDepth;
	int				m_iTextureColorDepth;
	int				m_iMaxTextureResolution;
	int				m_iRefreshRate;
	bool			m_bShowStats;
	enum { BGMODE_OFF, BGMODE_ANIMATIONS, BGMODE_MOVIEVIS, BGMODE_RANDOMMOVIES } m_BackgroundMode;
	int				m_iNumBackgrounds;
	float			m_fBGBrightness;
	int 			m_iMovieDecodeMS;
	bool			m_bHiddenSongs;
	bool			m_bVsync;
	bool			m_bDelayedTextureDelete;
	bool			m_bDelayedScreenLoad;
	bool			m_bBannerCache;

	bool			m_bIgnoreJoyAxes;	
	bool			m_bOnlyDedicatedMenuButtons;
	bool			m_bMenuTimer;
	bool			m_bShowDanger;
	int				m_iNumArcadeStages;
	bool			m_bEventMode;
	float			m_fJudgeWindowScale;
	float			m_fLifeDifficultyScale;
	float			m_fJudgeWindowMarvelousSeconds;
	float			m_fJudgeWindowPerfectSeconds;
	float			m_fJudgeWindowGreatSeconds;
	float			m_fJudgeWindowGoodSeconds;
	float			m_fJudgeWindowBooSeconds;
	float			m_fJudgeWindowOKSeconds;
	bool			m_bAutoPlay;
	bool			m_bDelayedEscape;
	bool			m_bInstructions, m_bShowDontDie, m_bShowSelectGroup;
	bool			m_bShowNative;
	bool			m_bArcadeOptionsNavigation;
	enum			{ NEVER, ALWAYS, ABC_ONLY } m_MusicWheelUsesSections;
	int				m_iMusicWheelSwitchSpeed;
	bool			m_bEasterEggs;
	bool			m_bMarvelousTiming;
	int				m_iCoinMode;
	int				m_iCoinsPerCredit;
	bool			m_bJointPremium;
	bool			m_bPickExtraStage;
	float			m_fLongVerSongSeconds;
	float			m_fMarathonVerSongSeconds;
	enum Maybe { ASK = -1, NO = 0, YES = 1 };
	Maybe			m_ShowSongOptions;
	bool			m_bSoloSingle;
	bool			m_bDancePointsForOni;	//DDR-Extreme style dance points instead of max2 percent
	bool			m_bTimestamping;
	bool			m_bShowLyrics;
	bool			m_bAutogenMissingTypes;
	bool			m_bAutogenGroupCourses;
	bool			m_bBreakComboToGetItem;
	bool			m_bShowDancingCharacters;
	float			m_fDancePointsAccumulated;
	bool			m_bUseUnlockSystem;
	bool			m_bFirstRun;
	bool			m_bAutoMapJoysticks;
	float			m_fGlobalOffsetSeconds;

	/* 0 = no; 1 = yes; -1 = auto (do whatever is appropriate for the arch). */
	int				m_iBoostAppPriority;

	CStringArray	m_asAdditionalSongFolders;
	CString			m_DWIPath;

	CString			m_sLastSeenVideoDriver;
#if defined(WIN32)
	int				m_iLastSeenMemory;
#endif
	CString			m_sVideoRenderers;
	bool			m_bAntiAliasing;
	CString			m_sSoundDrivers;
	float			m_fSoundVolume;
	bool			m_bSoundPreloadAll;
	bool			m_bAllowUnacceleratedRenderer;

	/* Game-specific prefs: */
	CString			m_sDefaultModifiers;

	void ReadGlobalPrefsFromDisk( bool bSwitchToLastPlayedGame );
	void SaveGlobalPrefsToDisk();


	void ReadGamePrefsFromDisk();
	void SaveGamePrefsToDisk();
};


extern PrefsManager*	PREFSMAN;	// global and accessable from anywhere in our program

#endif

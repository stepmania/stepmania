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
#include "SongOptions.h"

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
	bool			m_bShowTranslations;
	bool			m_bArcadeOptionsNavigation;
	bool			m_bCoinOpMode;
	enum			{ NEVER, ALWAYS, ABC_ONLY } m_MusicWheelUsesSections;
	int				m_iMusicWheelSwitchSpeed;
	bool			m_bChangeBannersWhenFast;
	bool			m_bDDRExtremeDifficultySelect;
	bool			m_bEasterEggs;
	bool			m_bMarvelousTiming;
	enum { COIN_HOME, COIN_PAY, COIN_FREE, NUM_COIN_MODES } m_CoinMode;
	int				m_iCoinsPerCredit;
	bool			m_bJointPremium;
	bool			m_bPickExtraStage;
	float			m_fLongVerSongSeconds;
	float			m_fMarathonVerSongSeconds;
	bool			m_bShowSongOptions;
	bool			m_bSoloSingle;
	SongOptions::FailType m_DefaultFailType;
	bool			m_bDancePointsForOni;
	bool			m_bTimestamping;
	bool			m_bShowLyrics;
	bool			m_bAutogenMissingTypes;
	bool			m_bAutogenGroupCourses;

	/* 0 = no; 1 = yes; -1 = auto (do whatever is appropriate for the arch). */
	int				m_iBoostAppPriority;

	/* 0 = no; 1 = yes; -1 = auto (turn on for known-bad drivers) */
	int				m_iPolygonRadar;

	CStringArray m_asAdditionalSongFolders;
	CString m_DWIPath;

	CString			m_sSoundDrivers;
	float			m_fSoundVolume;
	bool			m_bSoundPreloadAll;
	bool			m_bAllowSoftwareRenderer;

	/* Game-specific prefs: */
	CString			m_sDefaultNoteSkin;

	void ReadGlobalPrefsFromDisk( bool bSwitchToLastPlayedGame );
	void SaveGlobalPrefsToDisk();


	void ReadGamePrefsFromDisk();
	void SaveGamePrefsToDisk();
};


extern PrefsManager*	PREFSMAN;	// global and accessable from anywhere in our program

#endif

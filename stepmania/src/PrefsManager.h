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
#include "PlayerNumber.h"

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
	int				m_iMovieColorDepth;
	int				m_iMaxTextureResolution;
	int				m_iRefreshRate;
	bool			m_bShowStats;
	bool			m_bShowBanners;
	enum BackgroundModes { BGMODE_OFF, BGMODE_ANIMATIONS, BGMODE_MOVIEVIS, BGMODE_RANDOMMOVIES } m_BackgroundMode;
	int				m_iNumBackgrounds;
	float			m_fBGBrightness;
	bool			m_bHiddenSongs;
	bool			m_bVsync;
	bool			m_bInterlaced;
#ifdef _XBOX
	bool			m_bPAL;
	float			m_fScreenPosX ;
	float			m_fScreenPosY ;
	float			m_fScreenWidth ;
	float			m_fScreenHeight ;
#endif
	bool			m_bDelayedTextureDelete;
	bool			m_bTexturePreload;
	bool			m_bDelayedScreenLoad;
	enum BannerCacheMode { BNCACHE_OFF, BNCACHE_LOW_RES, BNCACHE_FULL };
	BannerCacheMode	m_BannerCache;

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
	float			m_fJudgeWindowMineSeconds;
	float			m_fJudgeWindowAttackSeconds;
	float			m_fLifeDeltaMarvelousPercentChange;
	float			m_fLifeDeltaPerfectPercentChange;
	float			m_fLifeDeltaGreatPercentChange;
	float			m_fLifeDeltaGoodPercentChange;
	float			m_fLifeDeltaBooPercentChange;
	float			m_fLifeDeltaMissPercentChange;
	float			m_fLifeDeltaOKPercentChange;
	float			m_fLifeDeltaNGPercentChange;
	float			m_fLifeDeltaMinePercentChange;
	int				m_iRegenComboAfterFail;
	int				m_iRegenComboAfterMiss;
	int				m_iMaxRegenComboAfterFail;
	int				m_iMaxRegenComboAfterMiss;
	bool			m_bTwoPlayerRecovery;
	bool			m_bAutoPlay;
	bool			m_bDelayedEscape;
	bool			m_bInstructions, m_bShowDontDie, m_bShowSelectGroup;
	bool			m_bShowNative;
	bool			m_bArcadeOptionsNavigation;
	enum MusicWheelUsesSections { NEVER, ALWAYS, ABC_ONLY } m_MusicWheelUsesSections;
	int				m_iMusicWheelSwitchSpeed;
	bool			m_bEasterEggs;
	int 			m_iMarvelousTiming;
	int				m_iCoinMode;
	int				m_iCoinsPerCredit;
	enum Premium { NO_PREMIUM, DOUBLES_PREMIUM, JOINT_PREMIUM };
	Premium			m_Premium;
	bool			m_bPickExtraStage;
	bool			m_bComboContinuesBetweenSongs;
	float			m_fLongVerSongSeconds;
	float			m_fMarathonVerSongSeconds;
	enum Maybe { ASK = -1, NO = 0, YES = 1 };
	Maybe			m_ShowSongOptions;
	bool			m_bSoloSingle;
	bool			m_bDancePointsForOni;	//DDR-Extreme style dance points instead of max2 percent
	bool			m_bPercentageScoring;
	float			m_fMinPercentageForHighScore;
	bool			m_bShowLyrics;
	bool			m_bAutogenMissingTypes;
	bool			m_bAutogenGroupCourses;
	bool			m_bBreakComboToGetItem;
	enum CharacterOption { CO_OFF = 0, CO_RANDOM = 1, CO_SELECT = 2};
	CharacterOption	m_ShowDancingCharacters;
	bool			m_bUseUnlockSystem;
	bool			m_bFirstRun;
	bool			m_bAutoMapOnJoyChange;
	float			m_fGlobalOffsetSeconds;
	bool			m_bTenFooterInRed;
	int				m_iProgressiveLifebar;
	int				m_iProgressiveStageLifebar;
	int				m_iProgressiveNonstopLifebar;
	bool			m_bShowBeginnerHelper;
	bool			m_bEndlessBreakEnabled;
	int				m_iEndlessNumStagesUntilBreak;
	int				m_iEndlessBreakLength;
	CString			m_sLanguage;
	CString			m_sMemoryCardProfileSubdir;	// if set, always use the device that mounts to this point
	CString			m_sDefaultLocalProfileID[NUM_PLAYERS];
	CString			m_sMemoryCardOsMountPoint[NUM_PLAYERS];	// if set, always use the device that mounts to this point
	int				m_iMemoryCardUsbBus[NUM_PLAYERS];	// take the first storage device on this usb bus
	int				m_iMemoryCardUsbPort[NUM_PLAYERS];	// take the first storage device on this usb bus
	int				m_iCenterImageTranslateX;
	int				m_iCenterImageTranslateY;
	float			m_fCenterImageScaleX;
	float			m_fCenterImageScaleY;
	bool			m_bAttractSound;
	bool			m_bDemonstrationSound;
	bool			m_bAllowExtraStage;

	// course ranking
	enum CourseSortOrders { COURSE_SORT_SONGS, COURSE_SORT_METER, COURSE_SORT_METER_SUM, COURSE_SORT_RANK } m_iCourseSortOrder;
	bool			m_bMoveRandomToEnd;
	enum GetRankingName { RANKING_OFF, RANKING_ON, RANKING_LIST } m_iGetRankingName;

	// scoring type; SCORING_MAX2 should always be first
	enum ScoringTypes { SCORING_MAX2, SCORING_5TH } m_iScoringType;

	/* 0 = no; 1 = yes; -1 = auto (do whatever is appropriate for the arch). */
	int				m_iBoostAppPriority;

	CStringArray	m_asAdditionalSongFolders;
	CString			m_DWIPath;

	CString			m_sLastSeenVideoDriver;
	CString			m_sLastSeenInputDevices;
#if defined(WIN32)
	int				m_iLastSeenMemory;
#endif
	CString			m_sVideoRenderers;
	bool			m_bAntiAliasing;
	CString			m_sSoundDrivers;
	int			m_iSoundWriteAhead;
	CString			m_sMovieDrivers;
	CString			m_sLightsDriver;
	float			m_fSoundVolume;
	bool			m_bSoundPreloadAll;
	int				m_iSoundResampleQuality;
	bool			m_bAllowUnacceleratedRenderer;
	bool			m_bThreadedInput;
	bool			m_bScreenTestMode;

	CString			m_sIgnoredMessageWindows;

	CString			m_sCoursesToShowRanking;

	/* Debug: */
	bool			m_bLogging;
	bool			m_bForceLogFlush;
	bool			m_bShowLogOutput;
	bool			m_bTimestamping;
	bool			m_bLogCheckpoints;

	/* Game-specific prefs: */
	CString			m_sDefaultModifiers;

	void ReadGlobalPrefsFromDisk();
	void SaveGlobalPrefsToDisk() const;


	void ReadGamePrefsFromDisk();
	void SaveGamePrefsToDisk();
};

/* This is global, because it can be accessed by crash handlers and error handlers
 * that are run after PREFSMAN shuts down (and probably don't want to deref tht
 * pointer anyway). */
extern bool			g_bAutoRestart;

extern PrefsManager*	PREFSMAN;	// global and accessable from anywhere in our program

#endif

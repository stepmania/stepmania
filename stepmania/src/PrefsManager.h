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
#include "Grade.h"	// for NUM_GRADE_TIERS

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
	bool			m_bFastLoad;

	bool			m_bOnlyDedicatedMenuButtons;
	bool			m_bMenuTimer;
	bool			m_bShowDanger;
	int				m_iNumArcadeStages;
	bool			m_bEventMode;

	float			m_fJudgeWindowScale;
	float			m_fJudgeWindowSecondsMarvelous;
	float			m_fJudgeWindowSecondsPerfect;
	float			m_fJudgeWindowSecondsGreat;
	float			m_fJudgeWindowSecondsGood;
	float			m_fJudgeWindowSecondsBoo;
	float			m_fJudgeWindowSecondsOK;
	float			m_fJudgeWindowSecondsMine;
	float			m_fJudgeWindowSecondsAttack;

	float			m_fLifeDifficultyScale;
	float			m_fLifeDeltaPercentChangeMarvelous;
	float			m_fLifeDeltaPercentChangePerfect;
	float			m_fLifeDeltaPercentChangeGreat;
	float			m_fLifeDeltaPercentChangeGood;
	float			m_fLifeDeltaPercentChangeBoo;
	float			m_fLifeDeltaPercentChangeMiss;
	float			m_fLifeDeltaPercentChangeHitMine;
	float			m_fLifeDeltaPercentChangeOK;
	float			m_fLifeDeltaPercentChangeNG;

	// tug meter used in rave
	float			m_fTugMeterPercentChangeMarvelous;
	float			m_fTugMeterPercentChangePerfect;
	float			m_fTugMeterPercentChangeGreat;
	float			m_fTugMeterPercentChangeGood;
	float			m_fTugMeterPercentChangeBoo;
	float			m_fTugMeterPercentChangeMiss;
	float			m_fTugMeterPercentChangeHitMine;
	float			m_fTugMeterPercentChangeOK;
	float			m_fTugMeterPercentChangeNG;
	
	// Whoever added these: Please add a comment saying what they do. -Chris
	int				m_iRegenComboAfterFail;
	int				m_iRegenComboAfterMiss;
	int				m_iMaxRegenComboAfterFail;
	int				m_iMaxRegenComboAfterMiss;
	bool			m_bTwoPlayerRecovery;
	bool			m_bMercifulDrain;	// negative life deltas are scaled by the players life percentage

	// percent score (the number that is shown on the screen and saved to memory card)
	int				m_iPercentScoreWeightMarvelous;
	int				m_iPercentScoreWeightPerfect;
	int				m_iPercentScoreWeightGreat;
	int				m_iPercentScoreWeightGood;
	int				m_iPercentScoreWeightBoo;
	int				m_iPercentScoreWeightMiss;
	int				m_iPercentScoreWeightHitMine;
	int				m_iPercentScoreWeightOK;
	int				m_iPercentScoreWeightNG;

	// grades are calculated based on a percentage, but might have different weights than the percent score
	int				m_iGradeWeightMarvelous;
	int				m_iGradeWeightPerfect;
	int				m_iGradeWeightGreat;
	int				m_iGradeWeightGood;
	int				m_iGradeWeightBoo;
	int				m_iGradeWeightMiss;
	int				m_iGradeWeightHitMine;
	int				m_iGradeWeightOK;
	int				m_iGradeWeightNG;

	int				m_iNumGradeTiersUsed;
	float			m_fGradePercentTier[NUM_GRADE_TIERS];	// the minimum percent necessary achieve a grade
	bool			m_bGradeTier02IsAllPerfects;	// DDR special case.  If true, m_fGradePercentTier[GRADE_TIER_2] is ignored

	float			m_fSuperMeterPercentChangeMarvelous;
	float			m_fSuperMeterPercentChangePerfect;
	float			m_fSuperMeterPercentChangeGreat;
	float			m_fSuperMeterPercentChangeGood;
	float			m_fSuperMeterPercentChangeBoo;
	float			m_fSuperMeterPercentChangeMiss;
	float			m_fSuperMeterPercentChangeHitMine;
	float			m_fSuperMeterPercentChangeOK;
	float			m_fSuperMeterPercentChangeNG;
	bool			m_bMercifulSuperMeter;	// negative super deltas are scaled by the players life percentage

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
	bool			m_bAutogenSteps;
	bool			m_bAutogenGroupCourses;
	bool			m_bBreakComboToGetItem;
	bool			m_bLockCourseDifficulties;
	enum CharacterOption { CO_OFF = 0, CO_RANDOM = 1, CO_SELECT = 2};
	CharacterOption	m_ShowDancingCharacters;
	bool			m_bUseUnlockSystem;
	bool			m_bFirstRun;
	bool			m_bAutoMapOnJoyChange;
	float			m_fGlobalOffsetSeconds;
	int				m_iProgressiveLifebar;
	int				m_iProgressiveStageLifebar;
	int				m_iProgressiveNonstopLifebar;
	bool			m_bShowBeginnerHelper;
	bool			m_bEndlessBreakEnabled;
	int				m_iEndlessNumStagesUntilBreak;
	int				m_iEndlessBreakLength;
	bool			m_bDisableScreenSaver;
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
	int				m_iAttractSoundFrequency;	// 0 = never, 1 = every time
	bool			m_bAllowExtraStage;
	
	// If true, then signatures created when writing profile data 
	// and verified when reading profile data.  Leave this false if 
	// you want to use a profile on different machines that don't 
	// have the same key, or else the profile's data will be discarded.
	bool			m_bSignProfileData;	

	/* Editor prefs: */
	bool			m_bEditorShowBGChangesPlay;

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
	bool			m_bSmoothLines;
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
	CString			m_sMachineName;

	CString			m_sIgnoredMessageWindows;

	CString			m_sCoursesToShowRanking;

	/* Debug: */
	bool			m_bLogging;
	bool			m_bForceLogFlush;
	bool			m_bShowLogOutput;
	bool			m_bTimestamping;
	bool			m_bLogSkips;
	bool			m_bLogCheckpoints;
	bool			m_bShowLoadingWindow;

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

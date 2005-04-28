/* PrefsManager - Holds user-chosen preferences that are saved between sessions. */

#ifndef PREFSMANAGER_H
#define PREFSMANAGER_H

#include "PlayerNumber.h"
#include "GameConstantsAndTypes.h"
#include "Grade.h"	// for NUM_GRADE_TIERS
#include "Preference.h"
class IniFile;

class PrefsManager
{
public:
	PrefsManager();
	~PrefsManager();
	IPreference *GetPreferenceByName( const CString &sName );

	void Init();

	// GameOptions (ARE saved between sessions)
	Preference<bool>	m_bWindowed;
	Preference<int>		m_iDisplayWidth;
	Preference<int>		m_iDisplayHeight;
	Preference<int>		m_iDisplayColorDepth;
	Preference<int>		m_iTextureColorDepth;
	Preference<int>		m_iMovieColorDepth;
	Preference<int>		m_iMaxTextureResolution;
	Preference<int>		m_iRefreshRate;
	Preference<float>	m_fDisplayAspectRatio;
	Preference<bool>	m_bShowStats;
	Preference<bool>	m_bShowBanners;

	enum BackgroundMode { BGMODE_OFF, BGMODE_ANIMATIONS, BGMODE_MOVIEVIS, BGMODE_RANDOMMOVIES };
	Preference<BackgroundMode>		m_BackgroundMode;
	Preference<int>		m_iNumBackgrounds;
	Preference<float>	m_fBGBrightness;
	Preference<bool>	m_bHiddenSongs;
	Preference<bool>	m_bVsync;
	Preference<bool>	m_bInterlaced;
	Preference<bool>	m_bPAL;
	Preference<bool>	m_bDelayedTextureDelete;
	Preference<bool>	m_bTexturePreload;
	Preference<bool>	m_bDelayedScreenLoad;
	Preference<bool>	m_bDelayedModelDelete;
	enum BannerCache {
		BNCACHE_OFF,
		BNCACHE_LOW_RES_PRELOAD, // preload low-res on start
		BNCACHE_LOW_RES_LOAD_ON_DEMAND, // preload low-res on screen load
		BNCACHE_FULL };
	Preference<BannerCache>		m_BannerCache;
	Preference<bool>	m_bPalettedBannerCache;
	Preference<bool>	m_bFastLoad;

	Preference<bool>	m_bOnlyDedicatedMenuButtons;
	Preference<bool>	m_bMenuTimer;
	Preference<bool>	m_bShowDanger;

	Preference<float>	m_fJudgeWindowScale;
	Preference<float>	m_fJudgeWindowAdd;		// this is useful for compensating for changes in sampling rate between devices
	Preference<float>	m_fJudgeWindowSecondsMarvelous;
	Preference<float>	m_fJudgeWindowSecondsPerfect;
	Preference<float>	m_fJudgeWindowSecondsGreat;
	Preference<float>	m_fJudgeWindowSecondsGood;
	Preference<float>	m_fJudgeWindowSecondsBoo;
	Preference<float>	m_fJudgeWindowSecondsOK;
	Preference<float>	m_fJudgeWindowSecondsRoll;
	Preference<float>	m_fJudgeWindowSecondsMine;
	Preference<float>	m_fJudgeWindowSecondsAttack;

	Preference<float>	m_fLifeDifficultyScale;
	Preference<float>	m_fLifeDeltaPercentChangeMarvelous;
	Preference<float>	m_fLifeDeltaPercentChangePerfect;
	Preference<float>	m_fLifeDeltaPercentChangeGreat;
	Preference<float>	m_fLifeDeltaPercentChangeGood;
	Preference<float>	m_fLifeDeltaPercentChangeBoo;
	Preference<float>	m_fLifeDeltaPercentChangeMiss;
	Preference<float>	m_fLifeDeltaPercentChangeHitMine;
	Preference<float>	m_fLifeDeltaPercentChangeOK;
	Preference<float>	m_fLifeDeltaPercentChangeNG;

	// tug meter used in rave
	Preference<float>	m_fTugMeterPercentChangeMarvelous;
	Preference<float>	m_fTugMeterPercentChangePerfect;
	Preference<float>	m_fTugMeterPercentChangeGreat;
	Preference<float>	m_fTugMeterPercentChangeGood;
	Preference<float>	m_fTugMeterPercentChangeBoo;
	Preference<float>	m_fTugMeterPercentChangeMiss;
	Preference<float>	m_fTugMeterPercentChangeHitMine;
	Preference<float>	m_fTugMeterPercentChangeOK;
	Preference<float>	m_fTugMeterPercentChangeNG;
	
	// Whoever added these: Please add a comment saying what they do. -Chris
	Preference<int>		m_iRegenComboAfterFail;
	Preference<int>		m_iRegenComboAfterMiss;
	Preference<int>		m_iMaxRegenComboAfterFail;
	Preference<int>		m_iMaxRegenComboAfterMiss;
	Preference<bool>	m_bTwoPlayerRecovery;
	Preference<bool>	m_bMercifulDrain;	// negative life deltas are scaled by the players life percentage
	Preference<bool>	m_bMinimum1FullSongInCourses;	// FEoS for 1st song, FailImmediate thereafter
	Preference<bool>	m_bFailOffInBeginner;
	Preference<bool>	m_bFailOffForFirstStageEasy;
	Preference<bool>	m_bMercifulBeginner;	// don't subtract from percent score or grade DP, larger boo window

	// percent score (the number that is shown on the screen and saved to memory card)
	Preference<int>		m_iPercentScoreWeightMarvelous;
	Preference<int>		m_iPercentScoreWeightPerfect;
	Preference<int>		m_iPercentScoreWeightGreat;
	Preference<int>		m_iPercentScoreWeightGood;
	Preference<int>		m_iPercentScoreWeightBoo;
	Preference<int>		m_iPercentScoreWeightMiss;
	Preference<int>		m_iPercentScoreWeightHitMine;
	Preference<int>		m_iPercentScoreWeightOK;
	Preference<int>		m_iPercentScoreWeightNG;

	// grades are calculated based on a percentage, but might have different weights than the percent score
	Preference<int>		m_iGradeWeightMarvelous;
	Preference<int>		m_iGradeWeightPerfect;
	Preference<int>		m_iGradeWeightGreat;
	Preference<int>		m_iGradeWeightGood;
	Preference<int>		m_iGradeWeightBoo;
	Preference<int>		m_iGradeWeightMiss;
	Preference<int>		m_iGradeWeightHitMine;
	Preference<int>		m_iGradeWeightOK;
	Preference<int>		m_iGradeWeightNG;

	// super meter used in rave
	Preference<float>	m_fSuperMeterPercentChangeMarvelous;
	Preference<float>	m_fSuperMeterPercentChangePerfect;
	Preference<float>	m_fSuperMeterPercentChangeGreat;
	Preference<float>	m_fSuperMeterPercentChangeGood;
	Preference<float>	m_fSuperMeterPercentChangeBoo;
	Preference<float>	m_fSuperMeterPercentChangeMiss;
	Preference<float>	m_fSuperMeterPercentChangeHitMine;
	Preference<float>	m_fSuperMeterPercentChangeOK;
	Preference<float>	m_fSuperMeterPercentChangeNG;
	Preference<float>	m_bMercifulSuperMeter;	// negative super deltas are scaled by the players life percentage

	// time meter used in survival
	Preference<float>	m_fTimeMeterSecondsChangeMarvelous;
	Preference<float>	m_fTimeMeterSecondsChangePerfect;
	Preference<float>	m_fTimeMeterSecondsChangeGreat;
	Preference<float>	m_fTimeMeterSecondsChangeGood;
	Preference<float>	m_fTimeMeterSecondsChangeBoo;
	Preference<float>	m_fTimeMeterSecondsChangeMiss;
	Preference<float>	m_fTimeMeterSecondsChangeHitMine;
	Preference<float>	m_fTimeMeterSecondsChangeOK;
	Preference<float>	m_fTimeMeterSecondsChangeNG;

	Preference<bool>	m_bAutoPlay;
	Preference<bool>	m_bDelayedBack;
	Preference<bool>	m_bShowInstructions;
	Preference<bool>	m_bShowSelectGroup;
	Preference<bool>	m_bShowCaution;
	Preference<bool>	m_bShowNativeLanguage;
	Preference<bool>	m_bArcadeOptionsNavigation;
	enum MusicWheelUsesSections { NEVER, ALWAYS, ABC_ONLY };
	Preference<MusicWheelUsesSections>		m_MusicWheelUsesSections;
	Preference<int>		m_iMusicWheelSwitchSpeed;
	Preference<bool>	m_bEasterEggs;
	enum MarvelousTiming { MARVELOUS_NEVER, MARVELOUS_COURSES_ONLY, MARVELOUS_EVERYWHERE };
	Preference<MarvelousTiming>		m_MarvelousTiming;
	Preference<bool>	m_bEventMode;
	Preference<int>		m_iCoinsPerCredit;
	Preference<int>		m_iSongsPerPlay;

	// These options have weird interactions depending on m_bEventMode, 
	// so wrap them.
	Preference<CoinMode>	m_CoinMode;
	Preference<Premium>		m_Premium;

	Preference<bool>	m_bDelayedCreditsReconcile;
	Preference<bool>	m_bPickExtraStage;	// DDR Extreme-style extra stage support.
	Preference<bool>	m_bComboContinuesBetweenSongs;
	Preference<float>	m_fLongVerSongSeconds;
	Preference<float>	m_fMarathonVerSongSeconds;
	enum Maybe { ASK = -1, NO = 0, YES = 1 };
	Preference<Maybe>		m_ShowSongOptions;
	Preference<bool>	m_bSoloSingle;
	Preference<bool>	m_bDancePointsForOni;	//DDR-Extreme style dance points instead of max2 percent
	Preference<bool>	m_bPercentageScoring;
	Preference<float>	m_fMinPercentageForMachineSongHighScore;
	Preference<float>	m_fMinPercentageForMachineCourseHighScore;
	Preference<bool>	m_bDisqualification;
	Preference<bool>	m_bShowLyrics;
	Preference<bool>	m_bAutogenSteps;
	Preference<bool>	m_bAutogenGroupCourses;
	Preference<bool>	m_bBreakComboToGetItem;
	Preference<bool>	m_bLockCourseDifficulties;
	enum CharacterOption { CO_OFF = 0, CO_RANDOM = 1, CO_SELECT = 2};
	Preference<CharacterOption>		m_ShowDancingCharacters;
	Preference<bool>	m_bUseUnlockSystem;
	Preference<bool>	m_bFirstRun;
	Preference<bool>	m_bAutoMapOnJoyChange;
	Preference<float>	m_fGlobalOffsetSeconds;
	Preference<int>		m_iProgressiveLifebar;
	Preference<int>		m_iProgressiveStageLifebar;
	Preference<int>		m_iProgressiveNonstopLifebar;
	Preference<bool>	m_bShowBeginnerHelper;
	Preference<bool>	m_bEndlessBreakEnabled;
	Preference<int>		m_iEndlessNumStagesUntilBreak;
	Preference<int>		m_iEndlessBreakLength;
	Preference<bool>	m_bDisableScreenSaver;
	Preference<CString>	m_sLanguage;
	Preference<CString>	m_sMemoryCardProfileSubdir;	// the directory on a memory card to look in for a profile
	Preference<int>		m_iProductID;		// Saved in HighScore to track what software version a score came from.
	CString				m_sDefaultLocalProfileID[NUM_PLAYERS];
	Preference<bool>	m_bMemoryCards;
	CString				m_sMemoryCardOsMountPoint[NUM_PLAYERS];	// if set, always use the device that mounts to this point
	int					m_iMemoryCardUsbBus[NUM_PLAYERS];	// look for this bus when assigning cards.  -1 = match any
	int					m_iMemoryCardUsbPort[NUM_PLAYERS];	// look for this port when assigning cards.  -1 = match any
	int					m_iMemoryCardUsbLevel[NUM_PLAYERS];	// look for this level when assigning cards.  -1 = match any
	Preference<int>		m_iCenterImageTranslateX;
	Preference<int>		m_iCenterImageTranslateY;
	Preference<int>		m_fCenterImageAddWidth;
	Preference<int>		m_fCenterImageAddHeight;
	Preference<int>		m_iAttractSoundFrequency;	// 0 = never, 1 = every time
	Preference<bool>	m_bAllowExtraStage;
	Preference<bool>	m_bHideDefaultNoteSkin;
	Preference<int>		m_iMaxHighScoresPerListForMachine;
	Preference<int>		m_iMaxHighScoresPerListForPlayer;
	Preference<int>		m_iMaxRecentScoresForMachine;
	Preference<int>		m_iMaxRecentScoresForPlayer;
	Preference<bool>	m_bAllowMultipleHighScoreWithSameName;
	Preference<bool>	m_bCelShadeModels;

	/* experimental: force a specific update rate.  This prevents big 
	 * animation jumps on frame skips. */
	float			m_fConstantUpdateDeltaSeconds;	// 0 to disable

	// Number of seconds it takes for a button on the controller to release
	// after pressed.
	float			m_fPadStickSeconds;

	// Useful for non 4:3 displays and resolutions < 640x480 where texels don't
	// map directly to pixels.
	bool			m_bForceMipMaps;
	bool			m_bTrilinearFiltering;		// has no effect without mipmaps on
	bool			m_bAnisotropicFiltering;	// has no effect without mipmaps on.  Not mutually exclusive with trilinear.

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
	bool			m_bSubSortByNumSteps;	
	enum GetRankingName { RANKING_OFF, RANKING_ON, RANKING_LIST } m_iGetRankingName;

	// scoring type; SCORING_MAX2 should always be first
	enum ScoringTypes { SCORING_MAX2, SCORING_5TH } m_iScoringType;

	/* 0 = no; 1 = yes; -1 = auto (do whatever is appropriate for the arch). */
	int				m_iBoostAppPriority;

	CString			m_sAdditionalSongFolders;
	CString			m_sAdditionalFolders;

	CString			m_sLastSeenVideoDriver;
	CString			m_sLastSeenInputDevices;
#if defined(WIN32)
	int				m_iLastSeenMemory;
#endif
	CString			m_sVideoRenderers;
	bool			m_bSmoothLines;
	CString			m_sSoundDrivers;
	int				m_iSoundWriteAhead;
	CString				m_iSoundDevice;
	float			m_fSoundVolume;
	int				m_iSoundResampleQuality;
	CString			m_sInputDrivers;
	CString			m_sMovieDrivers;
	CString			m_sLightsDriver;
	CString			m_sLightsStepsDifficulty;
	bool			m_bBlinkGameplayButtonLightsOnNote;
	bool			m_bAllowUnacceleratedRenderer;
	bool			m_bThreadedInput;
	bool			m_bThreadedMovieDecode;
	bool			m_bScreenTestMode;
	CString			m_sMachineName;

	CString			m_sIgnoredMessageWindows;

	CString			m_sCoursesToShowRanking;

	/* Debug: */
	bool			m_bLogToDisk;
	bool			m_bForceLogFlush;
	bool			m_bShowLogOutput;
	bool			m_bTimestamping;
	bool			m_bLogSkips;
	bool			m_bLogCheckpoints;
	bool			m_bShowLoadingWindow;

	/* Game-specific prefs: */
	CString			m_sDefaultModifiers;

#if defined(XBOX)
	// Virtual memory preferences
	bool			m_bEnableVirtualMemory;
	int				m_iPageFileSize; // page file size in megabytes
	int				m_iPageSize; // page size in kilobytes
	int				m_iPageThreshold; // threshold in kilobytes where virtual memory will be used
	bool			m_bLogVirtualMemory; // (under debug) log the virtual memory allocation, etc.
#endif

	void ReadGlobalPrefsFromIni( const IniFile &ini );
	void SaveGlobalPrefsToIni( IniFile &ini ) const;

	void ReadGlobalPrefsFromDisk();
	void SaveGlobalPrefsToDisk() const;

	void ResetToFactoryDefaults();

	//
	// For self-registering prefs
	//
	static void Subscribe( IPreference *p );
	static void Unsubscribe( IPreference *p );

	// Lua
	void PushSelf( lua_State *L );

protected:
	void ReadPrefsFromFile( CString sIni );

};




/* This is global, because it can be accessed by crash handlers and error handlers
 * that are run after PREFSMAN shuts down (and probably don't want to deref tht
 * pointer anyway). */
extern bool			g_bAutoRestart;

extern PrefsManager*	PREFSMAN;	// global and accessable from anywhere in our program

#endif

/*
 * (c) 2001-2004 Chris Danford, Chris Gomez
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

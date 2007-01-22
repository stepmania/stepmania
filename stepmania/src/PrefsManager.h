/* PrefsManager - Holds user-chosen preferences that are saved between sessions. */

#ifndef PREFSMANAGER_H
#define PREFSMANAGER_H

#include "Preference.h"

class IniFile;

const int MAX_SONGS_PER_PLAY = 7;

enum MusicWheelUsesSections { MusicWheelUsesSections_NEVER, MusicWheelUsesSections_ALWAYS, MusicWheelUsesSections_ABC_ONLY, NUM_MusicWheelUsesSections, MusicWheelUsesSections_Invalid };
enum AllowW1 { ALLOW_W1_NEVER, ALLOW_W1_COURSES_ONLY, ALLOW_W1_EVERYWHERE, NUM_AllowW1, AllowW1_Invalid };
enum Maybe { Maybe_ASK, Maybe_NO, Maybe_YES, NUM_Maybe, Maybe_Invalid };
enum GetRankingName { RANKING_OFF, RANKING_ON, RANKING_LIST, NUM_GetRankingName, GetRankingName_Invalid };

enum RandomBackgroundMode
{
	BGMODE_OFF,
	BGMODE_ANIMATIONS,
	BGMODE_RANDOMMOVIES,
	NUM_RandomBackgroundMode,
	RandomBackgroundMode_Invalid
};
enum ShowDancingCharacters
{
	SDC_Off,
	SDC_Random,
	SDC_Select,
	NUM_ShowDancingCharacters,
	ShowDancingCharacters_Invalid
};
enum BannerCacheMode
{
	BNCACHE_OFF,
	BNCACHE_LOW_RES_PRELOAD, // preload low-res on start
	BNCACHE_LOW_RES_LOAD_ON_DEMAND, // preload low-res on screen load
	BNCACHE_FULL,
	NUM_BannerCacheMode,
	BannerCacheMode_Invalid
};
enum AttractSoundFrequency
{
	ASF_NEVER,
	ASF_EVERY_TIME,
	ASF_EVERY_2_TIMES,
	ASF_EVERY_3_TIMES,
	ASF_EVERY_4_TIMES,
	ASF_EVERY_5_TIMES,
	NUM_AttractSoundFrequency,
	AttractSoundFrequency_Invalid
};
enum CourseSortOrders
{
	COURSE_SORT_PREFERRED,
	COURSE_SORT_SONGS,
	COURSE_SORT_METER,
	COURSE_SORT_METER_SUM,
	COURSE_SORT_RANK,
	NUM_CourseSortOrders,
	CourseSortOrders_Invalid
};
enum ScoringType
{
	SCORING_NEW,
	SCORING_OLD,
	NUM_ScoringType,
	ScoringType_Invalid
};

class PrefsManager
{
public:
	PrefsManager();
	~PrefsManager();

	void Init();

	void SetCurrentGame( const RString &sGame );
	RString	GetCurrentGame() { return m_sCurrentGame; }
protected:
	Preference<RString>	m_sCurrentGame;

public:
	// Game-specific prefs.  Copy these off and save them every time the game changes
	Preference<RString>	m_sAnnouncer;
	Preference<RString>	m_sTheme;
	Preference<RString>	m_sDefaultModifiers;
protected:
	void StoreGamePrefs();
	void RestoreGamePrefs();
	struct GamePrefs
	{
		// See GamePrefs::GamePrefs in PrefsManager.cpp for some default settings
		GamePrefs();

		RString	m_sAnnouncer;
		RString m_sTheme;
		RString	m_sDefaultModifiers;
	};
	map<RString, GamePrefs> m_mapGameNameToGamePrefs;

public:
	Preference<bool>	m_bWindowed;
	Preference<int>		m_iDisplayWidth;
	Preference<int>		m_iDisplayHeight;
	Preference<float>	m_fDisplayAspectRatio;
	Preference<int>		m_iDisplayColorDepth;
	Preference<int>		m_iTextureColorDepth;
	Preference<int>		m_iMovieColorDepth;
	Preference<int>		m_iMaxTextureResolution;
	Preference<int>		m_iRefreshRate;
	Preference<bool>	m_bAllowMultitexture;
	Preference<bool>	m_bShowStats;
	Preference<bool>	m_bShowBanners;

	Preference<bool>	m_bHiddenSongs;
	Preference<bool>	m_bVsync;
	Preference<bool>	m_bInterlaced;
	Preference<bool>	m_bPAL;
	Preference<bool>	m_bDelayedTextureDelete;
	Preference<bool>	m_bDelayedModelDelete;
	Preference<BannerCacheMode>		m_BannerCache;
	Preference<bool>	m_bPalettedBannerCache;
	Preference<bool>	m_bFastLoad;
	Preference<bool>        m_bFastLoadAdditionalSongs;

	Preference<bool>	m_bOnlyDedicatedMenuButtons;
	Preference<bool>	m_bMenuTimer;

	Preference<float>	m_fLifeDifficultyScale;

	// Whoever added these: Please add a comment saying what they do. -Chris
	Preference<int>		m_iRegenComboAfterMiss;
	Preference<bool>	m_bMercifulDrain;	// negative life deltas are scaled by the players life percentage
	Preference<bool>	m_bMinimum1FullSongInCourses;	// FEoS for 1st song, FailImmediate thereafter
	Preference<bool>	m_bFailOffInBeginner;
	Preference<bool>	m_bFailOffForFirstStageEasy;
	Preference<bool>	m_bMercifulBeginner;	// don't subtract from percent score or grade DP, larger W5 window
	Preference<bool>	m_bMercifulSuperMeter;	// negative super deltas are scaled by the players life percentage
	Preference<bool>	m_bDelayedBack;
	Preference<bool>	m_bShowInstructions;
	Preference<bool>	m_bShowSelectGroup;
	Preference<bool>	m_bShowCaution;
	Preference<bool>	m_bShowNativeLanguage;
	Preference<bool>	m_bArcadeOptionsNavigation;
	Preference<MusicWheelUsesSections>		m_MusicWheelUsesSections;
	Preference<int>		m_iMusicWheelSwitchSpeed;
	Preference<AllowW1>	m_AllowW1;
	Preference<bool>	m_bEventMode;
	Preference<int>		m_iCoinsPerCredit;
	Preference<int>		m_iSongsPerPlay;
	Preference<bool>	m_bDelayedCreditsReconcile;
	Preference<bool>	m_bPickExtraStage;
	Preference<bool>	m_bComboContinuesBetweenSongs;
	Preference<Maybe>		m_ShowSongOptions;
	Preference<bool>	m_bDancePointsForOni;
	Preference<bool>	m_bPercentageScoring;
	Preference<float>	m_fMinPercentageForMachineSongHighScore;
	Preference<float>	m_fMinPercentageForMachineCourseHighScore;
	Preference<bool>	m_bDisqualification;
	Preference<bool>	m_bAutogenSteps;
	Preference<bool>	m_bAutogenGroupCourses;
	Preference<bool>	m_bBreakComboToGetItem;
	Preference<bool>	m_bLockCourseDifficulties;
	Preference<ShowDancingCharacters>		m_ShowDancingCharacters;
	Preference<bool>	m_bUseUnlockSystem;
	Preference<float>	m_fGlobalOffsetSeconds;
	Preference<int>		m_iProgressiveLifebar;
	Preference<int>		m_iProgressiveStageLifebar;
	Preference<int>		m_iProgressiveNonstopLifebar;
	Preference<bool>	m_bShowBeginnerHelper;
	Preference<bool>	m_bDisableScreenSaver;
	Preference<RString>	m_sLanguage;
	Preference<RString>	m_sMemoryCardProfileSubdir;	// the directory on a memory card to look in for a profile
	Preference<int>		m_iProductID;		// Saved in HighScore to track what software version a score came from.
	Preference<int>		m_iCenterImageTranslateX;
	Preference<int>		m_iCenterImageTranslateY;
	Preference<int>		m_fCenterImageAddWidth;
	Preference<int>		m_fCenterImageAddHeight;
	Preference<AttractSoundFrequency>	m_AttractSoundFrequency;
	Preference<bool>	m_bAllowExtraStage;
	Preference<bool>	m_bHideDefaultNoteSkin;
	Preference<int>		m_iMaxHighScoresPerListForMachine;
	Preference<int>		m_iMaxHighScoresPerListForPlayer;
	Preference<int>		m_iMaxRecentScoresForMachine;
	Preference<int>		m_iMaxRecentScoresForPlayer;
	Preference<bool>	m_bAllowMultipleHighScoreWithSameName;
	Preference<bool>	m_bCelShadeModels;
	Preference<bool>	m_bPreferredSortUsesGroups;

	// Number of seconds it takes for a button on the controller to release
	// after pressed.
	Preference<float>	m_fPadStickSeconds;

	// Useful for non 4:3 displays and resolutions < 640x480 where texels don't
	// map directly to pixels.
	Preference<bool>	m_bForceMipMaps;
	Preference<bool>	m_bTrilinearFiltering;		// has no effect without mipmaps on
	Preference<bool>	m_bAnisotropicFiltering;	// has no effect without mipmaps on.  Not mutually exclusive with trilinear.

	// If true, then signatures created when writing profile data 
	// and verified when reading profile data.  Leave this false if 
	// you want to use a profile on different machines that don't 
	// have the same key, or else the profile's data will be discarded.
	Preference<bool>	m_bSignProfileData;
	
	// course ranking
	Preference<CourseSortOrders>	m_CourseSortOrder;
	Preference<bool>	m_bSubSortByNumSteps;	
	Preference<GetRankingName>	m_GetRankingName;

	Preference<ScoringType>	m_ScoringType;

	Preference<RString>	m_sAdditionalSongFolders;
	Preference<RString>	m_sAdditionalCourseFolders;
	Preference<RString>	m_sAdditionalFolders;

	Preference<RString>	m_sLastSeenVideoDriver;
	Preference<RString>	m_sVideoRenderers;		// StepMania.cpp sets these on first run based on the card
	Preference<bool>	m_bSmoothLines;
	Preference<float>	m_fSoundVolume;
	Preference<int>		m_iSoundWriteAhead;
	Preference<RString>	m_iSoundDevice;	
	Preference<int>		m_iSoundPreferredSampleRate;
	Preference<RString>	m_sLightsStepsDifficulty;
	Preference<bool>	m_bAllowUnacceleratedRenderer;
	Preference<bool>	m_bThreadedInput;
	Preference<bool>	m_bThreadedMovieDecode;
	Preference<bool>	m_bScreenTestMode;
	Preference<bool>	m_bDebugLights;
	Preference<bool>	m_bMonkeyInput;
	Preference<RString>	m_sMachineName;
	Preference<RString>	m_sCoursesToShowRanking;

	/* Debug: */
	Preference<bool>	m_bLogToDisk;
	Preference<bool>	m_bForceLogFlush;
	Preference<bool>	m_bShowLogOutput;
	Preference<bool>	m_bLogSkips;
	Preference<bool>	m_bLogCheckpoints;
	Preference<bool>	m_bShowLoadingWindow;
	Preference<bool>	m_bPseudoLocalize;

#if !defined(WITHOUT_NETWORKING)
	Preference<bool>	m_bEnableScoreboard;  //Alows disabling of scoreboard in network play
#endif


	// wrappers
	float GetSoundVolume();


	void ReadPrefsFromIni( const IniFile &ini, const RString &sSection, bool bIsStatic );
	void ReadGamePrefsFromIni( const RString &sIni );
	void ReadDefaultsFromIni( const IniFile &ini, const RString &sSection );
	void SavePrefsToIni( IniFile &ini );

	void ReadPrefsFromDisk();
	void SavePrefsToDisk();

	void ResetToFactoryDefaults();

	RString GetPreferencesSection() const;

	// Lua
	void PushSelf( lua_State *L );

protected:
	void ReadPrefsFromFile( const RString &sIni, const RString &sSection, bool bIsStatic );
	void ReadDefaultsFromFile( const RString &sIni, const RString &sSection );
};




/* This is global, because it can be accessed by crash handlers and error handlers
 * that are run after PREFSMAN shuts down (and probably don't want to deref that
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

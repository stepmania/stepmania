#pragma once
/*
-----------------------------------------------------------------------------
 Class: PrefsManager

 Desc: Holds user-chosen preferences and saves it between sessions.  This class 
    also has temporary holders for information that passed between windows - e.g.
	GameplayStatistics.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GameConstantsAndTypes.h"


const int NUM_PAD_TO_DEVICE_SLOTS	= 3;	// three device inputs may map to one pad input


class PrefsManager
{
public:
	PrefsManager();
	~PrefsManager();

	enum BackgroundMode { BGMODE_OFF, BGMODE_ANIMATIONS, BGMODE_MOVIEVIS };

	// GameOptions (ARE saved between sessions)
	bool	m_bWindowed;
	bool	m_bHighDetail;
	bool	m_bHighTextureDetail;
	bool	m_bIgnoreJoyAxes;
	bool	m_bShowFPS;
	BackgroundMode	m_BackgroundMode;
	float	m_fBGBrightness;
	bool	m_bMenuTimer;
	bool	m_bEventMode;
	int		m_iNumArcadeStages;
	bool	m_bAutoPlay;
	float	m_fJudgeWindow;

	// AppearanceOptions (ARE saved between sessions)
	CString m_sAnnouncer;
	CString m_sNoteSkin;

	CStringArray m_asSongFolders;

	void ReadPrefsFromDisk();
	void SavePrefsToDisk();


	// Options that are NOT saved between sessions
	DifficultyClass	m_PreferredDifficultyClass[NUM_PLAYERS];
	SongSortOrder	m_SongSortOrder;			// used by MusicWheel and should be saved until the app exits
	PlayMode		m_PlayMode;
	int				m_iCurrentStageIndex;		// starts at 0, and is incremented with each Stage Clear

	int				GetStageIndex();
	bool			IsFinalStage();
	bool			IsExtraStage();
	bool			IsExtraStage2();
	CString			GetStageText();
	D3DXCOLOR		GetStageColor();


	PlayerOptions	m_PlayerOptions[NUM_PLAYERS];
	SongOptions		m_SongOptions;
};


extern PrefsManager*	PREFSMAN;	// global and accessable from anywhere in our program

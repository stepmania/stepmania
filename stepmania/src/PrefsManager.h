#pragma once
/*
-----------------------------------------------------------------------------
 Class: PrefsManager

 Desc: Holds user-chosen preferences and saves it between sessions.  This class 
    also has temporary holders for information that passed between windows - e.g.
	ScoreSummary.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GameConstantsAndTypes.h"


const int NUM_PAD_TO_DEVICE_SLOTS	= 3;	// three device inputs may map to one pad input
const int MAX_NUM_STAGES = 10;

class PrefsManager
{
public:
	PrefsManager();
	~PrefsManager();

	// Options that ARE saved between sessions
	bool	m_bWindowed;
	bool	m_bHighDetail;
	bool	m_bHighTextureDetail;
	bool	m_bIgnoreJoyAxes;
	bool	m_bShowFPS;
	bool	m_bUseRandomVis;
	bool	m_bAnnouncer;
	bool	m_bEventMode;
	int		m_iNumArcadeStages;
	int		m_iDifficulty;

	void ReadPrefsFromDisk();
	void SavePrefsToDisk();


	// Options that are NOT saved between sessions
	DifficultyClass	m_PreferredDifficultyClass[NUM_PLAYERS];
	SongSortOrder	m_SongSortOrder;				// used by MusicWheel and should be saved until the app exits
	PlayMode		m_PlayMode;
	int				m_iCurrentStage;				// starts at 1, and is incremented with each Stage Clear

	int				GetStageNumber();
	bool			IsFinalStage();
	CString			GetStageText();


	PlayerOptions	m_PlayerOptions[NUM_PLAYERS];
	SongOptions		m_SongOptions;

	ScoreSummary	m_ScoreSummary[NUM_PLAYERS][MAX_NUM_STAGES];	// for passing from Dancing to Results
};


extern PrefsManager*	PREFSMAN;	// global and accessable from anywhere in our program

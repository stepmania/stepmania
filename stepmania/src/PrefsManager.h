#pragma once
/*
-----------------------------------------------------------------------------
 Class: PrefsManager

 Desc: Holds user-chosen preferences and saves it between sessions.  This class 
    also has temporary holders for information that passed between windows - e.g.
	ScoreSummary.

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "RageInput.h"
#include "GameTypes.h"



const int NUM_PAD_TO_DEVICE_SLOTS	= 3;	// three device inputs may map to one pad input


class Song;

class PrefsManager
{
public:
	PrefsManager();
	~PrefsManager();

	ScoreSummary	m_ScoreSummary[NUM_PLAYERS];	// for passing from Dancing to Results
	SongSortOrder	m_SongSortOrder;				// used by MusicWheel and should be saved until the app exits
	int				m_iCurrentStage;				// number of stages played +1
	CString			GetStageText();

	DifficultyClass m_PreferredDifficultyClass[NUM_PLAYERS];
	GameOptions		m_GameOptions;
	GraphicOptions	m_GraphicOptions;
	PlayerOptions	m_PlayerOptions[NUM_PLAYERS];
	SongOptions		m_SongOptions;


	void ReadPrefsFromDisk();
	void SavePrefsToDisk();

protected:
};


extern PrefsManager*	PREFS;	// global and accessable from anywhere in our program

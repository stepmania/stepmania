#pragma once
/*
-----------------------------------------------------------------------------
 Class: GameState

 Desc: Holds game data that is not saved between sessions.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GameConstantsAndTypes.h"
#include "PlayerOptions.h"
#include "SongOptions.h"


class Song;
struct Notes;
class Course;
class GameDef;
class StyleDef;


class GameState
{
public:
	GameState();
	~GameState();
	void Reset();

	//
	// Main State Info
	//
	Game			m_CurGame;
	Style			m_CurStyle;
	PlayerNumber	m_MasterPlayerNumber;

	GameDef*	GetCurrentGameDef();
	StyleDef*	GetCurrentStyleDef();

	bool IsPlayerEnabled( PlayerNumber pn );
	bool IsPlayerEnabled( int p ) { return IsPlayerEnabled( (PlayerNumber)p ); };	// for those too lasy to cast all those p's to a PlayerNumber

	CString			m_sLoadingMessage;
	CString			m_sPreferredGroup;
	DifficultyClass	m_PreferredDifficultyClass[NUM_PLAYERS];
	SongSortOrder	m_SongSortOrder;			// used by MusicWheel
	PlayMode		m_PlayMode;
	bool			m_bEditing;
	int				m_iCurrentStageIndex;	// incremented on Eval screen

	int				GetStageIndex();
	bool			IsFinalStage();
	bool			IsExtraStage();
	bool			IsExtraStage2();
	CString			GetStageText();
	D3DXCOLOR		GetStageColor();

	//
	// State Info used during gameplay
	//
	Song*		m_pCurSong;
	Notes*		m_pCurNotes[NUM_PLAYERS];
	Course*		m_pCurCourse;


	//
	// Let a lot of classes access this info here so the don't have to keep their own copies
	float		m_fMusicSeconds;	// time into the current song
	float		m_fSongBeat;
	float		m_fCurBPS;
	bool		m_bFreeze;	// in the middle of a freeze

	void ResetMusicStatistics();		// Clear the values above
	
	CArray<Song*,Song*>	m_apSongsPlayed;	// an array of completed songs.  
											// This is useful for the final evaluation screen,
											// and used to calculate the time into a course
	float	GetElapsedSeconds();			// Arcade: time into current song.  Oni/Endless: time into current course

	int	m_iSongsIntoCourse;					// In Arcade, this value is meaningless.
											// In Oni and Endless, this is the number of songs played in the current course.
	int	m_iSongsBeforeFail[NUM_PLAYERS];	// In Arcade, this value is meaningless.
											// In Oni and Endless, this is the number of songs played before failing.
	float m_fSecondsBeforeFail[NUM_PLAYERS];// -1 means not yet failed
											// In Arcade, is the time into the current stage before failing.
											// In Oni and Endless this is the time into the current course before failing
	bool m_bUsedAutoPlayer;					// Used autoplayer at any time during any stage/course/song

	float	GetPlayerSurviveTime( PlayerNumber p );

	//
	// Statistics for: Arcade: for the current stage.  Oni/Endless: for current course 
	//
	int		m_iPossibleDancePoints[NUM_PLAYERS];
	int		m_iActualDancePoints[NUM_PLAYERS];
	int		m_TapNoteScores[NUM_PLAYERS][NUM_TAP_NOTE_SCORES];
	int		m_HoldNoteScores[NUM_PLAYERS][NUM_HOLD_NOTE_SCORES];
	int		m_iMaxCombo[NUM_PLAYERS];
	float	m_fScore[NUM_PLAYERS];
	float	m_fRadarPossible[NUM_PLAYERS][NUM_RADAR_VALUES];	// filled in by ScreenGameplay on end of notes
	float	m_fRadarActual[NUM_PLAYERS][NUM_RADAR_VALUES];		// filled in by ScreenGameplay on end of notes
	
	void ResetStageStatistics();		// Clear the values above
	void AccumulateStageStatistics();			// Accumulate values above into the values below 

	// Only used in final evaluation screen in play mode Arcade.
	// Before being displayed, these values should be normalized by dividing by number of stages
	int		m_iAccumPossibleDancePoints[NUM_PLAYERS];
	int		m_iAccumActualDancePoints[NUM_PLAYERS];
	int		m_AccumTapNoteScores[NUM_PLAYERS][NUM_TAP_NOTE_SCORES];
	int		m_AccumHoldNoteScores[NUM_PLAYERS][NUM_HOLD_NOTE_SCORES];
	int		m_iAccumMaxCombo[NUM_PLAYERS];
	float	m_fAccumScore[NUM_PLAYERS];
	float	m_fAccumRadarPossible[NUM_PLAYERS][NUM_RADAR_VALUES];
	float	m_fAccumRadarActual[NUM_PLAYERS][NUM_RADAR_VALUES];



	PlayerOptions	m_PlayerOptions[NUM_PLAYERS];
	SongOptions		m_SongOptions;
};


extern GameState*	GAMESTATE;	// global and accessable from anywhere in our program

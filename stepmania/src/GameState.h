#ifndef GAMESTATE_H
#define GAMESTATE_H
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
#include "Game.h"
#include "Style.h"
#include "Grade.h"


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
	bool			m_bPlayersCanJoin;	// true if it's not too late for a player to join - this only has an effect on the credits message
	bool			m_bSideIsJoined[2];	// left side, right side
	int				m_iCoins;
	PlayerNumber	m_MasterPlayerNumber;
	int				GetNumSidesJoined()
	{ 
		int iNumSidesJoined = 0;
		for( int c=0; c<2; c++ )
			if( m_bSideIsJoined[c] )
				iNumSidesJoined++;	// left side, and right side
		return iNumSidesJoined;
	};


	GameDef*	GetCurrentGameDef();
	const StyleDef*	GetCurrentStyleDef();

	bool IsPlayerEnabled( PlayerNumber pn );
	bool IsPlayerEnabled( int p ) { return IsPlayerEnabled( (PlayerNumber)p ); };	// for those too lasy to cast all those p's to a PlayerNumber

	CString			m_sLoadingMessage;
	CString			m_sPreferredGroup;
	Difficulty		m_PreferredDifficulty[NUM_PLAYERS];
	SongSortOrder	m_SongSortOrder;			// used by MusicWheel
	PlayMode		m_PlayMode;
	bool			m_bEditing;			// NoteField does special stuff when this is true
	bool			m_bDemonstration;	// ScreenGameplay does special stuff when this is true
	int				m_iCurrentStageIndex;	// incremented on Eval screen

	int				GetStageIndex();
	bool			IsFinalStage();
	bool			IsExtraStage();
	bool			IsExtraStage2();
	CString			GetStageText();
	RageColor		GetStageColor();

	//
	// State Info used during gameplay
	//
	Song*		m_pCurSong;
	Notes*		m_pCurNotes[NUM_PLAYERS];
	Course*		m_pCurCourse;


	//
	// Music statistic:  Arcade: the current stage (one song).  Oni/Endles: a single song in a course
	//
	// Let a lot of classes access this info here so the don't have to keep their own copies.
	//
	float		m_fMusicSeconds;	// time into the current song
	float		m_fSongBeat;
	float		m_fCurBPS;
	bool		m_bFreeze;	// in the middle of a freeze

	void ResetMusicStatistics();		// Call this when it's time to play a new song.  Clears the values above.
	
	//
	// Stage Statistics: Arcade: for the current stage (one song).  Oni/Endless: for current course (multiple songs)
	//
	int		m_iPossibleDancePoints[NUM_PLAYERS];
	int		m_iActualDancePoints[NUM_PLAYERS];
	int		m_TapNoteScores[NUM_PLAYERS][NUM_TAP_NOTE_SCORES];
	int		m_HoldNoteScores[NUM_PLAYERS][NUM_HOLD_NOTE_SCORES];
	int		m_iMaxCombo[NUM_PLAYERS];
	float	m_fScore[NUM_PLAYERS];
	float	m_fRadarPossible[NUM_PLAYERS][NUM_RADAR_VALUES];	// filled in by ScreenGameplay on end of notes
	float	m_fRadarActual[NUM_PLAYERS][NUM_RADAR_VALUES];		// filled in by ScreenGameplay on end of notes
	
	float	GetElapsedSeconds();			// Arcade: time into current song.  Oni/Endless: time into current course

	Grade	GetCurrentGrade( PlayerNumber pn );

	int	m_iSongsIntoCourse;					// In Arcade, this value is meaningless.
											// In Oni and Endless, this is the number of songs played in the current course.
	int	m_iSongsBeforeFail[NUM_PLAYERS];	// In Arcade, this value is meaningless.
											// In Oni and Endless, this is the number of songs played before failing.
	float m_fSecondsBeforeFail[NUM_PLAYERS];// -1 means not yet failed
											// In Arcade, is the time into the current stage before failing.
											// In Oni and Endless this is the time into the current course before failing

	float	GetPlayerSurviveTime( PlayerNumber pn );	// Returns time player has survived

	void AccumulateStageStatistics();		// Call this before clearing values.  Accumulate values above into the Session values below.
	void ResetStageStatistics();			// Clears the values above


	//
	// Session Statistics: Arcade: 3 songs.  Oni/Endless: one course.
	//
	vector<Song*>	m_apSongsPlayed;	// an array of completed songs.  
											// This is useful for the final evaluation screen,
											// and used to calculate the time into a course

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


	// Session statistics are cleared by calling Reset()

	PlayerOptions	m_PlayerOptions[NUM_PLAYERS];    // The currently active options
	PlayerOptions   m_SelectedOptions[NUM_PLAYERS];  // Keep track of player-selected options for
													 // courses separately from the active options.
	SongOptions		m_SongOptions;


	vector<CString> m_asNetworkNames;
};


extern GameState*	GAMESTATE;	// global and accessable from anywhere in our program

#endif

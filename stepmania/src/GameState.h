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
#include "GameplayStatistics.h"


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

	Song*		m_pCurSong;
	Notes*		m_pCurNotes[NUM_PLAYERS];
	Course*		m_pCurCourse;
	int			m_iCoursePossibleDancePoints;

	// Info used during gameplay
	// Let lots of classes access the music beat here so we don't have to pass it around everywhere
	float		m_fMusicSeconds;
	float		m_fMusicBeat;
	float		m_fCurBPS;
	bool		m_bFreeze;
	
	CArray<GameplayStatistics,GameplayStatistics>	m_aGameplayStatistics;	// for passing from Dancing to Results
	GameplayStatistics&		GetLatestGameplayStatistics();

protected:
	Game			m_CurGame;
public:
	Game			GetCurGame()	{ return m_CurGame; };
	void			SwitchGame( Game newGame );
	Style			m_CurStyle;
	PlayerNumber	m_MasterPlayerNumber;

	GameDef*	GetCurrentGameDef();
	StyleDef*	GetCurrentStyleDef();

	bool IsPlayerEnabled( PlayerNumber pn );
	//bool IsPlayerEnabled( int p ) { return IsPlayerEnabled( (PlayerNumber)p ); };	// for those too lasy to cast all those p's to a PlayerNumber


	CString			m_sLoadingMessage;
	CString			m_sPreferredGroup;
	DifficultyClass	m_PreferredDifficultyClass[NUM_PLAYERS];
	SongSortOrder	m_SongSortOrder;			// used by MusicWheel
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


extern GameState*	GAMESTATE;	// global and accessable from anywhere in our program

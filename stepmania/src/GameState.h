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
#include "StageStats.h"
#include "UnlockSystem.h"

class Song;
class Notes;
class Course;
class GameDef;
class StyleDef;
struct ModeChoice;
class NoteFieldPositioning;


class GameState
{
public:
	GameState();
	~GameState();
	void Reset();
	void ResetLastRanking();

	void Update( float fDelta );

	//
	// Main State Info
	//
	UnlockSystem	UnlockingSys;
	Game			m_CurGame;
	Style			m_CurStyle;
	bool			m_bPlayersCanJoin;	// true if it's not too late for a player to join - this only has an effect on the credits message
	bool			m_bSideIsJoined[NUM_PLAYERS];	// left side, right side
	PlayMode		m_PlayMode;			// many screens display different info depending on this value
	int				m_iCoins;			// not "credits"
	PlayerNumber	m_MasterPlayerNumber;	// used in Styles where one player controls both sides
	bool			m_bIsOnSystemMenu; // system screens will not be effected by the operator key -- Miryokuteki
	bool			m_bDifficultCourses; //used in nonstop
	int				GetNumSidesJoined()
	{ 
		int iNumSidesJoined = 0;
		for( int c=0; c<NUM_PLAYERS; c++ )
			if( m_bSideIsJoined[c] )
				iNumSidesJoined++;	// left side, and right side
		return iNumSidesJoined;
	};

	GameDef*	GetCurrentGameDef();
	const StyleDef*	GetCurrentStyleDef();

	void ApplyModeChoice( const ModeChoice& mc, PlayerNumber pn );
	bool IsPlayable( const ModeChoice& mc );

	void GetPlayerInfo( PlayerNumber pn, bool& bIsEnabledOut, bool& bIsHumanOut );
	bool IsPlayerEnabled( PlayerNumber pn );
	bool IsPlayerEnabled( int p ) { return IsPlayerEnabled( (PlayerNumber)p ); };
	bool IsHumanPlayer( PlayerNumber pn );
	bool IsHumanPlayer( int p ) { return IsHumanPlayer( (PlayerNumber)p ); };
	PlayerNumber GetFirstHumanPlayer();
	bool IsCpuPlayer( PlayerNumber pn );
	bool IsCpuPlayer( int p ) { return IsCpuPlayer( (PlayerNumber)p ); };
	bool AnyPlayersAreCpu()
	{ 
		for( int p=0; p<NUM_PLAYERS; p++ )
			if( IsCpuPlayer(p) )
				return true;
		return false;
	}

	PlayerController	m_PlayerController[NUM_PLAYERS];
	int					m_iCpuSkill[NUM_PLAYERS];	// only used when m_PlayerController is PC_CPU
	float				m_fSuperMeterGrowth[NUM_PLAYERS];	// between 0.0 and 1.0

	bool IsCourseMode() const;

	CString			m_sLoadingMessage;	// used in loading screen
	CString			m_sPreferredGroup;	// GROUP_ALL_MUSIC denotes no preferred group
	Difficulty		m_PreferredDifficulty[NUM_PLAYERS];
	SongSortOrder	m_SongSortOrder;			// used by MusicWheel
	bool			m_bEditing;			// NoteField does special stuff when this is true
	bool			m_bDemonstrationOrJukebox;	// ScreenGameplay does special stuff when this is true
	bool			m_bJukeboxUsesModifiers;
	int				m_iCurrentStageIndex;	// incremented on Eval screen.  For a Course, this is always 0

	int				GetStageIndex();
	int				GetNumStagesLeft();
	bool			IsFinalStage();
	bool			IsExtraStage();
	bool			IsExtraStage2();
	CString			GetStageText();
	int				GetCourseSongIndex();


	//
	// State Info used during gameplay
	//
	Song*		m_pCurSong;
	Notes*		m_pCurNotes[NUM_PLAYERS];
	Course*		m_pCurCourse;


	//
	// Music statistics:  Arcade: the current stage (one song).  Oni/Endles: a single song in a course
	//
	// Let a lot of classes access this info here so the don't have to keep their own copies.
	//
	float		m_fMusicSeconds;	// time into the current song
	float		m_fSongBeat;
	float		m_fCurBPS;
	bool		m_bFreeze;	// in the middle of a freeze
	bool		m_bPastHereWeGo;
	static const float MUSIC_SECONDS_INVALID;

	void ResetMusicStatistics();		// Call this when it's time to play a new song.  Clears the values above.
	void UpdateSongPosition(float fPositionSeconds);

	//
	// Stage Statistics: 
	// Arcade:	for the current stage (one song).  
	// Nonstop/Oni/Endless:	 for current course (which usually contains multiple songs)
	//
	StageStats	m_CurStageStats;				// current stage (not necessarily passed if Extra Stage)
	vector<StageStats>	m_vPassedStageStats;	// Only useful in Arcade for final evaluation
												// A song is only inserted here if at least one player passed.
												// StageStats are added by the Evaluation screen
	void	GetFinalEvalStatsAndSongs( StageStats& statsOut, vector<Song*>& vSongsOut );	// shown on arcade final evaluation


	//
	// Options stuff
	//

	PlayerOptions	m_CurrentPlayerOptions[NUM_PLAYERS];    // current approaches destination
	PlayerOptions	m_PlayerOptions[NUM_PLAYERS];			// change this, and current will move gradually toward it
	PlayerOptions   m_StoredPlayerOptions[NUM_PLAYERS];	// user's choices on the PlayerOptions screen
	SongOptions		m_SongOptions;
	SongOptions		m_StoredSongOptions;

	void ApplyModifiers( PlayerNumber pn, CString sModifiers );
	void StoreSelectedOptions();
	void RestoreSelectedOptions();


	// used in PLAY_MODE_RAVE
	CString m_sCharacterName[NUM_PLAYERS];
	CString	m_sAttacks[NUM_PLAYERS][NUM_ATTACK_LEVELS][NUM_ATTACKS_PER_LEVEL];

	
	// used in PLAY_MODE_RAVE and PLAY_MODE_BATTLE
	struct Attack
	{
		AttackLevel	level;
		float fSecsRemaining;
		CString sModifier;
		bool IsBlank() { return sModifier.empty(); }
		void MakeBlank() { sModifier=""; }
	};
	Attack	m_ActiveAttacks[NUM_PLAYERS][NUM_INVENTORY_SLOTS];

	// used in PLAY_MODE_BATTLE
	Attack	m_Inventory[NUM_PLAYERS][NUM_INVENTORY_SLOTS];
	float	m_fOpponentHealthPercent;
	
	bool	m_bActiveAttackEndedThisUpdate[NUM_PLAYERS];	// flag for other objects to watch (play sounds)
	void LaunchAttack( PlayerNumber target, Attack aa );
	void RebuildPlayerOptionsFromActiveAttacks( PlayerNumber pn );
	void RemoveAllActiveAttacks();	// called on end of song
	void RemoveAllInventory();
	int GetSumOfActiveAttackLevels( PlayerNumber pn );


	bool HasEarnedExtraStage();
	PlayerNumber GetWinner();
	BattleResult GetBattleResult( PlayerNumber pn );
	bool m_bAllow2ndExtraStage; //only used when "Allow Selection of Extra Stage is on"


	//
	// Ranking Stuff
	//

	// Filled in by ScreenNameEntry and used by ScreenRanking to flash the recent high scores
	NotesType			m_RankingNotesType;	// meaningless if a course was played
	RankingCategory		m_RankingCategory[NUM_PLAYERS];	// meaningless if a course was played
	Course*				m_pRankingCourse;		// meaningless unless Course was played
	int					m_iRankingIndex[NUM_PLAYERS];		// -1 if no new high score

	//
	// Arrow positioning
	//
	NoteFieldPositioning *m_pPosition;
};


extern GameState*	GAMESTATE;	// global and accessable from anywhere in our program

#endif

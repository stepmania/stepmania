#ifndef UNLOCK_SYSTEM_H
#define UNLOCK_SYSTEM_H

#include "Grade.h"
/*
-----------------------------------------------------------------------------
 Class: UnlockSystem

 Desc: The unlock system for Stepmania.

 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Kevin Slaughter
	Andrew Wong
-----------------------------------------------------------------------------
*/

class Song;

struct UnlockEntry
{
	UnlockEntry();

	CString m_sSongName;
	
	/* A cached pointer to the song or course this entry refers to.  Only one of
	 * these will be non-NULL. */
	Song	*m_pSong;
	Course	*m_pCourse;

	float	m_fDancePointsRequired;	// Ways to unlock/lock songs.
	float	m_fArcadePointsRequired;
	float	m_fSongPointsRequired;
	float	m_fExtraStagesCleared;
	float	m_fExtraStagesFailed;
	float	m_fStagesCleared;
	float	m_fToastysSeen;
	int		m_iRouletteSeed;

	bool	isLocked;    // cached locked tag
	bool	IsCourse() const { return m_pCourse != NULL; }

	void	UpdateLocked();  // updates isLocked

	void	UpdateData();
};

class UnlockSystem
{
	friend struct UnlockEntry;

public:
	/* TODO:
	enum UnlockType
	{
	UNLOCK_ARCADE_POINTS,
	UNLOCK_DANCE_POINTS,
	UNLOCK_SONG_POINTS,
	UNLOCK_EXTRA_CLEARED,
	UNLOCK_EXTRA_FAILED,
	UNLOCK_TOASTY,
	UNLOCK_CLEARED,
	NUM_UNLOCK_TYPES
	};
	*/
	UnlockSystem();

	// returns # of points till next unlock - used for ScreenUnlock
	float DancePointsUntilNextUnlock();
	float ArcadePointsUntilNextUnlock();
	float SongPointsUntilNextUnlock();

	// Used on select screens:
	bool SongIsLocked( const Song *song );
	bool SongIsRoulette( const Song *song );
	bool CourseIsLocked( const Course *course );

	// executed when program is first executed
	bool LoadFromDATFile();

	// Gets number of unlocks for title screen
	int GetNumUnlocks() const;

	// functions that add to values
	float UnlockAddAP(Grade credit);
	float UnlockAddAP(float credit);
	float UnlockAddDP(float credit);
	float UnlockAddSP(Grade credit);
	float UnlockAddSP(float credit);
	float UnlockClearExtraStage();
	float UnlockFailExtraStage();
	float UnlockClearStage();
	float UnlockToasty();

	// unlocks given song in roulette
	void RouletteUnlock( const Song *song );

	// read and write unlock in values
	bool ReadValues( CString filename);
	bool WriteValues( CString filename);
	
	void UpdateSongs();

	UnlockEntry *FindLockEntry( CString lockname );

	// All locked songs are stored here
	vector<UnlockEntry>	m_SongEntries;

private:
	UnlockEntry *FindSong( const Song *pSong );
	UnlockEntry *FindCourse( const Course *pCourse );

	void InitRouletteSeeds(int MaxRouletteSlot);  
	// makes RouletteSeeds more efficient

//	float m_fScores[NUM_UNLOCK_TYPES];

	// unlock values, cached
	float ArcadePoints;
	float DancePoints;
	float SongPoints;
	float ExtraClearPoints;
	float ExtraFailPoints;
	float ToastyPoints;
	float StagesCleared;
	CString RouletteSeeds;
};

extern UnlockSystem*	UNLOCKSYS;  // global and accessable from anywhere in program

#endif

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

struct UnlockEntry
{
	UnlockEntry();

	CString m_sSongName;
	
	/* A cached pointer to the song or course this entry refers to.  Only one of
	 * these will be non-NULL. */
	Song	*m_pSong;
	Course	*m_pCourse;

	float	m_fRequired[NUM_UNLOCK_TYPES];
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
	UnlockSystem();

	// returns # of points till next unlock - used for ScreenUnlock
	float PointsUntilNextUnlock( UnlockType t ) const;
	float ArcadePointsUntilNextUnlock() const { return PointsUntilNextUnlock(UNLOCK_ARCADE_POINTS); }
	float DancePointsUntilNextUnlock() const { return PointsUntilNextUnlock(UNLOCK_DANCE_POINTS); }
	float SongPointsUntilNextUnlock() const { return PointsUntilNextUnlock(UNLOCK_SONG_POINTS); }

	// Used on select screens:
	bool SongIsLocked( const Song *song );
	bool SongIsRoulette( const Song *song );
	bool CourseIsLocked( const Course *course );

	// executed when program is first executed
	bool LoadFromDATFile();

	// Gets number of unlocks for title screen
	int GetNumUnlocks() const;

	void UpdateSongs();

	// functions that add to values
	void UnlockAddAP(Grade credit);
	void UnlockAddAP(float credit);
	void UnlockAddDP(float credit);
	void UnlockAddSP(Grade credit);
	void UnlockAddSP(float credit);
	void UnlockClearExtraStage();
	void UnlockFailExtraStage();
	void UnlockClearStage();
	void UnlockToasty();

	// unlocks given song in roulette
	void RouletteUnlock( const Song *song );

	// read and write unlock in values
	bool ReadValues( CString filename);
	bool WriteValues( CString filename);
	
	UnlockEntry *FindLockEntry( CString lockname );

	// All locked songs are stored here
	vector<UnlockEntry>	m_SongEntries;

private:
	UnlockEntry *FindSong( const Song *pSong );
	UnlockEntry *FindCourse( const Course *pCourse );

	// makes RouletteSeeds more efficient
	void InitRouletteSeeds(int MaxRouletteSlot);  

	// unlock values, cached
	float m_fScores[NUM_UNLOCK_TYPES];
	CString RouletteSeeds;
};

extern UnlockSystem*	UNLOCKSYS;  // global and accessable from anywhere in program

#endif

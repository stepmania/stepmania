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
enum UnlockTypes { UNLOCK_AP, UNLOCK_DP, UNLOCK_SP,
				UNLOCK_EC, UNLOCK_EF, UNLOCK_SC,
				UNLOCK_TT, UNLOCK_RO};

class Song;

struct SongEntry
{
	SongEntry();

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

	// if song is selectable vai two means
	bool	SelectableWheel();
	bool	SelectableRoulette();

	void	UpdateLocked();  // updates isLocked

	void	UpdateData();
};

class UnlockSystem
{
public:
	UnlockSystem();

	float DancePointsUntilNextUnlock();
	float ArcadePointsUntilNextUnlock();
	float SongPointsUntilNextUnlock();
	// returns # of points till next unlock - used for ScreenUnlock

	// Used on select screens:
	bool SongIsLocked( const Song *song );
	bool SongIsRoulette( const Song *song );
	bool CourseIsLocked( const Course *course );

	// executed when program is first executed
	bool LoadFromDATFile( CString sPath );

	// All locked songs are stored here
	vector<SongEntry>	m_SongEntries;	

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

	void RouletteUnlock( const Song *song );
	// unlocks given song in roulette

	// read and write unlock in values
	bool ReadValues( CString filename);
	bool WriteValues( CString filename);
	
	// unlock values, cached
	float ArcadePoints;
	float DancePoints;
	float SongPoints;
	float ExtraClearPoints;
	float ExtraFailPoints;
	float ToastyPoints;
	float StagesCleared;
	CString RouletteSeeds;

	void UpdateSongs();

	SongEntry *FindLockEntry( CString lockname );

private:
	SongEntry *FindSong( const Song *pSong );
	SongEntry *FindCourse( const Course *pCourse );

	void InitRouletteSeeds(int MaxRouletteSlot);  
	// makes RouletteSeeds more efficient
};


#endif

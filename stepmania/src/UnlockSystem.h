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
	CString m_sSongName;	/* Name of the song in the DWI/SM file itself.. This allows
								for a lot easier compatibility since a lot of people's 
								song folders are named differantly, song names tend to
								be the same in the file.*/
	
	Song* ActualSong;        // pointer to actual song

	float	m_fDancePointsRequired;	// Ways to unlock/lock songs.
	float	m_fArcadePointsRequired;
	float	m_fSongPointsRequired;
	float	m_fExtraStagesCleared;
	float	m_fExtraStagesFailed;
	float	m_fStagesCleared;
	float	m_fToastysSeen;
	int		m_iRouletteSeed;

	bool	isLocked;    // cached locked tag
	bool	isCourse;	// used for unlock screen

	SongEntry();

	// if song is selectable vai two means
	bool	SelectableWheel();
	bool	SelectableRoulette();

	bool	updateLocked();  //updates isLocked flag
	Song	*GetSong() const;
};

class UnlockSystem
{
public:
	UnlockSystem();

	float DancePointsUntilNextUnlock();
	float ArcadePointsUntilNextUnlock();
	float SongPointsUntilNextUnlock();
	// returns # of points till next unlock - used for ScreenUnlock

	SongEntry *FindSong( CString songname );
	// retrieves a corresponding SongEntry, or NULL if none exists

	bool SongIsLocked( const Song *song );
	bool SongIsRoulette( const Song *song );
	bool CourseIsLocked( const Course *course );
	// used on select screens

	bool LoadFromDATFile( CString sPath );
	// executed when program is first executed

	vector<SongEntry>	m_SongEntries;	
	// All locked songs are stored here

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

	bool RouletteUnlock( const Song *song );
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

private:
	void SortSongEntriesArray();  // sorts unlocks
	SongEntry *FindSong( const Song *pSong );
	SongEntry *FindCourse( const Course *pCourse );

	void InitRouletteSeeds(int MaxRouletteSlot);  
	// makes RouletteSeeds more efficient
};


#endif

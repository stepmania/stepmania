/* UnlockSystem - Unlocks handling. */

#ifndef UNLOCK_SYSTEM_H
#define UNLOCK_SYSTEM_H

#include "Grade.h"
#include <set>

class Song;
class Profile;

enum UnlockType
{
	UNLOCK_ARCADE_POINTS,
	UNLOCK_DANCE_POINTS,
	UNLOCK_SONG_POINTS,
	UNLOCK_EXTRA_CLEARED,
	UNLOCK_EXTRA_FAILED,
	UNLOCK_TOASTY,
	UNLOCK_CLEARED,
	NUM_UNLOCK_TYPES,
	UNLOCK_INVALID,
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
	int		m_iCode;

	bool	IsCourse() const { return m_pCourse != NULL; }

	bool	IsLocked() const;
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
	bool SongIsLocked( const Song *song ) const;
	bool SongIsRouletteOnly( const Song *song ) const;
	bool CourseIsLocked( const Course *course ) const;

	// Gets number of unlocks for title screen
	int GetNumUnlocks() const;

	const UnlockEntry *FindLockEntry( CString lockname ) const;

	void GetPoints( const Profile *pProfile, float fScores[NUM_UNLOCK_TYPES] ) const;

	void UnlockCode( int num );

	// unlocks the song's code
	void UnlockSong( const Song *song );

	// All locked songs are stored here
	vector<UnlockEntry>	m_SongEntries;

	// If global song or course points change, call to update
	void UpdateSongs();

private:
	// read unlocks
	bool Load();
	
	const UnlockEntry *FindSong( const Song *pSong ) const;
	const UnlockEntry *FindCourse( const Course *pCourse ) const;

	set<int> m_RouletteCodes; // "codes" which are available in roulette and which unlock if rouletted
};

extern UnlockSystem*	UNLOCKMAN;  // global and accessable from anywhere in program

#endif

/*
 * (c) 2001-2004 Kevin Slaughter, Andrew Wong, Glenn Maynard
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

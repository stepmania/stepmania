/* UnlockManager - Unlocks handling. */

#ifndef UNLOCK_SYSTEM_H
#define UNLOCK_SYSTEM_H

#include "Grade.h"
#include "Command.h"
#include <set>
#include "Difficulty.h"

class Song;
class Course;
class Steps;
class Profile;
struct lua_State;

enum UnlockTrigger
{
	UnlockTrigger_ArcadePoints,
	UnlockTrigger_DancePoints,
	UnlockTrigger_SongPoints,
	UnlockTrigger_ExtraCleared,
	UnlockTrigger_ExtraFailed,
	UnlockTrigger_Toasties,
	UnlockTrigger_StagesCleared,
	NUM_UnlockTrigger,
	UnlockTrigger_INVALID,
};


struct UnlockEntry
{
	UnlockEntry()
	{
		m_Type = TYPE_INVALID;

		m_pSong = NULL;
		m_dc = DIFFICULTY_INVALID;
		m_pCourse = NULL;

		ZERO( m_fRequired );
		m_iEntryID = -1;
	}

	enum Type {
		TYPE_SONG, 
		TYPE_STEPS, 
		TYPE_COURSE, 
		TYPE_MODIFIER, 
		NUM_TYPES, 
		TYPE_INVALID
	};
	Type m_Type;
	Command m_cmd;

	/* A cached pointer to the song or course this entry refers to.  Only one of
	 * these will be non-NULL. */
	Song	*m_pSong;
	Difficulty m_dc;
	Course	*m_pCourse;

	float	m_fRequired[NUM_UnlockTrigger];
	int	m_iEntryID;

	bool	IsValid() const;
	bool	IsLocked() const;
};

class UnlockManager
{
	friend struct UnlockEntry;

public:
	UnlockManager();

	// returns # of points till next unlock - used for ScreenUnlock
	float PointsUntilNextUnlock( UnlockTrigger t ) const;
	float ArcadePointsUntilNextUnlock() const { return PointsUntilNextUnlock(UnlockTrigger_ArcadePoints); }
	float DancePointsUntilNextUnlock() const { return PointsUntilNextUnlock(UnlockTrigger_DancePoints); }
	float SongPointsUntilNextUnlock() const { return PointsUntilNextUnlock(UnlockTrigger_SongPoints); }

	// Used on select screens:
	bool SongIsLocked( const Song *song ) const;
	bool SongIsRouletteOnly( const Song *song ) const;
	bool StepsIsLocked( const Song *pSong, const Steps *pSteps ) const;
	bool CourseIsLocked( const Course *course ) const;
	bool ModifierIsLocked( const RString &sOneMod ) const;

	// Gets number of unlocks for title screen
	int GetNumUnlocks() const;

	void GetPoints( const Profile *pProfile, float fScores[NUM_UnlockTrigger] ) const;

	// Unlock an entry by code.
	void UnlockEntryID( int iEntryID );

	/*
	 * If a code is associated with at least one song or course, set the preferred song
	 * and/or course in GAMESTATE to them.
	 */
	void PreferUnlockEntryID( int iEntryID );

	// Unlocks a song.
	void UnlockSong( const Song *song );

	// Return the associated EntryID.
	int FindEntryID( const RString &sName ) const;

	// All locked songs are stored here
	vector<UnlockEntry>	m_UnlockEntries;

	// If global song or course points change, call to update
	void UpdateCachedPointers();

	void GetUnlocksByType( UnlockEntry::Type t, vector<UnlockEntry *> &apEntries );
	void GetSongsUnlockedByEntryID( vector<Song *> &apSongsOut, int iEntryID );
	void GetStepsUnlockedByEntryID( vector<Song *> &apSongsOut, vector<Difficulty> &apStepsOut, int iEntryID );

	// Lua
	void PushSelf( lua_State *L );

private:
	// read unlocks
	void Load();
	
	const UnlockEntry *FindSong( const Song *pSong ) const;
	const UnlockEntry *FindSteps( const Song *pSong, const Steps *pSteps ) const;
	const UnlockEntry *FindCourse( const Course *pCourse ) const;
	const UnlockEntry *FindModifier( const RString &sOneMod ) const;

	set<int> m_RouletteCodes; // "codes" which are available in roulette and which unlock if rouletted
};

extern UnlockManager*	UNLOCKMAN;  // global and accessable from anywhere in program

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

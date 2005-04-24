/* ProfileManager - Interface to machine and memory card profiles. */

#ifndef ProfileManager_H
#define ProfileManager_H

#include "PlayerNumber.h"
#include "GameConstantsAndTypes.h"
#include "Profile.h"

class Song;
class Style;
struct lua_State;

class ProfileManager
{
public:
	ProfileManager();
	~ProfileManager();

	void Init();

	bool CreateLocalProfile( CString sName );
	bool RenameLocalProfile( CString sProfileID, CString sNewName );
	bool DeleteLocalProfile( CString sProfileID );

	void GetLocalProfileIDs( vector<CString> &asProfileIDsOut ) const;
	void GetLocalProfileNames( vector<CString> &asNamesOut ) const;

	bool LoadFirstAvailableProfile( PlayerNumber pn );	// memory card or local profile
	bool LoadLocalProfileFromMachine( PlayerNumber pn );
	bool LoadProfileFromMemoryCard( PlayerNumber pn );
	void SaveAllProfiles() const;
	bool SaveProfile( PlayerNumber pn ) const;
	void UnloadProfile( PlayerNumber pn );
	
	//
	// General data
	//
	void IncrementToastiesCount( PlayerNumber pn );
	void AddStepTotals( PlayerNumber pn, int iNumTapsAndHolds, int iNumJumps, int iNumHolds, int iNumMines, int iNumHands, float fCaloriesBurned );


	//
	// High scores
	//
	void LoadMachineProfile();
	void SaveMachineProfile() const;

	bool IsUsingProfile( PlayerNumber pn ) const { return !m_sProfileDir[pn].empty(); }
	bool IsUsingProfile( ProfileSlot slot ) const;
	const Profile* GetProfile( PlayerNumber pn ) const;
	Profile* GetProfile( PlayerNumber pn ) { return (Profile*) ((const ProfileManager *) this)->GetProfile(pn); }
	const Profile* GetProfile( ProfileSlot slot ) const;
	Profile* GetProfile( ProfileSlot slot ) { return (Profile*) ((const ProfileManager *) this)->GetProfile(slot); }
	CString GetProfileDir( ProfileSlot slot ) const;

	Profile* GetMachineProfile() { return &m_MachineProfile; }

	CString GetPlayerName( PlayerNumber pn ) const;
	bool ProfileWasLoadedFromMemoryCard( PlayerNumber pn ) const;
	bool LastLoadWasTamperedOrCorrupt( PlayerNumber pn ) const;
	bool LastLoadWasFromLastGood( PlayerNumber pn ) const;


	//
	// Song stats
	//
	int GetSongNumTimesPlayed( const Song* pSong, ProfileSlot card ) const;
	bool IsSongNew( const Song* pSong ) const { return GetSongNumTimesPlayed(pSong,PROFILE_SLOT_MACHINE)==0; }
	void AddStepsScore( const Song* pSong, const Steps* pSteps, PlayerNumber pn, HighScore hs, int &iPersonalIndexOut, int &iMachineIndexOut );
	void IncrementStepsPlayCount( const Song* pSong, const Steps* pSteps, PlayerNumber pn );
	HighScore GetHighScoreForDifficulty( const Song *s, const Style *st, ProfileSlot slot, Difficulty dc ) const;

	//
	// Course stats
	//
	void AddCourseScore( const Course* pCourse, const Trail* pTrail, PlayerNumber pn, HighScore hs, int &iPersonalIndexOut, int &iMachineIndexOut );
	void IncrementCoursePlayCount( const Course* pCourse, const Trail* pTrail, PlayerNumber pn );

	//
	// Category stats
	//
	void AddCategoryScore( StepsType st, RankingCategory rc, PlayerNumber pn, HighScore hs, int &iPersonalIndexOut, int &iMachineIndexOut );
	void IncrementCategoryPlayCount( StepsType st, RankingCategory rc, PlayerNumber pn );


	// Lua
	void PushSelf( lua_State *L );

private:
	Profile::LoadResult LoadProfile( PlayerNumber pn, CString sProfileDir, bool bIsMemCard );

	// Directory that contains the profile.  Either on local machine or
	// on a memory card.
	CString m_sProfileDir[NUM_PLAYERS];

	bool m_bWasLoadedFromMemoryCard[NUM_PLAYERS];
	bool m_bLastLoadWasTamperedOrCorrupt[NUM_PLAYERS];	// true if Stats.xml was present, but failed to load (probably because of a signature failure)
	bool m_bLastLoadWasFromLastGood[NUM_PLAYERS];		// if true, then m_bLastLoadWasTamperedOrCorrupt is also true
	
	// actual loaded profile data
	Profile	m_Profile[NUM_PLAYERS];	

	Profile m_MachineProfile;

};


extern ProfileManager*	PROFILEMAN;	// global and accessable from anywhere in our program

#endif

/*
 * (c) 2003-2004 Chris Danford
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

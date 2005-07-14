/* ProfileManager - Interface to machine and memory card profiles. */

#ifndef ProfileManager_H
#define ProfileManager_H

#include "PlayerNumber.h"
#include "GameConstantsAndTypes.h"
#include "Difficulty.h"

class Profile;
class Song;
class Steps;
class Style;
class Course;
class Trail;
struct HighScore;
struct lua_State;

const int MAX_NUM_LOCAL_PROFILES = 8;

class ProfileManager
{
public:
	ProfileManager();
	~ProfileManager();

	void Init();

	// local profiles
	void RefreshLocalProfilesFromDisk();
	Profile &GetLocalProfile( const CString &sProfileID );
	
	bool CreateLocalProfile( CString sName );
	bool RenameLocalProfile( CString sProfileID, CString sNewName );
	bool DeleteLocalProfile( CString sProfileID );
	CString GetNewLocalProfileDefaultName() const;
	void GetLocalProfileIDs( vector<CString> &vsProfileIDsOut ) const;
	void GetLocalProfileDisplayNames( vector<CString> &vsProfileDisplayNamesOut ) const;


	bool LoadFirstAvailableProfile( PlayerNumber pn );	// memory card or local profile
	bool LoadLocalProfileFromMachine( PlayerNumber pn );
	bool LoadProfileFromMemoryCard( PlayerNumber pn );
	bool FastLoadProfileNameFromMemoryCard( CString sRootDir, CString &sName ) const;
	void SaveAllProfiles() const;
	bool SaveProfile( PlayerNumber pn ) const;
	void UnloadProfile( PlayerNumber pn );
	
	//
	// General data
	//
	void IncrementToastiesCount( PlayerNumber pn );
	void AddStepTotals( PlayerNumber pn, int iNumTapsAndHolds, int iNumJumps, int iNumHolds, int iNumRolls, int iNumMines, int iNumHands, float fCaloriesBurned );


	//
	// High scores
	//
	void LoadMachineProfile();
	void SaveMachineProfile() const;

	bool IsPersistentProfile( PlayerNumber pn ) const { return !m_sProfileDir[pn].empty(); }
	bool IsPersistentProfile( ProfileSlot slot ) const;

	// return a profile even if !IsUsingProfile
	const Profile* GetProfile( PlayerNumber pn ) const;
	Profile* GetProfile( PlayerNumber pn ) { return (Profile*) ((const ProfileManager *) this)->GetProfile(pn); }
	const Profile* GetProfile( ProfileSlot slot ) const;
	Profile* GetProfile( ProfileSlot slot ) { return (Profile*) ((const ProfileManager *) this)->GetProfile(slot); }
	
	CString GetProfileDir( ProfileSlot slot ) const;
	CString GetProfileDirImportedFrom( ProfileSlot slot ) const;

	Profile* GetMachineProfile() { return m_pMachineProfile; }

	CString GetPlayerName( PlayerNumber pn ) const;
	bool ProfileWasLoadedFromMemoryCard( PlayerNumber pn ) const;
	bool LastLoadWasTamperedOrCorrupt( PlayerNumber pn ) const;
	bool LastLoadWasFromLastGood( PlayerNumber pn ) const;


	//
	// Song stats
	//
	int GetSongNumTimesPlayed( const Song* pSong, ProfileSlot card ) const;
	bool IsSongNew( const Song* pSong ) const { return GetSongNumTimesPlayed(pSong,PROFILE_SLOT_MACHINE)==0; }
	void AddStepsScore( const Song* pSong, const Steps* pSteps , PlayerNumber pn, const HighScore &hs, int &iPersonalIndexOut, int &iMachineIndexOut );
	void IncrementStepsPlayCount( const Song* pSong, const Steps* pSteps, PlayerNumber pn );
	void GetHighScoreForDifficulty( const Song *s, const Style *st, ProfileSlot slot, Difficulty dc, HighScore &hsOut ) const;

	//
	// Course stats
	//
	void AddCourseScore( const Course* pCourse, const Trail* pTrail, PlayerNumber pn, const HighScore &hs, int &iPersonalIndexOut, int &iMachineIndexOut );
	void IncrementCoursePlayCount( const Course* pCourse, const Trail* pTrail, PlayerNumber pn );

	//
	// Category stats
	//
	void AddCategoryScore( StepsType st, RankingCategory rc, PlayerNumber pn, const HighScore &hs, int &iPersonalIndexOut, int &iMachineIndexOut );
	void IncrementCategoryPlayCount( StepsType st, RankingCategory rc, PlayerNumber pn );


	// Lua
	void PushSelf( lua_State *L );


	static bool ValidateLocalProfileName( const CString &sAnswer, CString &sErrorOut );


private:
	// returns Profile::LoadResult, but we don't want to depend on Profile
	int LoadProfile( PlayerNumber pn, CString sProfileDir, bool bIsMemCard );
	void GetMemoryCardProfileDirectoriesToTry( vector<CString> &asDirsToTry ) const;

	// Directory that contains the profile.  Either on local machine or
	// on a memory card.
	CString m_sProfileDir[NUM_PLAYERS];

	// MemoryCardProfileImportSubdirs name, if the profile was imported.
	CString m_sProfileDirImportedFrom[NUM_PLAYERS];

	bool m_bWasLoadedFromMemoryCard[NUM_PLAYERS];
	bool m_bLastLoadWasTamperedOrCorrupt[NUM_PLAYERS];	// true if Stats.xml was present, but failed to load (probably because of a signature failure)
	bool m_bLastLoadWasFromLastGood[NUM_PLAYERS];		// if true, then m_bLastLoadWasTamperedOrCorrupt is also true
	
	Profile	*m_pMemoryCardProfile[NUM_PLAYERS];	// holds Profile for the currently inserted card
	Profile *m_pMachineProfile;
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

#ifndef ProfileManager_H
#define ProfileManager_H

#include "PlayerNumber.h"
#include "GameConstantsAndTypes.h"
#include "Difficulty.h"
#include "Preference.h"
#include "Grade.h"
#include "Profile.h"

#include <vector>


class Song;
class Steps;
class Style;
class Course;
class Trail;
struct HighScore;
struct lua_State;

/** @brief Interface to machine and memory card profiles. */
class ProfileManager
{
public:
	ProfileManager();
	~ProfileManager();

	void Init();

	bool FixedProfiles() const;	// If true, profiles shouldn't be added/deleted

	// local profiles
	void UnloadAllLocalProfiles();
	void RefreshLocalProfilesFromDisk();
	void LoadLocalProfilesByPriority();
	void LoadLocalProfilesByRecent();
	void LoadLocalProfilesByName();

	const Profile *GetLocalProfile( const RString &sProfileID ) const;
	Profile *GetLocalProfile( const RString &sProfileID ) { return (Profile*) ((const ProfileManager *) this)->GetLocalProfile(sProfileID); }
	Profile *GetLocalProfileFromIndex( int iIndex );
	RString GetLocalProfileIDFromIndex( int iIndex );

	bool CreateLocalProfile( RString sName, RString &sProfileIDOut );
	void AddLocalProfileByID( Profile *pProfile, RString sProfileID ); // transfers ownership of pProfile
	bool RenameLocalProfile( RString sProfileID, RString sNewName );
	bool DeleteLocalProfile( RString sProfileID );
	void GetLocalProfileIDs( std::vector<RString> &vsProfileIDsOut ) const;
	void GetLocalProfileDisplayNames( std::vector<RString> &vsProfileDisplayNamesOut ) const;
	int GetLocalProfileIndexFromID( RString sProfileID ) const;
	int GetNumLocalProfiles() const;

	RString GetStatsPrefix() { return m_stats_prefix; }
	void SetStatsPrefix(RString const& prefix);

	bool LoadFirstAvailableProfile( PlayerNumber pn, bool bLoadEdits = true );	// memory card or local profile
	bool LoadLocalProfileFromMachine( PlayerNumber pn );
	bool LoadProfileFromMemoryCard( PlayerNumber pn, bool bLoadEdits = true );
	bool FastLoadProfileNameFromMemoryCard( RString sRootDir, RString &sName ) const;
	bool SaveProfile( PlayerNumber pn ) const;
	bool SaveLocalProfile( RString sProfileID );
	void UnloadProfile( PlayerNumber pn );

	void MergeLocalProfiles(RString const& from_id, RString const& to_id);
	void MergeLocalProfileIntoMachine(RString const& from_id, bool skip_totals);
	void ChangeProfileType(int index, ProfileType new_type);
	void MoveProfilePriority(int index, bool up);
	void MoveProfileTopBottom(int index, bool top);
	void MoveProfileSorted(int index, bool bAscending);

	// General data
	void IncrementToastiesCount( PlayerNumber pn );
	void AddStepTotals( PlayerNumber pn, int iNumTapsAndHolds, int iNumJumps, int iNumHolds, int iNumRolls, int iNumMines, int iNumHands, int iNumLifts, float fCaloriesBurned );

	// High scores
	void LoadMachineProfile();	// including edits
	void LoadMachineProfileEdits();
	void SaveMachineProfile() const;

	bool IsPersistentProfile( PlayerNumber pn ) const { return !m_sProfileDir[pn].empty(); }
	bool IsPersistentProfile( ProfileSlot slot ) const;

	// return a profile even if !IsUsingProfile
	const Profile* GetProfile( PlayerNumber pn ) const;
	Profile* GetProfile( PlayerNumber pn ) { return (Profile*) ((const ProfileManager *) this)->GetProfile(pn); }
	const Profile* GetProfile( ProfileSlot slot ) const;
	Profile* GetProfile( ProfileSlot slot ) { return (Profile*) ((const ProfileManager *) this)->GetProfile(slot); }

	const RString& GetProfileDir( ProfileSlot slot ) const;
	RString GetProfileDirImportedFrom( ProfileSlot slot ) const;

	Profile* GetMachineProfile() { return m_pMachineProfile; }

	RString GetPlayerName( PlayerNumber pn ) const;
	bool ProfileWasLoadedFromMemoryCard( PlayerNumber pn ) const;
	bool ProfileFromMemoryCardIsNew( PlayerNumber pn ) const;
	bool LastLoadWasTamperedOrCorrupt( PlayerNumber pn ) const;
	bool LastLoadWasFromLastGood( PlayerNumber pn ) const;

	// Song stats
	int GetSongNumTimesPlayed( const Song* pSong, ProfileSlot card ) const;
	bool IsSongNew( const Song* pSong ) const { return GetSongNumTimesPlayed(pSong,ProfileSlot_Machine)==0; }
	void AddStepsScore( const Song* pSong, const Steps* pSteps , PlayerNumber pn, const HighScore &hs, int &iPersonalIndexOut, int &iMachineIndexOut );
	void IncrementStepsPlayCount( const Song* pSong, const Steps* pSteps, PlayerNumber pn );

	// Course stats
	void AddCourseScore( const Course* pCourse, const Trail* pTrail, PlayerNumber pn, const HighScore &hs, int &iPersonalIndexOut, int &iMachineIndexOut );
	void IncrementCoursePlayCount( const Course* pCourse, const Trail* pTrail, PlayerNumber pn );

	// Category stats
	void AddCategoryScore( StepsType st, RankingCategory rc, PlayerNumber pn, const HighScore &hs, int &iPersonalIndexOut, int &iMachineIndexOut );
	void IncrementCategoryPlayCount( StepsType st, RankingCategory rc, PlayerNumber pn );

	static void GetMemoryCardProfileDirectoriesToTry( std::vector<RString> &asDirsToTry );

	// Lua
	void PushSelf( lua_State *L );

	static Preference<bool> m_bProfileStepEdits;
	static Preference<bool> m_bProfileCourseEdits;
	static Preference1D<RString> m_sDefaultLocalProfileID;

private:
	ProfileLoadResult LoadProfile( PlayerNumber pn, RString sProfileDir, bool bIsMemCard );

	// Directory that contains the profile.  Either on local machine or
	// on a memory card.
	RString m_sProfileDir[NUM_PLAYERS];

	// MemoryCardProfileImportSubdirs name, if the profile was imported.
	RString m_sProfileDirImportedFrom[NUM_PLAYERS];

	RString m_stats_prefix;

	bool m_bWasLoadedFromMemoryCard[NUM_PLAYERS];
	bool m_bLastLoadWasTamperedOrCorrupt[NUM_PLAYERS];	// true if Stats.xml was present, but failed to load (probably because of a signature failure)
	bool m_bLastLoadWasFromLastGood[NUM_PLAYERS];		// if true, then m_bLastLoadWasTamperedOrCorrupt is also true
	mutable bool m_bNeedToBackUpLastLoad[NUM_PLAYERS];	// if true, back up profile on next save
	bool m_bNewProfile[NUM_PLAYERS];

	Profile	*m_pMemoryCardProfile[NUM_PLAYERS];	// holds Profile for the currently inserted card
	Profile *m_pMachineProfile;
};

extern ProfileManager*	PROFILEMAN;	// global and accessible from anywhere in our program

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

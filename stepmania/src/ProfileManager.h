#ifndef ProfileManager_H
#define ProfileManager_H
/*
-----------------------------------------------------------------------------
 Class: ProfileManager

 Desc: Interface to profiles that exist on the machine or a memory card
	plugged into the machine.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "PlayerNumber.h"
#include "GameConstantsAndTypes.h"
#include "Style.h"
#include "Profile.h"

class Song;
class StyleDef;

class ProfileManager
{
public:
	ProfileManager();
	~ProfileManager();

	bool CreateLocalProfile( CString sName );
	bool RenameLocalProfile( CString sProfileID, CString sNewName );
	bool DeleteLocalProfile( CString sProfileID );

	void GetLocalProfileIDs( vector<CString> &asProfileIDsOut ) const;
	void GetLocalProfileNames( vector<CString> &asNamesOut ) const;

	bool LoadFirstAvailableProfile( PlayerNumber pn, bool bLoadNamesOnly );	// memory card or local profile
	bool LoadProfileFromMemoryCard( PlayerNumber pn, bool bLoadNamesOnly );
	bool SaveProfile( PlayerNumber pn ) const;
	void UnloadProfile( PlayerNumber pn );
	
	//
	// General data
	//
	void IncrementToastiesCount( PlayerNumber pn );
	void AddStepTotals( PlayerNumber pn, int iNumTapsAndHolds, int iNumJumps, int iNumHolds, int iNumMines, int iNumHands );


	//
	// High scores
	//
	void LoadMachineProfile();
	void SaveMachineProfile();

	bool IsUsingProfile( PlayerNumber pn ) const { return !m_sProfileDir[pn].empty(); }
	bool IsUsingProfile( ProfileSlot slot ) const
	{
		switch( slot )
		{
		case PROFILE_SLOT_PLAYER_1:
		case PROFILE_SLOT_PLAYER_2:
			return !m_sProfileDir[slot].empty(); 
		case PROFILE_SLOT_MACHINE:
			return true;
		default:
			ASSERT(0);
			return false;
		}
	}
	const Profile* GetProfile( PlayerNumber pn ) const;
	Profile* GetProfile( PlayerNumber pn ) { return (Profile*) ((const ProfileManager *) this)->GetProfile(pn); }
	const Profile* GetProfile( ProfileSlot slot ) const;
	Profile* GetProfile( ProfileSlot slot ) { return (Profile*) ((const ProfileManager *) this)->GetProfile(slot); }
	CString GetProfileDir( ProfileSlot slot ) const;

	Profile* GetMachineProfile() { return &m_MachineProfile; }

	CString GetPlayerName( PlayerNumber pn ) const;
	bool ProfileWasLoadedFromMemoryCard( PlayerNumber pn ) const;


	//
	// Song stats
	//
	int GetSongNumTimesPlayed( const Song* pSong, ProfileSlot card ) const;
	bool IsSongNew( const Song* pSong ) const { return GetSongNumTimesPlayed(pSong,PROFILE_SLOT_MACHINE)==0; }
	void AddStepsHighScore( const Song* pSong, const Steps* pSteps, PlayerNumber pn, HighScore hs, int &iPersonalIndexOut, int &iMachineIndexOut );
	void IncrementStepsPlayCount( const Song* pSong, const Steps* pSteps, PlayerNumber pn );
	HighScore GetHighScoreForDifficulty( const Song *s, const StyleDef *st, ProfileSlot slot, Difficulty dc ) const;

	//
	// Course stats
	//
	void AddCourseHighScore( const Course* pCourse, StepsType st, CourseDifficulty cd, PlayerNumber pn, HighScore hs, int &iPersonalIndexOut, int &iMachineIndexOut );
	void IncrementCoursePlayCount( const Course* pCourse, StepsType st, CourseDifficulty cd, PlayerNumber pn );

	//
	// Category stats
	//
	void AddCategoryHighScore( StepsType nt, RankingCategory rc, PlayerNumber pn, HighScore hs, int &iPersonalIndexOut, int &iMachineIndexOut );
	void IncrementCategoryPlayCount( StepsType nt, RankingCategory rc, PlayerNumber pn );


//	void ReadSM300NoteScores();

private:
	bool LoadDefaultProfileFromMachine( PlayerNumber pn, bool bLoadNamesOnly );
	bool CreateMemoryCardProfile( PlayerNumber pn );
	bool LoadProfile( PlayerNumber pn, CString sProfileDir, bool bIsMemCard, bool bLoadNamesOnly );
	bool CreateProfile( CString sProfileDir, CString sName );

	// Directory that contains the profile.  Either on local machine or
	// on a memory card.
	CString m_sProfileDir[NUM_PLAYERS];

	bool m_bWasLoadedFromMemoryCard[NUM_PLAYERS];
	
	// actual loaded profile data
	Profile	m_Profile[NUM_PLAYERS];	

	Profile m_MachineProfile;

};


extern ProfileManager*	PROFILEMAN;	// global and accessable from anywhere in our program

#endif

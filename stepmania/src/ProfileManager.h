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

	void GetLocalProfileIDs( vector<CString> &asProfileIDsOut );
	void GetLocalProfileNames( vector<CString> &asNamesOut );

	bool LoadProfileFromMemoryCard( PlayerNumber pn );
//	bool LoadFirstAvailableProfile( PlayerNumber pn );	// memory card or local profile
	bool SaveProfile( PlayerNumber pn );
	void UnloadProfile( PlayerNumber pn );

	void SaveMachineProfile();

	bool IsUsingProfile( PlayerNumber pn ) { return !m_sProfileDir[pn].empty(); }
	bool IsUsingProfile( ProfileSlot slot )
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
	Profile* GetProfile( PlayerNumber pn );
	Profile* GetProfile( ProfileSlot slot );
	CString GetProfileDir( ProfileSlot slot );

	Profile* GetMachineProfile() { return &m_MachineProfile; }

	CString GetPlayerName( PlayerNumber pn );
	bool ProfileWasLoadedFromMemoryCard( PlayerNumber pn );


	//
	// High scores
	//
	void InitMachineScoresFromDisk();
	void SaveMachineScoresToDisk();

	//
	// Song stats
	//
	int GetSongNumTimesPlayed( Song* pSong, ProfileSlot card );
	bool IsSongNew( Song* pSong ) { return GetSongNumTimesPlayed(pSong,PROFILE_SLOT_MACHINE)==0; }
	void AddStepsHighScore( const Steps* pSteps, PlayerNumber pn, HighScore hs, int &iPersonalIndexOut, int &iMachineIndexOut );
	void IncrementStepsPlayCount( const Steps* pSteps, PlayerNumber pn );
	HighScore GetHighScoreForDifficulty( const Song *s, const StyleDef *st, ProfileSlot slot, Difficulty dc );

	//
	// Course stats
	//
	void AddCourseHighScore( const Course* pCourse, StepsType st, PlayerNumber pn, HighScore hs, int &iPersonalIndexOut, int &iMachineIndexOut );
	void IncrementCoursePlayCount( const Course* pCourse, StepsType st, PlayerNumber pn );

	//
	// Category stats
	//
	void AddCategoryHighScore( StepsType nt, RankingCategory rc, PlayerNumber pn, HighScore hs, int &iPersonalIndexOut, int &iMachineIndexOut );
	void IncrementCategoryPlayCount( StepsType nt, RankingCategory rc, PlayerNumber pn );


//	void ReadSM300NoteScores();

private:
	bool LoadDefaultProfileFromMachine( PlayerNumber pn );
	bool CreateMemoryCardProfile( PlayerNumber pn );
	bool LoadProfile( PlayerNumber pn, CString sProfileDir, bool bIsMemCard );
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

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

struct Profile
{
	Profile() { Init(); }
	void Init()
	{
		m_sName = "";
		m_sLastUsedHighScoreName = "";
		m_bUsingProfileDefaultModifiers = false;
		m_sDefaultModifiers = "";
		m_iTotalPlays = 0;
		m_iTotalPlaySeconds = 0;
		m_iTotalGameplaySeconds = 0;
		m_iCurrentCombo = 0;

		int i;
		for( i=0; i<NUM_PLAY_MODES; i++ )
			m_iNumSongsPlayedByPlayMode[i] = 0;
		for( i=0; i<NUM_STYLES; i++ )
			m_iNumSongsPlayedByStyle[i] = 0;
		for( i=0; i<NUM_DIFFICULTIES; i++ )
			m_iNumSongsPlayedByDifficulty[i] = 0;
		for( i=0; i<MAX_METER+1; i++ )
			m_iNumSongsPlayedByMeter[i] = 0;
	}

	bool LoadFromIni( CString sIniPath );
	bool SaveToIni( CString sIniPath );
	CString m_sName;
	CString m_sLastUsedHighScoreName;
	bool m_bUsingProfileDefaultModifiers;
	CString m_sDefaultModifiers;
	int m_iTotalPlays;
	int m_iTotalPlaySeconds;
	int m_iTotalGameplaySeconds;
	int m_iCurrentCombo;
	int m_iNumSongsPlayedByPlayMode[NUM_PLAY_MODES];
	int m_iNumSongsPlayedByStyle[NUM_STYLES];
	int m_iNumSongsPlayedByDifficulty[NUM_DIFFICULTIES];
	int m_iNumSongsPlayedByMeter[MAX_METER+1];
};

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

	bool LoadFirstAvailableProfile( PlayerNumber pn );
	bool SaveProfile( PlayerNumber pn );
	void UnloadProfile( PlayerNumber pn );

	void SaveMachineProfile();

	bool IsUsingProfile( PlayerNumber pn ) { return !m_sProfileDir[pn].empty(); }
	Profile* GetProfile( PlayerNumber pn );

	Profile* GetMachineProfile() { return &m_MachineProfile; }

	CString GetPlayerName( PlayerNumber pn );
	bool IsUsingMemoryCard( PlayerNumber pn ) { return m_bUsingMemoryCard[pn]; }


	//
	// High scores
	//
	void InitMachineScoresFromDisk();
	void SaveMachineScoresToDisk();

	struct CategoryData
	{
		struct HighScore
		{
			CString	sName;
			int iScore;
			float fPercentDP;

			HighScore()
			{
				iScore = 0;
				fPercentDP = 0;
			}

			bool operator>=( const HighScore& other ) const;
		};
		vector<HighScore> vHighScores;

		void AddHighScore( HighScore hs, int &iIndexOut );

	} m_CategoryDatas[NUM_MEMORY_CARDS][NUM_STEPS_TYPES][NUM_RANKING_CATEGORIES];

	void AddHighScore( StepsType nt, RankingCategory rc, PlayerNumber pn, CategoryData::HighScore hs, int &iPersonalIndexOut, int &iMachineIndexOut )
	{
		hs.sName = RANKING_TO_FILL_IN_MARKER[pn];
		m_CategoryDatas[pn][nt][rc].AddHighScore( hs, iPersonalIndexOut );
		m_CategoryDatas[MEMORY_CARD_MACHINE][nt][rc].AddHighScore( hs, iMachineIndexOut );
	}

	void ReadSM300NoteScores();
	void ReadSongScoresFromDir( CString sDir, MemoryCard mc );
	void ReadCourseScoresFromDir( CString sDir, MemoryCard mc );
	void ReadCategoryScoresFromDir( CString sDir, MemoryCard mc );

	void SaveSongScoresToDir( CString sDir, MemoryCard mc );
	void SaveCourseScoresToDir( CString sDir, MemoryCard mc );
	void SaveCategoryScoresToDir( CString sDir, MemoryCard mc );
	void SaveStatsWebPageToDir( CString sDir, MemoryCard mc );

private:
	bool LoadDefaultProfileFromMachine( PlayerNumber pn );
	bool CreateMemoryCardProfile( PlayerNumber pn );
	bool LoadProfileFromMemoryCard( PlayerNumber pn );
	bool LoadProfile( PlayerNumber pn, CString sProfileDir, bool bIsMemCard );
	bool CreateProfile( CString sProfileDir, CString sName );

	// Directory that contains the profile.  Either on local machine or
	// on a memory card.
	CString m_sProfileDir[NUM_PLAYERS];

	bool m_bUsingMemoryCard[NUM_PLAYERS];
	
	// actual loaded profile data
	Profile	m_Profile[NUM_PLAYERS];	

	Profile m_MachineProfile;

};


extern ProfileManager*	PROFILEMAN;	// global and accessable from anywhere in our program

#endif

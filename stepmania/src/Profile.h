#ifndef Profile_H
#define Profile_H
/*
-----------------------------------------------------------------------------
 Class: Profile

 Desc: Player data that persists between sessions.  Can be stored on a local
	disk or on a memory card.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GameConstantsAndTypes.h"
#include "Style.h"
#include "Grade.h"
#include <map>
#include "XmlFile.h"
#include "HighScore.h"

class Song;
class Steps;
class Course;

class Profile
{
public:
	Profile()
	{
		InitAll();
	}

	//
	// smart accessors
	//
	CString GetDisplayName() const;
	CString GetDisplayCaloriesBurned();
	int GetTotalNumSongsPlayed();
	static CString GetProfileDisplayNameFromDir( CString sDir );
	int GetSongNumTimesPlayed( const Song* pSong ) const;

	//
	// General data
	//
	CString m_sName;
	CString m_sLastUsedHighScoreName;
	bool m_bUsingProfileDefaultModifiers;
	CString m_sDefaultModifiers;
	int m_iTotalPlays;
	int m_iTotalPlaySeconds;
	int m_iTotalGameplaySeconds;
	int m_iCurrentCombo;
	float m_fWeightPounds;
	int m_fCaloriesBurned;
	mutable CString m_sLastMachinePlayed;	// mutable because we overwrite this on save, and I don't want to remove const from the whole save chain. -Chris
	int m_iNumSongsPlayedByPlayMode[NUM_PLAY_MODES];
	int m_iNumSongsPlayedByStyle[NUM_STYLES];
	int m_iNumSongsPlayedByDifficulty[NUM_DIFFICULTIES];
	int m_iNumSongsPlayedByMeter[MAX_METER+1];


	//
	// Steps high scores
	//
	struct HighScoresForASteps
	{
		HighScoreList hs;
	};
	std::map<const Steps*,HighScoresForASteps>	m_StepsHighScores;

	void AddStepsHighScore( const Steps* pSteps, HighScore hs, int &iIndexOut );
	HighScoreList& GetStepsHighScoreList( const Steps* pSteps );
	const HighScoreList& GetStepsHighScoreList( const Steps* pSteps ) const;
	int GetStepsNumTimesPlayed( const Steps* pSteps ) const;
	void IncrementStepsPlayCount( const Steps* pSteps );


	//
	// Course high scores
	//
	// struct was a typedef'd array of HighScores, but VC6 freaks out 
	// in processing the templates for map::operator[].
	struct HighScoresForACourse	
	{
		HighScoreList hs[NUM_STEPS_TYPES];
	};
	std::map<const Course*,HighScoresForACourse>	m_CourseHighScores;

	void AddCourseHighScore( const Course* pCourse, StepsType st, HighScore hs, int &iIndexOut );
	HighScoreList& GetCourseHighScoreList( const Course* pCourse, StepsType st );
	const HighScoreList& GetCourseHighScoreList( const Course* pCourse, StepsType st ) const;
	int GetCourseNumTimesPlayed( const Course* pCourse ) const;
	void IncrementCoursePlayCount( const Course* pCourse, StepsType st );


	//
	// Category high scores
	//
	HighScoreList m_CategoryHighScores[NUM_STEPS_TYPES][NUM_RANKING_CATEGORIES];

	void AddCategoryHighScore( StepsType st, RankingCategory rc, HighScore hs, int &iIndexOut );
	HighScoreList& GetCategoryHighScoreList( StepsType st, RankingCategory rc );
	const HighScoreList& GetCategoryHighScoreList( StepsType st, RankingCategory rc ) const;
	int GetCategoryNumTimesPlayed( StepsType st ) const;
	void IncrementCategoryPlayCount( StepsType st, RankingCategory rc );


	//
	// Init'ing
	//
	void InitAll()
	{
		InitGeneralData(); 
		InitSongScores(); 
		InitCourseScores(); 
		InitCategoryScores(); 
	}
	void InitGeneralData(); 
	void InitSongScores(); 
	void InitCourseScores(); 
	void InitCategoryScores(); 

	//
	// Loading and saving
	//
	bool LoadAllFromDir( CString sDir );
	bool SaveAllToDir( CString sDir ) const;

	bool LoadGeneralDataFromDir( CString sDir );
	bool SaveGeneralDataToDir( CString sDir ) const;

	void LoadSongScoresFromDirSM390a12( CString sDir );
	void LoadSongScoresFromDir( CString sDir );
	void SaveSongScoresToDir( CString sDir ) const;
	void DeleteSongScoresFromDirSM390a12( CString sDir ) const;
	
	void LoadCourseScoresFromDirSM390a12( CString sDir );
	void LoadCourseScoresFromDir( CString sDir );
	void SaveCourseScoresToDir( CString sDir ) const;
	void DeleteCourseScoresFromDirSM390a12( CString sDir ) const;
	
	void LoadCategoryScoresFromDirSM390a12( CString sDir );
	void LoadCategoryScoresFromDir( CString sDir );
	void SaveCategoryScoresToDir( CString sDir ) const;
	void DeleteCategoryScoresFromDirSM390a12( CString sDir ) const;

	void SaveStatsWebPageToDir( CString sDir ) const;

	void SaveMachinePublicKeyToDir( CString sDir ) const;
};


#endif

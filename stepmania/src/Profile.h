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

class Steps;
class Course;

struct HighScore
{
	CString	sName;
	Grade grade;
	int iScore;
	float fPercentDP;
	float fSurviveTime;

	HighScore()
	{
		grade = GRADE_NO_DATA;
		iScore = 0;
		fPercentDP = 0;
		fSurviveTime = 0;
	}

	bool operator>=( const HighScore& other ) const;
};

struct HighScoreList
{
	int iNumTimesPlayed;
	vector<HighScore> vHighScores;

	
	HighScoreList()
	{
		iNumTimesPlayed = 0;
	}

	void AddHighScore( HighScore hs, int &iIndexOut );

	const HighScore& GetTopScore() const;
};


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
		m_fWeightPounds = 0;
		m_fCaloriesBurned = 0;

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

	CString GetDisplayName();
	CString GetDisplayCaloriesBurned();
	int GetTotalNumSongsPlayed();

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
	float m_fWeightPounds;
	int m_fCaloriesBurned;
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
	int GetCourseNumTimesPlayed( const Course* pCourse ) const;
	void IncrementCoursePlayCount( const Course* pCourse, StepsType st );


	//
	// Category high scores
	//
	HighScoreList m_CategoryHighScores[NUM_STEPS_TYPES][NUM_RANKING_CATEGORIES];

	void AddCategoryHighScore( StepsType st, RankingCategory rc, HighScore hs, int &iIndexOut );
	HighScoreList& GetCategoryHighScoreList( StepsType st, RankingCategory rc );
	int GetCategoryNumTimesPlayed( RankingCategory rc ) const;
	void IncrementCategoryPlayCount( StepsType st, RankingCategory rc );
};


#endif

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
	// Editable data
	//
	CString m_sDisplayName;
	float m_fWeightPounds;

	//
	// General data
	//
	CString m_sLastUsedHighScoreName;
	bool m_bUsingProfileDefaultModifiers;
	CString m_sDefaultModifiers;
	int m_iTotalPlays;
	int m_iTotalPlaySeconds;
	int m_iTotalGameplaySeconds;
	int m_iCurrentCombo;
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
	// Screenshot Data
	//
	struct Screenshot
	{
		CString sFileName;	// no directory part - just the file name
		CString sSignature;	// sFile signed with the machine's private key
		HighScore highScore;	// high score that the screenshot is taken of
	};
	vector<Screenshot> m_vScreenshots;
	void AddScreenshot( Screenshot screenshot );

	
	//
	// Init'ing
	//
	void InitAll()
	{
		InitEditableData(); 
		InitGeneralData(); 
		InitSongScores(); 
		InitCourseScores(); 
		InitCategoryScores(); 
		InitScreenshotData(); 
	}
	void InitEditableData(); 
	void InitGeneralData(); 
	void InitSongScores(); 
	void InitCourseScores(); 
	void InitCategoryScores(); 
	void InitScreenshotData(); 

	//
	// Loading and saving
	//
	bool LoadAllFromDir( CString sDir );	// return false on 
	bool SaveAllToDir( CString sDir ) const;

	void LoadProfileDataFromDirSM390a12( CString sDir );
	void LoadSongScoresFromDirSM390a12( CString sDir );
	void LoadCourseScoresFromDirSM390a12( CString sDir );
	void LoadCategoryScoresFromDirSM390a12( CString sDir );
	
	void LoadEditableDataFromNode( const XNode* pNode );
	void LoadGeneralDataFromNode( const XNode* pNode );
	void LoadSongScoresFromNode( const XNode* pNode );
	void LoadCourseScoresFromNode( const XNode* pNode );
	void LoadCategoryScoresFromNode( const XNode* pNode );
	void LoadScreenshotDataFromNode( const XNode* pNode );

	XNode* SaveEditableDataCreateNode() const;
	XNode* SaveGeneralDataCreateNode() const;
	XNode* SaveSongScoresCreateNode() const;
	XNode* SaveCourseScoresCreateNode() const;
	XNode* SaveCategoryScoresCreateNode() const;
	XNode* SaveScreenshotDataCreateNode() const;

	void DeleteProfileDataFromDirSM390a12( CString sDir ) const;
	void DeleteSongScoresFromDirSM390a12( CString sDir ) const;
	void DeleteCourseScoresFromDirSM390a12( CString sDir ) const;
	void DeleteCategoryScoresFromDirSM390a12( CString sDir ) const;

	void SaveStatsWebPageToDir( CString sDir ) const;
	void SaveMachinePublicKeyToDir( CString sDir ) const;
};


#endif

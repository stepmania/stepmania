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
#include <set>
#include "XmlFile.h"
#include "HighScore.h"


//
// Current file versions
//
const CString STATS_XML				= "Stats.xml";

const CString EDITABLE_INI			= "Editable.ini";
// Editable data is an INI because the default INI file association on Windows 
// systems will open the ini file in an editor.  The default association for 
// XML will open in IE.  Users have a much better chance of discovering how to 
// edit this data if they don't have to fight against the file associations.

const CString DONT_SHARE_SIG		= "DontShare.sig";
// The "don't share" file is something that the user should always keep private.
// They can safely share STATS_XML with STATS_XML's signature so that others
// can authenticate the STATS_XML data.  However, others can't copy that data
// to their own profile for use in the game unless they also have the "don't 
// share" file.  DontShare contains a piece of information that we can 
// construct using STATS_XML but the user can't construct using STATS_XML.
// The file contains a signature of the STATS_XML's signature.

const CString PUBLIC_KEY_FILE		= "public.key";
const CString SCREENSHOTS_SUBDIR	= "Screenshots/";
const CString EDITS_SUBDIR			= "Edits/";

#define REASONABLE_EDITABLE_INI_SIZE_BYTES	2*1000		// 2KB
#define REASONABLE_STATS_XML_SIZE_BYTES		5*1000*1000	// 5MB



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
	int GetTotalNumSongsPlayed() const;
	int GetTotalNumSongsPassed() const;
	static CString GetProfileDisplayNameFromDir( CString sDir );
	int GetSongNumTimesPlayed( const Song* pSong ) const;

	//
	// Editable data
	//
	CString m_sDisplayName;
	CString m_sLastUsedHighScoreName;	// this doesn't really belong in "editable", but we need it in the smaller editable file so that it can be ready quickly.
	float m_fWeightPounds;

	//
	// General data
	//
	bool m_bUsingProfileDefaultModifiers;
	CString m_sDefaultModifiers;
	int m_iTotalPlays;
	int m_iTotalPlaySeconds;
	int m_iTotalGameplaySeconds;
	int m_iCurrentCombo;
	int m_fCaloriesBurned;
	int m_iTotalDancePoints;
	int m_iNumExtraStagesPassed;
	int m_iNumExtraStagesFailed;
	int m_iNumToasties;
	set<int> m_UnlockedSongs;
	mutable CString m_sLastMachinePlayed;	// mutable because we overwrite this on save, and I don't want to remove const from the whole save chain. -Chris
	int m_iNumSongsPlayedByPlayMode[NUM_PLAY_MODES];
	int m_iNumSongsPlayedByStyle[NUM_STYLES];
	int m_iNumSongsPlayedByDifficulty[NUM_DIFFICULTIES];
	int m_iNumSongsPlayedByMeter[MAX_METER+1];
	int m_iNumSongsPassedByPlayMode[NUM_PLAY_MODES];
	int m_iNumSongsPassedByGrade[NUM_GRADES];


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
		HighScoreList hs[NUM_STEPS_TYPES][NUM_COURSE_DIFFICULTIES];
	};
	std::map<const Course*,HighScoresForACourse>	m_CourseHighScores;

	void AddCourseHighScore( const Course* pCourse, StepsType st, CourseDifficulty cd, HighScore hs, int &iIndexOut );
	HighScoreList& GetCourseHighScoreList( const Course* pCourse, StepsType st, CourseDifficulty cd );
	const HighScoreList& GetCourseHighScoreList( const Course* pCourse, StepsType st, CourseDifficulty cd ) const;
	int GetCourseNumTimesPlayed( const Course* pCourse ) const;
	void IncrementCoursePlayCount( const Course* pCourse, StepsType st, CourseDifficulty cd );


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
		CString sMD5;		// MD5 hash of the screenshot file
		time_t time;		// return value of time() when screenshot was taken
		CString sLocation;	// location name where this screenshot was taken
	};
	vector<Screenshot> m_vScreenshots;
	void AddScreenshot( Screenshot screenshot );
	int GetNextScreenshotIndex() { return m_vScreenshots.size(); }

	
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
	
	void LoadEditableDataFromDir( CString sDir );
	void LoadGeneralDataFromNode( const XNode* pNode );
	void LoadSongScoresFromNode( const XNode* pNode );
	void LoadCourseScoresFromNode( const XNode* pNode );
	void LoadCategoryScoresFromNode( const XNode* pNode );
	void LoadScreenshotDataFromNode( const XNode* pNode );

	void SaveEditableDataToDir( CString sDir ) const;
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

/* Profile - Player data that persists between sessions.  Can be stored on a local disk or on a memory card. */

#ifndef Profile_H
#define Profile_H

#include "GameConstantsAndTypes.h"
#include "Grade.h"
#include <map>
#include <set>
#include <deque>
#include "HighScore.h"
#include "DateTime.h"
#include "SongUtil.h"	// for SongID
#include "StepsUtil.h"	// for StepsID
#include "CourseUtil.h"	// for CourseID
#include "TrailUtil.h"	// for TrailID
#include "StyleUtil.h"	// for StyleID
#include "LuaReference.h" // for LuaData

struct XNode;
struct lua_State;
class Character;

//
// Current file versions
//
extern const CString STATS_XML;

extern const CString EDITABLE_INI;

// Editable data is an INI because the default INI file association on Windows 
// systems will open the ini file in an editor.  The default association for 
// XML will open in IE.  Users have a much better chance of discovering how to 
// edit this data if they don't have to fight against the file associations.
extern const CString DONT_SHARE_SIG;

// The "don't share" file is something that the user should always keep private.
// They can safely share STATS_XML with STATS_XML's signature so that others
// can authenticate the STATS_XML data.  However, others can't copy that data
// to their own profile for use in the game unless they also have the "don't 
// share" file.  DontShare contains a piece of information that we can 
// construct using STATS_XML but the user can't construct using STATS_XML.
// The file contains a signature of the STATS_XML's signature.
extern const CString PUBLIC_KEY_FILE;
extern const CString SCREENSHOTS_SUBDIR;
extern const CString EDIT_STEPS_SUBDIR;
extern const CString EDIT_COURSES_SUBDIR;
extern const CString LASTGOOD_SUBDIR;

const unsigned int PROFILE_MAX_DISPLAY_NAME_LENGTH	= 12;


class Style;

class Song;
class Steps;
class Course;
class Game;

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
	CString GetDisplayNameOrHighScoreName() const;
	Character *GetCharacter() const;
	CString GetDisplayTotalCaloriesBurned() const;		// remove me and use Lua instead
	CString GetDisplayTotalCaloriesBurnedToday() const;	// remove me and use Lua instead
	int GetCalculatedWeightPounds() const;	// returns a default value if m_iWeightPounds isn't set
	float GetCaloriesBurnedToday() const;
	int GetTotalNumSongsPassed() const;
	int GetTotalStepsWithTopGrade( StepsType st, Difficulty d, Grade g ) const;
	int GetTotalTrailsWithTopGrade( StepsType st, CourseDifficulty d, Grade g ) const;
	float GetSongsPossible( StepsType st, Difficulty dc ) const;
	float GetCoursesPossible( StepsType st, CourseDifficulty cd ) const;
	float GetSongsActual( StepsType st, Difficulty dc ) const;
	float GetCoursesActual( StepsType st, CourseDifficulty cd ) const;
	float GetSongsPercentComplete( StepsType st, Difficulty dc ) const;
	float GetCoursesPercentComplete( StepsType st, CourseDifficulty cd ) const;
	float GetSongsAndCoursesPercentCompleteAllDifficulties( StepsType st ) const;
	bool GetDefaultModifiers( const Game* pGameType, CString &sModifiersOut ) const;
	void SetDefaultModifiers( const Game* pGameType, const CString &sModifiers );
	bool IsCodeUnlocked( int iCode ) const;
	Song *GetMostPopularSong() const;
	Course *GetMostPopularCourse() const;
	
	void AddStepTotals( int iNumTapsAndHolds, int iNumJumps, int iNumHolds, int iNumRolls, int iNumMines, int iNumHands, float fCaloriesBurned );

	bool IsMachine() const;

	//
	// Editable data
	//
	CString m_sDisplayName;
	CString m_sCharacter;
	CString m_sLastUsedHighScoreName;	// this doesn't really belong in "editable", but we need it in the smaller editable file so that it can be ready quickly.
	int m_iWeightPounds;	// 0 == not set

	//
	// General data
	//
	static CString MakeGuid();

	CString m_sGuid;
	map<CString,CString> m_sDefaultModifiers;
	SortOrder m_SortOrder;
	Difficulty m_LastDifficulty;
	CourseDifficulty m_LastCourseDifficulty;
	SongID m_lastSong;
	CourseID m_lastCourse;
	int m_iTotalPlays;
	int m_iTotalPlaySeconds;
	int m_iTotalGameplaySeconds;
	int m_iCurrentCombo;
	float m_fTotalCaloriesBurned;
	GoalType m_GoalType;
	int m_iGoalCalories;
	int m_iGoalSeconds;
	int m_iTotalDancePoints;
	int m_iNumExtraStagesPassed;
	int m_iNumExtraStagesFailed;
	int m_iNumToasties;
	int m_iTotalTapsAndHolds;
	int m_iTotalJumps;
	int m_iTotalHolds;
	int m_iTotalRolls;
	int m_iTotalMines;
	int m_iTotalHands;
	set<int> m_UnlockedSongs;
	mutable CString m_sLastPlayedMachineGuid;	// mutable because we overwrite this on save, and I don't want to remove const from the whole save chain. -Chris
	mutable DateTime m_LastPlayedDate;
	/* These stats count twice in the machine profile if two players are playing;
	 * that's the only approach that makes sense for ByDifficulty and ByMeter. */
	int m_iNumSongsPlayedByPlayMode[NUM_PLAY_MODES];
	map<StyleID,int> m_iNumSongsPlayedByStyle;
	int m_iNumSongsPlayedByDifficulty[NUM_DIFFICULTIES];
	int m_iNumSongsPlayedByMeter[MAX_METER+1];
	/* This stat counts once per song, even if two players are active. */
	int m_iNumTotalSongsPlayed;
	int m_iNumStagesPassedByPlayMode[NUM_PLAY_MODES];
	int m_iNumStagesPassedByGrade[NUM_GRADES];

	//
	// Song high scores
	//
	struct HighScoresForASteps
	{
		HighScoreList hsl;
	};
	struct HighScoresForASong
	{
		std::map<StepsID,HighScoresForASteps>	m_StepsHighScores;
		int GetNumTimesPlayed() const;
	};
	std::map<SongID,HighScoresForASong>	m_SongHighScores;

	void AddStepsHighScore( const Song* pSong, const Steps* pSteps, HighScore hs, int &iIndexOut );
	const HighScoreList& GetStepsHighScoreList( const Song* pSong, const Steps* pSteps ) const;
	HighScoreList& GetStepsHighScoreList( const Song* pSong, const Steps* pSteps );
	int GetStepsNumTimesPlayed( const Song* pSong, const Steps* pSteps ) const;
	void IncrementStepsPlayCount( const Song* pSong, const Steps* pSteps );
	void GetGrades( const Song* pSong, StepsType st, int iCounts[NUM_GRADES] ) const;
	int GetSongNumTimesPlayed( const Song* pSong ) const;
	int GetSongNumTimesPlayed( const SongID& songID ) const;
	DateTime GetSongLastPlayedDateTime( const Song* pSong ) const;

	//
	// Course high scores
	//
	// struct was a typedef'd array of HighScores, but VC6 freaks out 
	// in processing the templates for map::operator[].
	struct HighScoresForATrail
	{
		HighScoreList hsl;
	};
	struct HighScoresForACourse	
	{
		std::map<TrailID,HighScoresForATrail>	m_TrailHighScores;
		int GetNumTimesPlayed() const;
	};
	std::map<CourseID,HighScoresForACourse>	m_CourseHighScores;

	void AddCourseHighScore( const Course* pCourse, const Trail* pTrail, HighScore hs, int &iIndexOut );
	HighScoreList& GetCourseHighScoreList( const Course* pCourse, const Trail* pTrail );
	const HighScoreList& GetCourseHighScoreList( const Course* pCourse, const Trail* pTrail ) const;
	int GetCourseNumTimesPlayed( const Course* pCourse ) const;
	int GetCourseNumTimesPlayed( const CourseID& courseID ) const;
	DateTime GetCourseLastPlayedDateTime( const Course* pCourse ) const;
	void IncrementCoursePlayCount( const Course* pCourse, const Trail* pTrail );


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
	vector<Screenshot> m_vScreenshots;
	void AddScreenshot( const Screenshot &screenshot );
	int GetNextScreenshotIndex() { return m_vScreenshots.size(); }


	//
	// Calorie Data
	//
	// Why track calories in a map, and not in a static sized array like 
	// Bookkeeping?  The machine's clock is not guaranteed to be set correctly.
	// If calorie array is in a static sized array, playing on a machine with 
	// a mis-set clock could wipe out all your past data.  With this scheme, 
	// the worst that could happen is that playing on a mis-set machine will 
	// insert some garbage entries into the map.
	struct Calories
	{
		Calories() { fCals = 0; }
		float fCals;
	};
	map<DateTime,Calories> m_mapDayToCaloriesBurned;
	float GetCaloriesBurnedForDay( DateTime day ) const;
	

	//
	// RecentSongScores
	//
	struct HighScoreForASongAndSteps
	{
		StepsID stepsID;
		SongID songID;
		HighScore hs;

		HighScoreForASongAndSteps() { Unset(); }
		void Unset() { stepsID.Unset(); songID.Unset(); hs.Unset(); }

		XNode* CreateNode() const;
		void LoadFromNode( const XNode* pNode );
	};
	deque<HighScoreForASongAndSteps> m_vRecentStepsScores;
	void AddStepsRecentScore( const Song* pSong, const Steps* pSteps, HighScore hs );
	
	
	StepsType GetLastPlayedStepsType() const;
	
	//
	// RecentCourseScores
	//
	struct HighScoreForACourseAndTrail
	{
		CourseID courseID;
		TrailID	trailID;
		HighScore hs;

		HighScoreForACourseAndTrail() { Unset(); }
		void Unset() { courseID.Unset(); hs.Unset(); }

		XNode* CreateNode() const;
		void LoadFromNode( const XNode* pNode );
	};
	deque<HighScoreForACourseAndTrail> m_vRecentCourseScores;	// add to back, erase from front
	void AddCourseRecentScore( const Course* pCourse, const Trail* pTrail, HighScore hs );

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
		InitCalorieData(); 
		InitRecentSongScores(); 
		InitRecentCourseScores(); 
	}
	void InitEditableData(); 
	void InitGeneralData(); 
	void InitSongScores(); 
	void InitCourseScores(); 
	void InitCategoryScores(); 
	void InitScreenshotData(); 
	void InitCalorieData(); 
	void InitRecentSongScores(); 
	void InitRecentCourseScores(); 

	//
	// Loading and saving
	//
	ProfileLoadResult LoadAllFromDir( CString sDir, bool bRequireSignature );
	bool SaveAllToDir( CString sDir, bool bSignData ) const;

	ProfileLoadResult LoadEditableDataFromDir( CString sDir );
	ProfileLoadResult LoadStatsXmlFromNode( const XNode* pNode, bool bIgnoreEditable = true );
	void LoadGeneralDataFromNode( const XNode* pNode );
	void LoadSongScoresFromNode( const XNode* pNode );
	void LoadCourseScoresFromNode( const XNode* pNode );
	void LoadCategoryScoresFromNode( const XNode* pNode );
	void LoadScreenshotDataFromNode( const XNode* pNode );
	void LoadCalorieDataFromNode( const XNode* pNode );
	void LoadRecentSongScoresFromNode( const XNode* pNode );
	void LoadRecentCourseScoresFromNode( const XNode* pNode );

	void SaveEditableDataToDir( CString sDir ) const;
	bool SaveStatsXmlToDir( CString sDir, bool bSignData ) const;
	XNode* SaveStatsXmlCreateNode() const;
	XNode* SaveGeneralDataCreateNode() const;
	XNode* SaveSongScoresCreateNode() const;
	XNode* SaveCourseScoresCreateNode() const;
	XNode* SaveCategoryScoresCreateNode() const;
	XNode* SaveScreenshotDataCreateNode() const;
	XNode* SaveCalorieDataCreateNode() const;
	XNode* SaveRecentSongScoresCreateNode() const;
	XNode* SaveRecentCourseScoresCreateNode() const;

	XNode* SaveCoinDataCreateNode() const;

	void SaveStatsWebPageToDir( CString sDir ) const;
	void SaveMachinePublicKeyToDir( CString sDir ) const;

	static void BackupToDir( CString sFromDir, CString sToDir );

	// Lua
	void PushSelf( lua_State *L );
	LuaData m_SavedLuaData;

private:
	const HighScoresForASong *GetHighScoresForASong( const SongID& songID ) const;
	const HighScoresForACourse *GetHighScoresForACourse( const CourseID& courseID ) const;
};


#endif

/*
 * (c) 2001-2004 Chris Danford
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

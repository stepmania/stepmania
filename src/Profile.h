#ifndef Profile_H
#define Profile_H

#include "GameConstantsAndTypes.h"
#include "Grade.h"
#include "HighScore.h"
#include "DateTime.h"
#include "SongUtil.h"	// for SongID
#include "StepsUtil.h"	// for StepsID
#include "CourseUtil.h"	// for CourseID
#include "TrailUtil.h"	// for TrailID
#include "StyleUtil.h"	// for StyleID
#include "LuaReference.h"
#include "PlayerNumber.h"

#include <deque>
#include <map>
#include <set>
#include <vector>


class XNode;
struct lua_State;
class Character;

// Current file versions
extern const RString STATS_XML;

/**
 * @brief The filename where one can edit their personal profile data.
 *
 * Editable data is an INI because the default INI file association on Windows
 * systems will open the ini file in an editor.  The default association for
 * XML will open in IE.  Users have a much better chance of discovering how to
 * edit this data if they don't have to fight against the file associations. */
extern const RString EDITABLE_INI;

/**
 * @brief The filename containing the signature for STATS_XML's signature.
 *
 *
 * The "don't share" file is something that the user should always keep private.
 * They can safely share STATS_XML with STATS_XML's signature so that others
 * can authenticate the STATS_XML data.  However, others can't copy that data
 * to their own profile for use in the game unless they also have the "don't
 * share" file.  DontShare contains a piece of information that we can
 * construct using STATS_XML but the user can't construct using STATS_XML. */
extern const RString DONT_SHARE_SIG;

extern const RString PUBLIC_KEY_FILE;
extern const RString SCREENSHOTS_SUBDIR;
extern const RString EDIT_STEPS_SUBDIR;
extern const RString EDIT_COURSES_SUBDIR;
extern const RString LASTGOOD_SUBDIR;
// extern const RString RIVAL_SUBDIR;

/** @brief The max number of characters that can be used in a profile. */
const unsigned int PROFILE_MAX_DISPLAY_NAME_LENGTH	= 32;


class Style;

class Song;
class Steps;
class Course;
struct Game;

// Profile types exist for distinguishing profiles and facilitating sorting.
// Guest profiles at the top, test at the bottom.
enum ProfileType
{
	ProfileType_Guest,
	ProfileType_Normal,
	ProfileType_Test,
	NUM_ProfileType,
	ProfileType_Invalid
};

/**
 * @brief Player data that persists between sessions.
 *
 * This can be stored on a local disk or on a memory card. */
class Profile
{
public:
	/**
	 * @brief Set up the Profile with default values.
	 *
	 * Note: there are probably a lot of variables. */
	// When adding new score related data, add logic for handling it to
	// MergeScoresFromOtherProfile. -Kyz
	// When adding any new fields, add them to SwapExceptPriority.  Anything not
	// added to SwapExceptPriority won't be swapped correctly when the user
	// changes the list priority of a profile. -Kyz
	Profile():
	m_Type(ProfileType_Normal), m_ListPriority(0),
		m_sDisplayName(""), m_sCharacterID(""),
		m_sLastUsedHighScoreName(""), m_iWeightPounds(0),
		m_Voomax(0), m_BirthYear(0), m_IgnoreStepCountCalories(false),
		m_IsMale(true),
		m_sGuid(MakeGuid()), m_sDefaultModifiers(),
		m_SortOrder(SortOrder_Invalid),
		m_LastDifficulty(Difficulty_Invalid),
		m_LastCourseDifficulty(Difficulty_Invalid),
		m_LastStepsType(StepsType_Invalid), m_lastSong(),
		m_lastCourse(), m_iCurrentCombo(0), m_iTotalSessions(0),
		m_iTotalSessionSeconds(0), m_iTotalGameplaySeconds(0),
		m_fTotalCaloriesBurned(0), m_GoalType(GoalType_Calories),
		m_iGoalCalories(0), m_iGoalSeconds(0), m_iTotalDancePoints(0),
		m_iNumExtraStagesPassed(0), m_iNumExtraStagesFailed(0),
		m_iNumToasties(0), m_iTotalTapsAndHolds(0), m_iTotalJumps(0),
		m_iTotalHolds(0), m_iTotalRolls(0), m_iTotalMines(0),
		m_iTotalHands(0), m_iTotalLifts(0), m_bNewProfile(false),
		m_UnlockedEntryIDs(), m_sLastPlayedMachineGuid(""),
		m_LastPlayedDate(),m_iNumSongsPlayedByStyle(),
		m_iNumTotalSongsPlayed(0), m_UserTable(), m_SongHighScores(),
		m_CourseHighScores(), m_vScreenshots(),
		m_mapDayToCaloriesBurned()
	{
		m_lastSong.Unset();
		m_lastCourse.Unset();

		m_LastPlayedDate.Init();

		FOREACH_ENUM( PlayMode, i )
			m_iNumSongsPlayedByPlayMode[i] = 0;
		FOREACH_ENUM( Difficulty, i )
			m_iNumSongsPlayedByDifficulty[i] = 0;
		for( int i=0; i<MAX_METER+1; i++ )
			m_iNumSongsPlayedByMeter[i] = 0;

		ZERO( m_iNumStagesPassedByPlayMode );
		ZERO( m_iNumStagesPassedByGrade );
		m_UserTable.Unset();

		FOREACH_ENUM( StepsType,st )
			FOREACH_ENUM( RankingCategory,rc )
				m_CategoryHighScores[st][rc].Init();
	}


	~Profile();
	void ClearSongs();

	// smart accessors
	RString GetDisplayNameOrHighScoreName() const;
	Character *GetCharacter() const;
	void SetCharacter(const RString sCharacterID);
	RString GetDisplayTotalCaloriesBurned() const;		// remove me and use Lua instead
	RString GetDisplayTotalCaloriesBurnedToday() const;	// remove me and use Lua instead
	int GetCalculatedWeightPounds() const;	// returns a default value if m_iWeightPounds isn't set
	int GetAge() const; // returns a default value if m_Age isn't set
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
	bool GetDefaultModifiers( const Game* pGameType, RString &sModifiersOut ) const;
	void SetDefaultModifiers( const Game* pGameType, const RString &sModifiers );
	bool IsCodeUnlocked( RString sUnlockEntryID ) const;
	Song *GetMostPopularSong() const;
	Course *GetMostPopularCourse() const;

	void AddStepTotals( int iNumTapsAndHolds, int iNumJumps, int iNumHolds, int iNumRolls, int iNumMines,
			   int iNumHands, int iNumLifts, float fCaloriesBurned );
	void AddCaloriesToDailyTotal(float cals);
	float CalculateCaloriesFromHeartRate(float HeartRate, float Duration);

	bool IsMachine() const;

	ProfileType m_Type;
	// Profiles of the same type and priority are sorted by dir name.
	int m_ListPriority;

	// Editable data
	RString m_sDisplayName;
	RString m_sCharacterID;
	/**
	 * @brief The last used name for high scoring purposes.
	 *
	 * This really shouldn't be in "editable", but it's needed in the smaller editable file
	 * so that it can be ready quickly. */
	RString m_sLastUsedHighScoreName;
	int m_iWeightPounds;	// 0 == not set
	// Voomax and BirthYear are used for calculating calories from heart rate.
	float m_Voomax; // 0 == not set
	int m_BirthYear; // 0 == not set
	// m_IgnoreStepCountCalories is so that the step count based calorie
	// counter can be ignored in favor of calculating calories from heart rate
	// and voomax.
	bool m_IgnoreStepCountCalories;
	bool m_IsMale; // Used solely for calculating calories from heart rate.
	//RString m_sProfileImageName;	// todo: add a default image -aj

	// General data
	static RString MakeGuid();

	RString m_sGuid;
	std::map<RString,RString> m_sDefaultModifiers;
	SortOrder m_SortOrder;
	std::vector<Song*> m_songs;
	Difficulty m_LastDifficulty;
	CourseDifficulty m_LastCourseDifficulty;
	StepsType m_LastStepsType;
	SongID m_lastSong;
	CourseID m_lastCourse;
	int m_iCurrentCombo;
	int m_iTotalSessions;
	int m_iTotalSessionSeconds;
	int m_iTotalGameplaySeconds;
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
	int m_iTotalLifts;
	/** @brief Is this a brand new profile? */
	bool m_bNewProfile;
	std::set<RString> m_UnlockedEntryIDs;
	/**
	 * @brief Which machine did we play on last, based on the Guid?
	 *
	 * This is mutable because it's overwritten on save, but is usually
	 * const everywhere else. It was decided to keep const on the whole
	 * save chain and keep this mutable. -Chris */
	mutable RString m_sLastPlayedMachineGuid;
	mutable DateTime m_LastPlayedDate;
	/* These stats count twice in the machine profile if two players are playing;
	 * that's the only approach that makes sense for ByDifficulty and ByMeter. */
	int m_iNumSongsPlayedByPlayMode[NUM_PlayMode];
	std::map<StyleID,int> m_iNumSongsPlayedByStyle;
	int m_iNumSongsPlayedByDifficulty[NUM_Difficulty];
	int m_iNumSongsPlayedByMeter[MAX_METER+1];
	/**
	 * @brief Count the total number of songs played.
	 *
	 * This stat counts once per song, even if two players are active. */
	int m_iNumTotalSongsPlayed;
	int m_iNumStagesPassedByPlayMode[NUM_PlayMode];
	int m_iNumStagesPassedByGrade[NUM_Grade];

	/* store arbitrary data for the theme within a profile */
	LuaTable m_UserTable;

	// Song high scores
	struct HighScoresForASteps
	{
		HighScoreList hsl;
		HighScoresForASteps(): hsl() {}
	};
	struct HighScoresForASong
	{
		std::map<StepsID,HighScoresForASteps>	m_StepsHighScores;
		int GetNumTimesPlayed() const;
		HighScoresForASong(): m_StepsHighScores() {}
	};
	std::map<SongID,HighScoresForASong>	m_SongHighScores;

	void AddStepsHighScore( const Song* pSong, const Steps* pSteps, HighScore hs, int &iIndexOut );
	const HighScoreList& GetStepsHighScoreList( const Song* pSong, const Steps* pSteps ) const;
	HighScoreList& GetStepsHighScoreList( const Song* pSong, const Steps* pSteps );
	int GetStepsNumTimesPlayed( const Song* pSong, const Steps* pSteps ) const;
	void IncrementStepsPlayCount( const Song* pSong, const Steps* pSteps );
	void GetGrades( const Song* pSong, StepsType st, int iCounts[NUM_Grade] ) const;
	int GetSongNumTimesPlayed( const Song* pSong ) const;
	int GetSongNumTimesPlayed( const SongID& songID ) const;
	DateTime GetSongLastPlayedDateTime( const Song* pSong ) const;
	bool HasPassedSteps( const Song* pSong, const Steps* pSteps ) const;
	bool HasPassedAnyStepsInSong( const Song* pSong ) const;

	// Course high scores
	// struct was a typedef'd array of HighScores, but VC6 freaks out
	// in processing the templates for map::operator[].
	struct HighScoresForATrail
	{
		HighScoreList hsl;
		HighScoresForATrail(): hsl() {}
	};
	struct HighScoresForACourse
	{
		std::map<TrailID,HighScoresForATrail>	m_TrailHighScores;
		int GetNumTimesPlayed() const;
		HighScoresForACourse(): m_TrailHighScores() {}
	};
	std::map<CourseID,HighScoresForACourse>	m_CourseHighScores;

	void AddCourseHighScore( const Course* pCourse, const Trail* pTrail, HighScore hs, int &iIndexOut );
	HighScoreList& GetCourseHighScoreList( const Course* pCourse, const Trail* pTrail );
	const HighScoreList& GetCourseHighScoreList( const Course* pCourse, const Trail* pTrail ) const;
	int GetCourseNumTimesPlayed( const Course* pCourse ) const;
	int GetCourseNumTimesPlayed( const CourseID& courseID ) const;
	DateTime GetCourseLastPlayedDateTime( const Course* pCourse ) const;
	void IncrementCoursePlayCount( const Course* pCourse, const Trail* pTrail );

	void GetAllUsedHighScoreNames(std::set<RString>& names);

	void MergeScoresFromOtherProfile(Profile* other, bool skip_totals,
		RString const& from_dir, RString const& to_dir);

	// Category high scores
	HighScoreList m_CategoryHighScores[NUM_StepsType][NUM_RankingCategory];

	void AddCategoryHighScore( StepsType st, RankingCategory rc, HighScore hs, int &iIndexOut );
	HighScoreList& GetCategoryHighScoreList( StepsType st, RankingCategory rc );
	const HighScoreList& GetCategoryHighScoreList( StepsType st, RankingCategory rc ) const;
	int GetCategoryNumTimesPlayed( StepsType st ) const;
	void IncrementCategoryPlayCount( StepsType st, RankingCategory rc );


	// Screenshot Data
	std::vector<Screenshot> m_vScreenshots;
	void AddScreenshot( const Screenshot &screenshot );
	int GetNextScreenshotIndex() { return m_vScreenshots.size(); }


	/**
	 * @brief The basics for Calorie Data.
	 *
	 * Why track calories in a map, and not in a static sized array like
	 * Bookkeeping?  The machine's clock is not guaranteed to be set correctly.
	 * If calorie array is in a static sized array, playing on a machine with
	 * a mis-set clock could wipe out all your past data.  With this scheme,
	 * the worst that could happen is that playing on a mis-set machine will
	 * insert some garbage entries into the map. */
	struct Calories
	{
		Calories(): fCals(0) {}
		float fCals;
	};
	std::map<DateTime,Calories> m_mapDayToCaloriesBurned;
	float GetCaloriesBurnedForDay( DateTime day ) const;

/*
	// RecentSongScores
	struct HighScoreForASongAndSteps
	{
		StepsID stepsID;
		SongID songID;
		HighScore hs;

		HighScoreForASongAndSteps() { Unset(); }
		void Unset() { stepsID.Unset(); songID.Unset(); hs.Unset(); }

		XNode* CreateNode() const;
	};

	void SaveStepsRecentScore( const Song* pSong, const Steps* pSteps, HighScore hs );

	// RecentCourseScores
	struct HighScoreForACourseAndTrail
	{
		CourseID courseID;
		TrailID	trailID;
		HighScore hs;

		HighScoreForACourseAndTrail() { Unset(); }
		void Unset() { courseID.Unset(); hs.Unset(); }

		XNode* CreateNode() const;
	};

	void SaveCourseRecentScore( const Course* pCourse, const Trail* pTrail, HighScore hs );
*/
	// Init'ing
	void InitAll()
	{
		InitEditableData();
		InitGeneralData();
		InitSongScores();
		InitCourseScores();
		InitCategoryScores();
		InitScreenshotData();
		InitCalorieData();
		ClearSongs();
	}
	void InitEditableData();
	void InitGeneralData();
	void InitSongScores();
	void InitCourseScores();
	void InitCategoryScores();
	void InitScreenshotData();
	void InitCalorieData();
	void ClearStats();

	void swap(Profile& other);

	// Loading and saving
	void HandleStatsPrefixChange(RString dir, bool require_signature);
	ProfileLoadResult LoadAllFromDir( RString sDir, bool bRequireSignature );
	ProfileLoadResult LoadStatsFromDir(RString dir, bool require_signature);
	void LoadSongsFromDir(RString const& dir, ProfileSlot prof_slot);
	void LoadTypeFromDir(RString dir);
	void LoadCustomFunction(RString sDir, PlayerNumber pn);
	bool SaveAllToDir( RString sDir, bool bSignData ) const;

	ProfileLoadResult LoadEditableDataFromDir( RString sDir );
	ProfileLoadResult LoadStatsXmlFromNode( const XNode* pNode, bool bIgnoreEditable = true );
	void LoadGeneralDataFromNode( const XNode* pNode );
	void LoadSongScoresFromNode( const XNode* pNode );
	void LoadCourseScoresFromNode( const XNode* pNode );
	void LoadCategoryScoresFromNode( const XNode* pNode );
	void LoadScreenshotDataFromNode( const XNode* pNode );
	void LoadCalorieDataFromNode( const XNode* pNode );

	void SaveTypeToDir(RString dir) const;
	void SaveEditableDataToDir( RString sDir ) const;
	bool SaveStatsXmlToDir( RString sDir, bool bSignData ) const;
	XNode* SaveStatsXmlCreateNode() const;
	XNode* SaveGeneralDataCreateNode() const;
	XNode* SaveSongScoresCreateNode() const;
	XNode* SaveCourseScoresCreateNode() const;
	XNode* SaveCategoryScoresCreateNode() const;
	XNode* SaveScreenshotDataCreateNode() const;
	XNode* SaveCalorieDataCreateNode() const;

	XNode* SaveCoinDataCreateNode() const;

	void SaveStatsWebPageToDir( RString sDir ) const;
	void SaveMachinePublicKeyToDir( RString sDir ) const;

	static void MoveBackupToDir( RString sFromDir, RString sToDir );
	static RString MakeUniqueFileNameNoExtension( RString sDir, RString sFileNameBeginning );
	static RString MakeFileNameNoExtension( RString sFileNameBeginning, int iIndex );

	// Lua
	void PushSelf( lua_State *L );

private:
	const HighScoresForASong *GetHighScoresForASong( const SongID& songID ) const;
	const HighScoresForACourse *GetHighScoresForACourse( const CourseID& courseID ) const;
};


#endif

/**
 * @file
 * @author Chris Danford (c) 2001-2004
 * @section LICENSE
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

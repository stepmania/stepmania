#ifndef COURSE_H
#define COURSE_H
/*
-----------------------------------------------------------------------------
 Class: Course

 Desc: A queue of songs and notes.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "PlayerNumber.h"
#include "GameConstantsAndTypes.h"

struct PlayerOptions;
struct SongOptions;
class Song;
class Steps;

class Course
{
	struct Entry {
		enum Type { fixed, random, random_within_group, best, worst } type;
		bool mystery;			// show "??????"
		Song* pSong;			// used in type=fixed
		CString group_name;		// used in type=random_within_group
		Difficulty difficulty;	// = DIFFICULTY_INVALID if no difficulty specified
		int low_meter;			// = -1 if no meter range specified
		int high_meter;			// = -1 if no meter range specified
		int players_index;		// ignored if type isn't 'best' or 'worst'
		CString modifiers;		// set player and song options using these

		Entry()
		{
			difficulty = DIFFICULTY_INVALID;
			low_meter = -1;
			high_meter = -1;
			players_index = -1;
			mystery = false;
		}
	};
	vector<Entry> m_entries;

public:
	Course();

	bool		m_bIsAutogen;		// was this created by AutoGen?
	CString		m_sPath;
	CString		m_sName;
	CString		m_sTranslitName;	// used for unlocks

	bool HasBanner() const;

	CString		m_sBannerPath;
	CString		m_sCDTitlePath;

	bool		m_bRepeat;	// repeat after last song?  "Endless"
	bool		m_bRandomize;	// play the songs in a random order
	bool		m_bDifficult; // only make something difficult once
	int			m_iLives;	// -1 means use bar life meter
	int			m_iMeter;	// -1 autogens

	struct Info
	{
		Info(): pSong(NULL), pNotes(NULL), Random(false), Difficult(false) { }
		Song*	pSong;
		Steps*	pNotes;
		CString	Modifiers;
		bool	Random;
		bool	Mystery;
		bool	Difficult; /* set to true if this is the difficult version */
		/* Corresponding entry in m_entries: */
		int		CourseIndex;
	};

	// Dereferences course_entries and returns only the playable Songs and Steps
	void GetCourseInfo( StepsType nt, vector<Info> &ci, int Difficult = -1 ) const;

	int GetEstimatedNumStages() const { return m_entries.size(); }
	bool HasDifficult( StepsType nt ) const;
	bool IsPlayableIn( StepsType nt ) const;
	RageColor GetColor() const;
	Difficulty GetDifficulty( const Info &stage ) const;
	void GetMeterRange( const Info &stage, int& iMeterLowOut, int& iMeterHighOut ) const;
	bool GetTotalSeconds( float& fSecondsOut ) const;

	bool IsNonstop() const { return GetPlayMode() == PLAY_MODE_NONSTOP; }
	bool IsOni() const { return GetPlayMode() == PLAY_MODE_ONI; }
	bool IsEndless() const { return GetPlayMode() == PLAY_MODE_ENDLESS; }
	PlayMode GetPlayMode() const;
	int GetMeter( int Difficult = -1 ) const;

	void LoadFromCRSFile( CString sPath );
	void Save();
	void AutogenEndlessFromGroup( CString sGroupName, vector<Song*> &apSongsInGroup );
	void AutogenNonstopFromGroup( CString sGroupName, vector<Song*> &apSongsInGroup );


	// Statistics
	struct RankingScore
	{
		/* Dance points for oni, regular score for nonstop. */
		int iScore;
		float fSurviveTime;
		CString	sName;
	} m_RankingScores[NUM_STEPS_TYPES][NUM_RANKING_LINES];	// sorted highest to lowest by iDancePoints
	struct MemCardScore
	{
		int iNumTimesPlayed;
		int iScore;
		float fSurviveTime;
	} m_MemCardScores[NUM_MEMORY_CARDS][NUM_STEPS_TYPES];
	
	void AddScores( StepsType nt, bool bPlayerEnabled[NUM_PLAYERS], int iDancePoints[NUM_PLAYERS], float fSurviveTime[NUM_PLAYERS], int iRankingIndexOut[NUM_PLAYERS], bool bNewRecordOut[NUM_PLAYERS] );	// iNewRecordIndexOut[p] = -1 if not a new record

	// sorting values
	int		SortOrder_TotalDifficulty;
	float	SortOrder_AvgDifficulty;
	int		SortOrder_Ranking;

	void UpdateCourseStats();

private:
	void SetDefaultScore();
	void GetMeterRange( int stage, int& iMeterLowOut, int& iMeterHighOut, int Difficult = -1 ) const;
};


void SortCoursePointerArrayByDifficulty( vector<Course*> &apCourses );
void SortCoursePointerArrayByType( vector<Course*> &apCourses );
void SortCoursePointerArrayByAvgDifficulty( vector<Course*> &apCourses );
void SortCoursePointerArrayByTotalDifficulty( vector<Course*> &apCourses );
void SortCoursePointerArrayByRanking( vector<Course*> &apCourses );


#endif

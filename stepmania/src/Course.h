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
#include "Attack.h"
#include <map>
#include "Trail.h"

struct PlayerOptions;
struct SongOptions;
class Song;
class Steps;
class Profile;

enum CourseEntryType
{
	COURSE_ENTRY_FIXED, 
	COURSE_ENTRY_RANDOM, 
	COURSE_ENTRY_RANDOM_WITHIN_GROUP, 
	COURSE_ENTRY_BEST, 
	COURSE_ENTRY_WORST,
	NUM_COURSE_ENTRY_TYPES	// leave this at the end
};

inline CString CourseEntryTypeToString( CourseEntryType cet )
{
	switch( cet )
	{
	case COURSE_ENTRY_FIXED:				return "fixed";
	case COURSE_ENTRY_RANDOM:				return "random";
	case COURSE_ENTRY_RANDOM_WITHIN_GROUP:	return "random_within_group";
	case COURSE_ENTRY_BEST:					return "best";
	case COURSE_ENTRY_WORST:				return "worst";
	default:					ASSERT(0);	return "";
	}
}

class CourseEntry
{
public:
	CourseEntryType type;
	bool mystery;			// show "??????"
	Song* pSong;			// used in type=fixed
	CString group_name;		// used in type=random_within_group
	Difficulty difficulty;	// = DIFFICULTY_INVALID if no difficulty specified
	bool no_difficult;		// if true, difficult course setting doesn't affect this entry
	int low_meter;			// = -1 if no meter range specified
	int high_meter;			// = -1 if no meter range specified
	int players_index;		// ignored if type isn't 'best' or 'worst'
	CString modifiers;		// set player and song options using these
	AttackArray attacks;	// set timed modifiers

	CourseEntry()
	{
		type = (CourseEntryType)0;
		mystery = false;
		pSong = NULL;
		group_name = "";
		difficulty = DIFFICULTY_INVALID;
		no_difficult = false;
		low_meter = -1;
		high_meter = -1;
		players_index = 0;
		modifiers = "";
	}
};

class Course
{
public:
	Course();

	bool		m_bIsAutogen;		// was this created by AutoGen?
	CString		m_sPath;
	CString		m_sName, m_sNameTranslit;

	bool HasBanner() const;

	CString		m_sBannerPath;
	CString		m_sCDTitlePath;

	bool		m_bRepeat;	// repeat after last song?  "Endless"
	bool		m_bRandomize;	// play the songs in a random order
	int			m_iLives;	// -1 means use bar life meter
	int			m_iCustomMeter[NUM_COURSE_DIFFICULTIES];	// -1 = no meter specified
	bool		m_bSortByMeter;

	vector<CourseEntry> m_entries;

	/* If PREFSMAN->m_bShowNative is off, this are the same as GetTranslit* below. */
	CString GetDisplayName() const;
	CString GetTranslitName() const { return m_sNameTranslit.size()? m_sNameTranslit: m_sName; }

	// Dereferences course_entries and returns only the playable Songs and Steps
	Trail* GetTrail( StepsType st, CourseDifficulty cd ) const;
	float GetMeter( StepsType st, CourseDifficulty cd ) const;
	bool HasMods() const;
	bool AllSongsAreFixed() const;

	int GetEstimatedNumStages() const { return m_entries.size(); }
	bool HasCourseDifficulty( StepsType st, CourseDifficulty cd ) const;
	bool IsPlayableIn( StepsType st ) const;
	bool CourseHasBestOrWorst() const;
	RageColor GetColor() const;
	bool GetTotalSeconds( StepsType st, float& fSecondsOut ) const;

	bool IsNonstop() const { return GetPlayMode() == PLAY_MODE_NONSTOP; }
	bool IsOni() const { return GetPlayMode() == PLAY_MODE_ONI; }
	bool IsEndless() const { return GetPlayMode() == PLAY_MODE_ENDLESS; }
	PlayMode GetPlayMode() const;

	bool IsFixed() const;

	void LoadFromCRSFile( CString sPath );
	void Init();
	void Save();
	void AutogenEndlessFromGroup( CString sGroupName, Difficulty dc );
	void AutogenNonstopFromGroup( CString sGroupName, Difficulty dc );
	void AutogenOniFromArtist( CString sArtistName, CString sArtistNameTranslit, vector<Song*> aSongs, Difficulty dc );

	// sorting values
	int		m_SortOrder_TotalDifficulty;
	int		m_SortOrder_Ranking;
	bool	IsRanking() const;

	void UpdateCourseStats( StepsType st );

	/* Call per-screen, and if song or notes pointers change: */
	void ClearCache();

private:
	void GetTrailUnsorted( StepsType st, CourseDifficulty cd, Trail &trail ) const;

	typedef pair<StepsType,CourseDifficulty> TrailParams;
	typedef map<TrailParams, Trail> TrailCache;
	mutable TrailCache m_TrailCache;
};

#endif

/* Course - A queue of songs and notes. */

#ifndef COURSE_H
#define COURSE_H

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
private:
	CString		m_sMainTitle, m_sMainTitleTranslit;
	CString		m_sSubTitle, m_sSubTitleTranslit;

public:
	bool HasBanner() const;

	CString		m_sBannerPath;
	CString		m_sCDTitlePath;

	bool		m_bRepeat;	// repeat after last song?  "Endless"
	bool		m_bRandomize;	// play the songs in a random order
	int			m_iLives;	// -1 means use bar life meter
	int			m_iCustomMeter[NUM_DIFFICULTIES];	// -1 = no meter specified
	bool		m_bSortByMeter;

	vector<CourseEntry> m_entries;

	/* If PREFSMAN->m_bShowNative is off, these are the same as GetTranslit* below.
	 * Otherwise, they return the main titles. */
	CString GetDisplayMainTitle() const;
	CString GetDisplaySubTitle() const;

	/* Returns the transliterated titles, if any; otherwise returns the main titles. */
	CString GetTranslitMainTitle() const { return m_sMainTitleTranslit.size()? m_sMainTitleTranslit: m_sMainTitle; }
	CString GetTranslitSubTitle() const { return m_sSubTitleTranslit.size()? m_sSubTitleTranslit: m_sSubTitle; }

	/* "title subtitle" */
	CString GetFullDisplayTitle() const;
	CString GetFullTranslitTitle() const;

	// Dereferences course_entries and returns only the playable Songs and Steps
	Trail* GetTrail( StepsType st, CourseDifficulty cd=DIFFICULTY_MEDIUM ) const;
	void GetTrails( vector<Trail*> &AddTo, StepsType st ) const;
	float GetMeter( StepsType st, CourseDifficulty cd=DIFFICULTY_MEDIUM ) const;
	bool HasMods() const;
	bool AllSongsAreFixed() const;

	int GetEstimatedNumStages() const { return m_entries.size(); }
	bool IsPlayableIn( StepsType st ) const;
	bool CourseHasBestOrWorst() const;
	RageColor GetColor() const;
	bool GetTotalSeconds( StepsType st, float& fSecondsOut ) const;

	bool IsNonstop() const { return GetPlayMode() == PLAY_MODE_NONSTOP; }
	bool IsOni() const { return GetPlayMode() == PLAY_MODE_ONI; }
	bool IsEndless() const { return GetPlayMode() == PLAY_MODE_ENDLESS; }
	PlayMode GetPlayMode() const;

	bool IsFixed() const;

	bool ShowInDemonstrationAndRanking() const { return true; }

	void LoadFromCRSFile( CString sPath );
	void RevertFromDisk();
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

	/* Call to regenerate Trails with random entries */
	void RegenerateNonFixedTrails();

	/* Call when a Song or its Steps are deleted/changed. */
	void Invalidate( Song *pStaleSong );

	void GetAllCachedTrails( vector<Trail *> &out );

private:
	bool GetTrailUnsorted( StepsType st, CourseDifficulty cd, Trail &trail ) const;
	bool GetTrailSorted( StepsType st, CourseDifficulty cd, Trail &trail ) const;

	mutable Trail m_TrailCache[NUM_STEPS_TYPES][NUM_DIFFICULTIES];
	mutable bool m_TrailCacheValid[NUM_STEPS_TYPES][NUM_DIFFICULTIES];
	mutable bool m_TrailCacheNull[NUM_STEPS_TYPES][NUM_DIFFICULTIES];
	mutable int m_iTrailCacheSeed;
};

#endif

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
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

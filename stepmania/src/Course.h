/* Course - A queue of songs and notes. */

#ifndef COURSE_H
#define COURSE_H

#include "PlayerNumber.h"
#include "GameConstantsAndTypes.h"
#include "Attack.h"
#include <map>
#include "Trail.h"
#include <set>
#include "EnumHelper.h"

struct PlayerOptions;
struct SongOptions;
class Song;
class Steps;
class Profile;
struct lua_State;

enum CourseType
{
	COURSE_TYPE_NONSTOP,	// if life meter type is BAR
	COURSE_TYPE_ONI,		// if life meter type is BATTERY
	COURSE_TYPE_ENDLESS,	// if set to REPEAT
	COURSE_TYPE_SURVIVAL,	// if life meter type is TIME
	NUM_COURSE_TYPES
};
#define FOREACH_CourseType( i ) FOREACH_ENUM( CourseType, NUM_COURSE_TYPES, i )

inline PlayMode CourseTypeToPlayMode( CourseType ct ) { return (PlayMode)(PLAY_MODE_NONSTOP+ct); }
inline CourseType PlayModeToCourseType( PlayMode pm ) { return (CourseType)(pm-PLAY_MODE_NONSTOP); }

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
	bool bSecret;			// show "??????"
	Song* pSong;			// used in type=fixed
	CString group_name;		// used in type=random_within_group
	Difficulty difficulty;	// = DIFFICULTY_INVALID if no difficulty specified
	bool no_difficult;		// if true, difficult course setting doesn't affect this entry
	int low_meter;			// = -1 if no meter range specified
	int high_meter;			// = -1 if no meter range specified
	int players_index;		// ignored if type isn't 'best' or 'worst'
	CString modifiers;		// set player and song options using these
	AttackArray attacks;	// timed modifiers
	float fGainSeconds;		// time gained back at the beginning of the song.  LifeMeterTime only.

	CourseEntry()
	{
		type = (CourseEntryType)0;
		bSecret = false;
		pSong = NULL;
		group_name = "";
		difficulty = DIFFICULTY_INVALID;
		no_difficult = false;
		low_meter = -1;
		high_meter = -1;
		players_index = 0;
		modifiers = "";
		fGainSeconds = 0;
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
	CString GetDisplayFullTitle() const;
	CString GetTranslitFullTitle() const;

	// Dereferences course_entries and returns only the playable Songs and Steps
	Trail* GetTrail( StepsType st, CourseDifficulty cd=DIFFICULTY_MEDIUM ) const;
	void GetTrails( vector<Trail*> &AddTo, StepsType st ) const;
	void GetAllTrails( vector<Trail*> &AddTo ) const;
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
	CourseType GetCourseType() const;
	PlayMode GetPlayMode() const;

	bool IsFixed() const;

	bool ShowInDemonstrationAndRanking() const;

	void LoadFromCRSFile( CString sPath );
	void RevertFromDisk();
	void Init();
	void Save( CString sPath = "", bool bSavingCache=false ); /* default is source file */
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
	CString GetCacheFilePath() const;

	const CourseEntry *FindFixedSong( const Song *pSong ) const;

	// Lua
	void PushSelf( lua_State *L );

private:
	void CalculateRadarValues();

	bool GetTrailUnsorted( StepsType st, CourseDifficulty cd, Trail &trail ) const;
	bool GetTrailSorted( StepsType st, CourseDifficulty cd, Trail &trail ) const;

	typedef pair<StepsType,Difficulty> CacheEntry;
	struct CacheData
	{
		Trail trail;
		bool null;
	};
	typedef map<CacheEntry, CacheData> TrailCache_t;
	mutable TrailCache_t m_TrailCache;
	mutable int m_iTrailCacheSeed;

	typedef map<CacheEntry, RadarValues> RadarCache_t;
	RadarCache_t m_RadarCache;
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

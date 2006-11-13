/* SongManager - Holder for all Songs and Steps. */

#ifndef SONGMANAGER_H
#define SONGMANAGER_H

class LoadingWindow;
class Song;
class Style;
class Steps;
class PlayerOptions;
struct lua_State;

#include "RageTypes.h"
#include "GameConstantsAndTypes.h"
#include "SongOptions.h"
#include "PlayerOptions.h"
#include "PlayerNumber.h"
#include "Difficulty.h"
#include "Course.h"
#include "ThemeMetric.h"
#include "RageTexturePreloader.h"

const int MAX_EDIT_STEPS_PER_PROFILE	= 200;
const int MAX_EDIT_COURSES_PER_PROFILE	= 20;

class SongManager
{
public:
	SongManager();
	~SongManager();

	void InitSongsFromDisk( LoadingWindow *ld );
	void FreeSongs();
	void Cleanup();

	void Invalidate( const Song *pStaleSong );

	void RegenerateNonFixedCourses();
	void SetPreferences();

	void LoadAllFromProfileDir( const RString &sProfileDir, ProfileSlot slot );
	int GetNumStepsLoadedFromProfile();
	void FreeAllLoadedFromProfile( ProfileSlot slot = ProfileSlot_Invalid );

	void LoadGroupSymLinks( RString sDir, RString sGroupFolder );

	void InitCoursesFromDisk( LoadingWindow *ld );
	void InitAutogenCourses();
	void FreeCourses();
	void AddCourse( Course *pCourse );	// transfers ownership of pCourse
	void DeleteCourse( Course *pCourse );	// transfers ownership of pCourse
	void InvalidateCachedTrails();

	void InitAll( LoadingWindow *ld );	// songs, courses, groups - everything.
	void Reload( bool bAllowFastLoad, LoadingWindow *ld=NULL );	// songs, courses, groups - everything.
	void PreloadSongImages();

	RString GetSongGroupBannerPath( RString sSongGroup );
	void GetSongGroupNames( vector<RString> &AddTo );
	bool DoesSongGroupExist( RString sSongGroup );
	RageColor GetSongGroupColor( const RString &sSongGroupName );
	RageColor GetSongColor( const Song* pSong );

	RString GetCourseGroupBannerPath( const RString &sCourseGroup );
	void GetCourseGroupNames( vector<RString> &AddTo );
	bool DoesCourseGroupExist( const RString &sCourseGroup );
	RageColor GetCourseGroupColor( const RString &sCourseGroupName );
	RageColor GetCourseColor( const Course* pCourse );
	
	static RString ShortenGroupName( RString sLongGroupName );
	static int     GetNumStagesForSong( const Song* pSong );	// LongVer songs take 2 stages, MarathonVer take 3


	// Lookup
	const vector<Song*> &GetAllSongs() const { return m_pSongs; }
	void GetPopularSongs( vector<Song*> &AddTo, RString sGroupName, int iMaxStages = INT_MAX, ProfileSlot slot=ProfileSlot_Machine ) const;
	void GetPreferredSortSongs( vector<Song*> &AddTo, int iMaxStages = INT_MAX ) const;
	const vector<Song*> &GetPopularSongs( ProfileSlot slot=ProfileSlot_Machine ) const { return m_pPopularSongs[slot]; }
	const vector<Course*> &GetPopularCourses( CourseType ct, ProfileSlot slot=ProfileSlot_Machine ) const { return m_pPopularCourses[slot][ct]; }
	void GetSongs( vector<Song*> &AddTo, RString sGroupName, int iMaxStages = INT_MAX ) const;
	void GetSongs( vector<Song*> &AddTo, int iMaxStages ) const { GetSongs(AddTo,GROUP_ALL,iMaxStages); }
	void GetSongs( vector<Song*> &AddTo ) const { GetSongs(AddTo,GROUP_ALL,INT_MAX); }
	Song *FindSong( RString sPath );
	Song *FindSong( RString sGroup, RString sSong );
	Course *FindCourse( RString sPath );
	Course *FindCourse( RString sGroup, RString sName );
	int GetNumSongs() const;
	int GetNumUnlockedSongs() const;
	int GetNumSelectableAndUnlockedSongs() const;
	int GetNumAdditionalSongs() const;
	int GetNumSongGroups() const;
	int GetNumCourses() const;
	int GetNumAdditionalCourses() const;
	int GetNumCourseGroups() const;
	int GetNumEditCourses( ProfileSlot slot ) const;
	Song* GetRandomSong();
	Course* GetRandomCourse();

	void GetStepsLoadedFromProfile( vector<Steps*> &AddTo, ProfileSlot slot );
	Song *GetSongFromSteps( Steps *pSteps );
	void DeleteSteps( Steps *pSteps );	// transfers ownership of pSteps
	bool WasLoadedFromAdditionalSongs( const Song *pSong ) const;
	bool WasLoadedFromAdditionalCourses( const Course *pCourse ) const;

	void GetAllCourses( vector<Course*> &AddTo, bool bIncludeAutogen );
	void GetCourses( CourseType ct, vector<Course*> &AddTo, bool bIncludeAutogen ) const;
	void GetCoursesInGroup( vector<Course*> &AddTo, const RString &sCourseGroup, bool bIncludeAutogen );
	void GetPreferredSortCourses( CourseType ct, vector<Course*> &AddTo, bool bIncludeAutogen ) const;

	void GetExtraStageInfo( bool bExtra2, const Style *s, 
		Song*& pSongOut, Steps*& pStepsOut, PlayerOptions *pPlayerOptionsOut, SongOptions *pSongOptionsOut );
	Song* GetSongFromDir( RString sDir );
	Course* GetCourseFromPath( RString sPath );	// path to .crs file, or path to song group dir
	Course* GetCourseFromName( RString sName );


	void UpdatePopular();
	void UpdateShuffled();		// re-shuffle songs and courses
	void UpdatePreferredSort(); 
	void SortSongs();		// sort m_pSongs by CompareSongPointersByTitle

	void UpdateRankingCourses();	// courses shown on the ranking screen
	void RefreshCourseGroupInfo();

	// Lua
	void PushSelf( lua_State *L );

protected:
	void LoadStepManiaSongDir( RString sDir, LoadingWindow *ld );
	void LoadDWISongDir( RString sDir );
	bool GetExtraStageInfoFromCourse( bool bExtra2, RString sPreferredGroup,
					   Song*& pSongOut, Steps*& pStepsOut, PlayerOptions *pPlayerOptionsOut, SongOptions *pSongOptionsOut );
	void SanityCheckGroupDir( RString sDir ) const;
	void AddGroup( RString sDir, RString sGroupDirName );
	int GetNumEditsLoadedFromProfile( ProfileSlot slot ) const;

	vector<Song*>		m_pSongs;	// all songs that can be played
	vector<Song*>		m_pPopularSongs[NUM_ProfileSlot];
	vector<Song*>		m_pShuffledSongs;	// used by GetRandomSong
	typedef vector<Song*> SongPointerVector;
	vector<SongPointerVector> m_vPreferredSongSort;
	vector<RString>		m_sSongGroupNames;
	vector<RString>		m_sSongGroupBannerPaths; // each song group may have a banner associated with it

	vector<Course*>		m_pCourses;
	vector<Course*>		m_pPopularCourses[NUM_ProfileSlot][NUM_CourseType];
	vector<Course*>		m_pShuffledCourses;	// used by GetRandomCourse
	struct CourseGroupInfo
	{
		RString m_sBannerPath;
	};
	map<RString,CourseGroupInfo> m_mapCourseGroupToInfo;
	typedef vector<Course*> CoursePointerVector;
	vector<CoursePointerVector> m_vPreferredCourseSort;

	RageTexturePreloader m_TexturePreload;

	ThemeMetric<int>		NUM_SONG_GROUP_COLORS;
	ThemeMetric1D<RageColor>	SONG_GROUP_COLOR;
	ThemeMetric<int>		NUM_COURSE_GROUP_COLORS;
	ThemeMetric1D<RageColor>	COURSE_GROUP_COLOR;
};

extern SongManager*	SONGMAN;	// global and accessable from anywhere in our program

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

/* SongManager - Holder for all Songs and Steps. */

#ifndef SONGMANAGER_H
#define SONGMANAGER_H

class LoadingWindow;
class Song;
class Style;
class Steps;
struct PlayerOptions;
struct lua_State;

#include "RageTypes.h"
#include "GameConstantsAndTypes.h"
#include "SongOptions.h"
#include "PlayerOptions.h"
#include "PlayerNumber.h"
#include "Difficulty.h"
#include "Course.h"

class SongManager
{
public:
	SongManager();
	~SongManager();

	void InitSongsFromDisk( LoadingWindow *ld );
	void FreeSongs();
	void Cleanup();

	void Invalidate( Song *pStaleSong );
	void RevertFromDisk( Song *pSong, bool bAllowNotesLoss=false );

	void RegenerateNonFixedCourses();
	void SetPreferences();

	void LoadAllFromProfile( ProfileSlot s );	// songs, edits
	void LoadEditsFromDir( const CString &sDir, ProfileSlot slot );
	int GetNumStepsLoadedFromProfile();
	void FreeAllLoadedFromProfiles();

	void LoadGroupSymLinks( CString sDir, CString sGroupFolder );

	void InitCoursesFromDisk( LoadingWindow *ld );
	void InitAutogenCourses();
	void FreeCourses();

	void InitAll( LoadingWindow *ld );	// songs, courses, groups - everything.
	void Reload( LoadingWindow *ld=NULL );	// songs, courses, groups - everything.
	void PreloadSongImages();


	CString GetGroupBannerPath( CString sGroupName );
	void GetGroupNames( CStringArray &AddTo );
	bool DoesGroupExist( CString sGroupName );

	RageColor GetGroupColor( const CString &sGroupName );
	RageColor GetSongColor( const Song* pSong );
	RageColor GetDifficultyColor( Difficulty dc ) const;
	
	static CString ShortenGroupName( CString sLongGroupName );
	static int     GetNumStagesForSong( const Song* pSong );	// LongVer songs take 2 stages, MarathonVer take 3


	// Lookup
	const vector<Song*> &GetAllSongs() const { return m_pSongs; }
	void GetBestSongs( vector<Song*> &AddTo, CString sGroupName, int iMaxStages = INT_MAX, ProfileSlot slot=PROFILE_SLOT_MACHINE ) const;
	const vector<Song*> &GetBestSongs( ProfileSlot slot=PROFILE_SLOT_MACHINE ) const { return m_pBestSongs[slot]; }
	const vector<Course*> &GetBestCourses( CourseType ct, ProfileSlot slot=PROFILE_SLOT_MACHINE ) const { return m_pBestCourses[slot][ct]; }
	void GetSongs( vector<Song*> &AddTo, CString sGroupName, int iMaxStages = INT_MAX ) const;
	void GetSongs( vector<Song*> &AddTo, int iMaxStages ) const { GetSongs(AddTo,GROUP_ALL_MUSIC,iMaxStages); }
	void GetSongs( vector<Song*> &AddTo ) const { GetSongs(AddTo,GROUP_ALL_MUSIC,INT_MAX); }
	Song *FindSong( CString sPath );
	Course *FindCourse( CString sName );
	int GetNumSongs() const;
	int GetNumGroups() const;
	int GetNumCourses() const;
	Song* GetRandomSong();
	Course* GetRandomCourse();


	void GetAllCourses( vector<Course*> &AddTo, bool bIncludeAutogen );
	void GetCourses( CourseType ct, vector<Course*> &AddTo, bool bIncludeAutogen );

	void GetExtraStageInfo( bool bExtra2, const Style *s, 
		Song*& pSongOut, Steps*& pStepsOut, PlayerOptions& po_out, SongOptions& so_out );

	Song* GetSongFromDir( CString sDir );
	Course* GetCourseFromPath( CString sPath );	// path to .crs file, or path to song group dir
	Course* GetCourseFromName( CString sName );


	void UpdateBest();				// update Players Best
	void UpdateShuffled();			// re-shuffle songs and courses
	void SortSongs();				// sort m_pSongs

	void UpdateRankingCourses();	// courses shown on the ranking screen

	// Lua
	void PushSelf( lua_State *L );

protected:
	void LoadStepManiaSongDir( CString sDir, LoadingWindow *ld );
	void LoadDWISongDir( CString sDir );
	bool GetExtraStageInfoFromCourse( bool bExtra2, CString sPreferredGroup,
					   Song*& pSongOut, Steps*& pStepsOut, PlayerOptions& po_out, SongOptions& so_out );
	void SanityCheckGroupDir( CString sDir ) const;
	void AddGroup( CString sDir, CString sGroupDirName );

	Song *FindSong( CString sGroup, CString sSong );

	vector<Song*>		m_pSongs;	// all songs that can be played
	vector<Song*>		m_pBestSongs[NUM_PROFILE_SLOTS];
	vector<Song*>		m_pShuffledSongs;	// used by GetRandomSong
	CStringArray		m_sGroupNames;
	CStringArray		m_sGroupBannerPaths; // each song group may have a banner associated with it
	vector<Course*>		m_pCourses;
	vector<Course*>		m_pBestCourses[NUM_PROFILE_SLOTS][NUM_COURSE_TYPES];
	vector<Course*>		m_pShuffledCourses;	// used by GetRandomCourse
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

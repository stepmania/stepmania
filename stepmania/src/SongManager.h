#ifndef SONGMANAGER_H
#define SONGMANAGER_H
/*
-----------------------------------------------------------------------------
 Class: SongManager

 Desc: Holder for all Songs and Notes.  Also keeps track of the current 
	Song and Notes, and loads/saves statistics.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "Course.h"

class LoadingWindow;
class Song;
class StyleDef;
struct PlayerOptions;

class SongManager
{
public:
	SongManager( LoadingWindow *ld );
	~SongManager();

	vector<Song*>	m_pSongs;	// all songs that can be played

	void InitSongArrayFromDisk( LoadingWindow *ld );
	void FreeSongArray();
	void ReloadSongArray();

	void ReadStatisticsFromDisk();
	void SaveStatisticsToDisk();


	CString GetGroupBannerPath( CString sGroupName );
	void GetGroupNames( CStringArray &AddTo );
	RageColor GetGroupColor( const CString &sGroupName );
	RageColor GetSongColor( const Song* pSong );

	static CString ShortenGroupName( const CString &sOrigGroupName );

	void GetSongsInGroup( const CString sGroupName, vector<Song*> &AddTo );


	// for Oni
	vector<Course> m_aOniCourses;

	// for Extra Stages
	vector<Course> m_aExtraCourses;
	
	// for Endless
	vector<Course> m_aEndlessCourses;

	void InitCoursesFromDisk();
	void ReloadCourses();
	void CleanCourses();

	void GetExtraStageInfo( bool bExtra2, CString sPreferredGroup, const StyleDef *s, 
		Song*& pSongOut, Notes*& pNotesOut, PlayerOptions& po_out, SongOptions& so_out );

	Song* GetRandomSong();

	Song* GetSongFromDir( CString sDir );

protected:
	void LoadStepManiaSongDir( CString sDir, LoadingWindow *ld );
	void LoadDWISongDir( CString sDir );
	bool GetExtraStageInfoFromCourse( bool bExtra2, CString sPreferredGroup,
					   Song*& pSongOut, Notes*& pNotesOut, PlayerOptions& po_out, SongOptions& so_out );
	void SanityCheckGroupDir( CString sDir ) const;
	void AddGroup( CString sDir, CString sGroupDirName );

	CStringArray		m_arrayGroupNames;
	CStringArray		m_GroupBannerPaths;		// each song group has a banner associated with it
};


extern SongManager*	SONGMAN;	// global and accessable from anywhere in our program

#endif

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


	CString GetGroupBannerPath( CString sGroupName );
	void GetGroupNames( CStringArray &AddTo );
	RageColor GetGroupColor( const CString &sGroupName );
	RageColor GetSongColor( const Song* pSong );

	static CString ShortenGroupName( const CString &sOrigGroupName );



	//
	// Courses for Nonstop, Oni, and Endless
	//
	vector<Course*> m_pCourses;

	void InitCoursesFromDisk();
	void FreeCourses();
	void CleanCourses();



	// Lookup
	void GetSongsInGroup( const CString sGroupName, vector<Song*> &AddTo );
	Song* GetRandomSong();

	void GetNonstopCourses( vector<Course*> AddTo );	// add to if life meter type is BAR.
	void GetOniCourses( vector<Course*> AddTo );		// add to if life meter type is BATTERY.
	void GetEndlessCourses( vector<Course*> AddTo );	// add to if set to REPEAT.

	void GetExtraStageInfo( bool bExtra2, CString sPreferredGroup, const StyleDef *s, 
		Song*& pSongOut, Notes*& pNotesOut, PlayerOptions& po_out, SongOptions& so_out );

	Song* GetSongFromDir( CString sDir );
	Course* GetCourseFromPath( CString sPath );	// path to .crs file, or path to song group dir


	//
	// High scores
	//
	struct HighScore
	{
		int iScore;
		CString	sName;
	};
	HighScore m_MachineScores[NUM_STYLES][NUM_HIGH_SCORE_CATEGORIES][NUM_HIGH_SCORE_LINES];

	void InitMachineScoresFromDisk();
	void SaveMachineScoresToDisk();

	bool IsUsingMemoryCard( PlayerNumber pn );

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

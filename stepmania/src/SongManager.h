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

	void InitSongArrayFromDisk( LoadingWindow *ld );
	void FreeSongArray();
	void ReloadSongArray();


	CString GetGroupBannerPath( CString sGroupName );
	void GetGroupNames( CStringArray &AddTo );
	bool DoesGroupExist( CString sGroupName );

	RageColor GetGroupColor( const CString &sGroupName );
	RageColor GetSongColor( const Song* pSong );
	RageColor GetDifficultyColor( Difficulty dc );

	static CString ShortenGroupName( CString sLongGroupName );
	static int     GetNumStagesForSong( const Song* pSong );	// LongVer songs take 2 stages, MarathonVer take 3


	//
	// Courses for Nonstop, Oni, and Endless
	//
	vector<Course*> m_pCourses;

	void InitCoursesFromDisk();
	void InitAutogenCourses();
	void FreeCourses();
	void CleanData();



	// Lookup
	const vector<Song*> &GetAllSongs() const { return m_pSongs; }
	void GetSongs( vector<Song*> &AddTo, CString sGroupName, int iMaxStages = 100000 /*inf*/ ) const;
	void GetSongs( vector<Song*> &AddTo, int iMaxStages ) const { GetSongs(AddTo,"",iMaxStages); }
	void GetSongs( vector<Song*> &AddTo ) const { GetSongs(AddTo,"",100000 /*inf*/ ); }
	Song *FindSong( CString sGroup, CString sSong );
	Course *FindCourse( CString sName );
	int GetNumSongs() const;
	int GetNumGroups() const;
	int GetNumCourses() const;
	Song* GetRandomSong();


	void GetNonstopCourses( vector<Course*> &AddTo, bool bIncludeAutogen );	// add to if life meter type is BAR.
	void GetOniCourses( vector<Course*> &AddTo, bool bIncludeAutogen );		// add to if life meter type is BATTERY.
	void GetEndlessCourses( vector<Course*> &AddTo, bool bIncludeAutogen );	// add to if set to REPEAT.

	void GetExtraStageInfo( bool bExtra2, CString sPreferredGroup, const StyleDef *s, 
		Song*& pSongOut, Notes*& pNotesOut, PlayerOptions& po_out, SongOptions& so_out );

	Song* GetSongFromDir( CString sDir );
	Course* GetCourseFromPath( CString sPath );	// path to .crs file, or path to song group dir
	Course* GetCourseFromName( CString sName );


	//
	// High scores
	//
	void InitMachineScoresFromDisk();
	void SaveMachineScoresToDisk();

	bool MemoryCardIsInserted( PlayerNumber pn );
	bool IsUsingMemoryCard( PlayerNumber pn );

	void LoadMemoryCardScores( PlayerNumber pn );
	void SaveMemoryCardScores( PlayerNumber pn );

	struct MachineScore
	{
		int iScore;
		CString	sName;
	} m_MachineScores[NUM_NOTES_TYPES][NUM_RANKING_CATEGORIES][NUM_RANKING_LINES];
	void AddScores( NotesType nt, bool bPlayerEnabled[NUM_PLAYERS], RankingCategory hsc[NUM_PLAYERS], int iScore[NUM_PLAYERS], int iNewRecordIndexOut[NUM_PLAYERS] );	// set iNewRecordIndex = -1 if not a new record

protected:
	void LoadStepManiaSongDir( CString sDir, LoadingWindow *ld );
	void LoadDWISongDir( CString sDir );
	bool GetExtraStageInfoFromCourse( bool bExtra2, CString sPreferredGroup,
					   Song*& pSongOut, Notes*& pNotesOut, PlayerOptions& po_out, SongOptions& so_out );
	void SanityCheckGroupDir( CString sDir ) const;
	void AddGroup( CString sDir, CString sGroupDirName );

	void ReadNoteScoresFromFile( CString fn, int c );
	void ReadCourseScoresFromFile( CString fn, int c );
	void ReadCategoryRankingsFromFile( CString fn );
	void ReadCourseRankingsFromFile( CString fn );

	vector<Song*>		m_pSongs;	// all songs that can be played
	CStringArray		m_arrayGroupNames;
	CStringArray		m_GroupBannerPaths;		// each song group has a banner associated with it
};


extern SongManager*	SONGMAN;	// global and accessable from anywhere in our program

#endif

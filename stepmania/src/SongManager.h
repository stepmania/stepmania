#ifndef SONGMANAGER_H
#define SONGMANAGER_H
/*
-----------------------------------------------------------------------------
 Class: SongManager

 Desc: Holder for all Songs and Steps.  Also keeps track of the current 
	Song and Steps, and loads/saves statistics.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


class LoadingWindow;
class Song;
class StyleDef;
class Course;
class Steps;
struct PlayerOptions;

#include "RageTypes.h"
#include "GameConstantsAndTypes.h"
#include "SongOptions.h"
#include "PlayerOptions.h"
#include "PlayerNumber.h"

class SongManager
{
public:
	SongManager( LoadingWindow *ld );
	~SongManager();

	void InitSongsFromDisk( LoadingWindow *ld );
	void FreeSongs();
	void CompressSongs();

	void InitCoursesFromDisk( LoadingWindow *ld );
	void InitAutogenCourses();
	void FreeCourses();

	void Reload();	// songs, courses, groups - everything.


	CString GetGroupBannerPath( CString sGroupName );
	void GetGroupNames( CStringArray &AddTo );
	bool DoesGroupExist( CString sGroupName );

	RageColor GetGroupColor( const CString &sGroupName );
	RageColor GetSongColor( const Song* pSong );
	RageColor GetDifficultyColor( Difficulty dc );

	static CString ShortenGroupName( CString sLongGroupName );
	static int     GetNumStagesForSong( const Song* pSong );	// LongVer songs take 2 stages, MarathonVer take 3


	// Lookup
	const vector<Song*> &GetAllSongs() const { return m_pSongs; }
	const vector<Song*> &GetBestSongs() const { return m_pBestSongs; }
	void GetSongs( vector<Song*> &AddTo, CString sGroupName, int iMaxStages = 100000 /*inf*/ ) const;
	void GetSongs( vector<Song*> &AddTo, int iMaxStages ) const { GetSongs(AddTo,"",iMaxStages); }
	void GetSongs( vector<Song*> &AddTo ) const { GetSongs(AddTo,"",100000 /*inf*/ ); }
	Song *FindSong( CString sPath );
	Course *FindCourse( CString sName );
	int GetNumSongs() const;
	int GetNumGroups() const;
	int GetNumCourses() const;
	Song* GetRandomSong();


	void GetAllCourses( vector<Course*> &AddTo, bool bIncludeAutogen );
	void GetNonstopCourses( vector<Course*> &AddTo, bool bIncludeAutogen );	// add to if life meter type is BAR.
	void GetOniCourses( vector<Course*> &AddTo, bool bIncludeAutogen );		// add to if life meter type is BATTERY.
	void GetEndlessCourses( vector<Course*> &AddTo, bool bIncludeAutogen );	// add to if set to REPEAT.

	void GetExtraStageInfo( bool bExtra2, const StyleDef *s, 
		Song*& pSongOut, Steps*& pNotesOut, PlayerOptions& po_out, SongOptions& so_out );

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
	} m_MachineScores[NUM_STEPS_TYPES][NUM_RANKING_CATEGORIES][NUM_RANKING_LINES];
	void AddScores( StepsType nt, bool bPlayerEnabled[NUM_PLAYERS], RankingCategory hsc[NUM_PLAYERS], int iScore[NUM_PLAYERS], int iNewRecordIndexOut[NUM_PLAYERS] );	// set iNewRecordIndex = -1 if not a new record
	void UpdateBest();

	void UpdateRankingCourses();
protected:
	void LoadStepManiaSongDir( CString sDir, LoadingWindow *ld );
	void LoadDWISongDir( CString sDir );
	bool GetExtraStageInfoFromCourse( bool bExtra2, CString sPreferredGroup,
					   Song*& pSongOut, Steps*& pNotesOut, PlayerOptions& po_out, SongOptions& so_out );
	void SanityCheckGroupDir( CString sDir ) const;
	void AddGroup( CString sDir, CString sGroupDirName );

	void ReadNoteScoresFromFile( CString fn, int c );
	void ReadCourseScoresFromFile( CString fn, int c );
	void ReadCategoryRankingsFromFile( CString fn );
	void ReadCourseRankingsFromFile( CString fn );
	Song *FindSong( CString sGroup, CString sSong );

	void SaveNoteScoresToFile( CString fn, int c );
	void SaveCourseScoresToFile( CString fn, int c );
	void SaveCategoryRankingsToFile( CString fn );
	void SaveCourseRankingsToFile( CString fn );

	vector<Song*>		m_pSongs;	// all songs that can be played
	vector<Song*>		m_pBestSongs;
	CStringArray		m_sGroupNames;
	CStringArray		m_sGroupBannerPaths; // each song group may have a banner associated with it
	vector<Course*> m_pCourses;
};


extern SongManager*	SONGMAN;	// global and accessable from anywhere in our program

#endif

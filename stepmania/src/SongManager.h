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
	void LoadGroupSymLinks( CString sDir, CString sGroupFolder );

	void InitCoursesFromDisk( LoadingWindow *ld );
	void InitAutogenCourses();
	void FreeCourses();

	void Reload();	// songs, courses, groups - everything.
	void PreloadSongImages();


	CString GetGroupBannerPath( CString sGroupName );
	void GetGroupNames( CStringArray &AddTo );
	bool DoesGroupExist( CString sGroupName );

	RageColor GetGroupColor( const CString &sGroupName );
	RageColor GetSongColor( const Song* pSong );
	RageColor GetDifficultyColor( Difficulty dc ) const;
	CString GetDifficultyThemeName( const CString &in ) const;

	static CString ShortenGroupName( CString sLongGroupName );
	static int     GetNumStagesForSong( const Song* pSong );	// LongVer songs take 2 stages, MarathonVer take 3


	// Lookup
	const vector<Song*> &GetAllSongs() const { return m_pSongs; }
	const vector<Song*> &GetBestSongs( MemoryCard card=MEMORY_CARD_MACHINE ) const { return m_pBestSongs[MEMORY_CARD_MACHINE]; }
	const vector<Course*> &GetBestCourses( MemoryCard card=MEMORY_CARD_MACHINE ) const { return m_pBestCourses[MEMORY_CARD_MACHINE]; }
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

	struct CategoryData
	{
		struct HighScore
		{
			int iScore;
			CString	sName;

			HighScore()
			{
				iScore = 0;
			}

			bool operator>=( const HighScore& other ) const
			{
				return iScore >= other.iScore;
			}
		};
		vector<HighScore> vHighScores;

		void AddHighScore( HighScore hs, int &iIndexOut );

	} m_CategoryDatas[NUM_STEPS_TYPES][NUM_RANKING_CATEGORIES];

	void AddHighScore( StepsType nt, RankingCategory rc, PlayerNumber pn, CategoryData::HighScore hs, int &iMachineIndexOut )
	{
		hs.sName = RANKING_TO_FILL_IN_MARKER[pn];
		m_CategoryDatas[nt][rc].AddHighScore( hs, iMachineIndexOut );
	}

	void UpdateBest();

	void UpdateRankingCourses();

	void ReadSM300NoteScores();
	void ReadStepsMemCardDataFromFile( CString fn, int c );
	void ReadCourseMemCardDataFromFile( CString fn, int c );
	void ReadCategoryRankingsFromFile( CString fn );

	void SaveStepsMemCardDataToFile( CString fn, int c );
	void SaveCourseMemCardDataToFile( CString fn, int c );
	void SaveCategoryRankingsToFile( CString fn );

protected:
	void LoadStepManiaSongDir( CString sDir, LoadingWindow *ld );
	void LoadDWISongDir( CString sDir );
	bool GetExtraStageInfoFromCourse( bool bExtra2, CString sPreferredGroup,
					   Song*& pSongOut, Steps*& pNotesOut, PlayerOptions& po_out, SongOptions& so_out );
	void SanityCheckGroupDir( CString sDir ) const;
	void AddGroup( CString sDir, CString sGroupDirName );

	Song *FindSong( CString sGroup, CString sSong );

	void WriteStatsWebPage();

	vector<Song*>		m_pSongs;	// all songs that can be played
	vector<Song*>		m_pBestSongs[NUM_MEMORY_CARDS];
	CStringArray		m_sGroupNames;
	CStringArray		m_sGroupBannerPaths; // each song group may have a banner associated with it
	vector<Course*>		m_pCourses;
	vector<Course*>		m_pBestCourses[NUM_MEMORY_CARDS];
};


extern SongManager*	SONGMAN;	// global and accessable from anywhere in our program

#endif

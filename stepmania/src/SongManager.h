#pragma once 
/*
-----------------------------------------------------------------------------
 Class: SongManager

 Desc: Holder for all Songs and Notes.  Also keeps track of the current 
	Song and Notes, and loads/saves statistics.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "Song.h"
#include "Course.h"
#include "GamePlayStatistics.h"
//#include <d3dxmath.h>	// for D3DXCOLOR

const int MAX_SONG_QUEUE_SIZE = 400;	// this has to be gigantic to fit an "endless" number of songs
const int MAX_NUM_STAGES = 10;			// the max number of stages that can be played in arcade mode


class SongManager
{
public:
	SongManager();
	~SongManager();

	Song*		m_pCurSong;
	Notes*		m_pCurNotes[NUM_PLAYERS];
	Course*		m_pCurCourse;
	CString		m_sPreferredGroup;

	GameplayStatistics	m_GameplayStatistics[MAX_NUM_STAGES][NUM_PLAYERS];	// for passing from Dancing to Results


	CArray<Song*, Song*>	m_pSongs;	// all songs that can be played

	void InitSongArrayFromDisk();
	void FreeSongArray();
	void ReloadSongArray();


	Song* GetCurrentSong();
	Notes* GetCurrentNotes( PlayerNumber p );
	void SetCurrentSong( Song* pSong );
	void SetCurrentNotes( PlayerNumber p, Notes* pNotes );
	GameplayStatistics	GetLatestGameplayStatistics( PlayerNumber p );


	void ReadStatisticsFromDisk();
	void SaveStatisticsToDisk();


	CString GetGroupBannerPath( CString sGroupName );
	void GetGroupNames( CStringArray &AddTo );
	D3DXCOLOR GetGroupColor( const CString &sGroupName );

	CString ShortenGroupName( const CString &sOrigGroupName );

	void GetSongsInGroup( const CString sGroupName, CArray<Song*,Song*> &AddTo );


	// for Courses
	CArray<Course, Course>	m_aCourses;

	void InitCoursesFromDisk();
	void ReloadCourses();


protected:
	void LoadStepManiaSongDir( CString sDir );
	void LoadDWISongDir( CString sDir );
	CMapStringToString	m_mapGroupToBannerPath;		// each song group has a banner associated with it
	CStringArray		m_arrayGroupNames;
};


extern SongManager*	SONGMAN;	// global and accessable from anywhere in our program

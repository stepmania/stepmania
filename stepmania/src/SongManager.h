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
//#include <d3dxmath.h>	// for D3DXCOLOR

const int MAX_SONG_QUEUE_SIZE = 400;	// this has to be gigantic to fit an "endless" number of songs

class SongManager
{
public:
	SongManager( void(*callback)() );
	~SongManager();

	CArray<Song*, Song*>	m_pSongs;	// all songs that can be played

	void InitSongArrayFromDisk( void(*callback)() );
	void FreeSongArray();
	void ReloadSongArray();

	void ReadStatisticsFromDisk();
	void SaveStatisticsToDisk();


	CString GetGroupBannerPath( CString sGroupName );
	void GetGroupNames( CStringArray &AddTo );
	D3DXCOLOR GetGroupColor( const CString &sGroupName );

	static CString ShortenGroupName( const CString &sOrigGroupName );

	void GetSongsInGroup( const CString sGroupName, CArray<Song*,Song*> &AddTo );


	// for Oni
	CArray<Course, Course> m_aOniCourses;

	// for Extra Stages
	CArray<Course, Course> m_aExtraCourses;
	
	// for Endless
	CArray<Course, Course> m_aEndlessCourses;

	void InitCoursesFromDisk();
	void ReloadCourses();


protected:
	void LoadStepManiaSongDir( CString sDir, void(*callback)() );
	void LoadDWISongDir( CString sDir );
	CMapStringToString	m_mapGroupToBannerPath;		// each song group has a banner associated with it
	CStringArray		m_arrayGroupNames;
};


extern SongManager*	SONGMAN;	// global and accessable from anywhere in our program

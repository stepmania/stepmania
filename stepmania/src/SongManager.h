/*
-----------------------------------------------------------------------------
 Class: SongManager

 Desc: Holder for all Songs and Pattern.  Also keeps track of the current 
	Song and Pattern, and loads/saves statistics.

 Copyright (c) 2001-2002 by the names listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#ifndef _SongManager_H_
#define _SongManager_H_


#include "Song.h"



class SongManager
{
public:
	SongManager();
	~SongManager();

	Song*					m_pCurSong;
	Pattern*				m_pCurPattern[NUM_PLAYERS];

	CArray<Song*, Song*>	m_pSongs;	// all songs that can be played

	void InitSongArrayFromDisk();

	void CleanUpSongArray();
	void ReloadSongArray();


	void ReadStatisticsFromDisk();
	void SaveStatisticsToDisk();


	CString GetGroupBannerPath( CString sGroupName );
	
	void GetGroupNames( CStringArray &AddTo );

	void GetSongsInGroup( const CString sGroupName, CArray<Song*,Song*> &AddTo );


protected:
	void LoadStepManiaSongDir( CString sDir );
	void LoadDWISongDir( CString sDir );
	CMapStringToString	m_mapGroupToBannerPath;		// each song group has a banner associated with it

};


extern SongManager*	SONG;	// global and accessable from anywhere in our program


#endif
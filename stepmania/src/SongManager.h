/*
-----------------------------------------------------------------------------
 Class: SongManager

 Desc: Holder for all Songs and Steps.  Also keeps track of the current 
	Song and Steps, and loads/saves statistics.

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

	CArray<Song*, Song*>	m_pSongs;
	Song*					m_pCurSong;
	Steps*					m_pStepsPlayer[NUM_PLAYERS];

	void InitSongArrayFromDisk();
	void CleanUpSongArray();
	void ReloadSongArray();

	void ReadStatisticsFromDisk();
	void SaveStatisticsToDisk();

protected:

};


extern SongManager*	SONGS;	// global and accessable from anywhere in our program


#endif
/*
-----------------------------------------------------------------------------
 Class: SongManager

 Desc: Holder for all Songs and NoteMetadata.  Also keeps track of the current 
	Song and NoteMetadata, and loads/saves statistics.

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#ifndef _SongManager_H_
#define _SongManager_H_


#include "Song.h"
//#include <d3dxmath.h>	// for D3DXCOLOR


class SongManager
{
public:
	SongManager();
	~SongManager();

	Song*		m_pCurSong;
	NoteMetadata*	m_pCurNoteMetadata[NUM_PLAYERS];
	CString		m_sPreferredGroup;

	CArray<Song*, Song*>	m_pSongs;	// all songs that can be played

	void InitSongArrayFromDisk();

	void CleanUpSongArray();
	void ReloadSongArray();


	void ReadStatisticsFromDisk();
	void SaveStatisticsToDisk();


	CString GetGroupBannerPath( CString sGroupName );
	void GetGroupNames( CStringArray &AddTo );
	D3DXCOLOR GetGroupColor( const CString &sGroupName );

	CString ShortenGroupName( const CString &sOrigGroupName );

	void GetSongsInGroup( const CString sGroupName, CArray<Song*,Song*> &AddTo );


protected:
	void LoadStepManiaSongDir( CString sDir );
	void LoadDWISongDir( CString sDir );
	CMapStringToString	m_mapGroupToBannerPath;		// each song group has a banner associated with it
	CStringArray		m_arrayGroupNames;
};


extern SongManager*	SONG;	// global and accessable from anywhere in our program


#endif
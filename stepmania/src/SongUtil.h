#ifndef SONGUTIL_H
#define SONGUTIL_H
/*
-----------------------------------------------------------------------------
 Class: SongUtil

 Desc: Utility functions that deal with Song.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GameConstantsAndTypes.h"

class Song;
class Profile;
struct XNode;

namespace SongUtil
{
	CString MakeSortString( CString s );
	void SortSongPointerArrayByDifficulty( vector<Song*> &arraySongPointers );
	void SortSongPointerArrayByTitle( vector<Song*> &arraySongPointers );
	void SortSongPointerArrayByBPM( vector<Song*> &arraySongPointers );
	void SortSongPointerArrayByGrade( vector<Song*> &arraySongPointers );
	void SortSongPointerArrayByArtist( vector<Song*> &arraySongPointers );
	void SortSongPointerArrayByGroupAndDifficulty( vector<Song*> &arraySongPointers );
	void SortSongPointerArrayByGroupAndTitle( vector<Song*> &arraySongPointers );
	void SortSongPointerArrayByNumPlays( vector<Song*> &arraySongPointers, ProfileSlot slot, bool bDescending );
	void SortSongPointerArrayByNumPlays( vector<Song*> &arraySongPointers, const Profile* pProfile, bool bDescending );
	void SortSongPointerArrayByMeter( vector<Song*> &arraySongPointers, Difficulty dc );
	CString GetSectionNameFromSongAndSort( const Song* pSong, SortOrder so );
	void SortSongPointerArrayBySectionName( vector<Song*> &arraySongPointers, SortOrder so );
}

class SongID
{
	CString sDir;

public:
	SongID() { FromSong(NULL); }
	void FromSong( const Song *p );
	Song *ToSong() const;
	bool operator<( const SongID &other ) const
	{
		return sDir < other.sDir;
	}

	XNode* CreateNode() const;
	void LoadFromNode( const XNode* pNode );

	bool IsValid() const;
};


#endif

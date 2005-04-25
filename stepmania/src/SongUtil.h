/* SongUtil - Utility functions that deal with Song. */

#ifndef SONG_UTIL_H
#define SONG_UTIL_H

#include "GameConstantsAndTypes.h"
#include "Difficulty.h"

class Song;
class Profile;
struct XNode;

namespace SongUtil
{
	CString MakeSortString( CString s );
	void SortSongPointerArrayByTitle( vector<Song*> &vpSongsInOut );
	void SortSongPointerArrayByBPM( vector<Song*> &vpSongsInOut );
	void SortSongPointerArrayByGrade( vector<Song*> &vpSongsInOut );
	void SortSongPointerArrayByArtist( vector<Song*> &vpSongsInOut );
	void SortSongPointerArrayByDisplayArtist( vector<Song*> &vpSongsInOut );
	void SortSongPointerArrayByGenre( vector<Song*> &vpSongsInOut );
	void SortSongPointerArrayByGroupAndDifficulty( vector<Song*> &vpSongsInOut );
	void SortSongPointerArrayByGroupAndTitle( vector<Song*> &vpSongsInOut );
	void SortSongPointerArrayByNumPlays( vector<Song*> &vpSongsInOut, ProfileSlot slot, bool bDescending );
	void SortSongPointerArrayByNumPlays( vector<Song*> &vpSongsInOut, const Profile* pProfile, bool bDescending );
	void SortSongPointerArrayByMeter( vector<Song*> &vpSongsInOut, Difficulty dc );
	CString GetSectionNameFromSongAndSort( const Song* pSong, SortOrder so );
	void SortSongPointerArrayBySectionName( vector<Song*> &vpSongsInOut, SortOrder so );
	void SortByMostRecentlyPlayedForMachine( vector<Song*> &vpSongsInOut );
}

class SongID
{
	CString sDir;

public:
	SongID() { Unset(); }
	void Unset() { FromSong(NULL); }
	void FromSong( const Song *p );
	Song *ToSong() const;
	bool operator<( const SongID &other ) const
	{
		return sDir < other.sDir;
	}

	XNode* CreateNode() const;
	void LoadFromNode( const XNode* pNode );
	CString ToString() const;
	bool IsValid() const;
};


#endif

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

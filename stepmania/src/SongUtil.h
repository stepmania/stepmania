/* SongUtil - Utility functions that deal with Song. */

#ifndef SONG_UTIL_H
#define SONG_UTIL_H

#include "GameConstantsAndTypes.h"
#include "Difficulty.h"

class Song;
class Steps;
class Profile;
class XNode;

class SongCriteria
{
public:
	RString m_sGroupName;	// "" means don't match
	bool m_bUseSongGenreAllowedList;
	vector<RString> m_vsSongGenreAllowedList;
	enum Selectable { Selectable_Yes, Selectable_No, Selectable_DontCare } m_Selectable;
	bool m_bUseSongAllowedList;
	vector<Song*> m_vpSongAllowedList;
	int m_iStagesForSong;		// don't filter if -1
	enum Tutorial { Tutorial_Yes, Tutorial_No, Tutorial_DontCare } m_Tutorial;
	enum Locked { Locked_Locked, Locked_Unlocked, Locked_DontCare } m_Locked;

	SongCriteria()
	{
		m_bUseSongGenreAllowedList = false;
		m_Selectable = Selectable_DontCare;
		m_bUseSongAllowedList = false;
		m_iStagesForSong = -1;
		m_Tutorial = Tutorial_DontCare;
		m_Locked = Locked_DontCare;
	}

	bool Matches( const Song *p ) const;
	bool operator==( const SongCriteria &other ) const
	{
#define X(x) (x == other.x)
		return 
			X(m_sGroupName) && 
			X(m_bUseSongGenreAllowedList) && 
			X(m_vsSongGenreAllowedList) &&
			X(m_Selectable) && 
			X(m_bUseSongAllowedList) && 
			X(m_vpSongAllowedList) &&
			X(m_iStagesForSong) && 
			X(m_Tutorial) && 
			X(m_Locked);
#undef X
	}
	bool operator!=( const SongCriteria &other ) const { return !operator==( other ); }
};

namespace SongUtil
{
	void GetSteps( 
		const Song *pSong,
		vector<Steps*>& arrayAddTo, 
		StepsType st = StepsType_Invalid, 
		Difficulty dc = DIFFICULTY_INVALID, 
		int iMeterLow = -1, 
		int iMeterHigh = -1, 
		const RString &sDescription = "", 
		bool bIncludeAutoGen = true, 
		unsigned uHash = 0,
		int iMaxToGet = -1 
		);
	Steps* GetOneSteps( 
		const Song *pSong,
		StepsType st = StepsType_Invalid, 
		Difficulty dc = DIFFICULTY_INVALID, 
		int iMeterLow = -1, 
		int iMeterHigh = -1, 
		const RString &sDescription = "", 
		unsigned uHash = 0,
		bool bIncludeAutoGen = true
		);
	Steps* GetStepsByDifficulty(	const Song *pSong, StepsType st, Difficulty dc, bool bIncludeAutoGen = true );
	Steps* GetStepsByMeter(		const Song *pSong, StepsType st, int iMeterLow, int iMeterHigh );
	Steps* GetStepsByDescription(	const Song *pSong, StepsType st, RString sDescription );
	Steps* GetClosestNotes(		const Song *pSong, StepsType st, Difficulty dc, bool bIgnoreLocked=false );

	void AdjustDuplicateSteps( Song *pSong ); // part of TidyUpData
	void DeleteDuplicateSteps( Song *pSong, vector<Steps*> &vSteps );

	RString MakeSortString( RString s );
	void SortSongPointerArrayByTitle( vector<Song*> &vpSongsInOut );
	void SortSongPointerArrayByBPM( vector<Song*> &vpSongsInOut );
	void SortSongPointerArrayByGrades( vector<Song*> &vpSongsInOut, bool bDescending );
	void SortSongPointerArrayByArtist( vector<Song*> &vpSongsInOut );
	void SortSongPointerArrayByDisplayArtist( vector<Song*> &vpSongsInOut );
	void SortSongPointerArrayByGenre( vector<Song*> &vpSongsInOut );
	void SortSongPointerArrayByGroupAndTitle( vector<Song*> &vpSongsInOut );
	void SortSongPointerArrayByNumPlays( vector<Song*> &vpSongsInOut, ProfileSlot slot, bool bDescending );
	void SortSongPointerArrayByNumPlays( vector<Song*> &vpSongsInOut, const Profile* pProfile, bool bDescending );
	void SortSongPointerArrayByMeter( vector<Song*> &vpSongsInOut, Difficulty dc );
	RString GetSectionNameFromSongAndSort( const Song *pSong, SortOrder so );
	void SortSongPointerArrayBySectionName( vector<Song*> &vpSongsInOut, SortOrder so );
	void SortByMostRecentlyPlayedForMachine( vector<Song*> &vpSongsInOut );

	int CompareSongPointersByGroup(const Song *pSong1, const Song *pSong2);

	bool IsEditDescriptionUnique( const Song* pSong, StepsType st, const RString &sPreferredDescription, const Steps *pExclude );
	RString MakeUniqueEditDescription( const Song* pSong, StepsType st, const RString &sPreferredDescription );
	bool ValidateCurrentEditStepsDescription( const RString &sAnswer, RString &sErrorOut );
	bool ValidateCurrentStepsDescription( const RString &sAnswer, RString &sErrorOut );

	void GetAllSongGenres( vector<RString> &vsOut );
	void FilterSongs( const SongCriteria &sc, const vector<Song*> &in, vector<Song*> &out );
}

class SongID
{
	RString sDir;

public:
	SongID() { Unset(); }
	void Unset() { FromSong(NULL); }
	void FromSong( const Song *p );
	Song *ToSong() const;
	bool operator<( const SongID &other ) const
	{
		return sDir < other.sDir;
	}
	bool operator==( const SongID &other ) const
	{
		return sDir == other.sDir;
	}

	XNode* CreateNode() const;
	void LoadFromNode( const XNode* pNode );
	void LoadFromDir( RString _sDir ) { sDir = _sDir; }
	RString ToString() const;
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

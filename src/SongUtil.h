/** @brief SongUtil - Utility functions that deal with Song. */

#ifndef SONG_UTIL_H
#define SONG_UTIL_H
#include "PlayerNumber.h"
#include "GameConstantsAndTypes.h"
#include "Difficulty.h"

#include <set>
#include <vector>


class Song;
class Steps;
class Profile;
class XNode;

void AppendOctal( int n, int digits, RString &out );

/** @brief The criteria for dealing with songs. */
class SongCriteria
{
public:
	/**
	 * @brief What group name are we searching for for Songs?
	 *
	 * If an empty string, don't bother using this for searching. */
	std::vector<RString> m_vsGroupNames;
	std::vector<RString> m_vsSongNames;
	std::vector<RString> m_vsArtistNames;

	bool m_bUseSongGenreAllowedList;
	std::vector<RString> m_vsSongGenreAllowedList;
	enum Selectable { Selectable_Yes, Selectable_No, Selectable_DontCare } m_Selectable;
	bool m_bUseSongAllowedList;
	std::vector<Song*> m_vpSongAllowedList;
	/** @brief How many songs does this take max? Don't use this if it's -1. */
	int m_iMaxStagesForSong;		// don't filter if -1
	float m_fMinBPM;		// don't filter if -1
	float m_fMaxBPM;		// don't filter if -1
	float m_fMinDurationSeconds; // don't filter if -1
	float m_fMaxDurationSeconds; // don't filter if -1

	/** @brief Is this song used for tutorial purposes? */
	enum Tutorial
	{
		Tutorial_Yes, /**< This song is used for tutorial purposes. */
		Tutorial_No, /**< This song is not used for tutorial purposes. */
		Tutorial_DontCare /**< This song can or cannot be used for tutorial purposes. */
	} m_Tutorial;
	/** @brief Is this song used for locking/unlocking purposes? */
	enum Locked
	{
		Locked_Locked, /**< This song is a locked song. */
		Locked_Unlocked, /**< This song is an unlocked song. */
		Locked_DontCare /**< This song can or cannot be locked or unlocked. */
	} m_Locked;

	/** @brief Set up some initial song criteria. */
	SongCriteria(): m_vsGroupNames(), m_vsSongNames(), m_vsArtistNames(), m_bUseSongGenreAllowedList(false),
		m_vsSongGenreAllowedList(), m_Selectable(Selectable_DontCare),
		m_bUseSongAllowedList(false), m_vpSongAllowedList(),
		m_iMaxStagesForSong(-1), m_fMinBPM(-1), m_fMaxBPM(-1), m_fMinDurationSeconds(-1), m_fMaxDurationSeconds(-1),
		m_Tutorial(Tutorial_DontCare),
		m_Locked(Locked_DontCare)
	{
		// m_fMinBPM = -1;
		// m_fMaxBPM = -1;
	}

	/**
	 * @brief Determine if the song matches the current criteria.
	 * @param p the song to compare against the criteria.
	 * @return true of the song matches the criteria, false otherwise.
	 */
	bool Matches( const Song *p ) const;
	/**
	 * @brief Determine if two sets of criteria are equivalent.
	 * @param other the other criteria.
	 * @return true if the two sets of criteria are equal, false otherwise.
	 */
	bool operator==( const SongCriteria &other ) const
	{
/** @brief A quick way to match every part of the song criterium. */
#define X(x) (x == other.x)
		return
			X(m_vsGroupNames) &&
			X(m_vsSongNames) &&
			X(m_vsArtistNames) &&
			X(m_bUseSongGenreAllowedList) &&
			X(m_vsSongGenreAllowedList) &&
			X(m_Selectable) &&
			X(m_bUseSongAllowedList) &&
			X(m_vpSongAllowedList) &&
			X(m_iMaxStagesForSong) &&
			X(m_fMinBPM) &&
			X(m_fMaxBPM) &&
			X(m_fMinDurationSeconds) &&
			X(m_fMaxDurationSeconds) &&
			X(m_Tutorial) &&
			X(m_Locked);
#undef X
	}
	/**
	 * @brief Determine if two sets of criteria are not equivalent.
	 * @param other the other criteria.
	 * @return true if the two sets of criteria are not equal, false otherwise.
	 */
	bool operator!=( const SongCriteria &other ) const { return !operator==( other ); }
};

/** @brief A set of song utilities to make working with songs easier. */
namespace SongUtil
{
	void GetSteps(
		const Song *pSong,
		std::vector<Steps*>& arrayAddTo,
		StepsType st = StepsType_Invalid,
		Difficulty dc = Difficulty_Invalid,
		int iMeterLow = -1,
		int iMeterHigh = -1,
		const RString &sDescription = "",
		const RString &sCredit = "",
		bool bIncludeAutoGen = true,
		unsigned uHash = 0,
		int iMaxToGet = -1
		);
	Steps* GetOneSteps(
		const Song *pSong,
		StepsType st = StepsType_Invalid,
		Difficulty dc = Difficulty_Invalid,
		int iMeterLow = -1,
		int iMeterHigh = -1,
		const RString &sDescription = "",
		const RString &sCredit = "",
		unsigned uHash = 0,
		bool bIncludeAutoGen = true
		);
	Steps* GetStepsByDifficulty(	const Song *pSong, StepsType st, Difficulty dc, bool bIncludeAutoGen = true );
	Steps* GetStepsByMeter(		const Song *pSong, StepsType st, int iMeterLow, int iMeterHigh );
	Steps* GetStepsByDescription(	const Song *pSong, StepsType st, RString sDescription );
	Steps* GetStepsByCredit(	const Song *pSong, StepsType st, RString sCredit );
	Steps* GetClosestNotes(		const Song *pSong, StepsType st, Difficulty dc, bool bIgnoreLocked=false );

	void AdjustDuplicateSteps( Song *pSong ); // part of TidyUpData
	void DeleteDuplicateSteps( Song *pSong, std::vector<Steps*> &vSteps );

	RString MakeSortString( RString s );
	void SortSongPointerArrayByTitle( std::vector<Song*> &vpSongsInOut );
	void SortSongPointerArrayByBPM( std::vector<Song*> &vpSongsInOut );
	void SortSongPointerArrayByGrades( std::vector<Song*> &vpSongsInOut, bool bDescending );
	void SortSongPointerArrayByProfileGrades( std::vector<Song*> &vpSongsInOut, bool bDescending, PlayerNumber pn );
	void SortSongPointerArrayByArtist( std::vector<Song*> &vpSongsInOut );
	void SortSongPointerArrayByDisplayArtist( std::vector<Song*> &vpSongsInOut );
	void SortSongPointerArrayByGenre( std::vector<Song*> &vpSongsInOut );
	void SortSongPointerArrayByGroupAndTitle( std::vector<Song*> &vpSongsInOut );
	void SortSongPointerArrayByNumPlays( std::vector<Song*> &vpSongsInOut, ProfileSlot slot, bool bDescending );
	void SortSongPointerArrayByNumPlays( std::vector<Song*> &vpSongsInOut, const Profile* pProfile, bool bDescending );
	void SortSongPointerArrayByStepsTypeAndMeter( std::vector<Song*> &vpSongsInOut, StepsType st, Difficulty dc );
	RString GetSectionNameFromSongAndSort( const Song *pSong, SortOrder so );
	void SortSongPointerArrayBySectionName( std::vector<Song*> &vpSongsInOut, SortOrder so );
	void SortByMostRecentlyPlayedForMachine( std::vector<Song*> &vpSongsInOut );
	void SortSongPointerArrayByLength( std::vector<Song*> &vpSongsInOut );

	int CompareSongPointersByGroup(const Song *pSong1, const Song *pSong2);

	/**
	 * @brief Determine if the requested description for an edit is unique.
	 * @param pSong the song the edit is for.
	 * @param st the steps type for the edit.
	 * @param sPreferredDescription the requested description.
	 * @param pExclude the steps that want the description.
	 * @return true if it is unique, false otherwise.
	 */
	bool IsEditDescriptionUnique( const Song* pSong, StepsType st, const RString &sPreferredDescription, const Steps *pExclude );
	bool IsChartNameUnique( const Song* pSong, StepsType st, const RString &name, const Steps *pExclude );
	RString MakeUniqueEditDescription( const Song* pSong, StepsType st, const RString &sPreferredDescription );
	bool ValidateCurrentEditStepsDescription( const RString &sAnswer, RString &sErrorOut );
	bool ValidateCurrentStepsDescription( const RString &sAnswer, RString &sErrorOut );
	bool ValidateCurrentStepsCredit( const RString &sAnswer, RString &sErrorOut );
	bool ValidateCurrentStepsChartName(const RString &answer, RString &error);
	bool ValidateCurrentSongPreview(const RString& answer, RString& error);
	bool ValidateCurrentStepsMusic(const RString &answer, RString &error);

	void GetAllSongGenres( std::vector<RString> &vsOut );
	/**
	 * @brief Filter the selection of songs to only match certain criteria.
	 * @param sc the intended song criteria.
	 * @param in the starting batch of songs.
	 * @param out the resulting batch.
	 * @param doCareAboutGame a flag to see if we should only get playable steps. */
	void FilterSongs( const SongCriteria &sc, const std::vector<Song*> &in, std::vector<Song*> &out,
			 bool doCareAboutGame = false );

	void GetPlayableStepsTypes( const Song *pSong, std::set<StepsType> &vOut );
	void GetPlayableSteps( const Song *pSong, std::vector<Steps*> &vOut );
	bool IsStepsTypePlayable( Song *pSong, StepsType st );
	bool IsStepsPlayable( Song *pSong, Steps *pSteps );

	/**
	 * @brief Determine if the song has any playable steps in the present game.
	 * @param s the current song.
	 * @return true if the song has playable steps, false otherwise. */
	bool IsSongPlayable( Song *s );

	bool GetStepsTypeAndDifficultyFromSortOrder( SortOrder so, StepsType &st, Difficulty &dc );
}

class SongID
{
	RString sDir;

public:
	/**
	 * @brief Set up the SongID with default values.
	 *
	 * This used to call Unset() to do the same thing. */
	SongID(): sDir("") {}
	void Unset() { FromSong(nullptr); }
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
	void FromString( RString _sDir ) { sDir = _sDir; }
	RString ToString() const;
	bool IsValid() const;
};


#endif

/**
 * @file
 * @author Chris Danford, Glenn Maynard (c) 2001-2004
 * @section LICENSE
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

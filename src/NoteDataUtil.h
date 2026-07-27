#ifndef NOTEDATAUTIL_H
#define NOTEDATAUTIL_H

#include "GameConstantsAndTypes.h"
#include "NoteTypes.h"

#include <vector>


class PlayerOptions;
struct RadarValues;
class NoteData;
class Song;
struct AttackArray;
class TimingData;

void PlaceAutoKeysound( NoteData &out, int row, TapNote akTap );
int FindLongestOverlappingHoldNoteForAnyTrack( const NoteData &in, int iRow );
void LightTransformHelper( const NoteData &in, NoteData &out, const std::vector<int> &aiTracks );

/**
 * @brief Utility functions that deal with NoteData.
 *
 * Things should go in here if they can be (cleanly and efficiently)
 * implemented using only NoteData's primitives; this improves abstraction
 * and makes it much easier to change NoteData internally in the future. */
namespace NoteDataUtil
{
	NoteType GetSmallestNoteTypeForMeasure( const NoteData &nd, int iMeasureIndex );
	NoteType GetSmallestNoteTypeInRange( const NoteData &nd, int iStartIndex, int iEndIndex );
	void LoadFromSMNoteDataString( NoteData &out, const RString &sSMNoteData, bool bComposite );
	void GetSMNoteDataString( const NoteData &in, RString &notes_out );
	void SplitCompositeNoteData( const NoteData &in, std::vector<NoteData> &out );
	void CombineCompositeNoteData( NoteData &out, const std::vector<NoteData> &in );
	/**
	 * @brief Autogenerate notes from one type to another.
	 *
	 * TODO: Look into a more intelligent way of doing so.
	 * @param in The original NoteData.
	 * @param out the new NoteData.
	 * @param iNewNumTracks the number of tracks/columns of the new NoteData. */
	void LoadTransformedSlidingWindow( const NoteData &in, NoteData &out, int iNewNumTracks );
	/**
	 * @brief Autogenerate notes from one type to another.
	 *
	 * NOTE: This code assumes that there are more columns in the original type.
	 * @param in The original NoteData.
	 * @param out the new NoteData.
	 * @param iNewNumTracks the number of tracks/columns of the new NoteData. */
	void LoadOverlapped( const NoteData &in, NoteData &out, int iNewNumTracks );
	void LoadTransformedLights( const NoteData &in, NoteData &out, int iNewNumTracks );
	void LoadTransformedLightsFromTwo( const NoteData &marquee, const NoteData &bass, NoteData &out );
	void InsertHoldTails( NoteData &inout );

	// Special case so that kickbox can have autogen steps that are playable.
	// Hopefully I'll replace this with a good generalized autogen system
	// later.  -Kyz
	void AutogenKickbox(const NoteData& in, NoteData& out, const TimingData& timing, StepsType out_type, int nonrandom_seed);

	void CalculateRadarValues( const NoteData &in, float fSongSeconds, RadarValues& out );

	/**
	 * @brief Remove all of the Hold notes.
	 * @param inout the Notedata to be transformed.
	 * @param iStartIndex the starting point for transforming.
	 * @param iEndIndex the ending point for transforming. */
	void RemoveHoldNotes( NoteData &inout, int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW );
	void ChangeRollsToHolds( NoteData &in, int iStartIndex, int iEndIndex );
	void ChangeHoldsToRolls( NoteData &in, int iStartIndex, int iEndIndex );
	void RemoveSimultaneousNotes( NoteData &inout, int iMaxSimultaneous, int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW );
	void RemoveJumps( NoteData &inout, int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW );
	void RemoveHands( NoteData &inout, int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW );
	void RemoveQuads( NoteData &inout, int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW );
	/**
	 * @brief Remove all of a specific TapNote Type.
	 * @param inout the Notedata to be transformed.
	 * @param tn the TapNote Type to remove.
	 * @param iStartIndex the starting point for transforming.
	 * @param iEndIndex the ending point for transforming. */
	void RemoveSpecificTapNotes(NoteData &inout, TapNoteType tn, int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW);
	/**
	 * @brief Remove all of the mines from the chart.
	 * @param inout the Notedata to be transformed.
	 * @param iStartIndex the starting point for transforming.
	 * @param iEndIndex the ending point for transforming. */
	void RemoveMines(NoteData &inout, int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW);
	/**
	 * @brief Remove all of the lifts from the chart.
	 * @param inout the Notedata to be transformed.
	 * @param iStartIndex the starting point for transforming.
	 * @param iEndIndex the ending point for transforming. */
	void RemoveLifts(NoteData &inout, int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW);
	/**
	 * @brief Remove all of the fakes from the chart.
	 * @param inout the Notedata to be transformed.
	 * @param iStartIndex the starting point for transforming.
	 * @param iEndIndex the ending point for transforming. */
	void RemoveFakes(NoteData &inout, TimingData const& timing_data, int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW);
	void RemoveStretch( NoteData &inout, StepsType st, int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW );
	void RemoveAllButOneTap( NoteData &inout, int row );
	void RemoveAllButPlayer( NoteData &inout, PlayerNumber pn );
	/** @brief The types of transformations available for the NoteData. */
	enum TrackMapping
	{
		left, /**< The NoteData is arranged as if the player was facing to the left. */
		right, /**< The NoteData is arranged as if the player was facing to the right. */
		mirror, /**< The NoteData is arranged as if facing a straight mirror. */
		lrmirror, /**< The NoteData is arranged so that the left and right columns are flipped. */
		udmirror, /**< The NoteData is arranged so that the up and down columns are flipped. */
		backwards, /**< The NoteData is arranged as if the player was facing backwards.
					This is NOT always the same as mirror. */
		shuffle,
		soft_shuffle,
		super_shuffle,
		hyper_shuffle,
		stomp,
		swap_up_down,
		NUM_TRACK_MAPPINGS
	};
	void Turn( NoteData &inout, StepsType st, TrackMapping tt, int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW );
	void Little( NoteData &inout, int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW );
	void Wide( NoteData &inout, int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW );
	void Big( NoteData &inout, int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW );
	void Quick( NoteData &inout, int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW );
	void BMRize( NoteData &inout, int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW );
	void Skippy( NoteData &inout, int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW );
	void InsertIntelligentTaps(
		NoteData &in,
		int iWindowSizeRows,
		int iInsertOffsetRows,
		int iWindowStrideRows,
		bool bSkippy,
		int iStartIndex = 0,
		int iEndIndex = MAX_NOTE_ROW );
	void AddMines( NoteData &inout, int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW );
	void Echo( NoteData &inout, int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW );
	void Stomp( NoteData &inout, StepsType st, int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW );
	void Planted( NoteData &inout, int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW );
	void Floored( NoteData &inout, int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW );
	void Twister( NoteData &inout, int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW );
	void ConvertTapsToHolds( NoteData &inout, int iSimultaneousHolds, int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW );

	/**
	 * @brief Convert all taps added via transforms into original style tap notes.
	 * @param inout the NoteData to convert. */
	void ConvertAdditionsToRegular( NoteData &inout );

	void Backwards( NoteData &inout );
	void SwapSides( NoteData &inout );
	void CopyLeftToRight( NoteData &inout );
	void CopyRightToLeft( NoteData &inout );
	void ClearLeft( NoteData &inout );
	void ClearRight( NoteData &inout );
	void CollapseToOne( NoteData &inout );
	void CollapseLeft( NoteData &inout );
	void ShiftTracks( NoteData &inout, int iShiftBy );
	void ShiftLeft( NoteData &inout );
	void ShiftRight( NoteData &inout );
	void SwapUpDown( NoteData &inout, StepsType st );
	void ArbitraryRemap( NoteData &inout, int* mapping );

	void SnapToNearestNoteType( NoteData &inout, NoteType nt1, NoteType nt2, int iStartIndex, int iEndIndex );

	inline void SnapToNearestNoteType( NoteData &inout, NoteType nt, int iStartIndex, int iEndIndex )
	{
		SnapToNearestNoteType( inout, nt, NoteType_Invalid, iStartIndex, iEndIndex );
	}

	// True if no notes in row that aren't true in the mask
	bool RowPassesValidMask( NoteData &inout, int row, const bool bValidMask[] );

	void TransformNoteData(NoteData &nd, TimingData const& timing_data,
		const AttackArray &aa, StepsType st, Song* pSong);
	void TransformNoteData(NoteData &nd, TimingData const& timing_data,
		const PlayerOptions &po, StepsType st,
			       int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW );
	void AddTapAttacks( NoteData &nd, Song* pSong );

	void Scale( NoteData &nd, float fScale );
	void ScaleRegion( NoteData &nd, float fScale, int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW );

	void InsertRows( NoteData &nd, int iStartIndex, int iRowsToShift );
	void DeleteRows( NoteData &nd, int iStartIndex, int iRowsToShift );

	void RemoveAllTapsOfType( NoteData& ndInOut, TapNoteType typeToRemove );
	void RemoveAllTapsExceptForType( NoteData& ndInOut, TapNoteType typeToKeep );

	int GetMaxNonEmptyTrack( const NoteData& in );
	bool AnyTapsAndHoldsInTrackRange( const NoteData& in, int iTrack, int iStart, int iEnd );

	bool GetNextEditorPosition( const NoteData& in, int &rowInOut );
	bool GetPrevEditorPosition( const NoteData& in, int &rowInOut );

	/** @brief Count the number of hold ticks that will fire, assuming that tickholds are on.
	  * @param td The TimingData from the relevant Steps. */
	unsigned int GetTotalHoldTicks( NoteData* nd, const TimingData* td );
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

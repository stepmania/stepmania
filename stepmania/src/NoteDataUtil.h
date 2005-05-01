/* NoteDataUtil - Utility functions that deal with NoteData. */

#ifndef NOTEDATAUTIL_H
#define NOTEDATAUTIL_H

#include "GameConstantsAndTypes.h"	// for RadarCategory
#include "NoteTypes.h"
#include "NoteData.h"

struct PlayerOptions;
struct RadarValues;

/* Utils for NoteData.  Things should go in here if they can be (cleanly and
 * efficiently) implemented using only NoteData's primitives; this improves
 * abstraction and makes it much easier to change NoteData internally in
 * the future. */
namespace NoteDataUtil
{
	NoteType GetSmallestNoteTypeForMeasure( const NoteData &n, int iMeasureIndex );
	void LoadFromSMNoteDataString( NoteData &out, CString sSMNoteData );
	void GetSMNoteDataString( const NoteData &in, CString &notes_out );
	void LoadTransformedSlidingWindow( const NoteData &in, NoteData &out, int iNewNumTracks );
	void LoadOverlapped( const NoteData &in, NoteData &out, int iNewNumTracks );
	void LoadTransformedLights( const NoteData &in, NoteData &out, int iNewNumTracks );
	void InsertHoldTails( NoteData &inout );

	// radar values - return between 0.0 and 1.2
	float GetStreamRadarValue( const NoteData &in, float fSongSeconds );
	float GetVoltageRadarValue( const NoteData &in, float fSongSeconds );
	float GetAirRadarValue( const NoteData &in, float fSongSeconds );
	float GetFreezeRadarValue( const NoteData &in, float fSongSeconds );
	float GetChaosRadarValue( const NoteData &in, float fSongSeconds );

	void CalculateRadarValues( const NoteData &in, float fSongSeconds, RadarValues& out );

	void RemoveHoldNotes( NoteData &inout, int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW );
	void ChangeRollsToHolds( NoteData &in, int iStartIndex, int iEndIndex );
	void RemoveSimultaneousNotes( NoteData &inout, int iMaxSimultaneous, int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW );
	void RemoveJumps( NoteData &inout, int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW );
	void RemoveHands( NoteData &inout, int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW );
	void RemoveQuads( NoteData &inout, int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW );
	void RemoveMines( NoteData &inout, int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW );
	void RemoveStretch( NoteData &inout, StepsType st, int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW );
	void RemoveAllButOneTap( NoteData &inout, int row );
	enum TrackMapping { left, right, mirror, shuffle, super_shuffle, stomp, NUM_TRACK_MAPPINGS };
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

	// change all TAP_ADDITIONs to TAP_TAPs
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

	void SnapToNearestNoteType( NoteData &inout, NoteType nt1, NoteType nt2, int iStartIndex, int iEndIndex );

	inline void SnapToNearestNoteType( NoteData &inout, NoteType nt, int iStartIndex, int iEndIndex )
	{
		SnapToNearestNoteType( inout, nt, NOTE_TYPE_INVALID, iStartIndex, iEndIndex );
	}

	// True if no notes in row that aren't true in the mask
	bool RowPassesValidMask( NoteData &inout, int row, const bool bValidMask[] );

	void TransformNoteData( NoteData &nd, const AttackArray &aa, StepsType st, Song* pSong );
	void TransformNoteData( NoteData &nd, const PlayerOptions &po, StepsType st, int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW );
	void AddTapAttacks( NoteData &nd, Song* pSong );

	// void Scale( NoteData &nd, float fScale );
	void ScaleRegion( NoteData &nd, float fScale, int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW );
	inline void Scale( NoteData &nd, float fScale ) { NoteDataUtil::ScaleRegion(nd, fScale); }

	// If iRowsToShift > 0, add blank rows.  If iRowsToShift < 0, delete rows
	void ShiftRows( NoteData &nd, int iStartIndex, int iRowsToShift );

	void RemoveAllTapsOfType( NoteData& ndInOut, TapNote::Type typeToRemove );
	void RemoveAllTapsExceptForType( NoteData& ndInOut, TapNote::Type typeToKeep );

	int GetNumUsedTracks( const NoteData& in );
	bool AnyTapsAndHoldsInTrackRange( const NoteData& in, int iTrack, int iStart, int iEnd );
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

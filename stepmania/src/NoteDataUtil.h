#ifndef NOTEDATAUTIL_H
#define NOTEDATAUTIL_H
/*
-----------------------------------------------------------------------------
 File: NoteDataUtil.h

 Desc: Holds data about the notes that the player is supposed to hit.  NoteData
	is organized by:
	track - corresponds to different columns of notes on the screen
	index - corresponds to subdivisions of beats

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GameConstantsAndTypes.h"	// for RadarCategory
#include "NoteTypes.h"
#include "NoteData.h"


/* Utils for NoteData.  Things should go in here if they can be (cleanly and
 * efficiently) implemented using only NoteData's primitives; this improves
 * abstraction and makes it much easier to change NoteData internally in
 * the future. */
namespace NoteDataUtil
{
	NoteType GetSmallestNoteTypeForMeasure( const NoteData &n, int iMeasureIndex );
	void LoadFromSMNoteDataString( NoteData &out, CString sSMNoteData );
	CString GetSMNoteDataString(NoteData &in);

	float GetStreamRadarValue( const NoteData &in, float fSongSeconds );
	float GetVoltageRadarValue( const NoteData &in, float fSongSeconds );
	float GetAirRadarValue( const NoteData &in, float fSongSeconds );
	float GetFreezeRadarValue( const NoteData &in, float fSongSeconds );
	float GetChaosRadarValue( const NoteData &in, float fSongSeconds );

	// radar values - return between 0.0 and 1.2
	float GetRadarValue( const NoteData &in, RadarCategory rv, float fSongSeconds );

	void RemoveHoldNotes( NoteData &in );
	enum TurnType { left, right, mirror, shuffle, super_shuffle, NUM_TURN_TYPES };
	void Turn( NoteData &in, StepsType st, TurnType tt );
	void Little( NoteData &in );
	void Wide( NoteData &in );
	void Big( NoteData &in );
	void Quick( NoteData &in );
	void Skippy( NoteData &in );
	void Mines( NoteData &in );
	void InsertIntelligentTaps( NoteData &in, float fBeatInterval, float fInsertBeatOffset, bool bNewTapSameAsBeginning );
	void SuperShuffleTaps( NoteData &in );

	// change all TAP_ADDITIONs to TAP_TAPs
	void ConvertAdditionsToRegular( NoteData &in );

	void Backwards( NoteData &in );
	void SwapSides( NoteData &in );
	void CopyLeftToRight( NoteData &in );
	void CopyRightToLeft( NoteData &in );
	void ClearLeft( NoteData &in );
	void ClearRight( NoteData &in );
	void CollapseToOne( NoteData &in );
	void ShiftLeft( NoteData &in );
	void ShiftRight( NoteData &in );

	void SnapToNearestNoteType( NoteData &in, NoteType nt1, NoteType nt2, float fBeginBeat, float fEndBeat );

	inline void SnapToNearestNoteType( NoteData &in, NoteType nt, float fBeginBeat, float fEndBeat ) { SnapToNearestNoteType( in, nt, (NoteType)-1, fBeginBeat, fEndBeat ); }

	void FixImpossibleRows( NoteData &in, StepsType st );

	// True if no notes in row that aren't true in the mask
	bool RowPassesValidMask( NoteData &in, int row, const bool bValidMask[] );

	// Remove all tap notes that fail this mask
	void EliminateNonPassingTaps( NoteData &in, int row, const bool bValidMask[] ); 
};

#endif

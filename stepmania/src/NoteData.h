#ifndef NOTEDATA_H
#define NOTEDATA_H
/*
-----------------------------------------------------------------------------
 File: NoteData.h

 Desc: Holds data about the notes that the player is supposed to hit.  NoteData
	is organized by:
	track - corresponds to different columns of notes on the screen
	index - corresponds to subdivisions of beats

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

//#include "GameConstantsAndTypes.h"
#include "NoteTypes.h"

// '1' = tap note
// '2' = hold note begin
// '3' = hold note end  ('1' can also end a HoldNote) ('3' without a matching '2' is ignored
// ... for future expansion

class NoteData
{
	/* Keep this aligned, so that they all have the same size. */
	vector<TapNote> m_TapNotes[MAX_NOTE_TRACKS];
	int m_iNumTracks;

	vector<HoldNote>	m_HoldNotes;

	/* Pad m_TapNotes so it includes the row "rows". */
	void PadTapNotes(int rows);

public:
	/* Set up to hold the data in From; same number of tracks, same
	 * divisor.  Doesn't allocate or copy anything. */
	void Config( const NoteData &From );

	NoteData();
	~NoteData();
	void Init();

	int GetNumTracks();
	void SetNumTracks( int iNewNumTracks );

	/* Return the note at the given track and row.  Row may be out of
	 * range; pretend the song goes on with TAP_EMPTYs indefinitely. */
	inline TapNote GetTapNote(unsigned track, unsigned row) const
	{
		if(row < 0 || row >= m_TapNotes[track].size()) return TapNote(TAP_EMPTY);
		return m_TapNotes[track][row];
	}
	void MoveTapNoteTrack(int dest, int src);
	void SetTapNote(int track, int row, TapNote t);

	void ClearRange( int iNoteIndexBegin, int iNoteIndexEnd );
	void ClearAll();
	void CopyRange( NoteData* pFrom, int iFromIndexBegin, int iFromIndexEnd, int iToIndexBegin = -1 );
	void CopyAll( NoteData* pFrom );
	
	inline bool IsRowEmpty( int index ) const
	{
		for( int t=0; t<m_iNumTracks; t++ )
			if( GetTapNote(t, index) != TAP_EMPTY )
				return false;
		return true;
	}
	inline int GetNumTapNonEmptyTracks( int index ) const
	{
		int iNum = 0;
		for( int t=0; t<m_iNumTracks; t++ )
			if( GetTapNote(t, index) != TAP_EMPTY )
				iNum++;
		return iNum;
	}
	inline int GetFirstNonEmptyTrack( int index ) const
	{
		for( int t=0; t<m_iNumTracks; t++ )
			if( GetTapNote(t, index) != TAP_EMPTY )
				return t;
		return -1;
	}

	// used in edit/record
	void AddHoldNote( HoldNote newNote );	// add note hold note merging overlapping HoldNotes and destroying TapNotes underneath
	void RemoveHoldNote( int index );
	HoldNote &GetHoldNote( int index ) { return m_HoldNotes[index]; }
	const HoldNote &GetHoldNote( int index ) const { return m_HoldNotes[index]; }

	// statistics
	bool IsThereANoteAtRow( int iRow ) const;

	float GetFirstBeat() const;	// return the beat number of the first note
	int GetFirstRow() const;
	float GetLastBeat() const;	// return the beat number of the last note
	int GetLastRow() const;
	int GetNumTapNotes( const float fStartBeat = 0, const float fEndBeat = MAX_BEATS ) const;
	int GetNumDoubles( const float fStartBeat = 0, const float fEndBeat = MAX_BEATS ) const;
	/* optimization: for the default of start to end, use the second (faster) */
	int GetNumHoldNotes( const float fStartBeat, const float fEndBeat = MAX_BEATS ) const;
	int GetNumHoldNotes() const { return m_HoldNotes.size(); }

	int GetPossibleDancePoints();

	// Transformations
	void LoadTransformed( NoteData* pOriginal, int iNewNumTracks, const int iOriginalTrackToTakeFrom[] );	// -1 for iOriginalTracksToTakeFrom means no track
	void LoadTransformedSlidingWindow( NoteData* pOriginal, int iNewNumTracks );	// used by autogen

	// Convert between HoldNote representation and '2' and '3' markers in TapNotes
	void Convert2sAnd3sToHoldNotes();
	void ConvertHoldNotesTo2sAnd3s();

	void Convert4sToHoldNotes();
	void ConvertHoldNotesTo4s();
};

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
	void Turn( NoteData &in, TurnType tt );
	void Little( NoteData &in );
	void Wide( NoteData &in );
	void Big( NoteData &in );
	void Quick( NoteData &in );
	void InsertIntelligentTaps( NoteData &in, float fBeatInterval );
	void SuperShuffleTaps( NoteData &in );
	void Backwards( NoteData &in );
	void SwapSides( NoteData &in );

	void SnapToNearestNoteType( NoteData &in, NoteType nt1, NoteType nt2, float fBeginBeat, float fEndBeat );
	inline void SnapToNearestNoteType( NoteData &in, NoteType nt, float fBeginBeat, float fEndBeat ) { SnapToNearestNoteType( in, nt, (NoteType)-1, fBeginBeat, fEndBeat ); }
};

#endif

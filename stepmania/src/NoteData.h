/* NoteData - Holds data about the notes that the player is supposed to hit. */

#ifndef NOTEDATA_H
#define NOTEDATA_H

#include "NoteTypes.h"
#include <map>
#include <set>
#include "Attack.h"

#define FOREACH_NONEMPTY_ROW_IN_TRACK( nd, track, row ) \
	for( int row = -1; (nd).GetNextTapNoteRowForTrack(track,row); )
#define FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE( nd, track, row, start, last ) \
	for( int row = start-1; (nd).GetNextTapNoteRowForTrack(track,row) && row < (last); )
#define FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE_REVERSE( nd, track, row, start, last ) \
	for( int row = last; (nd).GetPrevTapNoteRowForTrack(track,row) && row >= (start); )
#define FOREACH_NONEMPTY_ROW_ALL_TRACKS( nd, row ) \
	for( int row = -1; (nd).GetNextTapNoteRowForAllTracks(row); )
#define FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( nd, row, start, last ) \
	for( int row = start-1; (nd).GetNextTapNoteRowForAllTracks(row) && row < (last); )

class NoteData
{
public:
	typedef map<int,TapNote> TrackMap;
	typedef map<int,TapNote>::iterator iterator;
	typedef map<int,TapNote>::const_iterator const_iterator;

private:
	// There's no point in inserting empty notes into the map.
	// Any blank space in the map is defined to be empty.
	vector<TrackMap> m_TapNotes;

public:
	NoteData();
	~NoteData();
	void Init();
	
	int GetNumTracks() const { return m_TapNotes.size(); }
	void SetNumTracks( int iNewNumTracks );

	/* Return the note at the given track and row.  Row may be out of
	 * range; pretend the song goes on with TAP_EMPTYs indefinitely. */
	inline const TapNote &GetTapNote(unsigned track, int row) const
	{
		const TrackMap &mapTrack = m_TapNotes[track];
		TrackMap::const_iterator iter = mapTrack.find( row );
		if( iter != mapTrack.end() )
			return iter->second;
		else
			return TAP_EMPTY;
	}

	iterator begin( int iTrack ) { return m_TapNotes[iTrack].begin(); }
	const_iterator begin( int iTrack ) const { return m_TapNotes[iTrack].begin(); }
	iterator end( int iTrack ) { return m_TapNotes[iTrack].end(); }
	const_iterator end( int iTrack ) const { return m_TapNotes[iTrack].end(); }

	inline iterator FindTapNote( unsigned iTrack, int iRow ) { return m_TapNotes[iTrack].find( iRow ); }
	inline const_iterator FindTapNote( unsigned iTrack, int iRow ) const { return m_TapNotes[iTrack].find( iRow ); }
	void RemoveTapNote( unsigned iTrack, iterator it ) { m_TapNotes[iTrack].erase( it ); }

	/* Return an iterator range including exactly iStartRow to iEndRow. */
	void GetTapNoteRange( int iTrack, int iStartRow, int iEndRow, const_iterator &begin, const_iterator &end ) const;
	void GetTapNoteRange( int iTrack, int iStartRow, int iEndRow, TrackMap::iterator &begin, TrackMap::iterator &end );

	/* Return an iterator range include iStartRow to iEndRow.  Extend the range to include
	 * hold notes overlapping the boundary. */
	void GetTapNoteRangeInclusive( int iTrack, int iStartRow, int iEndRow, const_iterator &begin, const_iterator &end, bool bIncludeAdjacent=false ) const;
	void GetTapNoteRangeInclusive( int iTrack, int iStartRow, int iEndRow, iterator &begin, iterator &end, bool bIncludeAdjacent=false );

	/* Return an iterator range include iStartRow to iEndRow.  Shrink the range to exclude
	 * hold notes overlapping the boundary. */
	void GetTapNoteRangeExclusive( int iTrack, int iStartRow, int iEndRow, const_iterator &begin, const_iterator &end ) const;
	void GetTapNoteRangeExclusive( int iTrack, int iStartRow, int iEndRow, iterator &begin, iterator &end );


	// Use this to iterate over notes.
	// Returns the row index of the first TapNote on the track that has a row
	// index > afterRow.
	bool GetNextTapNoteRowForTrack( int track, int &rowInOut ) const;
	bool GetNextTapNoteRowForAllTracks( int &rowInOut ) const;
	bool GetPrevTapNoteRowForTrack( int track, int &rowInOut ) const;
	bool GetPrevTapNoteRowForAllTracks( int &rowInOut ) const;
	
	void MoveTapNoteTrack( int dest, int src );
	void SetTapNote( int track, int row, const TapNote& tn );
	void AddHoldNote( int iTrack, int iStartRow, int iEndRow, TapNote tn ); // add note hold note merging overlapping HoldNotes and destroying TapNotes underneath

	void ClearRangeForTrack( int rowBegin, int rowEnd, int iTrack );
	void ClearRange( int rowBegin, int rowEnd );
	void ClearAll();
	void CopyRange( const NoteData& from, int rowFromBegin, int rowFromEnd, int rowToBegin = 0 );
	void CopyAll( const NoteData& from );

	bool IsRowEmpty( int row ) const;
	bool IsRangeEmpty( int track, int rowBegin, int rowEnd ) const;
	int GetNumTapNonEmptyTracks( int row ) const;
	void GetTapNonEmptyTracks( int row, set<int>& addTo ) const;
	bool GetTapFirstNonEmptyTrack( int row, int &iNonEmptyTrackOut ) const;		// return false if no non-empty tracks at row
	bool GetTapFirstEmptyTrack( int row, int &iEmptyTrackOut ) const;		// return false if no non-empty tracks at row
	bool GetTapLastEmptyTrack( int row, int &iEmptyTrackOut ) const;		// return false if no empty tracks at row
	int GetNumTracksWithTap( int row ) const;
	int GetNumTracksWithTapOrHoldHead( int row ) const;
	int GetFirstTrackWithTap( int row ) const;
	int GetFirstTrackWithTapOrHoldHead( int row ) const;

	inline bool IsThereATapAtRow( int row ) const
	{
		return GetFirstTrackWithTap( row ) != -1;
	}
	inline bool IsThereATapOrHoldHeadAtRow( int row ) const
	{
		return GetFirstTrackWithTapOrHoldHead( row ) != -1;
	}
	void GetTracksHeldAtRow( int row, set<int>& addTo );
	int GetNumTracksHeldAtRow( int row );

	/* Determine if a given spot is within a hold note (being on the tail doesn't count).
	 * Return true if so.  If pHeadRow is non-NULL, return the row of the head.  If pTailRow is
	 * non-NULL, return the row of the tail.  This function is faster if pTailRow is NULL. */
	bool IsHoldNoteAtBeat( int iTrack, int iRow, int *pHeadRow = NULL ) const;

	//
	// statistics
	//
	bool IsEmpty() const;
	int GetFirstRow() const;	// return the beat number of the first note
	float GetFirstBeat() const { return NoteRowToBeat( GetFirstRow() ); }
	int GetLastRow() const;	// return the beat number of the last note
	float GetLastBeat() const { return NoteRowToBeat( GetLastRow() ); }
	int GetNumTapNotes( int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW ) const;
	int GetNumMines( int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW ) const;
	int GetNumRowsWithTap( int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW ) const;
	int GetNumRowsWithTapOrHoldHead( int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW ) const;
	/* optimization: for the default of start to end, use the second (faster) */
	int GetNumHoldNotes( int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW ) const;
	int GetNumRolls( int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW ) const;

	// Count rows that contain iMinTaps or more taps.
	int GetNumRowsWithSimultaneousTaps( int iMinTaps, int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW ) const;
	int GetNumJumps( int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW ) const { return GetNumRowsWithSimultaneousTaps(2,iStartIndex,iEndIndex); }

	// This row needs at least iMinSimultaneousPresses either tapped or held.
	bool RowNeedsAtLeastSimultaneousPresses( int iMinSimultaneousPresses, int row ) const;
	bool RowNeedsHands( int row ) const { return RowNeedsAtLeastSimultaneousPresses(3,row); }

	// Count rows that need iMinSimultaneousPresses either tapped or held.
	int GetNumRowsWithSimultaneousPresses( int iMinSimultaneousPresses, int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW ) const;
	int GetNumHands( int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW ) const { return GetNumRowsWithSimultaneousPresses(3,iStartIndex,iEndIndex); }
	int GetNumQuads( int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW ) const { return GetNumRowsWithSimultaneousPresses(4,iStartIndex,iEndIndex); }

	// Transformations
	void LoadTransformed( const NoteData& original, int iNewNumTracks, const int iOriginalTrackToTakeFrom[] );	// -1 for iOriginalTracksToTakeFrom means no track
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

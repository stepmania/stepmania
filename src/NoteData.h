#ifndef NOTE_DATA_H
#define NOTE_DATA_H

#include "NoteTypes.h"
#include <map>
#include <set>
#include <iterator>

/** @brief Act on each non empty row in the specific track. */
#define FOREACH_NONEMPTY_ROW_IN_TRACK( nd, track, row ) \
	for( int row = -1; (nd).GetNextTapNoteRowForTrack(track,row); )
/** @brief Act on each non empty row in the specified track within the specified range. */
#define FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE( nd, track, row, start, last ) \
	for( int row = start-1; (nd).GetNextTapNoteRowForTrack(track,row) && row < (last); )
/** @brief Act on each non empty row in the specified track within the specified range,
 going in reverse order. */
#define FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE_REVERSE( nd, track, row, start, last ) \
	for( int row = last; (nd).GetPrevTapNoteRowForTrack(track,row) && row >= (start); )
/** @brief Act on each non empty row for all of the tracks. */
#define FOREACH_NONEMPTY_ROW_ALL_TRACKS( nd, row ) \
	for( int row = -1; (nd).GetNextTapNoteRowForAllTracks(row); )
/** @brief Act on each non empty row for all of the tracks within the specified range. */
#define FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( nd, row, start, last ) \
	for( int row = start-1; (nd).GetNextTapNoteRowForAllTracks(row) && row < (last); )

/** @brief Holds data about the notes that the player is supposed to hit. */
class NoteData
{
public:
	typedef map<int,TapNote> TrackMap;
	typedef map<int,TapNote>::iterator iterator;
	typedef map<int,TapNote>::const_iterator const_iterator;
	typedef map<int,TapNote>::reverse_iterator reverse_iterator;
	typedef map<int,TapNote>::const_reverse_iterator const_reverse_iterator;

	NoteData(): m_TapNotes() {}

	iterator begin( int iTrack )					{ return m_TapNotes[iTrack].begin(); }
	const_iterator begin( int iTrack ) const			{ return m_TapNotes[iTrack].begin(); }
	reverse_iterator rbegin( int iTrack )				{ return m_TapNotes[iTrack].rbegin(); }
	const_reverse_iterator rbegin( int iTrack ) const		{ return m_TapNotes[iTrack].rbegin(); }
	iterator end( int iTrack )					{ return m_TapNotes[iTrack].end(); }
	const_iterator end( int iTrack ) const				{ return m_TapNotes[iTrack].end(); }
	reverse_iterator rend( int iTrack )				{ return m_TapNotes[iTrack].rend(); }
	const_reverse_iterator rend( int iTrack ) const			{ return m_TapNotes[iTrack].rend(); }
	iterator lower_bound( int iTrack, int iRow )			{ return m_TapNotes[iTrack].lower_bound( iRow ); }
	const_iterator lower_bound( int iTrack, int iRow ) const	{ return m_TapNotes[iTrack].lower_bound( iRow ); }
	iterator upper_bound( int iTrack, int iRow )			{ return m_TapNotes[iTrack].upper_bound( iRow ); }
	const_iterator upper_bound( int iTrack, int iRow ) const	{ return m_TapNotes[iTrack].upper_bound( iRow ); }
	void swap( NoteData &nd )
	{
		m_TapNotes.swap(nd.m_TapNotes);
		m_atis.swap(nd.m_atis);
		m_const_atis.swap(nd.m_const_atis);
	}


	// This is ugly to make it templated but I don't want to have to write the same class twice.
	template<typename ND, typename iter, typename TN>
	class _all_tracks_iterator
	{
		ND		*m_pNoteData;
		vector<iter>	m_vBeginIters;

		/* There isn't a "past the beginning" iterator so this is hard to make a true bidirectional iterator.
		* Use the "past the end" iterator in place of the "past the beginning" iterator when in reverse. */
		vector<iter>	m_vCurrentIters;

		vector<iter>	m_vEndIters;
		int		m_iTrack;
		bool		m_bReverse;

		// These exist so that the iterator can be revalidated if the NoteData is
		// transformed during this iterator's lifetime.
		vector<int> m_PrevCurrentRows;
		bool m_Inclusive;
		int m_StartRow;
		int m_EndRow;

		void Find( bool bReverse );
	public:
		_all_tracks_iterator( ND &nd, int iStartRow, int iEndRow, bool bReverse, bool bInclusive );
		_all_tracks_iterator( const _all_tracks_iterator &other );
		~_all_tracks_iterator();
		_all_tracks_iterator &operator++();		// preincrement
		_all_tracks_iterator operator++( int dummy );	// postincrement
		//_all_tracks_iterator &operator--();		// predecrement
		//_all_tracks_iterator operator--( int dummy );	// postdecrement
		inline int Track() const		{ return m_iTrack; }
		inline int Row() const			{ return m_vCurrentIters[m_iTrack]->first; }
		inline bool IsAtEnd() const		{ return m_iTrack == -1; }
		inline iter GetIter( int iTrack ) const	{ return m_vCurrentIters[iTrack]; }
		inline TN &operator*()			{ DEBUG_ASSERT( !IsAtEnd() ); return m_vCurrentIters[m_iTrack]->second; }
		inline TN *operator->()			{ DEBUG_ASSERT( !IsAtEnd() ); return &m_vCurrentIters[m_iTrack]->second; }
		inline const TN &operator*() const	{ DEBUG_ASSERT( !IsAtEnd() ); return m_vCurrentIters[m_iTrack]->second; }
		inline const TN *operator->() const	{ DEBUG_ASSERT( !IsAtEnd() ); return &m_vCurrentIters[m_iTrack]->second; }
		// Use when transforming the NoteData.
		void Revalidate(ND* notedata, vector<int> const& added_or_removed_tracks, bool added);
	};
	typedef _all_tracks_iterator<NoteData, NoteData::iterator, TapNote> 			all_tracks_iterator;
	typedef _all_tracks_iterator<const NoteData, NoteData::const_iterator, const TapNote>	all_tracks_const_iterator;
	typedef all_tracks_iterator								all_tracks_reverse_iterator;
	typedef all_tracks_const_iterator							all_tracks_const_reverse_iterator;
	friend class _all_tracks_iterator<NoteData, NoteData::iterator, TapNote>;
	friend class _all_tracks_iterator<const NoteData, NoteData::const_iterator, const TapNote>;
private:
	// There's no point in inserting empty notes into the map.
	// Any blank space in the map is defined to be empty.
	vector<TrackMap>	m_TapNotes;

	/**
	 * @brief Determine whether this note is for Player 1 or Player 2.
	 * @param track the track/column the note is in.
	 * @param tn the note in question. Required for routine mode.
	 * @return true if it's for player 1, false for player 2. */
	bool IsPlayer1(const int track, const TapNote &tn) const;

	/**
	 * @brief Determine if the note in question should be counted as a tap.
	 * @param tn the note in question.
	 * @param row the row it lives in.
	 * @return true if it's a tap, false otherwise. */
	bool IsTap(const TapNote &tn, const int row) const;

	/**
	 * @brief Determine if the note in question should be counted as a mine.
	 * @param tn the note in question.
	 * @param row the row it lives in.
	 * @return true if it's a mine, false otherwise. */
	bool IsMine(const TapNote &tn, const int row) const;

	/**
	 * @brief Determine if the note in question should be counted as a lift.
	 * @param tn the note in question.
	 * @param row the row it lives in.
	 * @return true if it's a lift, false otherwise. */
	bool IsLift(const TapNote &tn, const int row) const;

	/**
	 * @brief Determine if the note in question should be counted as a fake.
	 * @param tn the note in question.
	 * @param row the row it lives in.
	 * @return true if it's a fake, false otherwise. */
	bool IsFake(const TapNote &tn, const int row) const;

	pair<int, int> GetNumRowsWithSimultaneousTapsTwoPlayer(int minTaps = 2,
														   int startRow = 0,
														   int endRow = MAX_NOTE_ROW) const;

	// These exist so that they can be revalidated when something that transforms
	// the NoteData occurs. -Kyz
	mutable set<all_tracks_iterator*> m_atis;
	mutable set<all_tracks_const_iterator*> m_const_atis;

	void AddATIToList(all_tracks_iterator* iter) const;
	void AddATIToList(all_tracks_const_iterator* iter) const;
	void RemoveATIFromList(all_tracks_iterator* iter) const;
	void RemoveATIFromList(all_tracks_const_iterator* iter) const;

	// Mina stuf (Used for chartkey hashing)
	std::vector<int> NonEmptyRowVector;

public:
	void Init();

	// Mina stuf (Used for chartkey hashing)
	void LogNonEmptyRows();
	std::vector<int>& GetNonEmptyRowVector() { return NonEmptyRowVector; };

	int GetNumTracks() const { return m_TapNotes.size(); }
	void SetNumTracks( int iNewNumTracks );
	bool IsComposite() const;
	bool operator==( const NoteData &nd ) const			{ return m_TapNotes == nd.m_TapNotes; }
	bool operator!=( const NoteData &nd ) const			{ return m_TapNotes != nd.m_TapNotes; }

	/* Return the note at the given track and row.  Row may be out of
	 * range; pretend the song goes on with TAP_EMPTYs indefinitely. */
	inline const TapNote &GetTapNote( unsigned track, int row ) const
	{
		const TrackMap &mapTrack = m_TapNotes[track];
		TrackMap::const_iterator iter = mapTrack.find( row );
		if( iter != mapTrack.end() )
			return iter->second;
		else
			return TAP_EMPTY;
	}


	inline iterator FindTapNote( unsigned iTrack, int iRow )	{ return m_TapNotes[iTrack].find( iRow ); }
	inline const_iterator FindTapNote( unsigned iTrack, int iRow ) const { return m_TapNotes[iTrack].find( iRow ); }
	void RemoveTapNote( unsigned iTrack, iterator it )		{ m_TapNotes[iTrack].erase( it ); }

	/**
	 * @brief Return an iterator range for [rowBegin,rowEnd).
	 *
	 * This can be used to efficiently iterate trackwise over a range of notes.
	 * It's like FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE, except it only requires
	 * two map searches (iterating is constant time), but the iterators will
	 * become invalid if the notes they represent disappear, so you need to
	 * pay attention to how you modify the data.
	 * @param iTrack the column to use.
	 * @param iStartRow the starting point.
	 * @param iEndRow the ending point.
	 * @param begin the eventual beginning point of the range.
	 * @param end the eventual end point of the range. */
	void GetTapNoteRange(int iTrack, int iStartRow, int iEndRow,
						 TrackMap::const_iterator &begin, TrackMap::const_iterator &end ) const;
	/**
	 * @brief Return a constant iterator range for [rowBegin,rowEnd).
	 * @param iTrack the column to use.
	 * @param iStartRow the starting point.
	 * @param iEndRow the ending point.
	 * @param begin the eventual beginning point of the range.
	 * @param end the eventual end point of the range. */
	void GetTapNoteRange( int iTrack, int iStartRow, int iEndRow, TrackMap::iterator &begin, TrackMap::iterator &end );
	all_tracks_iterator GetTapNoteRangeAllTracks( int iStartRow, int iEndRow, bool bInclusive = false )
	{
		return all_tracks_iterator( *this, iStartRow, iEndRow, false, bInclusive );
	}
	all_tracks_const_iterator GetTapNoteRangeAllTracks( int iStartRow, int iEndRow, bool bInclusive = false ) const
	{
		return all_tracks_const_iterator( *this, iStartRow, iEndRow, false, bInclusive );
	}
	all_tracks_reverse_iterator GetTapNoteRangeAllTracksReverse( int iStartRow, int iEndRow, bool bInclusive = false )
	{
		return all_tracks_iterator(*this, iStartRow, iEndRow, true, bInclusive );
	}
	all_tracks_const_reverse_iterator GetTapNoteRangeAllTracksReverse( int iStartRow, int iEndRow, bool bInclusive = false ) const
	{
		return all_tracks_const_iterator(*this, iStartRow, iEndRow, true, bInclusive );
	}

	// Call this after using any transform that changes the NoteData.
	void RevalidateATIs(vector<int> const& added_or_removed_tracks, bool added);
	void TransferATIs(NoteData& to);

	/* Return an iterator range include iStartRow to iEndRow.  Extend the range to include
	 * hold notes overlapping the boundary. */
	void GetTapNoteRangeInclusive(int iTrack, int iStartRow, int iEndRow,
								  TrackMap::const_iterator &begin, TrackMap::const_iterator &end, bool bIncludeAdjacent=false ) const;
	void GetTapNoteRangeInclusive(int iTrack, int iStartRow, int iEndRow,
								  TrackMap::iterator &begin, TrackMap::iterator &end, bool bIncludeAdjacent=false );

	/* Return an iterator range include iStartRow to iEndRow.  Shrink the range to exclude
	 * hold notes overlapping the boundary. */
	void GetTapNoteRangeExclusive(int iTrack, int iStartRow, int iEndRow,
								  TrackMap::const_iterator &begin, TrackMap::const_iterator &end ) const;
	void GetTapNoteRangeExclusive(int iTrack, int iStartRow, int iEndRow,
								  TrackMap::iterator &begin, TrackMap::iterator &end );


	/* Returns the row of the first TapNote on the track that has a row greater than rowInOut. */
	bool GetNextTapNoteRowForTrack( int track, int &rowInOut, bool ignoreKeySounds=false ) const;
	bool GetNextTapNoteRowForAllTracks( int &rowInOut ) const;
	bool GetPrevTapNoteRowForTrack( int track, int &rowInOut ) const;
	bool GetPrevTapNoteRowForAllTracks( int &rowInOut ) const;

	void MoveTapNoteTrack( int dest, int src );
	void SetTapNote( int track, int row, const TapNote& tn );
	/**
	 * @brief Add a hold note, merging other overlapping holds and destroying
	 * tap notes underneath.
	 * @param iTrack the column to work with.
	 * @param iStartRow the starting row.
	 * @param iEndRow the ending row.
	 * @param tn the tap note. */
	void AddHoldNote(int iTrack,
					 int iStartRow,
					 int iEndRow,
					 TapNote tn );

	void ClearRangeForTrack( int rowBegin, int rowEnd, int iTrack );
	void ClearRange( int rowBegin, int rowEnd );
	void ClearAll();
	void CopyRange( const NoteData& from, int rowFromBegin, int rowFromEnd, int rowToBegin = 0 );
	void CopyAll( const NoteData& from );

	bool IsRowEmpty( int row ) const;
	bool IsRangeEmpty( int track, int rowBegin, int rowEnd ) const;
	int GetNumTapNonEmptyTracks( int row ) const;
	void GetTapNonEmptyTracks( int row, set<int>& addTo ) const;
	bool GetTapFirstNonEmptyTrack( int row, int &iNonEmptyTrackOut ) const;	// return false if no non-empty tracks at row
	bool GetTapFirstEmptyTrack( int row, int &iEmptyTrackOut ) const;	// return false if no non-empty tracks at row
	bool GetTapLastEmptyTrack( int row, int &iEmptyTrackOut ) const;	// return false if no empty tracks at row
	int GetNumTracksWithTap( int row ) const;
	int GetNumTracksWithTapOrHoldHead( int row ) const;
	int GetFirstTrackWithTap( int row ) const;
	int GetFirstTrackWithTapOrHoldHead( int row ) const;
	int GetLastTrackWithTapOrHoldHead( int row ) const;

	inline bool IsThereATapAtRow( int row ) const			{ return GetFirstTrackWithTap( row ) != -1; }
	inline bool IsThereATapOrHoldHeadAtRow( int row ) const		{ return GetFirstTrackWithTapOrHoldHead( row ) != -1; }
	void GetTracksHeldAtRow( int row, set<int>& addTo );
	int GetNumTracksHeldAtRow( int row );

	bool IsHoldNoteAtRow( int iTrack, int iRow, int *pHeadRow = nullptr ) const;
	bool IsHoldHeadOrBodyAtRow( int iTrack, int iRow, int *pHeadRow ) const;

	// statistics
	bool IsEmpty() const;
	bool IsTrackEmpty( int iTrack ) const { return m_TapNotes[iTrack].empty(); }
	int GetFirstRow() const; // return the beat number of the first note
	int GetLastRow() const;	 // return the beat number of the last note
	float GetFirstBeat() const					{ return NoteRowToBeat( GetFirstRow() ); }
	float GetLastBeat() const					{ return NoteRowToBeat( GetLastRow() ); }
	int GetNumTapNotes( int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW ) const;
	int GetNumTapNotesNoTiming( int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW ) const;
	int GetNumTapNotesInRow( int iRow ) const;
	int GetNumMines( int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW ) const;
	int GetNumRowsWithTap( int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW ) const;
	int GetNumRowsWithTapOrHoldHead( int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW ) const;
	/* Optimization: for the default of start to end, use the second (faster). XXX: Second what? -- Steve */
	int GetNumHoldNotes( int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW ) const;
	int GetNumRolls( int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW ) const;

	// Count rows that contain iMinTaps or more taps.
	int GetNumRowsWithSimultaneousTaps( int iMinTaps, int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW ) const;
	int GetNumJumps( int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW ) const
	{
		return GetNumRowsWithSimultaneousTaps( 2, iStartIndex, iEndIndex );
	}



	// This row needs at least iMinSimultaneousPresses either tapped or held.
	bool RowNeedsAtLeastSimultaneousPresses( int iMinSimultaneousPresses, int row ) const;
	bool RowNeedsHands( int row ) const { return RowNeedsAtLeastSimultaneousPresses(3,row); }

	// Count rows that need iMinSimultaneousPresses either tapped or held.
	int GetNumRowsWithSimultaneousPresses( int iMinSimultaneousPresses, int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW ) const;
	int GetNumHands( int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW ) const
	{
		return GetNumRowsWithSimultaneousPresses( 3, iStartIndex, iEndIndex );
	}
	int GetNumQuads( int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW ) const
	{
		return GetNumRowsWithSimultaneousPresses( 4, iStartIndex, iEndIndex );
	}

	// and the other notetypes
	int GetNumLifts( int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW ) const;
	int GetNumFakes( int iStartIndex = 0, int iEndIndex = MAX_NOTE_ROW ) const;

	// the couple/routine style variants of the above.
	pair<int, int> GetNumTapNotesTwoPlayer(int startRow = 0,
										   int endRow = MAX_NOTE_ROW) const;

	pair<int, int> GetNumJumpsTwoPlayer(int startRow = 0,
										int endRow = MAX_NOTE_ROW) const;

	pair<int, int> GetNumHandsTwoPlayer(int startRow = 0,
										int endRow = MAX_NOTE_ROW) const;

	pair<int, int> GetNumQuadsTwoPlayer(int startRow = 0,
										int endRow = MAX_NOTE_ROW) const;

	pair<int, int> GetNumHoldNotesTwoPlayer(int startRow = 0,
											int endRow = MAX_NOTE_ROW) const;

	pair<int, int> GetNumMinesTwoPlayer(int startRow = 0,
										int endRow = MAX_NOTE_ROW) const;

	pair<int, int> GetNumRollsTwoPlayer(int startRow = 0,
										int endRow = MAX_NOTE_ROW) const;

	pair<int, int> GetNumLiftsTwoPlayer(int startRow = 0,
										int endRow = MAX_NOTE_ROW) const;

	pair<int, int> GetNumFakesTwoPlayer(int startRow = 0,
										int endRow = MAX_NOTE_ROW) const;

	// Transformations
	void LoadTransformed(const NoteData& original,
						 int iNewNumTracks,
						 const int iOriginalTrackToTakeFrom[] );	// -1 for iOriginalTracksToTakeFrom means no track

	// XML
	XNode* CreateNode() const;
	void LoadFromNode( const XNode* pNode );
};

/** @brief Allow a quick way to swap notedata. */
namespace std
{
	template<> inline void swap<NoteData>( NoteData &nd1, NoteData &nd2 )
#if !defined(_MSC_VER)
	noexcept(is_nothrow_move_constructible<NoteData>::value && is_nothrow_move_assignable<NoteData>::value)
#endif
	{
		nd1.swap( nd2 );
	}
}

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

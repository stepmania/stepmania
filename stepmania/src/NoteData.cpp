/*
 * NoteData is organized by:
 *  track - corresponds to different columns of notes on the screen
 *  row/index - corresponds to subdivisions of beats
 */

#include "global.h"
#include "NoteData.h"
#include "RageUtil.h"
#include "RageLog.h"


NoteData::NoteData()
{
	Init();
}

void NoteData::Init()
{
	ClearAll();
	m_TapNotes.clear();
}

NoteData::~NoteData()
{
}

void NoteData::SetNumTracks( int iNewNumTracks )
{
	ASSERT( iNewNumTracks > 0 );

	m_TapNotes.resize( iNewNumTracks );

	/* Remove all hold notes that are out of bounds. */
	// Iterate backwards so that we can delete.
	for( int h = m_HoldNotes.size()-1; h >= 0; --h )
		if( m_HoldNotes[h].iTrack >= iNewNumTracks )
			m_HoldNotes.erase( m_HoldNotes.begin()+h );
}


/* Clear [rowBegin,rowEnd]; that is, including rowEnd. */
void NoteData::ClearRangeForTrack( int rowBegin, int rowEnd, int iTrack )
{
	/* Optimization: if the range encloses everything, just clear the whole maps. */
	if( rowBegin == 0 && rowEnd == MAX_NOTE_ROW )
	{
		m_TapNotes[iTrack].clear();

		for( int i = GetNumHoldNotes()-1; i >= 0; --i )
		{
			HoldNote hn = GetHoldNote(i);
			if( hn.iTrack != iTrack )
				continue;
			this->RemoveHoldNote( i );
		}
	}

	/* Crop or split hold notes overlapping the range. */
	for( int i = GetNumHoldNotes()-1; i >= 0; --i )	// for each HoldNote
	{
		HoldNote hn = GetHoldNote(i);
		if( hn.iTrack != iTrack )
			continue;

		if( !hn.RangeOverlaps(rowBegin, rowEnd) )
			continue;

		this->RemoveHoldNote( i );

		/* If the range encloses the hold note completely, just delete it. */
		if( hn.ContainedByRange(rowBegin, rowEnd) )
			continue;

		if( hn.RangeInside(rowBegin, rowEnd) )
		{
			/* The hold note encloses the range, so we need to split the hold note. */
			HoldNote hnLater(hn);
			hn.iEndRow = rowBegin;
			hnLater.iStartRow = rowEnd;
			this->AddHoldNote( hn );
			this->AddHoldNote( hnLater );
			continue;
		}

		if( hn.iStartRow < rowBegin )
			hn.iEndRow = min( hn.iEndRow, rowBegin );
		else
			hn.iStartRow = max( hn.iStartRow, rowEnd );
		this->AddHoldNote( hn );
	}

	/* Clear other notes in the region. */
	FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE( *this, iTrack, r, rowBegin, rowEnd )
		SetTapNote( iTrack, r, TAP_EMPTY );
}

void NoteData::ClearRange( int rowBegin, int rowEnd )
{
	for( int t=0; t < GetNumTracks(); ++t )
		ClearRangeForTrack( rowBegin, rowEnd, t );
}

void NoteData::ClearAll()
{
	for( int t=0; t<GetNumTracks(); t++ )
		m_TapNotes[t].clear();
	m_HoldNotes.clear();
}

/* Copy a range from pFrom to this.  (Note that this does *not* overlay;
 * all data in the range is overwritten.) */
void NoteData::CopyRange( const NoteData& from, int rowFromBegin, int rowFromEnd, int rowToBegin )
{
	ASSERT( from.GetNumTracks() == GetNumTracks() );

	int rowToEnd = (rowFromEnd-rowFromBegin) + rowToBegin;

	/* Clear the region. */
	ClearRange( rowToBegin, rowToEnd );

	/* Copy everything except for hold notes. */
	for( int t=0; t<GetNumTracks(); t++ )
	{
		FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE( from, t, iFrom, rowFromBegin, rowFromEnd )
		{
			const TapNote &tn = from.GetTapNote( t, iFrom );
			switch( tn.type )
			{
			case TapNote::empty:
			case TapNote::hold_tail:
			case TapNote::hold_head:
				continue;
			}

			int iTo = rowToBegin + iFrom - rowFromBegin;
			this->SetTapNote( t, iTo, tn );
		}
	}

	/* Copy hold notes. */
	for( int i=0; i<from.GetNumHoldNotes(); i++ )	// for each HoldNote
	{
		HoldNote hn = from.GetHoldNote(i);

		if( !hn.RangeOverlaps(rowFromBegin, rowFromEnd) )
			continue;

		/* Move the hold note. */
		int iMoveBy = rowToBegin-rowFromBegin;
		hn.iStartRow += iMoveBy;
		hn.iEndRow += iMoveBy;

		/* Crop the hold note to the region. */
		hn.iStartRow = max( hn.iStartRow, rowToBegin );
		hn.iEndRow = min( hn.iEndRow, rowToEnd );

		/* The beginning of the hold might match up to the end of an existing hold, the end
		 * may match with the beginning, or both. */
		int iEarlierHoldNote = -1, iLaterHoldNote = -1;
		for( int j=0; j<this->GetNumHoldNotes(); ++j )	// for each HoldNote
		{
			const HoldNote &hn2 = this->GetHoldNote(j);
			if( hn2.iEndRow == hn.iStartRow )
				iEarlierHoldNote = j;
			if( hn2.iStartRow == hn.iEndRow )
				iLaterHoldNote = j;
		}

		if( iEarlierHoldNote != -1 && iLaterHoldNote != -1 )
		{
			HoldNote &hnEarlier = this->GetHoldNote( iEarlierHoldNote );
			const HoldNote &hnLater = this->GetHoldNote( iLaterHoldNote );
			hnEarlier.iEndRow = hnLater.iEndRow;

			this->RemoveHoldNote( iLaterHoldNote );
		}
		else if( iEarlierHoldNote == -1 && iLaterHoldNote == -1 )
			AddHoldNote( hn );
		else if( iEarlierHoldNote != -1 )
		{
			HoldNote &hn2 = this->GetHoldNote(iEarlierHoldNote);
			hn2.iEndRow = hn.iEndRow;
		}
		else if( iLaterHoldNote != -1 )
		{
			HoldNote &hn2 = this->GetHoldNote(iLaterHoldNote);
			hn2.iStartRow = hn.iStartRow;
		}
		else
			FAIL_M(ssprintf("%i,%i", iEarlierHoldNote, iLaterHoldNote));
	}
}

void NoteData::Config( const NoteData& from )
{
	SetNumTracks( from.GetNumTracks() );
}

void NoteData::CopyAll( const NoteData& from )
{
	Config(from);
	ClearAll();

	for( int c=0; c<GetNumTracks(); c++ )
		m_TapNotes[c] = from.m_TapNotes[c];
	m_HoldNotes = from.m_HoldNotes;
}

bool NoteData::IsRowEmpty( int row ) const
{
	for( int t=0; t<GetNumTracks(); t++ )
		if( GetTapNote(t, row).type != TapNote::empty )
			return false;
	return true;
}

bool NoteData::IsRangeEmpty( int track, int rowBegin, int rowEnd ) const
{
	ASSERT( track < GetNumTracks() );

	FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE( *this, track, r, rowBegin, rowEnd )
		if( GetTapNote(track,r).type != TapNote::empty )
			return false;
	return true;
}

int NoteData::GetNumTapNonEmptyTracks( int row ) const
{
	int iNum = 0;
	for( int t=0; t<GetNumTracks(); t++ )
		if( GetTapNote(t, row).type != TapNote::empty )
			iNum++;
	return iNum;
}

void NoteData::GetTapNonEmptyTracks( int row, set<int>& addTo ) const
{
	for( int t=0; t<GetNumTracks(); t++ )
		if( GetTapNote(t, row).type != TapNote::empty )
			addTo.insert(t);
}

bool NoteData::GetTapFirstNonEmptyTrack( int row, int &iNonEmptyTrackOut ) const
{
	for( int t=0; t<GetNumTracks(); t++ )
	{
		if( GetTapNote( t, row ).type != TapNote::empty )
		{
			iNonEmptyTrackOut = t;
			return true;
		}
	}
	return false;
}

bool NoteData::GetTapFirstEmptyTrack( int row, int &iEmptyTrackOut ) const
{
	for( int t=0; t<GetNumTracks(); t++ )
	{
		if( GetTapNote( t, row ).type == TapNote::empty )
		{
			iEmptyTrackOut = t;
			return true;
		}
	}
	return false;
}

bool NoteData::GetTapLastEmptyTrack( int row, int &iEmptyTrackOut ) const
{
	for( int t=GetNumTracks()-1; t>=0; t-- )
	{
		if( GetTapNote( t, row ).type == TapNote::empty )
		{
			iEmptyTrackOut = t;
			return true;
		}
	}
	return false;
}

int NoteData::GetNumTracksWithTap( int row ) const
{
	int iNum = 0;
	for( int t=0; t<GetNumTracks(); t++ )
	{
		const TapNote &tn = GetTapNote( t, row );
		if( tn.type == TapNote::tap )
			iNum++;
	}
	return iNum;
}

int NoteData::GetNumTracksWithTapOrHoldHead( int row ) const
{
	int iNum = 0;
	for( int t=0; t<GetNumTracks(); t++ )
	{
		const TapNote &tn = GetTapNote( t, row );
		if( tn.type == TapNote::tap || tn.type == TapNote::hold_head )
			iNum++;
	}
	return iNum;
}

int NoteData::GetFirstTrackWithTap( int row ) const
{
	for( int t=0; t<GetNumTracks(); t++ )
	{
		const TapNote &tn = GetTapNote( t, row );
		if( tn.type == TapNote::tap )
			return t;
	}
	return -1;
}

int NoteData::GetFirstTrackWithTapOrHoldHead( int row ) const
{
	for( int t=0; t<GetNumTracks(); t++ )
	{
		const TapNote &tn = GetTapNote( t, row );
		if( tn.type == TapNote::tap || tn.type == TapNote::hold_head )
			return t;
	}
	return -1;
}

void NoteData::AddHoldNote( HoldNote add )
{
	ASSERT( add.iStartRow>=0 && add.iEndRow>=0 );

	// look for other hold notes that overlap and merge them
	for( int i=0; i<GetNumHoldNotes(); i++ )	// for each HoldNote
	{
		HoldNote &other = GetHoldNote(i);
		if( add.iTrack == other.iTrack  &&		// the tracks correspond
			add.RangeOverlaps(other) ) // they overlap
		{
			add.iStartRow = min(add.iStartRow, other.iStartRow);
			add.iEndRow = max(add.iEndRow, other.iEndRow);

			// delete this HoldNote
			RemoveHoldNote( i );
			--i;
		}
	}

	int iAddStartIndex = add.iStartRow;
	int iAddEndIndex = add.iEndRow;

	// delete TapNotes under this HoldNote
	FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE( *this, add.iTrack, i, iAddStartIndex+1, iAddEndIndex )
		SetTapNote( add.iTrack, i, TAP_EMPTY );

	// add a tap note at the start of this hold
	SetTapNote( add.iTrack, iAddStartIndex, TAP_ORIGINAL_HOLD_HEAD );		// Hold begin marker.  Don't draw this, but do grade it.

	m_HoldNotes.push_back(add);
}

void NoteData::RemoveHoldNote( int iHoldIndex )
{
	ASSERT( iHoldIndex >= 0  &&  iHoldIndex < GetNumHoldNotes() );

	HoldNote& hn = GetHoldNote(iHoldIndex);

	const int iHoldStartIndex = hn.iStartRow;

	// delete a tap note at the start of this hold
	SetTapNote(hn.iTrack, iHoldStartIndex, TAP_EMPTY);

	// remove from list
	m_HoldNotes.erase(m_HoldNotes.begin()+iHoldIndex, m_HoldNotes.begin()+iHoldIndex+1);
}

/* Return true if a hold note lies on the given spot.  Must be in 2sAnd3s. */
bool NoteData::IsHoldNoteAtBeat( int iTrack, int iRow, int *pHeadRow, int *pTailRow ) const
{
	/* If neither the actual head nor tail row were requested, search for the head to
	 * determine the return value. */
	int iDummy;
	if( pHeadRow == NULL )
		pHeadRow = &iDummy;

	bool bFoundHead = false;
	if( pHeadRow != NULL )
	{
		/* Starting at iRow, search upwards.  If we find a TapNote::hold_head, we're within
		 * a hold.  If we find a tap, mine or attack, we're not--those never lie within hold
		 * notes.  Ignore autoKeysound. */
		FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE_REVERSE( *this, iTrack, r, 0, iRow )
		{
			const TapNote &tn = GetTapNote( iTrack, r );
			switch( tn.type )
			{
			case TapNote::hold_head:
				*pHeadRow = r;
				bFoundHead = true;
				break;

			case TapNote::tap:
			case TapNote::mine:
			case TapNote::attack:
			case TapNote::hold_tail:
				return false;
			case TapNote::empty:
			case TapNote::autoKeysound:
				/* ignore */
				continue;
			default:
				FAIL_M( ssprintf("%i", tn.type) );
			}

			if( bFoundHead )
				break;
		}

		/* If we didn't find a matching head, we're not within a hold note, so don't bother
		 * searching for a tail. */
		if( !bFoundHead )
			return false;
	}

	bool bFoundTail = false;
	if( pTailRow != NULL )
	{
		FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE( *this, iTrack, r, iRow, MAX_NOTE_ROW )
		{
			const TapNote &tn = GetTapNote( iTrack, r );

			switch( tn.type )
			{
			case TapNote::hold_tail:
				*pTailRow = r;
				bFoundTail = true;
				break;
			case TapNote::tap:
			case TapNote::mine:
			case TapNote::attack:
				return false;
			case TapNote::hold_head:
				/* If iRow is a hold head, we're within a hold note, and need to continue searching;
				 * if any row after that is a head, we're not in it, so stop.  This is different
				 * than above, since holds are [head,tail); the row of the tail isn't actually
				 * part of the hold. */
				if( r == iRow )
					continue;

				return false;

			case TapNote::empty:
			case TapNote::autoKeysound:
				/* ignore */
				continue;
			default:
				FAIL_M( ssprintf("%i", tn.type) );
			}
			
			if( bFoundTail )
				break;
			return true;
		}
	}
			
	return bFoundHead || bFoundTail;
}

int NoteData::GetFirstRow() const
{ 
	int iEarliestRowFoundSoFar = -1;
	
	for( int t=0; t < GetNumTracks(); t++ )
	{
		int iRow = -1;
		if( !GetNextTapNoteRowForTrack( t, iRow ) )
			continue;

		if( iEarliestRowFoundSoFar == -1 )
			iEarliestRowFoundSoFar = iRow;
		else
			iEarliestRowFoundSoFar = min( iEarliestRowFoundSoFar, iRow );
	}

	for( int i=0; i<GetNumHoldNotes(); i++ )
	{
		if( iEarliestRowFoundSoFar == -1 ||
			GetHoldNote(i).iStartRow < iEarliestRowFoundSoFar )
			iEarliestRowFoundSoFar = GetHoldNote(i).iStartRow;
	}

	if( iEarliestRowFoundSoFar == -1 )	// there are no notes
		return 0;

	return iEarliestRowFoundSoFar;
}

int NoteData::GetLastRow() const
{ 
	int iOldestRowFoundSoFar = 0;
	
	for( int t=0; t < GetNumTracks(); t++ )
	{
		int iRow = MAX_NOTE_ROW;
		if( !GetPrevTapNoteRowForTrack( t, iRow ) )
			continue;
		iOldestRowFoundSoFar = max( iOldestRowFoundSoFar, iRow );
	}

	for( int i=0; i<GetNumHoldNotes(); i++ )
	{
		if( GetHoldNote(i).iEndRow > iOldestRowFoundSoFar )
			iOldestRowFoundSoFar = GetHoldNote(i).iEndRow;
	}

	return iOldestRowFoundSoFar;
}

int NoteData::GetNumTapNotes( int iStartIndex, int iEndIndex ) const
{
	int iNumNotes = 0;
	for( int t=0; t<GetNumTracks(); t++ )
	{
		FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE( *this, t, r, iStartIndex, iEndIndex )
		{
			const TapNote &tn = GetTapNote(t, r);
			if( tn.type != TapNote::empty  &&  tn.type != TapNote::mine )
				iNumNotes++;
		}
	}
	
	return iNumNotes;
}

int NoteData::GetNumRowsWithTap( int iStartIndex, int iEndIndex ) const
{
	int iNumNotes = 0;
	FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( *this, r, iStartIndex, iEndIndex )
		if( IsThereATapAtRow(r) )
			iNumNotes++;
	
	return iNumNotes;
}

int NoteData::GetNumMines( int iStartIndex, int iEndIndex ) const
{
	int iNumMines = 0;

	for( int t=0; t<GetNumTracks(); t++ )
	{
		FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE( *this, t, r, iStartIndex, iEndIndex )
			if( GetTapNote(t, r).type == TapNote::mine )
				iNumMines++;
	}
	
	return iNumMines;
}

int NoteData::GetNumRowsWithTapOrHoldHead( int iStartIndex, int iEndIndex ) const
{
	int iNumNotes = 0;
	FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( *this, r, iStartIndex, iEndIndex )
		if( IsThereATapOrHoldHeadAtRow(r) )
			iNumNotes++;
	
	return iNumNotes;
}

int NoteData::RowNeedsHands( const int row ) const
{
	int iNumNotesThisIndex = 0;
	for( int t=0; t<GetNumTracks(); t++ )
	{
		const TapNote &tn = GetTapNote(t, row);
		switch( tn.type )
		{
		case TapNote::mine:
		case TapNote::empty:
		case TapNote::hold_tail:
			continue;	// skip these types - they don't count
		}
		++iNumNotesThisIndex;
	}

	/* We must have at least one non-hold-body at this row to count it. */
	if( !iNumNotesThisIndex )
		return false;

	if( iNumNotesThisIndex < 3 )
	{
		/* We have at least one, but not enough.  Count holds. */
		for( int j=0; j<GetNumHoldNotes(); j++ )
		{
			const HoldNote &hn = GetHoldNote(j);
			if( hn.iStartRow+1 <= row && row <= hn.iEndRow )
				++iNumNotesThisIndex;
		}
	}

	return iNumNotesThisIndex >= 3;
}

int NoteData::GetNumHands( int iStartIndex, int iEndIndex ) const
{
	/* Count the number of times you have to use your hands.  This includes
	 * three taps at the same time, a tap while two hold notes are being held,
	 * etc.  Only count rows that have at least one tap note (hold heads count).
	 * Otherwise, every row of hold notes counts, so three simultaneous hold
	 * notes will count as hundreds of "hands". */
	int iNum = 0;
	FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( *this, r, iStartIndex, iEndIndex )
	{
		if( !RowNeedsHands(r) )
			continue;

		iNum++;
	}

	return iNum;
}

int NoteData::GetNumN( int iMinTaps, int iStartIndex, int iEndIndex ) const
{
	int iNum = 0;
	FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( *this, r, iStartIndex, iEndIndex )
	{
		int iNumNotesThisIndex = 0;
		for( int t=0; t<GetNumTracks(); t++ )
		{
			const TapNote &tn = GetTapNote(t, r);
			if( tn.type != TapNote::mine  &&  tn.type != TapNote::empty )	// mines don't count
				iNumNotesThisIndex++;
		}
		if( iNumNotesThisIndex >= iMinTaps )
			iNum++;
	}
	
	return iNum;
}

int NoteData::GetNumHoldNotes( int iStartIndex, int iEndIndex ) const
{
	int iNumHolds = 0;
	for( int i=0; i<GetNumHoldNotes(); i++ )
	{
		const HoldNote &hn = GetHoldNote(i);
		if( iStartIndex <= hn.iStartRow &&  hn.iEndRow <= iEndIndex )
			iNumHolds++;
	}
	return iNumHolds;
}

void NoteData::Convert2sAnd3sToHoldNotes()
{
	// Any note will end a hold (not just a TAP_HOLD_TAIL).  This makes parsing DWIs much easier.
	// Plus, allowing tap notes in the middle of a hold doesn't make sense!

	for( int t=0; t<GetNumTracks(); t++ )	// foreach column
	{
		FOREACH_NONEMPTY_ROW_IN_TRACK( *this, t, r )
		{
			TapNote head = GetTapNote(t,r);
			if( head.type != TapNote::hold_head )
				continue;	// skip

			SetTapNote(t, r, TAP_EMPTY);	// clear the hold head marker

			// search for end of HoldNote
			FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE( *this, t, j, r+1, MAX_NOTE_ROW )
			{
				// End hold on the next note we see.  This should be a hold_tail if the 
				// data is in a consistent state, but doesn't have to be.
				if( GetTapNote(t, j).type == TapNote::empty )
					continue;

				SetTapNote(t, j, TAP_EMPTY);

				HoldNote hold(t, r, j);
				hold.result = head.HoldResult;
				AddHoldNote( hold );
				break;	// done searching for the end of this hold
			}
		}
	}
}


/* "102000301" ==
 * "104444001" */
void NoteData::ConvertHoldNotesTo2sAnd3s()
{
	// copy HoldNotes into the new structure, but expand them into 2s and 3s
	for( int i=0; i<GetNumHoldNotes(); i++ ) 
	{
		const HoldNote &hn = GetHoldNote(i);
		
		/* If they're the same, then they got clamped together, so just ignore it. */
		if( hn.iStartRow == hn.iEndRow )
			continue;

		TapNote head = TAP_ORIGINAL_HOLD_HEAD;
		head.HoldResult = hn.result;
		SetTapNote( hn.iTrack, hn.iStartRow, head );
		SetTapNote( hn.iTrack, hn.iEndRow, TAP_ORIGINAL_HOLD_TAIL );
	}
	m_HoldNotes.clear();
}


void NoteData::To2sAnd3s( const NoteData& from )
{
	CopyAll( from );
	ConvertHoldNotesTo2sAnd3s();
}

void NoteData::From2sAnd3s( const NoteData& from )
{
	CopyAll( from );
	Convert2sAnd3sToHoldNotes();
}

// -1 for iOriginalTracksToTakeFrom means no track
void NoteData::LoadTransformed( const NoteData& in, int iNewNumTracks, const int iOriginalTrackToTakeFrom[] )
{
	// reset all notes
	Init();
	
	SetNumTracks( iNewNumTracks );
	ConvertHoldNotesTo2sAnd3s();

	NoteData Input;
	Input.To2sAnd3s( in );

	// copy tracks
	for( int t=0; t<GetNumTracks(); t++ )
	{
		const int iOriginalTrack = iOriginalTrackToTakeFrom[t];
		ASSERT_M( iOriginalTrack < in.GetNumTracks(), ssprintf("from %i >= %i (to %i)", 
			iOriginalTrack, in.GetNumTracks(), iOriginalTrackToTakeFrom[t]));

		if( iOriginalTrack == -1 )
			continue;
		m_TapNotes[t] = Input.m_TapNotes[iOriginalTrack];
	}

	Convert2sAnd3sToHoldNotes();
}

void NoteData::MoveTapNoteTrack( int dest, int src )
{
	if(dest == src) return;
	m_TapNotes[dest] = m_TapNotes[src];
	m_TapNotes[src].clear();
}

void NoteData::SetTapNote( int track, int row, const TapNote& t )
{
	DEBUG_ASSERT( track>=0 && track<GetNumTracks() );
	
	if( row < 0 )
		return;

	// There's no point in inserting empty notes into the map.
	// Any blank space in the map is defined to be empty.
	// If we're trying to insert an empty at a spot where
	// another note already exists, then we're really deleting
	// from the map.
	if( t == TAP_EMPTY )
	{
		TrackMap &trackMap = m_TapNotes[track];
		// remove the element at this position (if any).  This will return either 0 or 1.
		trackMap.erase( row );
	}
	else
	{
		m_TapNotes[track][row] = t;
	}
}

void NoteData::GetTracksHeldAtRow( int row, set<int>& addTo )
{
	for( unsigned i=0; i<m_HoldNotes.size(); i++ )
	{
		const HoldNote& hn = m_HoldNotes[i];
		if( hn.RowIsInRange(row) )
			addTo.insert( hn.iTrack );
	}
}

int NoteData::GetNumTracksHeldAtRow( int row )
{
	static set<int> viTracks;
	viTracks.clear();
	GetTracksHeldAtRow( row, viTracks );
	return viTracks.size();
}

bool NoteData::GetNextTapNoteRowForTrack( int track, int &rowInOut ) const
{
	const TrackMap &mapTrack = m_TapNotes[track];

	// lower_bound and upper_bound have the same effect here because duplicate 
	// keys aren't allowed.
	//
	// lower_bound "finds the first element whose key is not less than k" (>=);
	// upper_bound "finds the first element whose key greater than k".  They don't
	// have the same effect, but lower_bound(row+1) should equal upper_bound(row). -glenn
	TrackMap::const_iterator iter = mapTrack.lower_bound( rowInOut+1 );	// "find the first note for which row+1 < key == false"
	if( iter == mapTrack.end() )
		return false;

	ASSERT( iter->first > rowInOut );
	rowInOut = iter->first;
	return true;
}

bool NoteData::GetPrevTapNoteRowForTrack( int track, int &rowInOut ) const
{
	const TrackMap &mapTrack = m_TapNotes[track];

	/* Find the first note >= rowInOut. */
	TrackMap::const_iterator iter = mapTrack.lower_bound( rowInOut );

	/* If we're at the beginning, we can't move back any more. */
	if( iter == mapTrack.begin() )
		return false;

	/* Move back by one. */
	--iter;	
	ASSERT( iter->first < rowInOut );
	rowInOut = iter->first;
	return true;
}

/* Return an iterator range.  This can be used to iterate trackwise over a range of
 * notes.  It's like FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE, except it only requires
 * two map searches (iterating is O(1)), but the iterators will become invalid if
 * the notes they represent disappear, so you need to pay attention to how you modify
 * the data. */
void NoteData::GetTapNoteRange( int iTrack, int iStartRow, int iEndRow, TrackMap::const_iterator &begin, TrackMap::const_iterator &end ) const
{
	ASSERT_M( iTrack < GetNumTracks(), ssprintf("%i,%i", iTrack, GetNumTracks())  );
	ASSERT_M( iStartRow <= iEndRow, ssprintf("%i > %i", iStartRow, iEndRow)  );

	const TrackMap &mapTrack = m_TapNotes[iTrack];
	begin = mapTrack.lower_bound( iStartRow );
	end = mapTrack.upper_bound( iEndRow );
}

bool NoteData::GetNextTapNoteRowForAllTracks( int &rowInOut ) const
{
	int iClosestNextRow = MAX_NOTE_ROW;
	bool bAnyHaveNextNote = false;
	for( int t=0; t<GetNumTracks(); t++ )
	{
		int iNewRowThisTrack = rowInOut;
		if( GetNextTapNoteRowForTrack( t, iNewRowThisTrack ) )
		{
			bAnyHaveNextNote = true;
			ASSERT( iNewRowThisTrack < MAX_NOTE_ROW );
			iClosestNextRow = min( iClosestNextRow, iNewRowThisTrack );
		}
	}

	if( bAnyHaveNextNote )
	{
		rowInOut = iClosestNextRow;
		return true;
	}
	else
	{
		return false;
	}
}

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

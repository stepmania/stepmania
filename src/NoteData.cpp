/*
 * NoteData is organized by:
 *  track - corresponds to different columns of notes on the screen
 *  row/index - corresponds to subdivisions of beats
 */

#include "global.h"
#include "NoteData.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "XmlFile.h"
#include "GameState.h" // blame radar calculations.
#include "RageUtil_AutoPtr.h"

REGISTER_CLASS_TRAITS( NoteData, new NoteData(*pCopy) )

void NoteData::Init()
{
	m_TapNotes = vector<TrackMap>();	// ensure that the memory is freed
}

void NoteData::SetNumTracks( int iNewNumTracks )
{
	ASSERT( iNewNumTracks > 0 );

	m_TapNotes.resize( iNewNumTracks );
}

bool NoteData::IsComposite() const
{
	for( int track = 0; track < GetNumTracks(); ++track )
	{
		for (std::pair<int, TapNote> const &tn : m_TapNotes[track])
			if( tn.second.pn != PLAYER_INVALID )
				return true;
	}
	return false;
}

// Clear (rowBegin,rowEnd).
void NoteData::ClearRangeForTrack( int rowBegin, int rowEnd, int iTrack )
{
	// Optimization: if the range encloses everything, just clear the whole maps.
	if( rowBegin == 0 && rowEnd == MAX_NOTE_ROW )
	{
		m_TapNotes[iTrack].clear();
		return;
	}

	/* If the range is empty, don't do anything. Otherwise, an empty range will
	 * cause hold notes to be split when they shouldn't be. */
	if( rowBegin == rowEnd )
		return;

	NoteData::TrackMap::iterator lBegin, lEnd;
	GetTapNoteRangeInclusive( iTrack, rowBegin, rowEnd, lBegin, lEnd );

	if( lBegin != lEnd && lBegin->first < rowBegin && lBegin->first + lBegin->second.iDuration > rowEnd )
	{
		/* A hold note overlaps the whole range. Truncate it, and add the
		 * remainder to the end. */
		TapNote tn1 = lBegin->second;
		TapNote tn2 = tn1;

		int iEndRow = lBegin->first + tn1.iDuration;
		int iRow = lBegin->first;

		tn1.iDuration = rowBegin - iRow;
		tn2.iDuration = iEndRow - rowEnd;

		SetTapNote( iTrack, iRow, tn1 );
		SetTapNote( iTrack, rowEnd, tn2 );

		// We may have invalidated our iterators.
		GetTapNoteRangeInclusive( iTrack, rowBegin, rowEnd, lBegin, lEnd );
	}
	else if( lBegin != lEnd && lBegin->first < rowBegin )
	{
		// A hold note overlaps the beginning of the range.  Truncate it.
		TapNote &tn1 = lBegin->second;
		int iRow = lBegin->first;
		tn1.iDuration = rowBegin - iRow;

		++lBegin;
	}

	if( lBegin != lEnd )
	{
		NoteData::TrackMap::iterator prev = lEnd;
		--prev;
		TapNote tn = prev->second;
		int iRow = prev->first;
		if( tn.type == TapNoteType_HoldHead && iRow + tn.iDuration > rowEnd )
		{
			// A hold note overlaps the end of the range.  Separate it.
			SetTapNote( iTrack, iRow, TAP_EMPTY );

			int iAdd = rowEnd - iRow;
			tn.iDuration -= iAdd;
			iRow += iAdd;
			SetTapNote( iTrack, iRow, tn );
			lEnd = prev;
		}

		// We may have invalidated our iterators.
		GetTapNoteRangeInclusive( iTrack, rowBegin, rowEnd, lBegin, lEnd );
	}

	m_TapNotes[iTrack].erase( lBegin, lEnd );
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
}

/* Copy [rowFromBegin,rowFromEnd) from pFrom to this. (Note that this does
 * *not* overlay; all data in the range is overwritten.) */
void NoteData::CopyRange( const NoteData& from, int rowFromBegin, int rowFromEnd, int rowToBegin )
{
	ASSERT( from.GetNumTracks() == GetNumTracks() );

	if( rowFromBegin > rowFromEnd )
		return; // empty range

	const int rowToEnd = (rowFromEnd-rowFromBegin) + rowToBegin;
	const int iMoveBy = rowToBegin-rowFromBegin;

	// Clear the region.
	ClearRange( rowToBegin, rowToEnd );

	for( int t=0; t<GetNumTracks(); t++ )
	{
		NoteData::TrackMap::const_iterator lBegin, lEnd;
		from.GetTapNoteRangeInclusive( t, rowFromBegin, rowFromEnd, lBegin, lEnd );
		for( ; lBegin != lEnd; ++lBegin )
		{
			TapNote head = lBegin->second;
			if( head.type == TapNoteType_Empty )
				continue;

			if( head.type == TapNoteType_HoldHead )
			{
				int iStartRow = lBegin->first + iMoveBy;
				int iEndRow = iStartRow + head.iDuration;

				iStartRow = clamp( iStartRow, rowToBegin, rowToEnd );
				iEndRow = clamp( iEndRow, rowToBegin, rowToEnd );

				this->AddHoldNote( t, iStartRow, iEndRow, head );
			}
			else
			{
				int iTo = lBegin->first + iMoveBy;
				if( iTo >= rowToBegin && iTo <= rowToEnd )
					this->SetTapNote( t, iTo, head );
			}
		}
	}
}

void NoteData::CopyAll( const NoteData& from )
{
	*this = from;
}

bool NoteData::IsRowEmpty( int row ) const
{
	for( int t=0; t<GetNumTracks(); t++ )
		if( GetTapNote(t, row).type != TapNoteType_Empty )
			return false;
	return true;
}

bool NoteData::IsRangeEmpty( int track, int rowBegin, int rowEnd ) const
{
	ASSERT( track < GetNumTracks() );

	FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE( *this, track, r, rowBegin, rowEnd )
		if( GetTapNote(track,r).type != TapNoteType_Empty )
			return false;
	return true;
}

int NoteData::GetNumTapNonEmptyTracks( int row ) const
{
	int iNum = 0;
	for( int t=0; t<GetNumTracks(); t++ )
		if( GetTapNote(t, row).type != TapNoteType_Empty )
			iNum++;
	return iNum;
}

void NoteData::GetTapNonEmptyTracks( int row, set<int>& addTo ) const
{
	for( int t=0; t<GetNumTracks(); t++ )
		if( GetTapNote(t, row).type != TapNoteType_Empty )
			addTo.insert(t);
}

bool NoteData::GetTapFirstNonEmptyTrack( int row, int &iNonEmptyTrackOut ) const
{
	for( int t=0; t<GetNumTracks(); t++ )
	{
		if( GetTapNote( t, row ).type != TapNoteType_Empty )
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
		if( GetTapNote( t, row ).type == TapNoteType_Empty )
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
		if( GetTapNote( t, row ).type == TapNoteType_Empty )
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
		if( tn.type == TapNoteType_Tap || tn.type == TapNoteType_Lift )
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
		if( tn.type == TapNoteType_Tap || tn.type == TapNoteType_Lift || tn.type == TapNoteType_HoldHead )
			iNum++;
	}
	return iNum;
}

void NoteData::LogNonEmptyRows() {
	NonEmptyRowVector.clear();
	FOREACH_NONEMPTY_ROW_ALL_TRACKS(*this, row)
		NonEmptyRowVector.push_back(row);
}

int NoteData::GetFirstTrackWithTap( int row ) const
{
	for( int t=0; t<GetNumTracks(); t++ )
	{
		const TapNote &tn = GetTapNote( t, row );
		if( tn.type == TapNoteType_Tap || tn.type == TapNoteType_Lift )
			return t;
	}
	return -1;
}

int NoteData::GetFirstTrackWithTapOrHoldHead( int row ) const
{
	for( int t=0; t<GetNumTracks(); t++ )
	{
		const TapNote &tn = GetTapNote( t, row );
		if( tn.type == TapNoteType_Tap || tn.type == TapNoteType_Lift || tn.type == TapNoteType_HoldHead )
			return t;
	}
	return -1;
}

int NoteData::GetLastTrackWithTapOrHoldHead( int row ) const
{
	for( int t=GetNumTracks()-1; t>=0; t-- )
	{
		const TapNote &tn = GetTapNote( t, row );
		if( tn.type == TapNoteType_Tap || tn.type == TapNoteType_Lift || tn.type == TapNoteType_HoldHead )
			return t;
	}
	return -1;
}

void NoteData::AddHoldNote( int iTrack, int iStartRow, int iEndRow, TapNote tn )
{
	ASSERT( iStartRow>=0 && iEndRow>=0 );
	ASSERT_M( iEndRow >= iStartRow, ssprintf("EndRow %d < StartRow %d",iEndRow,iStartRow) );

	/* Include adjacent (non-overlapping) hold notes, since we need to merge with them. */
	NoteData::TrackMap::iterator lBegin, lEnd;
	GetTapNoteRangeInclusive( iTrack, iStartRow, iEndRow, lBegin, lEnd, true );

	// Look for other hold notes that overlap and merge them into add.
	for( iterator it = lBegin; it != lEnd; ++it )
	{
		int iOtherRow = it->first;
		const TapNote &tnOther = it->second;
		if( tnOther.type == TapNoteType_HoldHead )
		{
			iStartRow = min( iStartRow, iOtherRow );
			iEndRow = max( iEndRow, iOtherRow + tnOther.iDuration );
		}
	}

	tn.iDuration = iEndRow - iStartRow;

	// Remove everything in the range.
	while( lBegin != lEnd )
	{
		iterator next = lBegin;
		++next;

		RemoveTapNote( iTrack, lBegin );

		lBegin = next;
	}

	/* Additionally, if there's a tap note lying at the end of our range,
	 * remove it too. */
	SetTapNote( iTrack, iEndRow, TAP_EMPTY );

	// add a tap note at the start of this hold
	SetTapNote( iTrack, iStartRow, tn );
}

/* Determine if a hold note lies on the given spot.  Return true if so.  If
 * pHeadRow is non-nullptr, return the row of the head. */
bool NoteData::IsHoldHeadOrBodyAtRow( int iTrack, int iRow, int *pHeadRow ) const
{
	const TapNote &tn = GetTapNote( iTrack, iRow );
	if( tn.type == TapNoteType_HoldHead )
	{
		if( pHeadRow != nullptr )
			*pHeadRow = iRow;
		return true;
	}

	return IsHoldNoteAtRow( iTrack, iRow, pHeadRow );
}

/* Determine if a hold note lies on the given spot. Return true if so.  If
 * pHeadRow is non-nullptr, return the row of the head. (Note that this returns
 * false if a hold head lies on iRow itself.) */
/* XXX: rename this to IsHoldBodyAtRow */
bool NoteData::IsHoldNoteAtRow( int iTrack, int iRow, int *pHeadRow ) const
{
	int iDummy;
	if( pHeadRow == nullptr )
		pHeadRow = &iDummy;

	/* Starting at iRow, search upwards. If we find a TapNoteType_HoldHead, we're within
	 * a hold. If we find a tap, mine or attack, we're not--those never lie
	 * within hold notes. Ignore autoKeysound. */
	FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE_REVERSE( *this, iTrack, r, 0, iRow )
	{
		const TapNote &tn = GetTapNote( iTrack, r );
		switch( tn.type )
		{
		case TapNoteType_HoldHead:
			if( tn.iDuration + r < iRow )
				return false;
			*pHeadRow = r;
			return true;

		case TapNoteType_Tap:
		case TapNoteType_Mine:
		case TapNoteType_Attack:
		case TapNoteType_Lift:
		case TapNoteType_Fake:
			return false;

		case TapNoteType_Empty:
		case TapNoteType_AutoKeysound:
			// ignore
			continue;
		DEFAULT_FAIL( tn.type );
		}
	}

	return false;
}

bool NoteData::IsEmpty() const
{ 
	for( int t=0; t < GetNumTracks(); t++ )
	{
		int iRow = -1;
		if( !GetNextTapNoteRowForTrack( t, iRow ) )
			continue;

		return false;
	}

	return true;
}

int NoteData::GetFirstRow() const
{ 
	int iEarliestRowFoundSoFar = -1;

	for( int t=0; t < GetNumTracks(); t++ )
	{
		int iRow = -1;
		if( !GetNextTapNoteRowForTrack( t, iRow, true ) )
			continue;

		if( iEarliestRowFoundSoFar == -1 )
			iEarliestRowFoundSoFar = iRow;
		else
			iEarliestRowFoundSoFar = min( iEarliestRowFoundSoFar, iRow );
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

		/* XXX: We might have a hold note near the end with autoplay sounds
		 * after it.  Do something else with autoplay sounds ... */
		const TapNote &tn = GetTapNote( t, iRow );
		if( tn.type == TapNoteType_HoldHead )
			iRow += tn.iDuration;

		iOldestRowFoundSoFar = max( iOldestRowFoundSoFar, iRow );
	}

	return iOldestRowFoundSoFar;
}

bool NoteData::IsTap(const TapNote &tn, const int row) const
{
	return (tn.type != TapNoteType_Empty && tn.type != TapNoteType_Mine
			&& tn.type != TapNoteType_Lift && tn.type != TapNoteType_Fake
			&& tn.type != TapNoteType_AutoKeysound
			&& GAMESTATE->GetProcessedTimingData()->IsJudgableAtRow(row));
}

bool NoteData::IsMine(const TapNote &tn, const int row) const
{
	return (tn.type == TapNoteType_Mine
			&& GAMESTATE->GetProcessedTimingData()->IsJudgableAtRow(row));
}

bool NoteData::IsLift(const TapNote &tn, const int row) const
{
	return (tn.type == TapNoteType_Lift
			&& GAMESTATE->GetProcessedTimingData()->IsJudgableAtRow(row));
}

bool NoteData::IsFake(const TapNote &tn, const int row) const
{
	return (tn.type == TapNoteType_Fake
			|| !GAMESTATE->GetProcessedTimingData()->IsJudgableAtRow(row));
}

int NoteData::GetNumTapNotes( int iStartIndex, int iEndIndex ) const
{
	int iNumNotes = 0;
	for( int t=0; t<GetNumTracks(); t++ )
	{
		FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE( *this, t, r, iStartIndex, iEndIndex )
		{
			if (this->IsTap(GetTapNote(t, r), r))
				iNumNotes++;
		}
	}

	return iNumNotes;
}

int NoteData::GetNumTapNotesNoTiming( int iStartIndex, int iEndIndex ) const
{
	int iNumNotes = 0;
	for( int t=0; t<GetNumTracks(); t++ )
	{
		FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE( *this, t, r, iStartIndex, iEndIndex )
		{
			if(GetTapNote(t, r).type != TapNoteType_Empty)
			{ iNumNotes++; }
		}
	}

	return iNumNotes;
}

int NoteData::GetNumTapNotesInRow( int iRow ) const
{
	int iNumNotes = 0;
	for( int t=0; t<GetNumTracks(); t++ )
	{
		if (this->IsTap(GetTapNote(t, iRow), iRow))
			iNumNotes++;
	}
	return iNumNotes;
}

int NoteData::GetNumRowsWithTap( int iStartIndex, int iEndIndex ) const
{
	int iNumNotes = 0;
	FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( *this, r, iStartIndex, iEndIndex )
		if( IsThereATapAtRow(r) && GAMESTATE->GetProcessedTimingData()->IsJudgableAtRow(r) )
			iNumNotes++;

	return iNumNotes;
}

int NoteData::GetNumMines( int iStartIndex, int iEndIndex ) const
{
	int iNumMines = 0;

	for( int t=0; t<GetNumTracks(); t++ )
	{
		FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE( *this, t, r, iStartIndex, iEndIndex )
			if (this->IsMine(GetTapNote(t, r), r))
				iNumMines++;
	}

	return iNumMines;
}

int NoteData::GetNumRowsWithTapOrHoldHead( int iStartIndex, int iEndIndex ) const
{
	int iNumNotes = 0;
	FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( *this, r, iStartIndex, iEndIndex )
		if( IsThereATapOrHoldHeadAtRow(r) && GAMESTATE->GetProcessedTimingData()->IsJudgableAtRow(r) )
			iNumNotes++;

	return iNumNotes;
}

bool NoteData::RowNeedsAtLeastSimultaneousPresses( int iMinSimultaneousPresses, const int row ) const
{
	int iNumNotesThisIndex = 0;
	for( int t=0; t<GetNumTracks(); t++ )
	{
		const TapNote &tn = GetTapNote(t, row);
		switch( tn.type )
		{
			case TapNoteType_Mine:
			case TapNoteType_Empty:
			case TapNoteType_Fake:
			case TapNoteType_Lift: // you don't "press" on a lift.
			case TapNoteType_AutoKeysound:
				continue;	// skip these types - they don't count
			default: break;
		}
		++iNumNotesThisIndex;
	}

	/* We must have at least one tap or hold head at this row to count it. */
	if( !iNumNotesThisIndex )
		return false;

	if( iNumNotesThisIndex < iMinSimultaneousPresses )
	{
		/* We have at least one, but not enough.  Count holds.  Do count adjacent holds. */
		for( int t=0; t<GetNumTracks(); ++t )
		{
			if( IsHoldNoteAtRow(t, row) )
				++iNumNotesThisIndex;
		}
	}

	return iNumNotesThisIndex >= iMinSimultaneousPresses;
}

int NoteData::GetNumRowsWithSimultaneousPresses( int iMinSimultaneousPresses, int iStartIndex, int iEndIndex ) const
{
	/* Count the number of times you have to use your hands.  This includes
	 * three taps at the same time, a tap while two hold notes are being held,
	 * etc.  Only count rows that have at least one tap note (hold heads count).
	 * Otherwise, every row of hold notes counts, so three simultaneous hold
	 * notes will count as hundreds of "hands". */
	int iNum = 0;
	FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( *this, r, iStartIndex, iEndIndex )
	{
		if( !RowNeedsAtLeastSimultaneousPresses(iMinSimultaneousPresses,r) )
			continue;
		if (!GAMESTATE->GetProcessedTimingData()->IsJudgableAtRow(r))
			continue;
		iNum++;
	}

	return iNum;
}

int NoteData::GetNumRowsWithSimultaneousTaps( int iMinTaps, int iStartIndex, int iEndIndex ) const
{
	int iNum = 0;
	FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( *this, r, iStartIndex, iEndIndex )
	{
		if (!GAMESTATE->GetProcessedTimingData()->IsJudgableAtRow(r))
			continue;
		int iNumNotesThisIndex = 0;
		for( int t=0; t<GetNumTracks(); t++ )
		{
			const TapNote &tn = GetTapNote(t, r);
			if (tn.type != TapNoteType_Mine &&     // mines don't count.
				tn.type != TapNoteType_Empty &&
				tn.type != TapNoteType_Fake &&
				tn.type != TapNoteType_AutoKeysound)
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
	for( int t=0; t<GetNumTracks(); ++t )
	{
		NoteData::TrackMap::const_iterator lBegin, lEnd;
		GetTapNoteRangeExclusive( t, iStartIndex, iEndIndex, lBegin, lEnd );
		for( ; lBegin != lEnd; ++lBegin )
		{
			if( lBegin->second.type != TapNoteType_HoldHead ||
				lBegin->second.subType != TapNoteSubType_Hold )
				continue;
			if (!GAMESTATE->GetProcessedTimingData()->IsJudgableAtRow(lBegin->first))
				continue;
			iNumHolds++;
		}
	}
	return iNumHolds;
}

int NoteData::GetNumRolls( int iStartIndex, int iEndIndex ) const
{
	int iNumRolls = 0;
	for( int t=0; t<GetNumTracks(); ++t )
	{
		NoteData::TrackMap::const_iterator lBegin, lEnd;
		GetTapNoteRangeExclusive( t, iStartIndex, iEndIndex, lBegin, lEnd );
		for( ; lBegin != lEnd; ++lBegin )
		{
			if( lBegin->second.type != TapNoteType_HoldHead ||
				lBegin->second.subType != TapNoteSubType_Roll )
				continue;
			if (!GAMESTATE->GetProcessedTimingData()->IsJudgableAtRow(lBegin->first))
				continue;
			iNumRolls++;
		}
	}
	return iNumRolls;
}

int NoteData::GetNumLifts( int iStartIndex, int iEndIndex ) const
{
	int iNumLifts = 0;

	for( int t=0; t<GetNumTracks(); t++ )
	{
		FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE( *this, t, r, iStartIndex, iEndIndex )
			if( this->IsLift(GetTapNote(t, r), r))
				iNumLifts++;
	}

	return iNumLifts;
}

int NoteData::GetNumFakes( int iStartIndex, int iEndIndex ) const
{
	int iNumFakes = 0;
	
	for( int t=0; t<GetNumTracks(); t++ )
	{
		FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE( *this, t, r, iStartIndex, iEndIndex )
			if( this->IsFake(GetTapNote(t, r), r))
				iNumFakes++;
	}
	
	return iNumFakes;
}

bool NoteData::IsPlayer1(const int track, const TapNote &tn) const
{
	if (this->IsComposite())
	{
		return tn.pn == PLAYER_1;
	}
	return track < (this->GetNumTracks() / 2);
}

pair<int, int> NoteData::GetNumTapNotesTwoPlayer( int iStartIndex, int iEndIndex ) const
{
	pair<int, int> num(0, 0);
	for( int t=0; t<GetNumTracks(); t++ )
	{
		FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE( *this, t, r, iStartIndex, iEndIndex )
		{
			const TapNote &tn = GetTapNote(t, r);
			if (this->IsTap(tn, r))
			{
				if (this->IsPlayer1(t, tn))
					num.first++;
				else
					num.second++;
			}
		}
	}
	return num;
}

pair<int, int> NoteData::GetNumRowsWithSimultaneousTapsTwoPlayer(int minTaps,
																 int startRow,
																 int endRow) const
{
	pair<int, int> num(0, 0);
	FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( *this, r, startRow, endRow )
	{
		pair<int, int> found(0, 0);
		for( int t=0; t<GetNumTracks(); t++ )
		{
			const TapNote &tn = GetTapNote(t, r);
			if (this->IsTap(tn, r))
			{
				if (this->IsPlayer1(t, tn))
					found.first++;
				else
					found.second++;
			}
		}
		if (found.first >= minTaps)
			num.first++;
		if (found.second >= minTaps)
			num.second++;
	}
	return num;
}

pair<int, int> NoteData::GetNumJumpsTwoPlayer( int iStartIndex, int iEndIndex ) const
{
	return GetNumRowsWithSimultaneousTapsTwoPlayer( 2, iStartIndex, iEndIndex );
}

pair<int, int> NoteData::GetNumHandsTwoPlayer( int iStartIndex, int iEndIndex ) const
{
	return GetNumRowsWithSimultaneousTapsTwoPlayer( 3, iStartIndex, iEndIndex );
}

pair<int, int> NoteData::GetNumQuadsTwoPlayer( int iStartIndex, int iEndIndex ) const
{
	return GetNumRowsWithSimultaneousTapsTwoPlayer( 4, iStartIndex, iEndIndex );
}

pair<int, int> NoteData::GetNumHoldNotesTwoPlayer( int iStartIndex, int iEndIndex ) const
{
	pair<int, int> num(0, 0);
	for( int t=0; t<GetNumTracks(); ++t )
	{
		NoteData::TrackMap::const_iterator lBegin, lEnd;
		GetTapNoteRangeExclusive( t, iStartIndex, iEndIndex, lBegin, lEnd );
		for( ; lBegin != lEnd; ++lBegin )
		{
			if( lBegin->second.type != TapNoteType_HoldHead ||
			   lBegin->second.subType != TapNoteSubType_Hold )
				continue;
			if (!GAMESTATE->GetProcessedTimingData()->IsJudgableAtRow(lBegin->first))
				continue;
			if (this->IsPlayer1(t, lBegin->second))
				num.first++;
			else
				num.second++;
		}
	}
	return num;
}

pair<int, int> NoteData::GetNumMinesTwoPlayer( int iStartIndex, int iEndIndex ) const
{
	pair<int, int> num(0, 0);
	for( int t=0; t<GetNumTracks(); t++ )
	{
		FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE( *this, t, r, iStartIndex, iEndIndex )
		{
			const TapNote &tn = GetTapNote(t, r);
			if (this->IsMine(tn, r))
			{
				if (this->IsPlayer1(t, tn))
					num.first++;
				else
					num.second++;
			}
		}
	}
	return num;
}

pair<int, int> NoteData::GetNumRollsTwoPlayer( int iStartIndex, int iEndIndex ) const
{
	pair<int, int> num(0, 0);
	for( int t=0; t<GetNumTracks(); ++t )
	{
		NoteData::TrackMap::const_iterator lBegin, lEnd;
		GetTapNoteRangeExclusive( t, iStartIndex, iEndIndex, lBegin, lEnd );
		for( ; lBegin != lEnd; ++lBegin )
		{
			if( lBegin->second.type != TapNoteType_HoldHead ||
			   lBegin->second.subType != TapNoteSubType_Roll )
				continue;
			if (!GAMESTATE->GetProcessedTimingData()->IsJudgableAtRow(lBegin->first))
				continue;
			if (this->IsPlayer1(t, lBegin->second))
				num.first++;
			else
				num.second++;
		}
	}
	return num;
}

pair<int, int> NoteData::GetNumLiftsTwoPlayer( int iStartIndex, int iEndIndex ) const
{
	pair<int, int> num(0, 0);
	for( int t=0; t<GetNumTracks(); t++ )
	{
		FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE( *this, t, r, iStartIndex, iEndIndex )
		{
			const TapNote &tn = GetTapNote(t, r);
			if (this->IsLift(tn, r))
			{
				if (this->IsPlayer1(t, tn))
					num.first++;
				else
					num.second++;
			}
		}
	}
	return num;
}

pair<int, int> NoteData::GetNumFakesTwoPlayer( int iStartIndex, int iEndIndex ) const
{
	pair<int, int> num(0, 0);
	for( int t=0; t<GetNumTracks(); t++ )
	{
		FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE( *this, t, r, iStartIndex, iEndIndex )
		{
			const TapNote &tn = GetTapNote(t, r);
			if (this->IsFake(tn, r))
			{
				if (this->IsPlayer1(t, tn))
					num.first++;
				else
					num.second++;
			}
		}
	}
	return num;
}

/*
int NoteData::GetNumMinefields( int iStartIndex, int iEndIndex ) const
{
	int iNumMinefields = 0;
	for( int t=0; t<GetNumTracks(); ++t )
	{
		NoteData::TrackMap::const_iterator begin, end;
		GetTapNoteRangeExclusive( t, iStartIndex, iEndIndex, begin, end );
		for( ; begin != end; ++begin )
		{
			if( begin->second.type != TapNoteType_HoldHead ||
				begin->second.subType != TapNoteSubType_mine )
				continue;
			iNumMinefields++;
		}
	}
	return iNumMinefields;
}
*/

// -1 for iOriginalTracksToTakeFrom means no track
void NoteData::LoadTransformed( const NoteData& in, int iNewNumTracks, const int iOriginalTrackToTakeFrom[] )
{
	// reset all notes
	Init();

	SetNumTracks( iNewNumTracks );

	// copy tracks
	for( int t=0; t<GetNumTracks(); t++ )
	{
		const int iOriginalTrack = iOriginalTrackToTakeFrom[t];
		ASSERT_M( iOriginalTrack < in.GetNumTracks(), ssprintf("from OriginalTrack %i >= %i (#tracks) (taking from %i)", 
			iOriginalTrack, in.GetNumTracks(), iOriginalTrackToTakeFrom[t]));

		if( iOriginalTrack == -1 )
			continue;
		m_TapNotes[t] = in.m_TapNotes[iOriginalTrack];
	}
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
	// If we're trying to insert an empty at a spot where another note
	// already exists, then we're really deleting from the map.
	if( t == TAP_EMPTY )
	{
		TrackMap &trackMap = m_TapNotes[track];
		// remove the element at this position (if any).
		// This will return either 0 or 1.
		trackMap.erase( row );
	}
	else
	{
		m_TapNotes[track][row] = t;
	}
}

void NoteData::GetTracksHeldAtRow( int row, set<int>& addTo )
{
	for( int t=0; t<GetNumTracks(); ++t )
		if( IsHoldNoteAtRow( t, row ) )
			addTo.insert( t );
}

int NoteData::GetNumTracksHeldAtRow( int row )
{
	static set<int> viTracks;
	viTracks.clear();
	GetTracksHeldAtRow( row, viTracks );
	return viTracks.size();
}

bool NoteData::GetNextTapNoteRowForTrack( int track, int &rowInOut, bool ignoreAutoKeysounds ) const
{
	const TrackMap &mapTrack = m_TapNotes[track];

	// lower_bound and upper_bound have the same effect here because duplicate 
	// keys aren't allowed.

	// lower_bound "finds the first element whose key is not less than k" (>=);
	// upper_bound "finds the first element whose key greater than k".  They don't
	// have the same effect, but lower_bound(row+1) should equal upper_bound(row). -glenn
	TrackMap::const_iterator iter = mapTrack.lower_bound( rowInOut+1 );	// "find the first note for which row+1 < key == false"
	if( iter == mapTrack.end() )
		return false;

	ASSERT( iter->first > rowInOut );

	// If we want to ignore autokeysounds, keep going until we find a real note.
	if(ignoreAutoKeysounds) {
		while(iter->second.type == TapNoteType_AutoKeysound) {
			iter++;
			if(iter==mapTrack.end()) return false;
		}
	}
	rowInOut = iter->first;
	return true;
}

bool NoteData::GetPrevTapNoteRowForTrack( int track, int &rowInOut ) const
{
	const TrackMap &mapTrack = m_TapNotes[track];

	// Find the first note >= rowInOut.
	TrackMap::const_iterator iter = mapTrack.lower_bound( rowInOut );

	// If we're at the beginning, we can't move back any more.
	if( iter == mapTrack.begin() )
		return false;

	// Move back by one.
	--iter;	
	ASSERT( iter->first < rowInOut );
	rowInOut = iter->first;
	return true;
}

void NoteData::GetTapNoteRange( int iTrack, int iStartRow, int iEndRow, TrackMap::iterator &lBegin, TrackMap::iterator &lEnd )
{
	ASSERT_M( iTrack < GetNumTracks(), ssprintf("%i,%i", iTrack, GetNumTracks())  );
	TrackMap &mapTrack = m_TapNotes[iTrack];

	if( iStartRow > iEndRow )
	{
		lBegin = lEnd = mapTrack.end();
		return;
	}

	if( iStartRow <= 0 )
		lBegin = mapTrack.begin(); // optimization
	else if( iStartRow >= MAX_NOTE_ROW )
		lBegin = mapTrack.end(); // optimization
	else
		lBegin = mapTrack.lower_bound( iStartRow );

	if( iEndRow <= 0 )
		lEnd = mapTrack.begin(); // optimization
	else if( iEndRow >= MAX_NOTE_ROW )
		lEnd = mapTrack.end(); // optimization
	else
		lEnd = mapTrack.lower_bound( iEndRow );
}


/* Include hold notes that overlap the edges.  If a hold note completely surrounds the given
 * range, included it, too.  If bIncludeAdjacent is true, also include hold notes adjacent to,
 * but not overlapping, the edge. */
void NoteData::GetTapNoteRangeInclusive( int iTrack, int iStartRow, int iEndRow, TrackMap::iterator &lBegin, TrackMap::iterator &lEnd, bool bIncludeAdjacent )
{
	GetTapNoteRange( iTrack, iStartRow, iEndRow, lBegin, lEnd );

	if( lBegin != this->begin(iTrack) )
	{
		iterator prev = Decrement(lBegin);

		const TapNote &tn = prev->second;
		if( tn.type == TapNoteType_HoldHead )
		{
			int iHoldStartRow = prev->first;
			int iHoldEndRow = iHoldStartRow + tn.iDuration;
			if( bIncludeAdjacent )
				++iHoldEndRow;
			if( iHoldEndRow > iStartRow )
			{
				// The previous note is a hold.
				lBegin = prev;
			}
		}
	}

	if( bIncludeAdjacent && lEnd != this->end(iTrack) )
	{
		// Include the next note if it's a hold and starts on iEndRow.
		const TapNote &tn = lEnd->second;
		int iHoldStartRow = lEnd->first;
		if( tn.type == TapNoteType_HoldHead && iHoldStartRow == iEndRow )
			++lEnd;
	}
}

void NoteData::GetTapNoteRangeExclusive( int iTrack, int iStartRow, int iEndRow, TrackMap::iterator &lBegin, TrackMap::iterator &lEnd )
{
	GetTapNoteRange( iTrack, iStartRow, iEndRow, lBegin, lEnd );

	// If end-1 is a hold_head, and extends beyond iEndRow, exclude it.
	if( lBegin != lEnd && lEnd != this->begin(iTrack) )
	{
		iterator prev = lEnd;
		--prev;
		if( prev->second.type == TapNoteType_HoldHead )
		{
			int localStartRow = prev->first;
			const TapNote &tn = prev->second;
			if( localStartRow + tn.iDuration >= iEndRow )
				lEnd = prev;
		}
	}
}

void NoteData::GetTapNoteRange( int iTrack, int iStartRow, int iEndRow, TrackMap::const_iterator &lBegin, TrackMap::const_iterator &lEnd ) const
{
	TrackMap::iterator const_begin, const_end;
	const_cast<NoteData *>(this)->GetTapNoteRange( iTrack, iStartRow, iEndRow, const_begin, const_end );
	lBegin = const_begin;
	lEnd = const_end;
}

void NoteData::GetTapNoteRangeInclusive( int iTrack, int iStartRow, int iEndRow, TrackMap::const_iterator &lBegin, TrackMap::const_iterator &lEnd, bool bIncludeAdjacent ) const
{
	TrackMap::iterator const_begin, const_end;
	const_cast<NoteData *>(this)->GetTapNoteRangeInclusive( iTrack, iStartRow, iEndRow, const_begin, const_end, bIncludeAdjacent );
	lBegin = const_begin;
	lEnd = const_end;
}

void NoteData::GetTapNoteRangeExclusive( int iTrack, int iStartRow, int iEndRow, TrackMap::const_iterator &lBegin, TrackMap::const_iterator &lEnd ) const
{
	TrackMap::iterator const_begin, const_end;
	const_cast<NoteData *>(this)->GetTapNoteRange( iTrack, iStartRow, iEndRow, const_begin, const_end );
	lBegin = const_begin;
	lEnd = const_end;
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

bool NoteData::GetPrevTapNoteRowForAllTracks( int &rowInOut ) const
{
	int iClosestPrevRow = 0;
	bool bAnyHavePrevNote = false;
	for( int t=0; t<GetNumTracks(); t++ )
	{
		int iNewRowThisTrack = rowInOut;
		if( GetPrevTapNoteRowForTrack( t, iNewRowThisTrack ) )
		{
			bAnyHavePrevNote = true;
			ASSERT( iNewRowThisTrack < MAX_NOTE_ROW );
			iClosestPrevRow = max( iClosestPrevRow, iNewRowThisTrack );
		}
	}

	if( bAnyHavePrevNote )
	{
		rowInOut = iClosestPrevRow;
		return true;
	}
	else
	{
		return false;
	}
}

XNode* NoteData::CreateNode() const
{
	XNode *p = new XNode( "NoteData" );

	all_tracks_const_iterator iter = GetTapNoteRangeAllTracks( 0, GetLastRow() );

	for( ; !iter.IsAtEnd(); ++iter )
	{
		XNode *p2 = iter->CreateNode();

		p2->AppendAttr( "Track", iter.Track() );
		p2->AppendAttr( "Row", iter.Row() );
		p->AppendChild( p2 );
	}
	return p;
}

void NoteData::LoadFromNode( const XNode* pNode )
{
	FAIL_M("NoteData::LoadFromNode() not implemented");
}

void NoteData::AddATIToList(all_tracks_iterator* iter) const
{
	m_atis.insert(iter);
}

void NoteData::AddATIToList(all_tracks_const_iterator* iter) const
{
	m_const_atis.insert(iter);
}

void NoteData::RemoveATIFromList(all_tracks_iterator* iter) const
{
	set<all_tracks_iterator*>::iterator pos= m_atis.find(iter);
	if(pos != m_atis.end())
	{
		m_atis.erase(pos);
	}
}

void NoteData::RemoveATIFromList(all_tracks_const_iterator* iter) const
{
	set<all_tracks_const_iterator*>::iterator pos= m_const_atis.find(iter);
	if(pos != m_const_atis.end())
	{
		m_const_atis.erase(pos);
	}
}

void NoteData::RevalidateATIs(vector<int> const& added_or_removed_tracks, bool added)
{
	for(set<all_tracks_iterator*>::iterator cur= m_atis.begin();
			cur != m_atis.end(); ++cur)
	{
		(*cur)->Revalidate(this, added_or_removed_tracks, added);
	}
	for(set<all_tracks_const_iterator*>::iterator cur= m_const_atis.begin();
			cur != m_const_atis.end(); ++cur)
	{
		(*cur)->Revalidate(this, added_or_removed_tracks, added);
	}
}

template<typename ND, typename iter, typename TN>
void NoteData::_all_tracks_iterator<ND, iter, TN>::Find( bool bReverse )
{
	// If no notes can be found in the range, m_iTrack will stay -1 and IsAtEnd() will return true.
	m_iTrack = -1;
	if( bReverse )
	{
		int iMaxRow = INT_MIN;
		for( int iTrack = m_pNoteData->GetNumTracks() - 1; iTrack >= 0; --iTrack )
		{
			iter &i( m_vCurrentIters[iTrack] );
			const iter &end = m_vEndIters[iTrack];
			if( i != end  &&  i->first > iMaxRow )
			{
				iMaxRow = i->first;
				m_iTrack = iTrack;
			}
		}
	}
	else
	{

		int iMinRow = INT_MAX;
		for( int iTrack = 0; iTrack < m_pNoteData->GetNumTracks(); ++iTrack )
		{
			iter &i = m_vCurrentIters[iTrack];
			const iter &end = m_vEndIters[iTrack];
			if( i != end  &&  i->first < iMinRow )
			{
				iMinRow = i->first;
				m_iTrack = iTrack;
			}
		}
	}
}

template<typename ND, typename iter, typename TN>
	NoteData::_all_tracks_iterator<ND, iter, TN>::_all_tracks_iterator( ND &nd, int iStartRow, int iEndRow, bool bReverse, bool bInclusive ) :
	m_pNoteData(&nd), m_iTrack(0), m_bReverse(bReverse)
{
	ASSERT( m_pNoteData->GetNumTracks() > 0 );

	m_StartRow= iStartRow;
	m_EndRow= iEndRow;

	for( int iTrack = 0; iTrack < m_pNoteData->GetNumTracks(); ++iTrack )
	{
		iter begin, end;
		m_Inclusive= bInclusive;
		if( bInclusive )
			m_pNoteData->GetTapNoteRangeInclusive( iTrack, iStartRow, iEndRow, begin, end );
		else
			m_pNoteData->GetTapNoteRange( iTrack, iStartRow, iEndRow, begin, end );

		m_vBeginIters.push_back( begin );
		m_vEndIters.push_back( end );
		m_PrevCurrentRows.push_back(0);

		iter cur;
		if( m_bReverse )
		{
			cur = end;
			if( cur != begin )
				cur--;
		}
		else
		{
			cur = begin;
		}
		m_vCurrentIters.push_back( cur );
	}
	m_pNoteData->AddATIToList(this);

	Find( bReverse );
}

template<typename ND, typename iter, typename TN>
NoteData::_all_tracks_iterator<ND, iter, TN>::_all_tracks_iterator( const _all_tracks_iterator &other ) :
#define COPY_OTHER( x ) x( other.x )
	COPY_OTHER( m_pNoteData ),
	COPY_OTHER( m_vBeginIters ),
	COPY_OTHER( m_vCurrentIters ),
	COPY_OTHER( m_vEndIters ),
	COPY_OTHER( m_iTrack ),
	COPY_OTHER( m_bReverse ),
	COPY_OTHER( m_PrevCurrentRows ),
	COPY_OTHER( m_StartRow ),
	COPY_OTHER( m_EndRow )
#undef COPY_OTHER
{
	m_pNoteData->AddATIToList(this);
}

template<typename ND, typename iter, typename TN>
	NoteData::_all_tracks_iterator<ND, iter, TN>::~_all_tracks_iterator()
{
	if(m_pNoteData != nullptr)
	{
		m_pNoteData->RemoveATIFromList(this);
	}
}

template<typename ND, typename iter, typename TN>
NoteData::_all_tracks_iterator<ND, iter, TN> &NoteData::_all_tracks_iterator<ND, iter, TN>::operator++() // preincrement
{
	m_PrevCurrentRows[m_iTrack]= Row();
	if( m_bReverse )
	{
		if( m_vCurrentIters[m_iTrack] == m_vBeginIters[m_iTrack] )
			m_vCurrentIters[m_iTrack] = m_vEndIters[m_iTrack];
		else
			--m_vCurrentIters[m_iTrack];
	}
	else
	{
		++m_vCurrentIters[m_iTrack];
	}
	Find( m_bReverse );
	return *this;
}

template<typename ND, typename iter, typename TN>
NoteData::_all_tracks_iterator<ND, iter, TN> NoteData::_all_tracks_iterator<ND, iter, TN>::operator++( int ) // postincrement
{
	_all_tracks_iterator<ND, iter, TN> ret( *this );
	operator++();
	return ret;
}

template<typename ND, typename iter, typename TN>
	void NoteData::_all_tracks_iterator<ND, iter, TN>::Revalidate(
		ND* notedata, vector<int> const& added_or_removed_tracks, bool added)
{
	m_pNoteData= notedata;
	ASSERT( m_pNoteData->GetNumTracks() > 0 );
	if(!added_or_removed_tracks.empty())
	{
		if(added)
		{
			int avg_row= 0;
			for(size_t p= 0; p < m_PrevCurrentRows.size(); ++p)
			{
				avg_row+= m_PrevCurrentRows[p];
			}
			avg_row/= m_PrevCurrentRows.size();
			for(size_t a= 0; a < added_or_removed_tracks.size(); ++a)
			{
				int track_id= added_or_removed_tracks[a];
				m_PrevCurrentRows.insert(m_PrevCurrentRows.begin()+track_id, avg_row);
			}
			m_vBeginIters.resize(m_pNoteData->GetNumTracks());
			m_vCurrentIters.resize(m_pNoteData->GetNumTracks());
			m_vEndIters.resize(m_pNoteData->GetNumTracks());
		}
		else
		{
			for(size_t a= 0; a < added_or_removed_tracks.size(); ++a)
			{
				int track_id= added_or_removed_tracks[a];
				m_PrevCurrentRows.erase(m_PrevCurrentRows.begin()+track_id);
			}
			m_vBeginIters.resize(m_pNoteData->GetNumTracks());
			m_vCurrentIters.resize(m_pNoteData->GetNumTracks());
			m_vEndIters.resize(m_pNoteData->GetNumTracks());
		}
	}
	for(int track= 0; track < m_pNoteData->GetNumTracks(); ++track)
	{
		iter begin, end;
		if(m_Inclusive)
		{
			m_pNoteData->GetTapNoteRangeInclusive(track, m_StartRow, m_EndRow, begin, end);
		}
		else
		{
			m_pNoteData->GetTapNoteRange(track, m_StartRow, m_EndRow, begin, end);
		}
		m_vBeginIters[track]= begin;
		m_vEndIters[track]= end;
		iter cur;
		if(m_bReverse)
		{
			cur= m_pNoteData->upper_bound(track, m_PrevCurrentRows[track]);
		}
		else
		{
			cur= m_pNoteData->lower_bound(track, m_PrevCurrentRows[track]);
		}
		m_vCurrentIters[track]= cur;
	}
	Find(m_bReverse);
}

/* XXX: This doesn't satisfy the requirements that ++iter; --iter; is a no-op so it cannot be bidirectional for now. */
#if 0
template<typename ND, typename iter, typename TN>
NoteData::_all_tracks_iterator<ND, iter, TN> &NoteData::_all_tracks_iterator<ND, iter, TN>::operator--() // predecrement
{
	if( m_bReverse )
	{
		++m_vCurrentIters[m_iTrack];
	}
	else
	{
		if( m_vCurrentIters[m_iTrack] == m_vEndIters[m_iTrack] )
			m_vCurrentIters[m_iTrack] = m_vEndIters[m_iTrack];
		else
			--m_vCurrentIters[m_iTrack];
	}
	Find( !m_bReverse );
	return *this;
}

template<typename ND, typename iter, typename TN>
NoteData::_all_tracks_iterator<ND, iter, TN> NoteData::_all_tracks_iterator<ND, iter, TN>::operator--( int dummy ) // postdecrement
{
	_all_tracks_iterator<ND, iter, TN> ret( *this );
	operator--();
	return ret;
}
#endif

// Explicit instantiation.
template class NoteData::_all_tracks_iterator<NoteData, NoteData::iterator, TapNote>;
template class NoteData::_all_tracks_iterator<const NoteData, NoteData::const_iterator, const TapNote>;

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

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

int NoteData::GetNumTracks() const
{
	return m_TapNotes.size();
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


/* Clear [iNoteIndexBegin,iNoteIndexEnd]; that is, including iNoteIndexEnd. */
void NoteData::ClearRange( int iNoteIndexBegin, int iNoteIndexEnd )
{
	this->ConvertHoldNotesTo4s();
	for( unsigned t=0; t<GetNumTracks(); t++ )
	{
		FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE( *this, t, i, iNoteIndexBegin, iNoteIndexEnd )
			SetTapNote(t, i, TAP_EMPTY);
	}
	this->Convert4sToHoldNotes();
}

void NoteData::ClearAll()
{
	for( unsigned t=0; t<GetNumTracks(); t++ )
		m_TapNotes[t].clear();
	m_HoldNotes.clear();
}

/* Copy a range from pFrom to this.  (Note that this does *not* overlay;
 * all data in the range is overwritten.) */
void NoteData::CopyRange( const NoteData* pFrom, int iFromIndexBegin, int iFromIndexEnd, int iToIndexBegin )
{
	ASSERT( pFrom->GetNumTracks() == GetNumTracks() );

	NoteData From, To;
	From.To4s( *pFrom );
	To.To4s( *this );

	// copy recorded TapNotes
	
	for( int t=0; t<GetNumTracks(); t++ )
	{
		FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE( From, t, from, iFromIndexBegin, iFromIndexEnd )
		{
			int to = iToIndexBegin + from - iFromIndexBegin;

			TapNote tn = From.GetTapNote( t, from );
			if( tn.type == TapNote::attack )
			{
				Attack attack = From.GetAttackAt( t, from );
				To.SetTapAttackNote( t, to, attack );
			}
			else
			{
				To.SetTapNote( t, to, tn );
			}
		}
	}

	this->From4s( To );
}

void NoteData::Config( const NoteData &From )
{
	SetNumTracks( From.GetNumTracks() );
}

void NoteData::CopyAll( const NoteData* pFrom )
{
	Config(*pFrom);
	ClearAll();

	for( int c=0; c<GetNumTracks(); c++ )
		m_TapNotes[c] = pFrom->m_TapNotes[c];
	m_HoldNotes = pFrom->m_HoldNotes;
	m_AttackMap = pFrom->m_AttackMap;
}

bool NoteData::IsRowEmpty( int index ) const
{
	for( int t=0; t<GetNumTracks(); t++ )
		if( GetTapNoteX(t, index).type != TapNote::empty )
			return false;
	return true;
}

bool NoteData::IsRangeEmpty( int track, int iIndexBegin, int iIndexEnd ) const
{
	ASSERT( track < GetNumTracks() );

	FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE( *this, track, i, iIndexBegin, iIndexEnd )
		if( GetTapNoteX(track,i).type != TapNote::empty )
			return false;
	return true;
}

int NoteData::GetNumTapNonEmptyTracks( int index ) const
{
	int iNum = 0;
	for( int t=0; t<GetNumTracks(); t++ )
		if( GetTapNote(t, index).type != TapNote::empty )
			iNum++;
	return iNum;
}

void NoteData::GetTapNonEmptyTracks( int index, set<int>& addTo ) const
{
	for( int t=0; t<GetNumTracks(); t++ )
		if( GetTapNote(t, index).type != TapNote::empty )
			addTo.insert(t);
}

int NoteData::GetFirstNonEmptyTrack( int index ) const
{
	for( int t=0; t<GetNumTracks(); t++ )
		if( GetTapNoteX( t, index ).type != TapNote::empty )
			return t;
	return -1;
}

int NoteData::GetNumTracksWithTap( int index ) const
{
	int iNum = 0;
	for( int t=0; t<GetNumTracks(); t++ )
	{
		TapNote tn = GetTapNoteX( t, index );
		if( tn.type == TapNote::tap )
			iNum++;
	}
	return iNum;
}

int NoteData::GetNumTracksWithTapOrHoldHead( int index ) const
{
	int iNum = 0;
	for( int t=0; t<GetNumTracks(); t++ )
	{
		TapNote tn = GetTapNoteX( t, index );
		if( tn.type == TapNote::tap || tn.type == TapNote::hold_head )
			iNum++;
	}
	return iNum;
}

int NoteData::GetFirstTrackWithTap( int index ) const
{
	for( int t=0; t<GetNumTracks(); t++ )
	{
		TapNote tn = GetTapNoteX( t, index );
		if( tn.type == TapNote::tap )
			return t;
	}
	return -1;
}

int NoteData::GetFirstTrackWithTapOrHoldHead( int index ) const
{
	for( int t=0; t<GetNumTracks(); t++ )
	{
		TapNote tn = GetTapNoteX( t, index );
		if( tn.type == TapNote::tap || tn.type == TapNote::hold_head )
			return t;
	}
	return -1;
}

void NoteData::AddHoldNote( HoldNote add )
{
	ASSERT( add.iStartRow>=0 && add.iEndRow>=0 );

	// look for other hold notes that overlap and merge them
	// XXX: this is done implicitly with 4s, but 4s uses this function.
	// Rework this later.
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

/* Get a hold note with the same track and end row as hn. */
int NoteData::GetMatchingHoldNote( const HoldNote &hn ) const
{
	for( int i=0; i<GetNumHoldNotes(); i++ )	// for each HoldNote
	{
		const HoldNote &ret = GetHoldNote(i);
		if( ret.iTrack == hn.iTrack && ret.iEndRow == hn.iEndRow )
			return i;
	}
	FAIL_M( ssprintf("%i..%i, %i", hn.iStartRow, hn.iEndRow, hn.iTrack) );
}


void NoteData::SetTapAttackNote( int track, int row, Attack attack )
{
	PruneUnusedAttacksFromMap();

	// find first unused attack index
	for( unsigned i=0; i<MAX_NUM_ATTACKS; i++ )
	{
		if( m_AttackMap.find(i) == m_AttackMap.end() )	// this index is free to use
		{
			m_AttackMap[i] = attack;
			TapNote tn;
			tn.Set( TapNote::attack, TapNote::original, i );
			SetTapNote( track, row, tn );
			return;
		}
	}
	// TODO: need to increase MAX_NUM_ATTACKS or handle "no more room" case
	ASSERT(0);
}

void NoteData::PruneUnusedAttacksFromMap()
{
	// Add all used AttackNote index values to a map.
	set<unsigned> setUsedIndices;

	for( int t=0; t<GetNumTracks(); t++ )
	{
		FOREACH_NONEMPTY_ROW_IN_TRACK( *this, t, r )
		{
			TapNote tn = GetTapNote(t, r);
			if( tn.type == TapNote::attack )
				setUsedIndices.insert( tn.attackIndex );
		}
	}

	// Remove all items from m_AttackMap that don't have corresponding
	// TapNotes in use.
	for( unsigned i=0; i<MAX_NUM_ATTACKS; i++ )
	{
		bool bInAttackMap = m_AttackMap.find(i) != m_AttackMap.end();
		bool bActuallyUsed = setUsedIndices.find(i) != setUsedIndices.end();

		if( bActuallyUsed && !bInAttackMap )
			ASSERT(0);	// something earlier than us didn't enforce consistency 

		if( bInAttackMap && !bActuallyUsed )
			m_AttackMap.erase( i );
	}
}

const Attack& NoteData::GetAttackAt( int track, int row )
{
	TapNote tn = GetTapNote(track, row);
	ASSERT( tn.type == TapNote::attack );	// don't call this if the TapNote here isn't an attack
	map<unsigned,Attack>::iterator iter = m_AttackMap.find( tn.attackIndex );
	ASSERT( iter != m_AttackMap.end() );
	return iter->second;
}


int NoteData::GetFirstRow() const
{ 
	int iEarliestRowFoundSoFar = -1;
	
	for( unsigned t=0; t < GetNumTracks(); t++ )
	{
		const TrackMap &trackMap = m_TapNotes[t];
		TrackMap::const_iterator iter = trackMap.begin();
		if( iter == trackMap.end() )	// trackMap is empty
			continue;
		if( iEarliestRowFoundSoFar == -1 )
			iEarliestRowFoundSoFar = iter->first;
		else
			iEarliestRowFoundSoFar = min( iEarliestRowFoundSoFar, iter->first );
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
	
	for( unsigned t=0; t < GetNumTracks(); t++ )
	{
		const TrackMap &trackMap = m_TapNotes[t];
		const TrackMap::const_reverse_iterator  iter = trackMap.rbegin();
		if( iter == trackMap.rend() )	// trackMap is empty
			continue;
		iOldestRowFoundSoFar = max( iOldestRowFoundSoFar, iter->first );
	}

	for( int i=0; i<GetNumHoldNotes(); i++ )
	{
		if( GetHoldNote(i).iEndRow > iOldestRowFoundSoFar )
			iOldestRowFoundSoFar = GetHoldNote(i).iEndRow;
	}

	return iOldestRowFoundSoFar;
}

int NoteData::GetNumTapNotes( float fStartBeat, float fEndBeat ) const
{
	if( fEndBeat == -1 )
		fEndBeat = 999999;

	int iNumNotes = 0;

	int iStartIndex = BeatToNoteRow( fStartBeat );
	int iEndIndex = BeatToNoteRow( fEndBeat );

	/* Clamp to known-good ranges. */
	iStartIndex = max( iStartIndex, 0 );
	iEndIndex = min( iEndIndex, GetLastRow() );
	
	for( int t=0; t<GetNumTracks(); t++ )
	{
		FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE( *this, t, i, iStartIndex, iEndIndex )
		{
			TapNote tn = GetTapNoteX(t, i);
			if( tn.type != TapNote::empty  &&  tn.type != TapNote::mine )
				iNumNotes++;
		}
	}
	
	return iNumNotes;
}

int NoteData::GetNumRowsWithTap( float fStartBeat, float fEndBeat ) const
{
	if( fEndBeat == -1 ) 
		fEndBeat = GetLastBeat();

	int iNumNotes = 0;

	int iStartIndex = BeatToNoteRow( fStartBeat );
	int iEndIndex = BeatToNoteRow( fEndBeat );
	
	FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( *this, i, iStartIndex, iEndIndex )
		if( IsThereATapAtRow(i) )
			iNumNotes++;
	
	return iNumNotes;
}

int NoteData::GetNumMines( float fStartBeat, float fEndBeat ) const
{
	if( fEndBeat == -1 )
		fEndBeat = 999999;

	int iNumMines = 0;

	int iStartIndex = BeatToNoteRow( fStartBeat );
	int iEndIndex = BeatToNoteRow( fEndBeat );

	/* Clamp to known-good ranges. */
	iStartIndex = max( iStartIndex, 0 );
	iEndIndex = min( iEndIndex, GetLastRow() );
	
	for( int t=0; t<GetNumTracks(); t++ )
	{
		FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE( *this, t, i, iStartIndex, iEndIndex )
			if( GetTapNoteX(t, i).type == TapNote::mine )
				iNumMines++;
	}
	
	return iNumMines;
}

int NoteData::GetNumRowsWithTapOrHoldHead( float fStartBeat, float fEndBeat ) const
{
	if( fEndBeat == -1 )
		fEndBeat = GetLastBeat();

	int iNumNotes = 0;

	int iStartIndex = BeatToNoteRow( fStartBeat );
	int iEndIndex = BeatToNoteRow( fEndBeat );
	
	FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( *this, i, iStartIndex, iEndIndex )
		if( IsThereATapOrHoldHeadAtRow(i) )
			iNumNotes++;
	
	return iNumNotes;
}

int NoteData::RowNeedsHands( const int row ) const
{
	int iNumNotesThisIndex = 0;
	for( int t=0; t<GetNumTracks(); t++ )
	{
		TapNote tn = GetTapNoteX(t, row);
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

int NoteData::GetNumHands( float fStartBeat, float fEndBeat ) const
{
	/* Count the number of times you have to use your hands.  This includes
	 * three taps at the same time, a tap while two hold notes are being held,
	 * etc.  Only count rows that have at least one tap note (hold heads count).
	 * Otherwise, every row of hold notes counts, so three simultaneous hold
	 * notes will count as hundreds of "hands". */
	if( fEndBeat == -1 )
		fEndBeat = GetLastBeat();

	int iStartIndex = BeatToNoteRow( fStartBeat );
	int iEndIndex = BeatToNoteRow( fEndBeat );

	/* Clamp to known-good ranges. */
	iStartIndex = max( iStartIndex, 0 );
	iEndIndex = min( iEndIndex, GetLastRow() );

	int iNum = 0;
	FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( *this, i, iStartIndex, iEndIndex )
	{
		if( !RowNeedsHands(i) )
			continue;

		iNum++;
	}

	return iNum;
}

int NoteData::GetNumN( int MinTaps, float fStartBeat, float fEndBeat ) const
{
	if( fEndBeat == -1 )
		fEndBeat = GetLastBeat();

	int iStartIndex = BeatToNoteRow( fStartBeat );
	int iEndIndex = BeatToNoteRow( fEndBeat );

	/* Clamp to known-good ranges. */
	iStartIndex = max( iStartIndex, 0 );
	iEndIndex = min( iEndIndex, GetLastRow() );

	int iNum = 0;
	FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( *this, i, iStartIndex, iEndIndex )
	{
		int iNumNotesThisIndex = 0;
		for( int t=0; t<GetNumTracks(); t++ )
		{
			TapNote tn = GetTapNoteX(t, i);
			if( tn.type != TapNote::mine  &&  tn.type != TapNote::empty )	// mines don't count
				iNumNotesThisIndex++;
		}
		if( iNumNotesThisIndex >= MinTaps )
			iNum++;
	}
	
	return iNum;
}

int NoteData::GetNumHoldNotes( float fStartBeat, float fEndBeat ) const
{
	if( fEndBeat == -1 )
		fEndBeat = GetLastBeat();

	int iStartIndex = BeatToNoteRow( fStartBeat );
	int iEndIndex = BeatToNoteRow( fEndBeat );

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

	int rows = GetLastRow();
	for( int t=0; t<GetNumTracks(); t++ )	// foreach column
	{
		FOREACH_NONEMPTY_ROW_IN_TRACK( *this, t, i )
		{
			if( GetTapNote(t,i).type != TapNote::hold_head )
				continue;	// skip

			SetTapNote(t, i, TAP_EMPTY);	// clear the hold head marker

			// search for end of HoldNote
			FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE( *this, t, j, i+1, 999999 )
			{
				// End hold on the next note we see.  This should be a hold_tail if the 
				// data is in a consistent state, but doesn't have to be.
				if( GetTapNote(t, j).type == TapNote::empty )
					continue;

				SetTapNote(t, j, TAP_EMPTY);

				AddHoldNote( HoldNote(t, i, j) );
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
		if( hn.iStartRow != hn.iEndRow )
		{
			SetTapNote( hn.iTrack, hn.iStartRow, TAP_ORIGINAL_HOLD_HEAD );
			SetTapNote( hn.iTrack, hn.iEndRow, TAP_ORIGINAL_HOLD_TAIL );
		}
	}
	m_HoldNotes.clear();
}


void NoteData::To2sAnd3s( const NoteData &out )
{
	CopyAll( &out );
	ConvertHoldNotesTo2sAnd3s();
}

void NoteData::From2sAnd3s( const NoteData &out )
{
	CopyAll( &out );
	Convert2sAnd3sToHoldNotes();
}

void NoteData::To4s( const NoteData &out )
{
	CopyAll( &out );
	ConvertHoldNotesTo4s();
}

void NoteData::From4s( const NoteData &out )
{
	CopyAll( &out );
	Convert4sToHoldNotes();
}

/* "104444001" ==
 * "102000301"
 *
 * "4441" basically means "hold for three rows then hold for another tap";
 * since taps don't really have a length, it's equivalent to "4440".
 * So, make sure the character after a 4 is always a 0. */
void NoteData::Convert4sToHoldNotes()
{
	for( int t=0; t<GetNumTracks(); t++ )	// foreach column
	{
		FOREACH_NONEMPTY_ROW_IN_TRACK( *this, t, i )
		{
			if( GetTapNote(t, i).type == TapNote::hold )	// this is a HoldNote body
			{
				HoldNote hn( t, i, 0 );
				// search for end of HoldNote
				do {
					SetTapNote( t, i, TAP_EMPTY );
					i++;
				} while( GetTapNote(t, i).type == TapNote::hold );
				SetTapNote( t, i, TAP_EMPTY );

				hn.iEndRow = i;
				AddHoldNote( hn );
			}
		}
	}
}

void NoteData::ConvertHoldNotesTo4s()
{
	// copy HoldNotes into the new structure, but expand them into 4s
	for( int i=0; i<GetNumHoldNotes(); i++ ) 
	{
		const HoldNote &hn = GetHoldNote(i);
		for( int j = hn.iStartRow; j < hn.iEndRow; ++j)
			SetTapNote(hn.iTrack, j, TAP_ORIGINAL_HOLD);
	}
	m_HoldNotes.clear();
}

// -1 for iOriginalTracksToTakeFrom means no track
void NoteData::LoadTransformed( const NoteData* pOriginal, int iNewNumTracks, const int iOriginalTrackToTakeFrom[] )
{
	// reset all notes
	Init();
	
	NoteData Original;
	Original.To4s( *pOriginal );

	Config( Original );
	SetNumTracks( iNewNumTracks );

	// copy tracks
	for( int t=0; t<GetNumTracks(); t++ )
	{
		const int iOriginalTrack = iOriginalTrackToTakeFrom[t];
		ASSERT_M( iOriginalTrack < Original.GetNumTracks(), ssprintf("from %i >= %i (to %i)", 
			iOriginalTrack, Original.GetNumTracks(), iOriginalTrackToTakeFrom[t]));

		if( iOriginalTrack == -1 )
			continue;
		m_TapNotes[t] = Original.m_TapNotes[iOriginalTrack];
	}

	Convert4sToHoldNotes();

	m_AttackMap = Original.GetAttackMap();
}

void NoteData::PadTapNotes(int rows)
{
	// Nothing to do for a track map.
}

void NoteData::MoveTapNoteTrack(int dest, int src)
{
	if(dest == src) return;
	m_TapNotes[dest] = m_TapNotes[src];
	m_TapNotes[src].clear();
}

void NoteData::SetTapNote( int track, int row, TapNote t )
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
		unsigned numDeleted = trackMap.erase( row );
	}
	else
	{
		PadTapNotes(row);
		m_TapNotes[track][row] = t;
	}
}

void NoteData::ReserveRows( int row )
{
	// Nothing to do for a track map.
}

void NoteData::EliminateAllButOneTap(int row)
{
	if(row < 0) return;

	PadTapNotes(row);

	int track;
	for(track = 0; track < GetNumTracks(); ++track)
	{
		if( m_TapNotes[track][row].type == TapNote::tap )
			break;
	}

	track++;

	for( ; track < GetNumTracks(); ++track)
	{
		if( m_TapNotes[track][row].type == TapNote::tap )
			m_TapNotes[track][row] = TAP_EMPTY;
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
	TrackMap::const_iterator iter = mapTrack.lower_bound( rowInOut+1 );	// "find the first note for which row+1 < key == false"
	if( iter != mapTrack.end() )
	{
		ASSERT( iter->first > rowInOut );
		rowInOut = iter->first;
		return true;
	}
	else
	{
		return false;
	}
}

bool NoteData::GetNextTapNoteRowForAllTracks( int &rowInOut ) const
{
	int iClosestNextRow = 999999;
	bool bAnyHaveNextNote = false;
	for( int t=0; t<GetNumTracks(); t++ )
	{
		int iNewRowThisTrack = rowInOut;
		if( GetNextTapNoteRowForTrack( t, iNewRowThisTrack ) )
		{
			bAnyHaveNextNote = true;
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

#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: NoteData

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "NoteData.h"
#include "RageUtil.h"
#include "RageLog.h"


NoteData::NoteData()
{
	m_iNumTracks = 0;
	Init();
}

void NoteData::Init()
{
	ClearAll();
	m_iNumTracks = 0;	// must do this after calling ClearAll()!
}

NoteData::~NoteData()
{
}

int NoteData::GetNumTracks() const
{
	return m_iNumTracks;
}

void NoteData::SetNumTracks( int iNewNumTracks )
{
	m_iNumTracks = iNewNumTracks;

	// Make sure that all tracks are of the same length
	ASSERT( m_iNumTracks > 0 );
	int rows = m_TapNotes[0].size();

	for( int t=0; t<MAX_NOTE_TRACKS; t++ )
	{
		if( t<m_iNumTracks )
			m_TapNotes[t].resize( rows, TAP_EMPTY );
		else
			m_TapNotes[t].clear();
	}
}


/* Clear [iNoteIndexBegin,iNoteIndexEnd]; that is, including iNoteIndexEnd. */
void NoteData::ClearRange( int iNoteIndexBegin, int iNoteIndexEnd )
{
	this->ConvertHoldNotesTo4s();
	for( int c=0; c<m_iNumTracks; c++ )
	{
		for( int i=iNoteIndexBegin; i <= iNoteIndexEnd; i++ )
			SetTapNote(c, i, TAP_EMPTY);
	}
	this->Convert4sToHoldNotes();
}

void NoteData::ClearAll()
{
	for( int t=0; t<m_iNumTracks; t++ )
		m_TapNotes[t].clear();
	m_HoldNotes.clear();
}

/* Copy a range from pFrom to this.  (Note that this does *not* overlay;
 * all data in the range is overwritten.) */
void NoteData::CopyRange( const NoteData* pFrom, int iFromIndexBegin, int iFromIndexEnd, int iToIndexBegin )
{
	ASSERT( pFrom->m_iNumTracks == m_iNumTracks );

	if( iToIndexBegin == -1 )
		iToIndexBegin = 0;

	NoteData From, To;
	From.To4s( *pFrom );
	To.To4s( *this );

	// copy recorded TapNotes
	int f = iFromIndexBegin, t = iToIndexBegin;
	
	while( f<=iFromIndexEnd )
	{
		for( int c=0; c<m_iNumTracks; c++ )
			To.SetTapNote(c, t, From.GetTapNote(c, f));
		f++;
		t++;
	}

	this->From4s( To );
}

void NoteData::Config( const NoteData &From )
{
	m_iNumTracks = From.m_iNumTracks;
}

void NoteData::CopyAll( const NoteData* pFrom )
{
	Config(*pFrom);
	ClearAll();

	for( int c=0; c<m_iNumTracks; c++ )
		m_TapNotes[c] = pFrom->m_TapNotes[c];
	m_HoldNotes = pFrom->m_HoldNotes;
}

void NoteData::AddHoldNote( HoldNote add )
{
	ASSERT( add.fStartBeat>=0 && add.fEndBeat>=0 );

	int i;

	// look for other hold notes that overlap and merge them
	// XXX: this is done implicitly with 4s, but 4s uses this function.
	// Rework this later.
	for( i=0; i<GetNumHoldNotes(); i++ )	// for each HoldNote
	{
		HoldNote &other = GetHoldNote(i);
		if( add.iTrack == other.iTrack  &&		// the tracks correspond
			// they overlap
			(
				( other.fStartBeat <= add.fStartBeat && other.fEndBeat >= add.fEndBeat ) ||		// other consumes add
				( other.fStartBeat >= add.fStartBeat && other.fEndBeat <= add.fEndBeat ) ||		// other inside add
				( add.fStartBeat <= other.fStartBeat && other.fStartBeat <= add.fEndBeat ) ||	// other overlaps add
				( add.fStartBeat <= other.fEndBeat && other.fEndBeat <= add.fEndBeat )			// other overlaps add
			)
			)
		{
			add.fStartBeat = min(add.fStartBeat, other.fStartBeat);
			add.fEndBeat   = max(add.fEndBeat, other.fEndBeat);

			// delete this HoldNote
			RemoveHoldNote( i );
			--i;
		}
	}

	int iAddStartIndex = BeatToNoteRow(add.fStartBeat);
	int iAddEndIndex = BeatToNoteRow(add.fEndBeat);

	// delete TapNotes under this HoldNote
	for( i=iAddStartIndex+1; i<=iAddEndIndex; i++ )
		SetTapNote(add.iTrack, i, TAP_EMPTY);

	// add a tap note at the start of this hold
	SetTapNote(add.iTrack, iAddStartIndex, TAP_HOLD_HEAD);		// Hold begin marker.  Don't draw this, but do grade it.

	m_HoldNotes.push_back(add);
}

void NoteData::RemoveHoldNote( int iHoldIndex )
{
	ASSERT( iHoldIndex >= 0  &&  iHoldIndex < GetNumHoldNotes() );

	HoldNote& hn = GetHoldNote(iHoldIndex);

	int iHoldStartIndex = BeatToNoteRow(hn.fStartBeat);

	// delete a tap note at the start of this hold
	SetTapNote(hn.iTrack, iHoldStartIndex, TAP_EMPTY);

	// remove from list
	m_HoldNotes.erase(m_HoldNotes.begin()+iHoldIndex, m_HoldNotes.begin()+iHoldIndex+1);
}

bool NoteData::IsThereATapAtRow( int iRow ) const
{
	for( int t=0; t<m_iNumTracks; t++ )
		if( GetTapNote(t, iRow) != TAP_EMPTY )
			return true;

	return false;
}

int NoteData::GetFirstRow() const
{ 
	return BeatToNoteRow( GetFirstBeat() );
}

float NoteData::GetFirstBeat() const		
{ 
	float fEarliestBeatFoundSoFar = -1;
	
	int i;

	for( i=0; i <= int(m_TapNotes[0].size()); i++ )
	{
		if( !IsRowEmpty(i) )
		{
			fEarliestBeatFoundSoFar = NoteRowToBeat(i);
			break;
		}
	}

	for( i=0; i<GetNumHoldNotes(); i++ )
	{
		if( fEarliestBeatFoundSoFar == -1 ||
			GetHoldNote(i).fStartBeat < fEarliestBeatFoundSoFar )
			fEarliestBeatFoundSoFar = GetHoldNote(i).fStartBeat;
	}

	if( fEarliestBeatFoundSoFar == -1 )	// there are no notes
		return 0;

	return fEarliestBeatFoundSoFar;
}

int NoteData::GetLastRow() const
{ 
	return BeatToNoteRow( GetLastBeat() );
}

float NoteData::GetLastBeat() const
{ 
	float fOldestBeatFoundSoFar = 0;
	
	int i;

	for( i = int(m_TapNotes[0].size()); i>=0; i-- )		// iterate back to front
	{
		if( !IsRowEmpty(i) )
		{
			fOldestBeatFoundSoFar = NoteRowToBeat(i);
			break;
		}
	}

	for( i=0; i<GetNumHoldNotes(); i++ )
	{
		if( GetHoldNote(i).fEndBeat > fOldestBeatFoundSoFar )
			fOldestBeatFoundSoFar = GetHoldNote(i).fEndBeat;
	}

	return fOldestBeatFoundSoFar;
}

int NoteData::GetNumTapNotes( float fStartBeat, float fEndBeat ) const
{
	int iNumNotes = 0;

	if(fEndBeat == -1) fEndBeat = GetMaxBeat();
	int iStartIndex = BeatToNoteRow( fStartBeat );
	int iEndIndex = BeatToNoteRow( fEndBeat );
	
	for( int t=0; t<m_iNumTracks; t++ )
	{
		for( int i=iStartIndex; i<=iEndIndex; i++ )
		{
			if( GetTapNote(t, i) != TAP_EMPTY )
				iNumNotes++;
		}
	}
	
	return iNumNotes;
}

int NoteData::GetNumRowsWithTaps( float fStartBeat, float fEndBeat ) const
{
	int iNumNotes = 0;

	if(fEndBeat == -1) fEndBeat = GetMaxBeat();
	int iStartIndex = BeatToNoteRow( fStartBeat );
	int iEndIndex = BeatToNoteRow( fEndBeat );
	
	for( int i=iStartIndex; i<=iEndIndex; i++ )
		if( IsThereATapAtRow(i) )
			iNumNotes++;
	
	return iNumNotes;
}

int NoteData::GetNumDoubles( float fStartBeat, float fEndBeat ) const
{
	int iNumDoubles = 0;

	if(fEndBeat == -1) fEndBeat = GetMaxBeat();
	int iStartIndex = BeatToNoteRow( fStartBeat );
	int iEndIndex = BeatToNoteRow( fEndBeat );

	for( int i=iStartIndex; i<=iEndIndex; i++ )
	{
		int iNumNotesThisIndex = 0;
		for( int t=0; t<m_iNumTracks; t++ )
		{
			if( GetTapNote(t, i) != TAP_EMPTY )
				iNumNotesThisIndex++;
		}
		if( iNumNotesThisIndex >= 2 )
			iNumDoubles++;
	}
	
	return iNumDoubles;
}

int NoteData::GetNumHoldNotes( float fStartBeat, float fEndBeat ) const
{
	int iNumHolds = 0;

	if(fEndBeat == -1) fEndBeat = GetMaxBeat();
	for( int i=0; i<GetNumHoldNotes(); i++ )
	{
		const HoldNote &hn = GetHoldNote(i);
		if( fStartBeat <= hn.fStartBeat  &&  hn.fEndBeat <= fEndBeat )
			iNumHolds++;
	}
	return iNumHolds;
}

void NoteData::Convert2sAnd3sToHoldNotes()
{
	// Any note will end a hold (not just a TAP_HOLD_TAIL).  This makes parsing DWIs much easier.
	// Plus, allowing tap notes in the middle of a hold doesn't make sense!

	int rows = GetLastRow();
	for( int col=0; col<m_iNumTracks; col++ )	// foreach column
	{
		for( int i=0; i<=rows; i++ )	// foreach TapNote element
		{
			if( GetTapNote(col, i) != TAP_HOLD_HEAD )	// this is a HoldNote begin marker
				continue;
			SetTapNote(col, i, TAP_EMPTY);

			for( int j=i+1; j<=rows; j++ )	// search for end of HoldNote
			{
				// end hold on the next note we see
				if( GetTapNote(col, j) == TAP_EMPTY )
					continue;

				SetTapNote(col, j, TAP_EMPTY);

				AddHoldNote( HoldNote(col, NoteRowToBeat(i), NoteRowToBeat(j)) );
				break;
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
		int iHoldStartIndex = max(BeatToNoteRow(hn.fStartBeat), 0);
		int iHoldEndIndex   = max(BeatToNoteRow(hn.fEndBeat), 0);
		
		/* If they're the same, then they got clamped together, so just ignore it. */
		if(iHoldStartIndex != iHoldEndIndex) {
			SetTapNote(hn.iTrack, iHoldStartIndex, TAP_HOLD_HEAD);
			SetTapNote(hn.iTrack, iHoldEndIndex, TAP_HOLD_TAIL);
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
	int rows = GetLastRow();
	for( int col=0; col<m_iNumTracks; col++ )	// foreach column
	{
		for( int i=0; i<=rows; i++ )	// foreach TapNote element
		{
			if( GetTapNote(col, i) != TAP_HOLD )	// this is a HoldNote body
				continue;

			HoldNote hn( col, NoteRowToBeat(i), 0 );
			// search for end of HoldNote
			do {
				SetTapNote(col, i, TAP_EMPTY);
				i++;
			} while( GetTapNote(col, i) == TAP_HOLD);
			SetTapNote(col, i, TAP_EMPTY);

			hn.fEndBeat = NoteRowToBeat(i);
			AddHoldNote( hn );
		}
	}
}

void NoteData::ConvertHoldNotesTo4s()
{
	// copy HoldNotes into the new structure, but expand them into 4s
	for( int i=0; i<GetNumHoldNotes(); i++ ) 
	{
		const HoldNote &hn = GetHoldNote(i);
		int iHoldStartIndex = max(BeatToNoteRow(hn.fStartBeat), 0);
		int iHoldEndIndex   = max(BeatToNoteRow(hn.fEndBeat), 0);

		for( int j = iHoldStartIndex; j < iHoldEndIndex; ++j)
			SetTapNote(hn.iTrack, j, TAP_HOLD);
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
	m_iNumTracks = iNewNumTracks;

	// copy tracks
	for( int t=0; t<m_iNumTracks; t++ )
	{
		const int iOriginalTrack = iOriginalTrackToTakeFrom[t];
		RAGE_ASSERT_M( iOriginalTrack < Original.m_iNumTracks, ssprintf("from %i >= %i (to %i)", 
			iOriginalTrack, Original.m_iNumTracks, iOriginalTrackToTakeFrom[t]));

		if( iOriginalTrack == -1 )
			continue;
		m_TapNotes[t] = Original.m_TapNotes[iOriginalTrack];
	}

	Convert4sToHoldNotes();
}

void NoteData::LoadOverlapped( const NoteData* pOriginal, int iNewNumTracks )
{
	SetNumTracks( pOriginal->GetNumTracks() );
	CopyRange( pOriginal, 0, pOriginal->GetLastRow(), 0 );

	const int iOriginalTracks = pOriginal->GetNumTracks();
	const int iTracksToOverlap = iOriginalTracks / iNewNumTracks;
	if( iTracksToOverlap )
	{
		for ( int i = 0; i < iOriginalTracks; i++ )
		{
			CombineTracks( i % iNewNumTracks, i );
		}
	}
	SetNumTracks( iNewNumTracks );
}

void NoteData::LoadTransformedSlidingWindow( const NoteData* pOriginal, int iNewNumTracks )
{
	// reset all notes
	Init();

	if( pOriginal->GetNumTracks() > iNewNumTracks )
	{
		/* Use a ifferent algorithm for reducing tracks. */
		LoadOverlapped( pOriginal, iNewNumTracks );
		return;
	}

	NoteData Original;
	Original.To4s( *pOriginal );

	Config(*pOriginal);
	m_iNumTracks = iNewNumTracks;


	int iCurTrackOffset = 0;
	int iTrackOffsetMin = 0;
	int iTrackOffsetMax = abs( iNewNumTracks - Original.m_iNumTracks );
	int bOffsetIncreasing = true;

	int iLastRow = Original.GetLastRow();
	for( int r=0; r<=iLastRow; )
	{
		// copy notes in this measure
		for( int t=0; t<Original.m_iNumTracks; t++ )
		{
			int iOldTrack = t;
			int iNewTrack = (iOldTrack + iCurTrackOffset) % iNewNumTracks;
			this->SetTapNote(iNewTrack, r, Original.GetTapNote(iOldTrack, r));
		}
		r++;

		if( r % (ROWS_PER_MEASURE*4) == 0 )	// adjust sliding window every 4 measures
		{
			// See if there is a hold crossing the beginning of this measure
			bool bHoldCrossesThisMeasure = false;

			if( r )
			for( int t=0; t<=Original.m_iNumTracks; t++ )
			{
				if( Original.GetTapNote(t, r) == TAP_HOLD &&
					Original.GetTapNote(t, r-1) == TAP_HOLD)
				{
					bHoldCrossesThisMeasure = true;
					break;
				}
			}

			// adjust offset
			if( !bHoldCrossesThisMeasure )
			{
				iCurTrackOffset += bOffsetIncreasing ? 1 : -1;
				if( iCurTrackOffset == iTrackOffsetMin  ||  iCurTrackOffset == iTrackOffsetMax )
					bOffsetIncreasing ^= true;
				CLAMP( iCurTrackOffset, iTrackOffsetMin, iTrackOffsetMax );
			}
		}
	}

	Convert4sToHoldNotes();
}

void NoteData::PadTapNotes(int rows)
{
	int needed = rows - m_TapNotes[0].size() + 1;
	if(needed < 0) 
		return;

	needed += 100; /* optimization: give it a little more than it needs */

	for(int track = 0; track < m_iNumTracks; ++track)
		m_TapNotes[track].insert(m_TapNotes[track].end(), needed, TAP_EMPTY);
}

void NoteData::MoveTapNoteTrack(int dest, int src)
{
	if(dest == src) return;
	m_TapNotes[dest] = m_TapNotes[src];
	m_TapNotes[src].clear();
}

void NoteData::SetTapNote(int track, int row, TapNote t)
{
	if(row < 0) return;

	PadTapNotes(row);
	m_TapNotes[track][row]=t;
}

void NoteData::EliminateAllButOneTap(int row)
{
	if(row < 0) return;

	PadTapNotes(row);

	int track;
        for(track = 0; track < m_iNumTracks; ++track)
	{
		if( m_TapNotes[track][row] == TAP_TAP )
			break;
	}

	track++;

	for( ; track < m_iNumTracks; ++track)
	{
		if( m_TapNotes[track][row] == TAP_TAP )
			m_TapNotes[track][row] = TAP_EMPTY;
	}
}

void NoteData::CombineTracks( int iTrackTo, int iTrackFrom )
{
	LOG->Trace("NoteData::CombineTracks( %i , %i )", iTrackTo, iTrackFrom);
	if( iTrackFrom < 0 || iTrackTo < 0 )
		return;

	const int iLastRow = GetMaxRow();
	LOG->Trace("NoteData::CombineTracks - %i rows", iLastRow);

	for( int row = 0; row < iLastRow; ++row )
	{
		const TapNote iStepFrom = m_TapNotes[iTrackFrom][row];
		const TapNote iStepTo = m_TapNotes[iTrackTo][row];

		if( iStepFrom == iStepTo )
		{
			// no reason to combine same steps
			continue;
		}
		if( iStepTo != TAP_EMPTY ) 
		{
			// Mines from "from" track will not knock out any steps in
			// the "to" track, but this behavior is fine, I think...
			continue;
		}
		m_TapNotes[iTrackTo][row] = iStepFrom;
	}

	return;
}

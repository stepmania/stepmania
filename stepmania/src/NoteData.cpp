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
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "ArrowEffects.h"
#include "PrefsManager.h"
#include "GameConstantsAndTypes.h"
#include "RageException.h"
#include "GameState.h"
#include "math.h"
#include "NotesLoaderSM.h"
#include "RageLog.h"


NoteData::NoteData()
{
	Init();
}

void NoteData::Init()
{
	m_iNumTracks = 0;	// must do this before calling ClearAll()!
	ClearAll();
}

NoteData::~NoteData()
{
}

int NoteData::GetNumTracks()
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
void NoteData::CopyRange( NoteData* pFrom, int iFromIndexBegin, int iFromIndexEnd, int iToIndexBegin )
{
	ASSERT( pFrom->m_iNumTracks == m_iNumTracks );

	if( iToIndexBegin == -1 )
		iToIndexBegin = 0;

	pFrom->ConvertHoldNotesTo4s();
	this->ConvertHoldNotesTo4s();

	// copy recorded TapNotes
	int f = iFromIndexBegin, t = iToIndexBegin;
	
	while( f<=iFromIndexEnd )
	{
		for( int c=0; c<m_iNumTracks; c++ )
			SetTapNote(c, t, pFrom->GetTapNote(c, f));
		f++;
		t++;
	}

	pFrom->Convert4sToHoldNotes();
	this->Convert4sToHoldNotes();
}

void NoteData::Config( const NoteData &From )
{
	m_iNumTracks = From.m_iNumTracks;
}

void NoteData::CopyAll( NoteData* pFrom )
{
	Config(*pFrom);
	ClearAll();
	CopyRange( pFrom, 0, pFrom->GetLastRow() );
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
			m_HoldNotes.erase(m_HoldNotes.begin()+i, m_HoldNotes.begin()+i+1);
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

bool NoteData::IsThereANoteAtRow( int iRow ) const
{
	for( int t=0; t<m_iNumTracks; t++ )
		if( GetTapNote(t, iRow) != TAP_EMPTY )
			return true;

// There is a tap note at the beginning of every HoldNote start
//
//	for( int i=0; i<GetNumHoldNotes(); i++ )	// for each HoldNote
//		if( GetHoldNote(i).m_iStartIndex == NoteRowToBeatRow(iRow) )
//			return true;

	return false;
}


int NoteData::GetFirstRow() const
{ 
	return BeatToNoteRow( GetFirstBeat() );
}

float NoteData::GetFirstBeat() const		
{ 
	float fEarliestBeatFoundSoFar = MAX_BEATS;
	
	int i;

	int rows = GetLastRow();
	for( i=0; i<=rows; i++ )
	{
		if( !IsRowEmpty(i) )
		{
			fEarliestBeatFoundSoFar = NoteRowToBeat(i);
			break;
		}
	}

	for( i=0; i<GetNumHoldNotes(); i++ )
		if( GetHoldNote(i).fStartBeat < fEarliestBeatFoundSoFar )
			fEarliestBeatFoundSoFar = GetHoldNote(i).fStartBeat;


	if( fEarliestBeatFoundSoFar == MAX_BEATS )	// there are no notes
		return 0;
	else
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

int NoteData::GetNumTapNotes( const float fStartBeat, const float fEndBeat ) const
{
	int iNumNotes = 0;

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

int NoteData::GetNumDoubles( const float fStartBeat, const float fEndBeat ) const
{
	int iNumDoubles = 0;

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

int NoteData::GetNumHoldNotes( const float fStartBeat, const float fEndBeat ) const
{
	int iNumHolds = 0;

	for( int i=0; i<GetNumHoldNotes(); i++ )
	{
		const HoldNote &hn = GetHoldNote(i);
		if( fStartBeat <= hn.fStartBeat  &&  hn.fEndBeat <= fEndBeat )
			iNumHolds++;
	}
	return iNumHolds;
}

int NoteData::GetPossibleDancePoints()
{
//Each song has a certain number of "Dance Points" assigned to it. For regular arrows, this is 2 per arrow. For freeze arrows, it is 6 per arrow. When you add this all up, you get the maximum number of possible "Dance Points". 
//
//Your "Dance Points" are calculated as follows: 
//
//A "Perfect" is worth 2 points
//A "Great" is worth 1 points
//A "Good" is worth 0 points
//A "Boo" will subtract 4 points
//A "Miss" will subtract 8 points
//An "OK" (Successful Freeze step) will add 6 points
//A "NG" (Unsuccessful Freeze step) is worth 0 points

	return GetNumTapNotes()*TapNoteScoreToDancePoints(TNS_PERFECT) +	// not Marvelous
		   GetNumHoldNotes()*HoldNoteScoreToDancePoints(HNS_OK);
}

/* ConvertHoldNotesTo2sAnd3s also clears m_iHoldNotes;
 * shouldn't this do likewise and set all 2/3's to 0? 
 * -glenn
 *
 * Other code assumes != TAP_EMPTY means there's a tap note, so I changed
 * this to do this. -glenn
 *

 * This code intentially leaves the TAP_HOLD_HEAD behind because a hold head is 
 * treated exactly like a tap note for scoring purposes.

 */
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

				HoldNote hn = { col, NoteRowToBeat(i), NoteRowToBeat(j) };
				AddHoldNote( hn );
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

			HoldNote hn = { col, NoteRowToBeat(i), 0 };
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
void NoteData::LoadTransformed( NoteData* pOriginal, int iNewNumTracks, const int iOriginalTrackToTakeFrom[] )
{
	// reset all notes
	Init();
	
	pOriginal->ConvertHoldNotesTo4s();

	Config(*pOriginal);
	m_iNumTracks = iNewNumTracks;

	// copy tracks
	for( int t=0; t<m_iNumTracks; t++ )
	{
		const int iOriginalTrack = iOriginalTrackToTakeFrom[t];

		ASSERT( iOriginalTrack < pOriginal->m_iNumTracks );



		if( iOriginalTrack == -1 )
			continue;
		m_TapNotes[t] = pOriginal->m_TapNotes[iOriginalTrack];
	}

	pOriginal->Convert4sToHoldNotes();
	Convert4sToHoldNotes();
}
#include "RageLog.h"

void NoteData::LoadTransformedSlidingWindow( NoteData* pOriginal, int iNewNumTracks )
{
	// reset all notes
	Init();

	pOriginal->ConvertHoldNotesTo4s();
	Config(*pOriginal);
	m_iNumTracks = iNewNumTracks;


	int iCurTrackOffset = 0;
	int iTrackOffsetMin = 0;
	int iTrackOffsetMax = abs( iNewNumTracks - pOriginal->m_iNumTracks );
	int bOffsetIncreasing = true;

	int iLastRow = pOriginal->GetLastRow();
	for( int r=0; r<=iLastRow; )
	{
		// copy notes in this measure
		for( int t=0; t<pOriginal->m_iNumTracks; t++ )
		{
			int iOldTrack = t;
			int iNewTrack = (iOldTrack + iCurTrackOffset) % iNewNumTracks;
			this->SetTapNote(iNewTrack, r, pOriginal->GetTapNote(iOldTrack, r));
		}
		r++;

		if( r % (ROWS_PER_MEASURE*4) == 0 )	// adjust sliding window every 4 measures
		{
			// See if there is a hold crossing the beginning of this measure
			bool bHoldCrossesThisMeasure = false;

			if( r )
			for( int t=0; t<=pOriginal->m_iNumTracks; t++ )
			{
				if( pOriginal->GetTapNote(t, r) == TAP_HOLD &&
					pOriginal->GetTapNote(t, r-1) == TAP_HOLD)
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

	pOriginal->Convert4sToHoldNotes();
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

NoteType NoteDataUtil::GetSmallestNoteTypeForMeasure( const NoteData &n, int iMeasureIndex )
{
	const int iMeasureStartIndex = iMeasureIndex * ROWS_PER_MEASURE;
	const int iMeasureLastIndex = (iMeasureIndex+1) * ROWS_PER_MEASURE - 1;

	// probe to find the smallest note type
	NoteType nt;
	for( nt=(NoteType)0; nt<NUM_NOTE_TYPES; nt=NoteType(nt+1) )		// for each NoteType, largest to largest
	{
		float fBeatSpacing = NoteTypeToBeat( nt );
		int iRowSpacing = int(roundf( fBeatSpacing * ROWS_PER_BEAT ));

		bool bFoundSmallerNote = false;
		for( int i=iMeasureStartIndex; i<=iMeasureLastIndex; i++ )	// for each index in this measure
		{
			if( i % iRowSpacing == 0 )
				continue;	// skip
			
			if( !n.IsRowEmpty(i) )
			{
				bFoundSmallerNote = true;
				break;
			}
		}

		if( bFoundSmallerNote )
			continue;	// searching the next NoteType
		else
			break;	// stop searching.  We found the smallest NoteType
	}

	if( nt == NUM_NOTE_TYPES )	// we didn't find one
		return NOTE_TYPE_INVALID;	// well-formed notes created in the editor should never get here
	else
		return nt;
}

void NoteDataUtil::LoadFromSMNoteDataString( NoteData &out, CString sSMNoteData )
{
	/* Clear notes, but keep the same number of tracks. */
	int iNumTracks = out.GetNumTracks();
	out.Init();
	out.SetNumTracks( iNumTracks );

	// strip comments out of sSMNoteData
	while( sSMNoteData.Find("//") != -1 )
	{
		int iIndexCommentStart = sSMNoteData.Find("//");
		int iIndexCommentEnd = sSMNoteData.Find("\n", iIndexCommentStart);
		if( iIndexCommentEnd == -1 )	// comment doesn't have an end?
			sSMNoteData.erase( iIndexCommentStart, 2 );
		else
			sSMNoteData.erase( iIndexCommentStart, iIndexCommentEnd-iIndexCommentStart );
	}

	CStringArray asMeasures;
	split( sSMNoteData, ",", asMeasures, true );	// ignore empty is important
	for( unsigned m=0; m<asMeasures.size(); m++ )	// foreach measure
	{
		CString &sMeasureString = asMeasures[m];
		TrimLeft(sMeasureString);
		TrimRight(sMeasureString);

		CStringArray asMeasureLines;
		split( sMeasureString, "\n", asMeasureLines, true );	// ignore empty is important

		//ASSERT( asMeasureLines.size() == 4  ||
		//	    asMeasureLines.size() == 8  ||
		//	    asMeasureLines.size() == 12  ||
		//	    asMeasureLines.size() == 16 );


		for( unsigned l=0; l<asMeasureLines.size(); l++ )
		{
			CString &sMeasureLine = asMeasureLines[l];
			TrimLeft(sMeasureLine);
			TrimRight(sMeasureLine);

			const float fPercentIntoMeasure = l/(float)asMeasureLines.size();
			const float fBeat = (m + fPercentIntoMeasure) * BEATS_PER_MEASURE;
			const int iIndex = BeatToNoteRow( fBeat );

//			if( m_iNumTracks != sMeasureLine.GetLength() )
//				RageException::Throw( "Actual number of note columns (%d) is different from the NotesType (%d).", m_iNumTracks, sMeasureLine.GetLength() );

			for( int c=0; c<min(sMeasureLine.GetLength(),out.GetNumTracks()); c++ )
			{
				TapNote t;
				switch(sMeasureLine[c])
				{
				case '0': t = TAP_EMPTY; break;
				case '1': t = TAP_TAP; break;
				case '2': t = TAP_HOLD_HEAD; break;
				case '3': t = TAP_HOLD_TAIL; break;
				default: ASSERT(0); t = TAP_EMPTY; break;
				}

				out.SetTapNote(c, iIndex, t);
			}
		}
	}
	out.Convert2sAnd3sToHoldNotes();
}

CString NoteDataUtil::GetSMNoteDataString(NoteData &in)
{
	in.ConvertHoldNotesTo2sAnd3s();

	float fLastBeat = in.GetLastBeat();
	int iLastMeasure = int( fLastBeat/BEATS_PER_MEASURE );

	CString sRet;

	for( int m=0; m<=iLastMeasure; m++ )	// foreach measure
	{
		NoteType nt = GetSmallestNoteTypeForMeasure( in, m );
		int iRowSpacing;
		if( nt == NOTE_TYPE_INVALID )
			iRowSpacing = 1;
		else
			iRowSpacing = int(roundf( NoteTypeToBeat(nt) * ROWS_PER_BEAT ));

		sRet += ssprintf("  // measure %d\n", m+1);

		const int iMeasureStartRow = m * ROWS_PER_MEASURE;
		const int iMeasureLastRow = (m+1) * ROWS_PER_MEASURE - 1;

		for( int r=iMeasureStartRow; r<=iMeasureLastRow; r+=iRowSpacing )
		{
			for( int t=0; t<in.GetNumTracks(); t++ ) {
				char c;
				switch(in.GetTapNote(t, r)) {
				case TAP_EMPTY: c = '0'; break;
				case TAP_TAP:   c = '1'; break;
				case TAP_HOLD_HEAD: c = '2'; break;
				case TAP_HOLD_TAIL: c = '3'; break;
				default: ASSERT(0); c = '0'; break;
				}
				sRet.append(1, c);
			}
			
			sRet.append(1, '\n');
		}

		sRet.append(1, ',');
	}

	in.Convert2sAnd3sToHoldNotes();

	return sRet;
}

float NoteDataUtil::GetRadarValue( const NoteData &in, RadarCategory rv, float fSongSeconds )
{
	switch( rv )
	{
	case RADAR_STREAM:	return GetStreamRadarValue( in, fSongSeconds );
	case RADAR_VOLTAGE:	return GetVoltageRadarValue( in, fSongSeconds );
	case RADAR_AIR:		return GetAirRadarValue( in, fSongSeconds );
	case RADAR_FREEZE:	return GetFreezeRadarValue( in, fSongSeconds );
	case RADAR_CHAOS:	return GetChaosRadarValue( in, fSongSeconds );
	default:	ASSERT(0);  return 0;
	}
}

float NoteDataUtil::GetStreamRadarValue( const NoteData &in, float fSongSeconds )
{
	// density of steps
	int iNumNotes = in.GetNumTapNotes() + in.GetNumHoldNotes();
	float fNotesPerSecond = iNumNotes/fSongSeconds;
	float fReturn = fNotesPerSecond / 7;
	return min( fReturn, 1.0f );
}

float NoteDataUtil::GetVoltageRadarValue( const NoteData &in, float fSongSeconds )
{
	float fAvgBPS = in.GetLastBeat() / fSongSeconds;

	// peak density of steps
	float fMaxDensitySoFar = 0;

	const int BEAT_WINDOW = 8;

	for( int i=0; i<MAX_BEATS; i+=BEAT_WINDOW )
	{
		int iNumNotesThisWindow = in.GetNumTapNotes((float)i,(float)i+BEAT_WINDOW) + in.GetNumHoldNotes((float)i,(float)i+BEAT_WINDOW);
		float fDensityThisWindow = iNumNotesThisWindow/(float)BEAT_WINDOW;
		fMaxDensitySoFar = max( fMaxDensitySoFar, fDensityThisWindow );
	}

	float fReturn = fMaxDensitySoFar*fAvgBPS/10;
	return min( fReturn, 1.0f );
}

float NoteDataUtil::GetAirRadarValue( const NoteData &in, float fSongSeconds )
{
	// number of doubles
	int iNumDoubles = in.GetNumDoubles();
	float fReturn = iNumDoubles / fSongSeconds;
	return min( fReturn, 1.0f );
}

float NoteDataUtil::GetFreezeRadarValue( const NoteData &in, float fSongSeconds )
{
	// number of hold steps
	float fReturn = in.GetNumHoldNotes() / fSongSeconds;
	return min( fReturn, 1.0f );
}

float NoteDataUtil::GetChaosRadarValue( const NoteData &in, float fSongSeconds )
{
	// count number of triplets or 16ths
	int iNumChaosNotes = 0;
	int rows = in.GetLastRow();
	for( int r=0; r<=rows; r++ )
	{
		if( !in.IsRowEmpty(r)  &&  GetNoteType(r) >= NOTE_TYPE_12TH )
			iNumChaosNotes++;
	}

	float fReturn = iNumChaosNotes / fSongSeconds * 0.5f;
	return min( fReturn, 1.0f );
}

void NoteDataUtil::RemoveHoldNotes(NoteData &in)
{
	vector<int> tracks, rows;

	// turn all the HoldNotes into TapNotes
	for( int i=0; i<in.GetNumHoldNotes(); i++ )
	{
		const HoldNote &hn = in.GetHoldNote(i);
		
		tracks.push_back(hn.iTrack);
		rows.push_back(BeatToNoteRow(hn.fStartBeat));
	}

	// Remove all HoldNotes
	while(in.GetNumHoldNotes())
		in.RemoveHoldNote(in.GetNumHoldNotes()-1);

	for(unsigned j = 0; j < tracks.size(); ++j)
		in.SetTapNote(tracks[j], rows[j], TAP_TAP);
}


void NoteDataUtil::Turn( NoteData &in, TurnType tt )
{
	int iTakeFromTrack[MAX_NOTE_TRACKS];	// New track "t" will take from old track iTakeFromTrack[t]

	int t;

	switch( tt )
	{
	case left:
	case right:
		// FIXME: TurnRight does the same thing as TurnLeft.
		// Is there a way to do this withoutn handling each NotesType? -Chris
		switch( GAMESTATE->GetCurrentStyleDef()->m_NotesType )
		{
		case NOTES_TYPE_DANCE_SINGLE:
		case NOTES_TYPE_DANCE_DOUBLE:
		case NOTES_TYPE_DANCE_COUPLE:
			iTakeFromTrack[0] = 2;
			iTakeFromTrack[1] = 0;
			iTakeFromTrack[2] = 3;
			iTakeFromTrack[3] = 1;
			iTakeFromTrack[4] = 6;
			iTakeFromTrack[5] = 4;
			iTakeFromTrack[6] = 7;
			iTakeFromTrack[7] = 5;
			break;
		case NOTES_TYPE_DANCE_SOLO:
			iTakeFromTrack[0] = 5;
			iTakeFromTrack[1] = 4;
			iTakeFromTrack[2] = 0;
			iTakeFromTrack[3] = 3;
			iTakeFromTrack[4] = 1;
			iTakeFromTrack[5] = 2;
			break;
		case NOTES_TYPE_PUMP_SINGLE:
		case NOTES_TYPE_PUMP_COUPLE:
			iTakeFromTrack[0] = 3;
			iTakeFromTrack[1] = 4;
			iTakeFromTrack[2] = 2;
			iTakeFromTrack[3] = 0;
			iTakeFromTrack[4] = 1;
			iTakeFromTrack[5] = 8;
			iTakeFromTrack[6] = 9;
			iTakeFromTrack[7] = 7;
			iTakeFromTrack[8] = 5;
			iTakeFromTrack[9] = 6;
			break;
		case NOTES_TYPE_PUMP_DOUBLE:
			iTakeFromTrack[0] = 8;
			iTakeFromTrack[1] = 9;
			iTakeFromTrack[2] = 7;
			iTakeFromTrack[3] = 5;
			iTakeFromTrack[4] = 6;
			iTakeFromTrack[5] = 3;
			iTakeFromTrack[6] = 4;
			iTakeFromTrack[7] = 2;
			iTakeFromTrack[8] = 0;
			iTakeFromTrack[9] = 1;
			break;
		case NOTES_TYPE_EZ2_SINGLE:
		case NOTES_TYPE_EZ2_DOUBLE:
		case NOTES_TYPE_EZ2_REAL:
			// identity transform.  What should we do here?
			iTakeFromTrack[0] = 0;
			iTakeFromTrack[1] = 1;
			iTakeFromTrack[2] = 2;
			iTakeFromTrack[3] = 3;
			iTakeFromTrack[4] = 4;
			iTakeFromTrack[5] = 5;
			iTakeFromTrack[6] = 6;
			iTakeFromTrack[7] = 7;
			iTakeFromTrack[8] = 8;
			iTakeFromTrack[9] = 9;
			iTakeFromTrack[10]= 10;
			iTakeFromTrack[11]= 11;
			break;
		}
		break;
	case mirror:
		for( t=0; t<in.GetNumTracks(); t++ )
			iTakeFromTrack[t] = in.GetNumTracks()-t-1;
		break;
	case shuffle:
	case super_shuffle:		// use this code to shuffle the HoldNotes
		{
			vector<int> aiTracksLeftToMap;
			for( t=0; t<in.GetNumTracks(); t++ )
				aiTracksLeftToMap.push_back( t );
			
			for( t=0; t<in.GetNumTracks(); t++ )
			{
				int iRandTrackIndex = rand()%aiTracksLeftToMap.size();
				int iRandTrack = aiTracksLeftToMap[iRandTrackIndex];
				aiTracksLeftToMap.erase( aiTracksLeftToMap.begin()+iRandTrackIndex,
										 aiTracksLeftToMap.begin()+iRandTrackIndex+1 );
				iTakeFromTrack[t] = iRandTrack;
			}
		}
		break;
	default:
		ASSERT(0);
	}

	NoteData tempNoteData;	// write into here as we tranform
	tempNoteData.Config(in);

	in.ConvertHoldNotesTo2sAnd3s();

	// transform notes
	int max_row = in.GetLastRow();
	for( t=0; t<in.GetNumTracks(); t++ )
		for( int r=0; r<=max_row; r++ ) 			
			tempNoteData.SetTapNote(t, r, in.GetTapNote(iTakeFromTrack[t], r));

	in.CopyAll( &tempNoteData );		// copy note data from newData back into this
	in.Convert2sAnd3sToHoldNotes();

	if( tt == PlayerOptions::TURN_SUPER_SHUFFLE )
		SuperShuffleTaps( in );
}

void NoteDataUtil::SuperShuffleTaps( NoteData &in )
{
	// We already did the normal shuffling code above, which did a good job
	// of shuffling HoldNotes without creating impossible patterns.
	// Now, go in and shuffle the TapNotes per-row.
	in.ConvertHoldNotesTo4s();

	int max_row = in.GetLastRow();
	for( int r=0; r<=max_row; r++ )
	{
		for( int t1=0; t1<in.GetNumTracks(); t1++ )
		{
			TapNote tn1 = in.GetTapNote(t1, r);
			if( tn1!=TAP_HOLD )	// a tap that is not part of a hold
			{
				// probe for a spot to swap with
				while( 1 )
				{
					int t2 = rand() % in.GetNumTracks();
					TapNote tn2 = in.GetTapNote(t2, r);
					if( tn2!=TAP_HOLD )	// a tap that is not part of a hold
					{
						// swap
						in.SetTapNote(t1, r, tn2);
						in.SetTapNote(t2, r, tn1);
						break;
					}
				}
			}
		}
	}
	in.Convert4sToHoldNotes();
}

void NoteDataUtil::Backwards( NoteData &in )
{
	in.ConvertHoldNotesTo4s();
	int max_row = in.GetLastRow();
	for( int r=0; r<=max_row/2; r++ )
	{
		int iRowEarlier = r;
		int iRowLater = max_row-r;

		for( int t=0; t<in.GetNumTracks(); t++ )
		{
			TapNote tnEarlier = in.GetTapNote(t, iRowEarlier);
			TapNote tnLater = in.GetTapNote(t, iRowLater);
			in.SetTapNote(t, iRowEarlier, tnLater);
			in.SetTapNote(t, iRowLater, tnEarlier);
		}
	}
	in.Convert4sToHoldNotes();
}

void NoteDataUtil::SwapSides( NoteData &in )
{
	in.ConvertHoldNotesTo4s();
	int max_row = in.GetLastRow();
	for( int r=0; r<=max_row; r++ )
	{
		for( int t=0; t<in.GetNumTracks()/2; t++ )
		{
			int iTrackEarlier = t;
			int iTrackLater = in.GetNumTracks()-1-t;

			TapNote tnEarlier = in.GetTapNote(iTrackEarlier, r);
			TapNote tnLater = in.GetTapNote(iTrackLater, r);
			in.SetTapNote(iTrackEarlier, r, tnLater);
			in.SetTapNote(iTrackLater, r, tnEarlier);
		}
	}
	in.Convert4sToHoldNotes();
}

void NoteDataUtil::Little(NoteData &in)
{
	// filter out all non-quarter notes
	int max_row = in.GetLastRow();
	for( int i=0; i<=max_row; i++ ) 
	{
		if( i%ROWS_PER_BEAT != 0 )
		{
			// filter out all non-quarter notes
			for( int c=0; c<in.GetNumTracks(); c++ ) 
			{
				in.SetTapNote(c, i, TAP_EMPTY);
			}
		}
	}

	// leave HoldNotes unchanged (what should be do with them?)
}

void NoteDataUtil::Big( NoteData &in )
{
	in.ConvertHoldNotesTo4s();

	// make all all quarter notes jumps
	int max_row = in.GetLastRow();
	for( int i=0; i<=max_row; i+=ROWS_PER_BEAT ) 
	{
		int iNum = in.GetNumTapNonEmptyTracks(i);
		if( iNum == 1 )
		{
			// add a note determinitsitcally
			int iBeat = (int)roundf( NoteRowToBeat(i) );
			int iTrackOfNote = in.GetFirstNonEmptyTrack(i);
			int iTrackToAdd = iTrackOfNote + (iBeat%5)-2;	// won't be more than 2 tracks away from the existing note
			CLAMP( iTrackToAdd, 0, in.GetNumTracks()-1 );
			if( iTrackToAdd == iTrackOfNote )
				iTrackToAdd++;
			CLAMP( iTrackToAdd, 0, in.GetNumTracks()-1 );
			if( iTrackToAdd == iTrackOfNote )
				iTrackToAdd--;
			CLAMP( iTrackToAdd, 0, in.GetNumTracks()-1 );

			if( in.GetTapNote(iTrackToAdd, i) != TAP_EMPTY )
			{
				iTrackToAdd = (iTrackToAdd+1) % in.GetNumTracks();
			}
			in.SetTapNote(iTrackToAdd, i, TAP_TAP);
		}
	}

	in.Convert4sToHoldNotes();
}


void NoteDataUtil::SnapToNearestNoteType( NoteData &in, NoteType nt1, NoteType nt2, float fBeginBeat, float fEndBeat )
{
	// nt2 is optional and should be -1 if it is not used

	float fSnapInterval1, fSnapInterval2;
	switch( nt1 )
	{
		case NOTE_TYPE_4TH:		fSnapInterval1 = 1/1.0f;	break;
		case NOTE_TYPE_8TH:		fSnapInterval1 = 1/2.0f;	break;
		case NOTE_TYPE_12TH:	fSnapInterval1 = 1/3.0f;	break;
		case NOTE_TYPE_16TH:	fSnapInterval1 = 1/4.0f;	break;
		default:	ASSERT( false );						return;
	}

	switch( nt2 )
	{
		case NOTE_TYPE_4TH:		fSnapInterval2 = 1/1.0f;	break;
		case NOTE_TYPE_8TH:		fSnapInterval2 = 1/2.0f;	break;
		case NOTE_TYPE_12TH:	fSnapInterval2 = 1/3.0f;	break;
		case NOTE_TYPE_16TH:	fSnapInterval2 = 1/4.0f;	break;
		case -1:				fSnapInterval2 = 10000;		break;	// nothing will ever snap to this.  That's what we want!
		default:	ASSERT( false );						return;
	}

	int iNoteIndexBegin = BeatToNoteRow( fBeginBeat );
	int iNoteIndexEnd = BeatToNoteRow( fEndBeat );

	//ConvertHoldNotesTo2sAnd3s();

	// iterate over all TapNotes in the interval and snap them
	for( int i=iNoteIndexBegin; i<=iNoteIndexEnd; i++ )
	{
		int iOldIndex = i;
		float fOldBeat = NoteRowToBeat( iOldIndex );
		float fNewBeat1 = froundf( fOldBeat, fSnapInterval1 );
		float fNewBeat2 = froundf( fOldBeat, fSnapInterval2 );

		bool bNewBeat1IsCloser = fabsf(fNewBeat1-fOldBeat) < fabsf(fNewBeat2-fOldBeat);
		float fNewBeat = bNewBeat1IsCloser ? fNewBeat1 : fNewBeat2;
		int iNewIndex = BeatToNoteRow( fNewBeat );

		for( int c=0; c<in.GetNumTracks(); c++ )
		{
			TapNote note = in.GetTapNote(c, iOldIndex);
			in.SetTapNote(c, iOldIndex, TAP_EMPTY);
			// HoldNotes override TapNotes
			if(in.GetTapNote(c, iNewIndex) == TAP_TAP)
				note = TAP_HOLD_HEAD;
			in.SetTapNote(c, iNewIndex, note );
		}
	}

	//Convert2sAnd3sToHoldNotes();
}

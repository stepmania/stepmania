#include "stdafx.h"
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



NoteData::NoteData()
{
	Init();
}

void NoteData::Init()
{
	m_iNumTracks = 0;
	TapRowDivisor = 1;
}

NoteData::~NoteData()
{
}

void NoteData::LoadFromSMNoteDataString( CString sSMNoteData )
{
	int iNumTracks = m_iNumTracks;
	Init();
	m_iNumTracks = iNumTracks;

	// strip comments out of sSMNoteData
	while( sSMNoteData.Find("//") != -1 )
	{
		int iIndexCommentStart = sSMNoteData.Find("//");
		int iIndexCommentEnd = sSMNoteData.Find("\n", iIndexCommentStart);
		if( iIndexCommentEnd == -1 )	// comment doesn't have an end?
			sSMNoteData.Delete( iIndexCommentStart, 2 );
		else
			sSMNoteData.Delete( iIndexCommentStart, iIndexCommentEnd-iIndexCommentStart );
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
//				throw RageException( "Actual number of note columns (%d) is different from the NotesType (%d).", m_iNumTracks, sMeasureLine.GetLength() );

			for( int c=0; c<min(sMeasureLine.GetLength(),m_iNumTracks); c++ )
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

				SetTapNote(c, iIndex, t);
			}
		}
	}
	this->Convert2sAnd3sToHoldNotes();

	Compress();
}

CString NoteData::GetSMNoteDataString()
{
	this->ConvertHoldNotesTo2sAnd3s();

	float fLastBeat = GetLastBeat();
	int iLastMeasure = int( fLastBeat/BEATS_PER_MEASURE );

	CStringArray asMeasureStrings;

	for( int m=0; m<=iLastMeasure; m++ )	// foreach measure
	{
		NoteType nt = GetSmallestNoteTypeForMeasure( m );
		int iRowSpacing;
		if( nt == NOTE_TYPE_INVALID )
			iRowSpacing = 1;
		else
			iRowSpacing = int(roundf( NoteTypeToBeat(nt) * ROWS_PER_BEAT ));

		CStringArray asMeasureLines;
		asMeasureLines.push_back( ssprintf("  // measure %d", m+1) );

		const int iMeasureStartRow = m * ROWS_PER_MEASURE;
		const int iMeasureLastRow = (m+1) * ROWS_PER_MEASURE - 1;

		for( int r=iMeasureStartRow; r<=iMeasureLastRow; r+=iRowSpacing )
		{
			CString szLineString;
			for( int t=0; t<m_iNumTracks; t++ ) {
				char c;
				switch(GetTapNote(t, r)) {
				case TAP_EMPTY: c = '0'; break;
				case TAP_TAP:   c = '1'; break;
				case TAP_HOLD_HEAD: c = '2'; break;
				case TAP_HOLD_TAIL: c = '3'; break;
				default: ASSERT(0); c = '0'; break;
				}
				szLineString.append(1, c);
			}
			
			asMeasureLines.push_back( szLineString );
		}

		CString sMeasureString = join( "\n", asMeasureLines );

		asMeasureStrings.push_back( sMeasureString );
	}

	this->Convert2sAnd3sToHoldNotes();

	return join( "\n,", asMeasureStrings );
}

/* Clear [iNoteIndexBegin,iNoteIndexEnd]; that is, including iNoteIndexEnd. */
void NoteData::ClearRange( int iNoteIndexBegin, int iNoteIndexEnd )
{
	this->ConvertHoldNotesTo4s();
	/* XXX: if iNoteIndexEnd >= m_TapNotes[0].size(), just erase() */
	for( int c=0; c<m_iNumTracks; c++ )
	{
		for( int i=iNoteIndexBegin; i <= iNoteIndexEnd && i < MAX_TAP_NOTE_ROWS; i++ )
			SetTapNote(c, i, TAP_EMPTY);
	}
	this->Convert4sToHoldNotes();
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
	
	while( f<=iFromIndexEnd && f < MAX_TAP_NOTE_ROWS && t < MAX_TAP_NOTE_ROWS )
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
	TapRowDivisor = From.TapRowDivisor;
}

void NoteData::CopyAll( NoteData* pFrom )
{
	Config(*pFrom);
	m_HoldNotes.clear();
	CopyRange( pFrom, 0, MAX_TAP_NOTE_ROWS );
}

void NoteData::AddHoldNote( HoldNote add )
{
	ASSERT( add.m_fStartBeat>=0 && add.m_fEndBeat>=0 );

	int i;

	// look for other hold notes that overlap and merge them
	// XXX: this is done implicitly with 4s, but 4s uses this function.
	// Rework this later.
	for( i=0; i<GetNumHoldNotes(); i++ )	// for each HoldNote
	{
		HoldNote &other = GetHoldNote(i);
		if( add.m_iTrack == other.m_iTrack  &&		// the tracks correspond
			// they overlap
			(
				( other.m_fStartBeat <= add.m_fStartBeat && other.m_fEndBeat >= add.m_fEndBeat ) ||		// other consumes add
				( other.m_fStartBeat >= add.m_fStartBeat && other.m_fEndBeat <= add.m_fEndBeat ) ||		// other inside add
				( add.m_fStartBeat <= other.m_fStartBeat && other.m_fStartBeat <= add.m_fEndBeat ) ||	// other overlaps add
				( add.m_fStartBeat <= other.m_fEndBeat && other.m_fEndBeat <= add.m_fEndBeat )			// other overlaps add
			)
			)
		{
			add.m_fStartBeat = min(add.m_fStartBeat, other.m_fStartBeat);
			add.m_fEndBeat   = max(add.m_fEndBeat, other.m_fEndBeat);

			// delete this HoldNote
			m_HoldNotes.erase(m_HoldNotes.begin()+i, m_HoldNotes.begin()+i+1);
		}
	}

	int iAddStartIndex = BeatToNoteRow(add.m_fStartBeat);
	int iAddEndIndex = BeatToNoteRow(add.m_fEndBeat);

	// delete TapNotes under this HoldNote
	for( i=iAddStartIndex+1; i<=iAddEndIndex; i++ )
		SetTapNote(add.m_iTrack, i, TAP_EMPTY);

	// add a tap note at the start of this hold
	SetTapNote(add.m_iTrack, iAddStartIndex, TAP_HOLD_HEAD);		// Hold begin marker.  Don't draw this, but do grade it.

	m_HoldNotes.push_back(add);
}

void NoteData::RemoveHoldNote( int iHoldIndex )
{
	ASSERT( iHoldIndex >= 0  &&  iHoldIndex < GetNumHoldNotes() );

	HoldNote& hn = GetHoldNote(iHoldIndex);

	int iHoldStartIndex = BeatToNoteRow(hn.m_fStartBeat);

	// delete a tap note at the start of this hold
	SetTapNote(hn.m_iTrack, iHoldStartIndex, TAP_EMPTY);

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


int NoteData::GetFirstRow()
{ 
	return BeatToNoteRow( GetFirstBeat() );
}

float NoteData::GetFirstBeat()			
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
		if( GetHoldNote(i).m_fStartBeat < fEarliestBeatFoundSoFar )
			fEarliestBeatFoundSoFar = GetHoldNote(i).m_fStartBeat;


	if( fEarliestBeatFoundSoFar == MAX_BEATS )	// there are no notes
		return 0;
	else
		return fEarliestBeatFoundSoFar;
}

int NoteData::GetLastRow()
{ 
	return BeatToNoteRow( GetLastBeat() );
}

float NoteData::GetLastBeat()			
{ 
	float fOldestBeatFoundSoFar = 0;
	
	int i;

	for( i = TapRowDivisor * int(m_TapNotes[0].size()); i>=0; i-- )		// iterate back to front
	{
		if( !IsRowEmpty(i) )
		{
			fOldestBeatFoundSoFar = NoteRowToBeat(i);
			break;
		}
	}

	for( i=0; i<GetNumHoldNotes(); i++ )
	{
		if( GetHoldNote(i).m_fEndBeat > fOldestBeatFoundSoFar )
			fOldestBeatFoundSoFar = GetHoldNote(i).m_fEndBeat;
	}

	return fOldestBeatFoundSoFar;
}

int NoteData::GetNumTapNotes( const float fStartBeat, const float fEndBeat )			
{ 
	int iNumNotes = 0;

	int iStartIndex = BeatToNoteRow( fStartBeat );
	int iEndIndex = BeatToNoteRow( fEndBeat );

	for( int i=iStartIndex; i<min(iEndIndex, MAX_TAP_NOTE_ROWS); i++ )
	{
		for( int t=0; t<m_iNumTracks; t++ )
			if( GetTapNote(t, i) != TAP_EMPTY )
				iNumNotes++;
	}
	
	return iNumNotes;
}

int NoteData::GetNumDoubles( const float fStartBeat, const float fEndBeat )			
{
	int iNumDoubles = 0;

	int iStartIndex = BeatToNoteRow( fStartBeat );
	int iEndIndex = BeatToNoteRow( fEndBeat );

	for( int i=iStartIndex; i<min(iEndIndex, MAX_TAP_NOTE_ROWS); i++ )
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

int NoteData::GetNumHoldNotes( const float fStartBeat, const float fEndBeat )			
{
	int iNumHolds = 0;

	for( int i=0; i<GetNumHoldNotes(); i++ )
	{
		const HoldNote &hn = GetHoldNote(i);
		if( fStartBeat <= hn.m_fStartBeat  &&  hn.m_fEndBeat <= fEndBeat )
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

	return GetNumTapNotes()*TapNoteScoreToDancePoints(TNS_PERFECT) +
		   GetNumHoldNotes()*HoldNoteScoreToDancePoints(HNS_OK);
}

void NoteData::CropToLeftSide()
{
	int iFirstRightSideColumn = 4;
	int iLastRightSideColumn = 7;

	// clear out the right half
	int c;
	for( c=iFirstRightSideColumn; c<=iLastRightSideColumn; c++ )
	{
		for( int i=0; i<MAX_TAP_NOTE_ROWS; i++ ) // foreach TapNote
			SetTapNote(c, i, TAP_EMPTY);
	}

	for( int i=GetNumHoldNotes()-1; i>=0; i-- ) // foreach HoldNote
	{
		if( c >= iFirstRightSideColumn )
		{
			// delete this HoldNote
			m_HoldNotes.erase(m_HoldNotes.begin()+i, m_HoldNotes.begin()+i+1);
		}
	}
}


void NoteData::CropToRightSide()
{
	int iFirstRightSideColumn = 4;
	int iLastRightSideColumn = 7;

	int c;

	// clear out the left half
	for( c=0; c<iFirstRightSideColumn; c++ )
	{
		for( int i=0; i<MAX_TAP_NOTE_ROWS; i++ ) // foreach TapNote
			SetTapNote(c, i, TAP_EMPTY);
	}

	// copy the right side into the left
	for( c=iFirstRightSideColumn; c<=iLastRightSideColumn; c++ )
	{
		for( int i=0; i<MAX_TAP_NOTE_ROWS; i++ ) // foreach TapNote
		{
			SetTapNote(c-4, i, GetTapNote(c, i));
			SetTapNote(c, i, TAP_EMPTY);
		}
	}

	for( int i=GetNumHoldNotes()-1; i>=0; i-- ) // foreach HoldNote
	{
		HoldNote &hn = GetHoldNote(i);

		if( hn.m_iTrack < iFirstRightSideColumn )
		{
			// delete this HoldNote by shifting everything down
			RemoveHoldNote( i );
		}
		else
		{
			hn.m_iTrack -= 4;
		}
	}
}

void NoteData::RemoveHoldNotes()
{
	// turn all the HoldNotes into TapNotes
	for( int i=0; i<GetNumHoldNotes(); i++ )
	{
		const HoldNote &hn = GetHoldNote(i);
		
		int iIndex = BeatToNoteRow(hn.m_fStartBeat);
		int iCol = hn.m_iTrack;

		SetTapNote(iCol, iIndex, TAP_TAP);
	}

	// Remove all HoldNotes
	m_HoldNotes.clear();
}


void NoteData::Turn( PlayerOptions::TurnType tt )
{
	int iTakeFromTrack[MAX_NOTE_TRACKS];	// New track "t" will take from old track iTakeFromTrack[t]

	int t;

	switch( tt )
	{
	case PlayerOptions::TURN_NONE:
		return;		// nothing to do
	case PlayerOptions::TURN_MIRROR:
		for( t=0; t<m_iNumTracks; t++ )
			iTakeFromTrack[t] = m_iNumTracks-t-1;
		break;
	case PlayerOptions::TURN_LEFT:
	case PlayerOptions::TURN_RIGHT:		// HACK: TurnRight does the same thing as TurnLeft.  I'll fix this later...
		// Chris: Handling each NotesType case is a terrible way to do this, but oh well...
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
	case PlayerOptions::TURN_SHUFFLE:
	case PlayerOptions::TURN_SUPER_SHUFFLE:		// use this code to shuffle the HoldNotes
		{
			CArray<int,int> aiTracksLeftToMap;
			for( t=0; t<m_iNumTracks; t++ )
				aiTracksLeftToMap.push_back( t );
			
			for( t=0; t<m_iNumTracks; t++ )
			{
				int iRandTrackIndex = rand()%aiTracksLeftToMap.size();
				int iRandTrack = aiTracksLeftToMap[iRandTrackIndex];
				aiTracksLeftToMap.erase( aiTracksLeftToMap.begin()+iRandTrackIndex,
										 aiTracksLeftToMap.begin()+iRandTrackIndex+1 );
				iTakeFromTrack[t] = iRandTrack;
			}
		}
		break;
		// handle this below
		break;
	default:
		ASSERT(0);
	}

	NoteData tempNoteData;	// write into here as we tranform
	tempNoteData.Config(*this);

	this->ConvertHoldNotesTo2sAnd3s();

	// transform notes
	for( t=0; t<m_iNumTracks; t++ )
		for( int r=0; r<MAX_TAP_NOTE_ROWS; r++ ) 			
			tempNoteData.SetTapNote(t, r, GetTapNote(iTakeFromTrack[t], r));

	this->CopyAll( &tempNoteData );		// copy note data from newData back into this
	this->Convert2sAnd3sToHoldNotes();





	if( tt == PlayerOptions::TURN_SUPER_SHUFFLE )
	{
		// We already did the normal shuffling code above, which did a good job
		// of shuffling HoldNotes without creating impossible patterns.
		// Now, go in and shuffle the TapNotes some more.

		// clear tempNoteData because we're going to use it as a scratch buffer again
		tempNoteData.Init();
		tempNoteData.Config(*this);

		// copy all HoldNotes before copying taps
		for( int i=0; i<this->GetNumHoldNotes(); i++ )
			tempNoteData.AddHoldNote( this->GetHoldNote(i) );

		this->ConvertHoldNotesTo4s();

		for( int r=0; r<=this->GetLastRow(); r++ )	// foreach row
		{
			if( this->IsRowEmpty(r) )
				continue;	// no need to super shuffle this row

			// shuffle this row
			CArray<int,int> aiTracksThatCouldHaveTapNotes;
			for( t=0; t<m_iNumTracks; t++ )
				if( GetTapNote(t, r) != TAP_HOLD )	// any point that isn't part of a hold
					aiTracksThatCouldHaveTapNotes.push_back( t );

			for( t=0; t<m_iNumTracks; t++ )
			{
				if( GetTapNote(t, r) != TAP_HOLD  &&  GetTapNote(t, r) != TAP_EMPTY )	// there is a tap note here (and not a HoldNote)
				{
					int iRandIndex = rand() % aiTracksThatCouldHaveTapNotes.size();
					int iTo = aiTracksThatCouldHaveTapNotes[ iRandIndex ];
					aiTracksThatCouldHaveTapNotes.erase( aiTracksThatCouldHaveTapNotes.begin()+iRandIndex,
													     aiTracksThatCouldHaveTapNotes.begin()+iRandIndex+1 );
	
					tempNoteData.SetTapNote(iTo, r, GetTapNote(t, r));
				}
			}
		}

		this->CopyAll( &tempNoteData );		// copy note data from newData back into this
	}
}

void NoteData::MakeLittle()
{
	// filter out all non-quarter notes
	for( int i=0; i<MAX_TAP_NOTE_ROWS; i++ ) 
	{
		if( i%ROWS_PER_BEAT != 0 )
		{
			// filter out all non-quarter notes
			for( int c=0; c<m_iNumTracks; c++ ) 
			{
				SetTapNote(c, i, TAP_EMPTY);
			}
		}
	}

	// leave HoldNotes unchanged (what should be do with them?)
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
		int iHoldStartIndex = clamp(BeatToNoteRow(hn.m_fStartBeat), 0, MAX_TAP_NOTE_ROWS-1);
		int iHoldEndIndex   = clamp(BeatToNoteRow(hn.m_fEndBeat), 0, MAX_TAP_NOTE_ROWS-1);
		
		/* If they're the same, then they got clamped together, so just ignore it. */
		if(iHoldStartIndex != iHoldEndIndex) {
			SetTapNote(hn.m_iTrack, iHoldStartIndex, TAP_HOLD_HEAD);
			SetTapNote(hn.m_iTrack, iHoldEndIndex, TAP_HOLD_TAIL);
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

			hn.m_fEndBeat = NoteRowToBeat(i);
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
		int iHoldStartIndex = clamp(BeatToNoteRow(hn.m_fStartBeat), 0, MAX_TAP_NOTE_ROWS);
		int iHoldEndIndex   = clamp(BeatToNoteRow(hn.m_fEndBeat), 0, MAX_TAP_NOTE_ROWS);

		for( int j = iHoldStartIndex; j < iHoldEndIndex; ++j)
			SetTapNote(hn.m_iTrack, j, TAP_HOLD);
	}
	m_HoldNotes.clear();
}

void NoteData::SnapToNearestNoteType( NoteType nt1, NoteType nt2, float fBeginBeat, float fEndBeat )
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

		for( int c=0; c<m_iNumTracks; c++ )
		{
			TapNote note = GetTapNote(c, iOldIndex);
			SetTapNote(c, iOldIndex, TAP_EMPTY);
			// HoldNotes override TapNotes
			if(GetTapNote(c, iNewIndex) == TAP_TAP)
				note = TAP_HOLD_HEAD;
			SetTapNote(c, iNewIndex, note );
		}
	}

	//Convert2sAnd3sToHoldNotes();
}


float NoteData::GetStreamRadarValue( float fSongSeconds )
{
	// density of steps
	int iNumNotes = GetNumTapNotes() + GetNumHoldNotes();
	float fNotesPerSecond = iNumNotes/fSongSeconds;
	float fReturn = fNotesPerSecond / 7;
	return min( fReturn, 1.0f );
}

float NoteData::GetVoltageRadarValue( float fSongSeconds )
{
	float fAvgBPS = GetLastBeat() / fSongSeconds;

	// peak density of steps
	float fMaxDensitySoFar = 0;

	const int BEAT_WINDOW = 8;

	for( int i=0; i<MAX_BEATS; i+=BEAT_WINDOW )
	{
		int iNumNotesThisWindow = GetNumTapNotes((float)i,(float)i+BEAT_WINDOW) + GetNumHoldNotes((float)i,(float)i+BEAT_WINDOW);
		float fDensityThisWindow = iNumNotesThisWindow/(float)BEAT_WINDOW;
		fMaxDensitySoFar = max( fMaxDensitySoFar, fDensityThisWindow );
	}

	float fReturn = fMaxDensitySoFar*fAvgBPS/10;
	return min( fReturn, 1.0f );
}

float NoteData::GetAirRadarValue( float fSongSeconds )
{
	// number of doubles
	int iNumDoubles = GetNumDoubles();
	float fReturn = iNumDoubles / fSongSeconds;
	return min( fReturn, 1.0f );
}

float NoteData::GetChaosRadarValue( float fSongSeconds )
{
	// count number of triplets or 16ths
	int iNumChaosNotes = 0;
	int rows = GetLastRow();
	for( int r=0; r<=rows; r++ )
	{
		if( !IsRowEmpty(r)  &&  GetNoteType(r) >= NOTE_TYPE_12TH )
			iNumChaosNotes++;
	}

	float fReturn = iNumChaosNotes / fSongSeconds * 0.5f;
	return min( fReturn, 1.0f );
}

float NoteData::GetFreezeRadarValue( float fSongSeconds )
{
	// number of hold steps
	float fReturn = GetNumHoldNotes() / fSongSeconds;
	return min( fReturn, 1.0f );
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
	for( int r=0; r<=iLastRow; r++ )
	{
		// copy notes in this measure
		for( int t=0; t<pOriginal->m_iNumTracks; t++ )
		{
			int iOldTrack = t;
			int iNewTrack = (iOldTrack + iCurTrackOffset) % iNewNumTracks;
			if( pOriginal->GetTapNote(iOldTrack, r) != TAP_EMPTY )
				this->SetTapNote(iNewTrack, r, pOriginal->GetTapNote(iOldTrack, r));
		}

		if( r % (ROWS_PER_MEASURE*4) == 0 )	// adjust sliding window every 4 measures
		{
			// See if there is a hold crossing the beginning of this measure
			bool bHoldCrossesThisMeasure = false;
#if 0
			pOriginal->Convert4sToHoldNotes();
			for( int i=0; i<pOriginal->GetNumHoldNotes(); i++ )
			{
				const HoldNote& hn = pOriginal->GEtHoldNote(i);
				/* Bug here: NoteRowToBeat() may have floating point error,
				 * so we need to do an epsilon here.  But we can do this in
				 * 4s easier anyway ... XXX -glenn */
				if( hn.m_fStartBeat < NoteRowToBeat(r)  &&  hn.m_fEndBeat > NoteRowToBeat(r) )
				{
					bHoldCrossesThisMeasure = true;
					break;
				}
			}
			pOriginal->ConvertHoldNotesTo4s();
#endif

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


NoteType NoteData::GetSmallestNoteTypeForMeasure( int iMeasureIndex )
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
			
			if( !IsRowEmpty(i) )
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

void NoteData::PadTapNotes(int rows)
{
	int needed = rows - m_TapNotes[0].size() + 1;
	if(needed < 0) return;
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

	Decompress();

	PadTapNotes(row);
	m_TapNotes[track][row]=t;
}

void NoteData::SetDivisor(int div)
{
	if(div == TapRowDivisor)
		return;

	int src_step = div,
		dst_step = TapRowDivisor;

	/* I think this should work to go from one divisor to another, but
	 * I havn't tested it and don't see a need. */
	ASSERT(src_step == 1 || dst_step == 1);

	for(int t=0; t < m_iNumTracks; t++)
	{
		vector<TapNote> TapNotes = m_TapNotes[t];
		/* Try to make sure the memory is actually released. */
		int dst_size = m_TapNotes[0].size() * dst_step / src_step;
		m_TapNotes[t].clear();
		m_TapNotes[t] = vector<TapNote>(dst_size, TAP_EMPTY);

		unsigned src_row = 0, dst_row = 0;
		while(src_row < TapNotes.size() && dst_row < m_TapNotes[t].size())
		{
			m_TapNotes[t][dst_row] = TapNotes[src_row];
			dst_row += dst_step;
			src_row += src_step;
		}
	}

	TapRowDivisor = div;
}

void NoteData::Compress()
{
	/* We could optimize a little by being able to compress compressed data,
	 * but let's not bother for now. */
	Decompress();

	/* Find the largest divisor; the largest D such that all notes M such that
	 * (M%D) != 0 are empty. */
	const int max_divisor = 64;
	bool *ok = new bool[max_divisor];
	memset(ok, 1, sizeof(ok));

	int t, row, rows = GetLastRow();
	
	for(t=0; t < m_iNumTracks; t++)
	{
		for(row = 0; row < rows; ++row)
		{
			/* If there's nothing here, then it doesn't make any divisors invalid. */
			if(GetTapNote(t, row) == TAP_EMPTY)
				continue;

			for(int div = 2; div < max_divisor; ++div)
			{
				if((row % div) != 0) ok[div] = false;
			}
		}
	}

	ASSERT(ok[1]);
	int best_div = max_divisor-1;
	while(!ok[best_div])
		best_div--;

	delete [] ok;

	SetDivisor(best_div);
}

void NoteData::Decompress()
{
	SetDivisor(1);
}

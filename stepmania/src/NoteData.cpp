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



NoteData::NoteData()
{
	memset(m_TapNotes, 0, sizeof(m_TapNotes));
	Init();
}

NoteData::NoteData(const NoteData &cpy)
{
	memset(m_TapNotes, 0, sizeof(m_TapNotes));
	*this = cpy;
}

NoteData &NoteData::operator= (const NoteData &cpy)
{
	Init();
	for(int t = 0; t < MAX_NOTE_TRACKS; ++t)
		memmove( m_TapNotes[t], cpy.m_TapNotes[t], sizeof(TapNote) * MAX_TAP_NOTE_ROWS );
	memmove(m_HoldNotes, cpy.m_HoldNotes, sizeof(m_HoldNotes));

	m_iNumTracks = cpy.m_iNumTracks;
	m_iNumHoldNotes = cpy.m_iNumHoldNotes;
	return *this;
}

void NoteData::Init()
{
	for(int t = 0; t < MAX_NOTE_TRACKS; ++t) {
		if(!m_TapNotes[t])
			m_TapNotes[t] = new TapNote[MAX_TAP_NOTE_ROWS];
		memset( m_TapNotes[t], '0', sizeof(TapNote) * MAX_TAP_NOTE_ROWS );
	}
	m_iNumTracks = 0;
	m_iNumHoldNotes = 0;
}

NoteData::~NoteData()
{
	for(int t = 0; t < MAX_NOTE_TRACKS; ++t)
		delete[] m_TapNotes[t];
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
	for( int m=0; m<asMeasures.GetSize(); m++ )	// foreach measure
	{
		CString &sMeasureString = asMeasures[m];
		sMeasureString.TrimLeft();
		sMeasureString.TrimRight();

		CStringArray asMeasureLines;
		split( sMeasureString, "\n", asMeasureLines, true );	// ignore empty is important

		//ASSERT( asMeasureLines.GetSize() == 4  ||
		//	    asMeasureLines.GetSize() == 8  ||
		//	    asMeasureLines.GetSize() == 12  ||
		//	    asMeasureLines.GetSize() == 16 );


		for( int l=0; l<asMeasureLines.GetSize(); l++ )
		{
			CString &sMeasureLine = asMeasureLines[l];
			sMeasureLine.TrimLeft();
			sMeasureLine.TrimRight();

			const float fPercentIntoMeasure = l/(float)asMeasureLines.GetSize();
			const float fBeat = (m + fPercentIntoMeasure) * BEATS_PER_MEASURE;
			const int iIndex = BeatToNoteRow( fBeat );

//			if( m_iNumTracks != sMeasureLine.GetLength() )
//				throw RageException( "Actual number of note columns (%d) is different from the NotesType (%d).", m_iNumTracks, sMeasureLine.GetLength() );

			for( int c=0; c<min(sMeasureLine.GetLength(),m_iNumTracks); c++ )
				m_TapNotes[c][iIndex] = sMeasureLine[c];
		}
	}
	this->Convert2sAnd3sToHoldNotes();
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
		asMeasureLines.Add( ssprintf("  // measure %d", m+1) );

		const int iMeasureStartRow = m * ROWS_PER_MEASURE;
		const int iMeasureLastRow = (m+1) * ROWS_PER_MEASURE - 1;

		for( int r=iMeasureStartRow; r<=iMeasureLastRow; r+=iRowSpacing )
		{
			char szLineString[MAX_NOTE_TRACKS];
			for( int t=0; t<m_iNumTracks; t++ )
				szLineString[t] = m_TapNotes[t][r];
			szLineString[t] = '\0';
			asMeasureLines.Add( szLineString );
		}

		CString sMeasureString = join( "\n", asMeasureLines );

		asMeasureStrings.Add( sMeasureString );
	}

	this->Convert2sAnd3sToHoldNotes();

	return join( "\n,", asMeasureStrings );
}

/* Clear [iNoteIndexBegin,iNoteIndexEnd]; that is, including iNoteIndexEnd. */
void NoteData::ClearRange( int iNoteIndexBegin, int iNoteIndexEnd )
{
	this->ConvertHoldNotesTo4s();
	for( int c=0; c<m_iNumTracks; c++ )
	{
		for( int i=iNoteIndexBegin; i <= iNoteIndexEnd && i < MAX_TAP_NOTE_ROWS; i++ )
			m_TapNotes[c][i] = '0';
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
			m_TapNotes[c][t] = pFrom->m_TapNotes[c][f];
		f++;
		t++;
	}

	pFrom->Convert4sToHoldNotes();
	this->Convert4sToHoldNotes();
}

void NoteData::AddHoldNote( HoldNote add )
{
	ASSERT( add.m_fStartBeat>=0 && add.m_fEndBeat>=0 );

	int i;

	// look for other hold notes that overlap and merge them
	for( i=0; i<m_iNumHoldNotes; i++ )	// for each HoldNote
	{
		HoldNote &other = m_HoldNotes[i];
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
			for( int j=i; j<m_iNumHoldNotes-1; j++ )
				m_HoldNotes[j] = m_HoldNotes[j+1];

			m_iNumHoldNotes--;
		}
	}

	int iAddStartIndex = BeatToNoteRow(add.m_fStartBeat);
	int iAddEndIndex = BeatToNoteRow(add.m_fEndBeat);

	// delete TapNotes under this HoldNote
	for( i=iAddStartIndex+1; i<=iAddEndIndex; i++ )
		m_TapNotes[add.m_iTrack][i] = '0';

	// add a tap note at the start of this hold
	m_TapNotes[add.m_iTrack][iAddStartIndex] = '2';		// '2' means a Hold begin marker.  Don't draw this, but do grade it.

	ASSERT( m_iNumHoldNotes < MAX_HOLD_NOTES );
	m_HoldNotes[m_iNumHoldNotes++] = add;
}

void NoteData::RemoveHoldNote( int iHoldIndex )
{
	ASSERT( iHoldIndex >= 0  &&  iHoldIndex < m_iNumHoldNotes );

	HoldNote& hn = m_HoldNotes[iHoldIndex];

	int iHoldStartIndex = BeatToNoteRow(hn.m_fStartBeat);

	// delete a tap note at the start of this hold
	m_TapNotes[hn.m_iTrack][iHoldStartIndex] = '0';

	// remove from list
	for( int j=iHoldIndex; j<m_iNumHoldNotes-1; j++ )
		m_HoldNotes[j] = m_HoldNotes[j+1];

	m_iNumHoldNotes--;
}

bool NoteData::IsThereANoteAtRow( int iRow ) const
{
	for( int t=0; t<m_iNumTracks; t++ )
		if( m_TapNotes[t][iRow] != '0' )
			return true;

// There is a tap note at the beginning of every HoldNote start
//
//	for( int i=0; i<m_iNumHoldNotes; i++ )	// for each HoldNote
//		if( m_HoldNotes[i].m_iStartIndex == NoteRowToBeatRow(iRow) )
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

	for( i=0; i<MAX_TAP_NOTE_ROWS; i++ )
	{
		if( !IsRowEmpty(i) )
		{
			fEarliestBeatFoundSoFar = NoteRowToBeat(i);
			break;
		}
	}

	for( i=0; i<m_iNumHoldNotes; i++ )
		if( m_HoldNotes[i].m_fStartBeat < fEarliestBeatFoundSoFar )
			fEarliestBeatFoundSoFar = m_HoldNotes[i].m_fStartBeat;


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

	for( i=MAX_TAP_NOTE_ROWS-1; i>=0; i-- )		// iterate back to front
	{
		if( !IsRowEmpty(i) )
		{
			fOldestBeatFoundSoFar = NoteRowToBeat(i);
			break;
		}
	}

	for( i=0; i<m_iNumHoldNotes; i++ )
	{
		if( m_HoldNotes[i].m_fEndBeat > fOldestBeatFoundSoFar )
			fOldestBeatFoundSoFar = m_HoldNotes[i].m_fEndBeat;
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
			if( m_TapNotes[t][i] != '0' )
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
			if( m_TapNotes[t][i] != '0' )
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

	for( int i=0; i<m_iNumHoldNotes; i++ )
	{
		HoldNote &hn = m_HoldNotes[i];
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
	for( int c=iFirstRightSideColumn; c<=iLastRightSideColumn; c++ )
	{
		for( int i=0; i<MAX_TAP_NOTE_ROWS; i++ ) // foreach TapNote
			m_TapNotes[c][i] = '0';
	}

	for( int i=m_iNumHoldNotes-1; i>=0; i-- ) // foreach HoldNote
	{
		if( c >= iFirstRightSideColumn )
		{
			// delete this HoldNote by shifting everything down
			for( int j=i; j<m_iNumHoldNotes-1; j++ )
				m_HoldNotes[j] = m_HoldNotes[j+1];
			m_iNumHoldNotes--;
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
			m_TapNotes[c][i] = '0';
	}

	// copy the right side into the left
	for( c=iFirstRightSideColumn; c<=iLastRightSideColumn; c++ )
	{
		for( int i=0; i<MAX_TAP_NOTE_ROWS; i++ ) // foreach TapNote
		{
			m_TapNotes[c-4][i] = m_TapNotes[c][i];
			m_TapNotes[c][i] = '0';
		}
	}

	for( int i=m_iNumHoldNotes-1; i>=0; i-- ) // foreach HoldNote
	{
		HoldNote &hn = m_HoldNotes[i];

		if( hn.m_iTrack < iFirstRightSideColumn )
		{
			// delete this HoldNote by shifting everything down
			RemoveHoldNote( i );
		}
		else
		{
			m_HoldNotes[i].m_iTrack -= 4;
		}
	}
}

void NoteData::RemoveHoldNotes()
{
	// turn all the HoldNotes into TapNotes
	for( int i=0; i<m_iNumHoldNotes; i++ )
	{
		HoldNote &hn = m_HoldNotes[i];
		
		int iIndex = BeatToNoteRow(hn.m_fStartBeat);
		int iCol = hn.m_iTrack;

		m_TapNotes[iCol][iIndex] = '1';
	}

	// Remove all HoldNotes
	m_iNumHoldNotes = 0;
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
		/* XXX: This assumes each NotesType in a style turns the same. */
		switch( GAMESTATE->GetCurrentStyleDef()->m_NotesTypes[0] )
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
		case NOTES_TYPE_PUMP_DOUBLE:
		case NOTES_TYPE_PUMP_COUPLE:
			iTakeFromTrack[0] = 1;
			iTakeFromTrack[1] = 3;
			iTakeFromTrack[2] = 2;
			iTakeFromTrack[3] = 4;
			iTakeFromTrack[4] = 0;
			iTakeFromTrack[5] = 6;
			iTakeFromTrack[6] = 8;
			iTakeFromTrack[7] = 7;
			iTakeFromTrack[8] = 9;
			iTakeFromTrack[9] = 5;
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
		{
			CArray<int,int> aiTracksLeftToMap;
			for( t=0; t<m_iNumTracks; t++ )
				aiTracksLeftToMap.Add( t );
			
			for( t=0; t<m_iNumTracks; t++ )
			{
				int iRandTrackIndex = rand()%aiTracksLeftToMap.GetSize();
				int iRandTrack = aiTracksLeftToMap[iRandTrackIndex];
				aiTracksLeftToMap.RemoveAt( iRandTrackIndex );
				iTakeFromTrack[t] = iRandTrack;
			}
		}
		break;
	case PlayerOptions::TURN_SUPER_SHUFFLE:
		// handle this below
		break;
	default:
		ASSERT(0);
	}

	NoteData tempNoteData;	// write into here as we tranform
	tempNoteData.m_iNumTracks = m_iNumTracks;

	this->ConvertHoldNotesTo2sAnd3s();

	// transform notes
	for( t=0; t<m_iNumTracks; t++ )
		for( int r=0; r<MAX_TAP_NOTE_ROWS; r++ ) 			
			tempNoteData.m_TapNotes[t][r] = m_TapNotes[iTakeFromTrack[t]][r];


	if( tt == PlayerOptions::TURN_SUPER_SHUFFLE )
	{
		this->Convert2sAnd3sToHoldNotes();		// so we don't super-shuffle HoldNotes
		for( int r=0; r<this->GetLastRow(); r++ )	// foreach row
		{
			if( !this->IsRowEmpty(r) )
			{
				// shuffle this row
				CArray<int,int> aiTracksLeftToMap;
				for( t=0; t<m_iNumTracks; t++ )
					aiTracksLeftToMap.Add( t );
				
				for( t=0; t<m_iNumTracks; t++ )
				{
					int iRandTrackIndex = rand()%aiTracksLeftToMap.GetSize();
					int iRandTrack = aiTracksLeftToMap[iRandTrackIndex];
					aiTracksLeftToMap.RemoveAt( iRandTrackIndex );
					iTakeFromTrack[t] = iRandTrack;
				}

				for( t=0; t<m_iNumTracks; t++ )
					tempNoteData.m_TapNotes[t][r] = m_TapNotes[iTakeFromTrack[t]][r];
			}
		}
		for( int i=0; i<this->m_iNumHoldNotes; i++ )
		{
			HoldNote& hn = this->m_HoldNotes[i];
			HoldNote newHN = hn;
			hn.m_iTrack = rand() % m_iNumTracks;
			tempNoteData.AddHoldNote( newHN );
		}
	}

	this->CopyAll( &tempNoteData );		// copy note data from newData back into this
	this->Convert2sAnd3sToHoldNotes();
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
				m_TapNotes[c][i] = '0';
			}
		}
	}

	// leave HoldNotes unchanged (what should be do with them?)
}

/* ConvertHoldNotesTo2sAnd3s also clears m_iNumHoldNotes;
 * shouldn't this do likewise and set all 2/3's to 0? 
 * -glenn
 *
 * Other code assumes != '0' means there's a tap note, so I changed
 * this to do this. -glenn
 *

 */
void NoteData::Convert2sAnd3sToHoldNotes()
{
	// Any note will end a hold (not just a '3').  This makes parsing DWIs much easier, 
	// plus we don't want tap notes in the middle of a hold!

	for( int col=0; col<m_iNumTracks; col++ )	// foreach column
	{
		for( int i=0; i<MAX_TAP_NOTE_ROWS; i++ )	// foreach TapNote element
		{
			if( m_TapNotes[col][i] != '2' )	// this is a HoldNote begin marker
				continue;
			m_TapNotes[col][i] = '0';

			for( int j=i+1; j<MAX_TAP_NOTE_ROWS; j++ )	// search for end of HoldNote
			{
				// end hold on the next note we see
				if( m_TapNotes[col][j] == '0' )
					continue;

				m_TapNotes[col][j] = '0';

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
	for( int i=0; i<m_iNumHoldNotes; i++ ) 
	{
		HoldNote &hn = m_HoldNotes[i];
		int iHoldStartIndex = BeatToNoteRow(hn.m_fStartBeat);
		int iHoldEndIndex   = BeatToNoteRow(hn.m_fEndBeat);
		m_TapNotes[hn.m_iTrack][iHoldStartIndex] = '2';
		m_TapNotes[hn.m_iTrack][iHoldEndIndex]   = '3';
	}
	m_iNumHoldNotes = 0;
}

/* "104444001" ==
 * "102000301"
 *
 * "4441" basically means "hold for three rows then hold for another tap";
 * since taps don't really have a length, it's equivalent to "4440".
 * So, make sure the character after a 4 is always a 0. */
void NoteData::Convert4sToHoldNotes()
{
	for( int col=0; col<m_iNumTracks; col++ )	// foreach column
	{
		for( int i=0; i<MAX_TAP_NOTE_ROWS; i++ )	// foreach TapNote element
		{
			if( m_TapNotes[col][i] != '4' )	// this is a HoldNote begin marker
				continue;

			HoldNote hn = { col, NoteRowToBeat(i), 0 };
			// search for end of HoldNote
			do {
				m_TapNotes[col][i] = '0';
				i++;
			} while( i<MAX_TAP_NOTE_ROWS && m_TapNotes[col][i] == '4');
			m_TapNotes[col][i] = '0';

			hn.m_fEndBeat = NoteRowToBeat(i);
			AddHoldNote( hn );
		}
	}
}

void NoteData::ConvertHoldNotesTo4s()
{
	// copy HoldNotes into the new structure, but expand them into 4s
	for( int i=0; i<m_iNumHoldNotes; i++ ) 
	{
		HoldNote &hn = m_HoldNotes[i];
		int iHoldStartIndex = BeatToNoteRow(hn.m_fStartBeat);
		int iHoldEndIndex   = BeatToNoteRow(hn.m_fEndBeat);
		for( int j = iHoldStartIndex; j < iHoldEndIndex; ++j)
			m_TapNotes[hn.m_iTrack][j] = '4';
	}
	m_iNumHoldNotes = 0;
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
			TapNote note = m_TapNotes[c][iOldIndex];
			m_TapNotes[c][iOldIndex] = '0';
			m_TapNotes[c][iNewIndex] = max( m_TapNotes[c][iNewIndex], note );	// this causes HoldNotes to override TapNotes
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
	for( int r=0; r<MAX_TAP_NOTE_ROWS; r++ )
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
	pOriginal->ConvertHoldNotesTo4s();

	// reset all notes
	Init();
	
	m_iNumTracks = iNewNumTracks;

	// copy tracks
	for( int t=0; t<m_iNumTracks; t++ )
	{
		const int iOriginalTrack = iOriginalTrackToTakeFrom[t];
		ASSERT( iOriginalTrack < pOriginal->m_iNumTracks );
		if( iOriginalTrack == -1 )
			continue;
		memcpy( m_TapNotes[t], pOriginal->m_TapNotes[iOriginalTrack], MAX_TAP_NOTE_ROWS*sizeof(TapNote) );
	}

	pOriginal->Convert4sToHoldNotes();
	Convert4sToHoldNotes();
}

void NoteData::LoadTransformedSlidingWindow( NoteData* pOriginal, int iNewNumTracks )
{
	pOriginal->ConvertHoldNotesTo4s();
	m_iNumTracks = iNewNumTracks;

	int iCurTrackOffset = 0;
	int iTrackOffsetMin = 0;
	int iTrackOffsetMax = abs( iNewNumTracks - pOriginal->m_iNumTracks );
	int bOffsetIncreasing = true;

	int iLastRow = pOriginal->GetLastRow();
	for( int r=0; r<=iLastRow; r++ )
	{
		// copy notes in this measure
		for( int t=0; t<=pOriginal->m_iNumTracks; t++ )
		{
			int iOldTrack = t;
			int iNewTrack = (iOldTrack + iCurTrackOffset) % iNewNumTracks;
			if( pOriginal->m_TapNotes[iOldTrack][r] != '0' )
				this->m_TapNotes[iNewTrack][r] = pOriginal->m_TapNotes[iOldTrack][r];
		}

		if( r % (ROWS_PER_MEASURE*4) == 0 )	// adjust sliding window every 4 measures
		{
			// See if there is a hold crossing the beginning of this measure
			bool bHoldCrossesThisMeasure = false;
#if 0
			pOriginal->Convert4sToHoldNotes();
			for( int i=0; i<pOriginal->m_iNumHoldNotes; i++ )
			{
				const HoldNote& hn = pOriginal->m_HoldNotes[i];
				/* Bug here: NoteRowToBeat() may have floating point error,
				 * so we need to do an epsilon here.  But we can do this in
				 * 4s easier anyway ... */
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
				if( pOriginal->m_TapNotes[t][r] == '4' &&
					pOriginal->m_TapNotes[t][r-1] == '4')
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
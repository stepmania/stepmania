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



NoteData::NoteData()
{
	Init();
}

void NoteData::Init()
{
	memset( m_TapNotes, '0', MAX_NOTE_TRACKS*MAX_TAP_NOTE_ROWS*sizeof(m_TapNotes[0][0]) );
	m_iNumTracks = 0;
	m_iNumHoldNotes = 0;
}

NoteData::~NoteData()
{
}


void NoteData::LoadFromSMNoteDataString( CString sSMNoteData )
{
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

			if( m_iNumTracks != sMeasureLine.GetLength() )
				throw RageException( "Actual number of note columns (%d) is different from the NotesType (%d).", m_iNumTracks, sMeasureLine.GetLength() );

			for( int c=0; c<sMeasureLine.GetLength(); c++ )
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
		int iMeasureStartIndex = m * ELEMENTS_PER_MEASURE;
		int iMeasureLastIndex = (m+1) * ELEMENTS_PER_MEASURE - 1;

		// probe to find the smallest note type
		NoteType nt;
		int iNoteIndexSpacing;
		for( nt=(NoteType)0; nt<NUM_NOTE_TYPES; nt=NoteType(nt+1) )
		{
			float fBeatSpacing = NoteTypeToBeat( nt );
			iNoteIndexSpacing = roundf( fBeatSpacing * ELEMENTS_PER_BEAT );

			bool bFoundSmallerNote = false;
			for( int i=iMeasureStartIndex; i<=iMeasureLastIndex; i++ )	// for each index in this measure
			{
				if( i % iNoteIndexSpacing == 0 )
					continue;	// skip
				
				if( !IsRowEmpty(i) )
				{
					bFoundSmallerNote = true;
					break;
				}
				
			}
			if( bFoundSmallerNote )
				continue;	// keep searching
			else
				break;	// stop searching
		}

		if( nt == NUM_NOTE_TYPES )	// we didn't find one
			iNoteIndexSpacing = 1;


		CStringArray asMeasureLines;
		asMeasureLines.Add( ssprintf("  // measure %d", m+1) );

		for( int i=iMeasureStartIndex; i<=iMeasureLastIndex; i+=iNoteIndexSpacing )
		{
			char szLineString[MAX_NOTE_TRACKS];
			for( int t=0; t<m_iNumTracks; t++ )
				szLineString[t] = m_TapNotes[t][i];
			szLineString[t] = '\0';
			asMeasureLines.Add( szLineString );
		}

		CString sMeasureString = join( "\n", asMeasureLines );

		asMeasureStrings.Add( sMeasureString );
	}

	this->Convert2sAnd3sToHoldNotes();

	return join( "\n,", asMeasureStrings );
}


void NoteData::ClearRange( int iNoteIndexBegin, int iNoteIndexEnd )
{
	// delete old TapNotes in the range
	for( int i=iNoteIndexBegin; i<=iNoteIndexEnd; i++ )
	{
		for( int c=0; c<m_iNumTracks; c++ )
			m_TapNotes[c][i] = '0';
	}

	// delete old HoldNotes in the range
	int iIndexRead, iIndexWrite = 0;
	const int iOrigNumHoldNotes = m_iNumHoldNotes;
	for( iIndexRead = 0; iIndexRead < iOrigNumHoldNotes; iIndexRead++ )
	{
		HoldNote &hn = m_HoldNotes[iIndexRead];
		if( (hn.m_iStartIndex >= iNoteIndexBegin && hn.m_iStartIndex <= iNoteIndexEnd)  || 
			(hn.m_iEndIndex >= iNoteIndexBegin && hn.m_iEndIndex <= iNoteIndexEnd) )		// overlap (kinda)
		{
			;	// do nothing
		}
		else	// ! overlap
		{
			m_HoldNotes[iIndexWrite] = m_HoldNotes[iIndexRead];
			iIndexWrite++;
		}
	}
	m_iNumHoldNotes = iIndexWrite;

}
	
void NoteData::CopyRange( NoteData* pFrom, int iFromIndexBegin, int iFromIndexEnd, int iToIndexBegin )
{
	ASSERT( pFrom->m_iNumTracks == m_iNumTracks );

	if( iToIndexBegin == -1 )
		iToIndexBegin = 0;

	pFrom->ConvertHoldNotesTo2sAnd3s();

	int f, t;

	// copy recorded TapNotes
	f = iFromIndexBegin;
	t = iToIndexBegin;
	
	while( f<=iFromIndexEnd )
	{
		for( int c=0; c<m_iNumTracks; c++ )
			m_TapNotes[c][t] = pFrom->m_TapNotes[c][f];
		f++;
		t++;
	}

	pFrom->Convert2sAnd3sToHoldNotes();
	this->Convert2sAnd3sToHoldNotes();
}

void NoteData::AddHoldNote( HoldNote add )
{
	int i;

	// look for other hold notes that overlap and merge them
	for( i=0; i<m_iNumHoldNotes; i++ )	// for each HoldNote
	{
		HoldNote &other = m_HoldNotes[i];
		if( add.m_iTrack == other.m_iTrack  &&		// the notes correspond
			other.m_iStartIndex <= add.m_iEndIndex  &&
			other.m_iEndIndex >= add.m_iStartIndex )	// these HoldNotes overlap (this isn't entirely correct!)
		{
			add.m_iStartIndex = min(add.m_iStartIndex, other.m_iStartIndex);
			add.m_iEndIndex = max(add.m_iEndIndex, other.m_iEndIndex);

			// delete this HoldNote
			for( int j=i; j<m_iNumHoldNotes-1; j++ )
			{
				m_HoldNotes[j] = m_HoldNotes[j+1];
			}
			m_iNumHoldNotes--;
		}
	}

	// delete TapNotes under this HoldNote
	for( i=add.m_iStartIndex+1; i<=add.m_iEndIndex; i++ )
	{
		m_TapNotes[add.m_iTrack][i] = '0';
	};

	// add a tap note at the start of this hold
	m_TapNotes[add.m_iTrack][add.m_iStartIndex] = '1';

	ASSERT( m_iNumHoldNotes < MAX_HOLD_NOTES );
	m_HoldNotes[m_iNumHoldNotes++] = add;
}

void NoteData::RemoveHoldNote( int index )
{
	ASSERT( index > 0  &&  index < m_iNumHoldNotes );

	// delete a tap note at the start of this hold
	HoldNote &hn = m_HoldNotes[index];
	m_TapNotes[hn.m_iTrack][hn.m_iStartIndex] = '0';

	// remove from list
	for( int j=index; j<m_iNumHoldNotes-1; j++ )
	{
		m_HoldNotes[j] = m_HoldNotes[j+1];
	}
	m_iNumHoldNotes--;
}

bool NoteData::IsThereANoteAtRow( int iRow )
{
	for( int t=0; t<m_iNumTracks; t++ )
	{
		if( m_TapNotes[t][iRow] != '0' )
			return true;
	}

	for( int i=0; i<m_iNumHoldNotes; i++ )	// for each HoldNote
	{
		if( m_HoldNotes[i].m_iStartIndex == iRow )
			return true;
	}

	return false;
}


int NoteData::GetFirstRow()
{ 
	int iFirstIndexFoundSoFar = MAX_TAP_NOTE_ROWS-1;
	
	int i;

	for( i=0; i<MAX_TAP_NOTE_ROWS, i<iFirstIndexFoundSoFar; i++ )		// iterate back to front
	{
		if( !IsRowEmpty(i) )
		{
			iFirstIndexFoundSoFar = i;
			break;
		}
	}

	for( i=0; i<m_iNumHoldNotes; i++ )
	{
		if( m_HoldNotes[i].m_iStartIndex < iFirstIndexFoundSoFar )
			iFirstIndexFoundSoFar = m_HoldNotes[i].m_iStartIndex;
	}


	return iFirstIndexFoundSoFar;
}

float NoteData::GetFirstBeat()			
{ 
	return NoteRowToBeat( GetFirstRow() );
}

int NoteData::GetLastRow()
{ 
	int iOldestIndexFoundSoFar = 0;
	
	int i;

	for( i=MAX_TAP_NOTE_ROWS-1; i>=iOldestIndexFoundSoFar; i-- )		// iterate back to front
	{
		if( !IsRowEmpty(i) )
		{
			iOldestIndexFoundSoFar = i;
			break;
		}
	}

	for( i=0; i<m_iNumHoldNotes; i++ )
	{
		if( m_HoldNotes[i].m_iEndIndex > iOldestIndexFoundSoFar )
			iOldestIndexFoundSoFar = m_HoldNotes[i].m_iEndIndex;
	}

	return iOldestIndexFoundSoFar;
}

float NoteData::GetLastBeat()			
{ 
	return NoteRowToBeat( GetLastRow() );
}

int NoteData::GetNumTapNotes( const float fStartBeat, const float fEndBeat )			
{ 
	int iNumNotes = 0;

	int iStartIndex = BeatToNoteRow( fStartBeat );
	int iEndIndex = BeatToNoteRow( fEndBeat );

	for( int i=iStartIndex; i<min(iEndIndex, MAX_TAP_NOTE_ROWS); i++ )
	{
		for( int t=0; t<m_iNumTracks; t++ )
		{
			if( m_TapNotes[t][i] != '0' )
				iNumNotes++;
		}
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

	int iStartIndex = BeatToNoteRow( fStartBeat );
	int iEndIndex = BeatToNoteRow( fEndBeat );

	for( int i=0; i<m_iNumHoldNotes; i++ )
	{
		HoldNote &hn = m_HoldNotes[i];
		if( iStartIndex <= hn.m_iStartIndex  &&  hn.m_iEndIndex <= iEndIndex )
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
		
		int iIndex = hn.m_iStartIndex;
		int iCol = hn.m_iTrack;

		m_TapNotes[iCol][iIndex] = '1';
	}

	// Remove all HoldNotes
	m_iNumHoldNotes = 0;
}


void NoteData::Turn( PlayerOptions::TurnType tt )
{
	// transform the NoteData
	int oldColToNewCol[12][2];	// 0 is old Step, 1 is new Step mapping
	oldColToNewCol[0][0] = 0;		oldColToNewCol[0][1] = 0;
	oldColToNewCol[1][0] = 1;		oldColToNewCol[1][1] = 1;
	oldColToNewCol[2][0] = 2;		oldColToNewCol[2][1] = 2;
	oldColToNewCol[3][0] = 3;		oldColToNewCol[3][1] = 3;
	oldColToNewCol[4][0] = 4;		oldColToNewCol[4][1] = 4;
	oldColToNewCol[5][0] = 5;		oldColToNewCol[5][1] = 5;
	oldColToNewCol[6][0] = 6;		oldColToNewCol[6][1] = 6;
	oldColToNewCol[7][0] = 7;		oldColToNewCol[7][1] = 7;
	oldColToNewCol[8][0] = 8;		oldColToNewCol[8][1] = 8;
	oldColToNewCol[9][0] = 9;		oldColToNewCol[9][1] = 9;
	oldColToNewCol[10][0]= 10;	oldColToNewCol[10][1]= 10;
	oldColToNewCol[11][0]= 11;	oldColToNewCol[11][1]= 11;

	switch( tt )
	{
	case PlayerOptions::TURN_NONE:
		break;
	case PlayerOptions::TURN_MIRROR:
		oldColToNewCol[0][1] = 3;
		oldColToNewCol[2][1] = 2;
		oldColToNewCol[3][1] = 1;
		oldColToNewCol[5][1] = 0;
		oldColToNewCol[6][1] = 7;
		oldColToNewCol[8][1] = 6;
		oldColToNewCol[9][1] = 5;
		oldColToNewCol[11][1]= 4;
		break;
	case PlayerOptions::TURN_LEFT:
		oldColToNewCol[0][1] = 1;
		oldColToNewCol[2][1] = 3;
		oldColToNewCol[3][1] = 0;
		oldColToNewCol[5][1] = 2;
		oldColToNewCol[6][1] = 5;
		oldColToNewCol[8][1] = 7;
		oldColToNewCol[9][1] = 4;
		oldColToNewCol[11][1]= 6;
		break;
	case PlayerOptions::TURN_RIGHT:
		oldColToNewCol[0][1] = 3;
		oldColToNewCol[2][1] = 0;
		oldColToNewCol[3][1] = 4;
		oldColToNewCol[5][1] = 1;
		oldColToNewCol[6][1] = 6;
		oldColToNewCol[8][1] = 4;
		oldColToNewCol[9][1] = 7;
		oldColToNewCol[11][1]= 5;
		break;
	case PlayerOptions::TURN_SHUFFLE:
		bool bAlreadyMapped[12];
		for( int i=0; i<12; i++ )
			bAlreadyMapped[i] = false;
		
		// hack: don't shuffle the Up+... NoteData
		bAlreadyMapped[1] = true;
		bAlreadyMapped[4] = true;
		bAlreadyMapped[7] = true;
		bAlreadyMapped[10] = true;

		// shuffle left side
		for( i=0; i<6; i++ )
		{
			if( i == 1  ||  i == 4 )
				continue;

			int iMapsTo;
			do {
				iMapsTo = rand()%6;
			} while( bAlreadyMapped[iMapsTo] );
			bAlreadyMapped[iMapsTo] = true;

			oldColToNewCol[i][1] = oldColToNewCol[iMapsTo][0];
		}
		// shuffle right side
		for( i=6; i<12; i++ )
		{
			if( i == 7  ||  i == 10 )
				continue;

			int iMapsTo;
			do {
				iMapsTo = rand()%6+6;
			} while( bAlreadyMapped[iMapsTo] );
			bAlreadyMapped[iMapsTo] = true;

			oldColToNewCol[i][1] = oldColToNewCol[iMapsTo][0];
		}
		break;
	}

	NoteData tempNoteData;	// write into here as we tranform
	tempNoteData.m_iNumTracks = m_iNumTracks;

	// transform m_TapNotes 
	for( int i=0; i<MAX_TAP_NOTE_ROWS; i++ ) 
	{
		for( int j=0; j<12; j++ )
		{
			int iOldCol = oldColToNewCol[j][0];
			int iNewCol = oldColToNewCol[j][1];
			
			tempNoteData.m_TapNotes[iNewCol][i] = m_TapNotes[iOldCol][i];
		}
	}

	// transform m_HoldNotes 
	for( i=0; i<m_iNumHoldNotes; i++ ) 
	{
		for( int j=0; j<12; j++ )
		{
			HoldNote &hn = m_HoldNotes[i];

			int iOldCol = oldColToNewCol[j][0];
			int iNewCol = oldColToNewCol[j][1];
			
			if( hn.m_iTrack == iOldCol )
			{
				HoldNote newHN = { iNewCol, hn.m_iStartIndex, hn.m_iEndIndex };
				tempNoteData.AddHoldNote( newHN );
			}
		}
	}

	//
	// copy note data from newData back into this
	//
	(*this) = tempNoteData;
}

void NoteData::MakeLittle()
{
	// filter out all non-quarter notes
	for( int i=0; i<MAX_TAP_NOTE_ROWS; i++ ) 
	{
		if( i%ELEMENTS_PER_BEAT != 0 )
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

void NoteData::Convert2sAnd3sToHoldNotes()
{
	// Any note will end a hold (not just a '3').  This makes parsing DWIs much easier, 
	// plus we don't want tap notes in the middle of a hold!

	for( int col=0; col<m_iNumTracks; col++ )	// foreach column
	{
		for( int i=0; i<MAX_TAP_NOTE_ROWS; i++ )	// foreach TapNote element
		{
			if( m_TapNotes[col][i] == '2' )	// this is a HoldNote begin marker
			{
				for( int j=i+1; j<MAX_TAP_NOTE_ROWS; j++ )	// search for end of HoldNote
				{
					if( m_TapNotes[col][j] != '0' )	// end hold on the next note we see
					{
						HoldNote hn = { col, i, j };
						AddHoldNote( hn );
						break;
					}
				}
			}
		}
	}
}

void NoteData::ConvertHoldNotesTo2sAnd3s()
{
	// copy HoldNotes into the new structure, but expand them into 2s and 3s
	for( int i=0; i<m_iNumHoldNotes; i++ ) 
	{
		HoldNote &hn = m_HoldNotes[i];
		m_TapNotes[hn.m_iTrack][hn.m_iStartIndex] = '2';
		m_TapNotes[hn.m_iTrack][hn.m_iEndIndex] = '3';
	}
	m_iNumHoldNotes = 0;
}

void NoteData::SnapToNearestNoteType( NoteType nt1, NoteType nt2, float fBeginBeat, float fEndBeat )
{
	// nt2 is optional and should be -1 if it is not used

	float fSnapInterval1, fSnapInterval2;
	switch( nt1 )
	{
		case NOTE_4TH:	fSnapInterval1 = 1/1.0f;	break;
		case NOTE_8TH:	fSnapInterval1 = 1/2.0f;	break;
		case NOTE_12TH:	fSnapInterval1 = 1/3.0f;	break;
		case NOTE_16TH:	fSnapInterval1 = 1/4.0f;	break;
		default:	ASSERT( false );
	}

	switch( nt2 )
	{
		case NOTE_4TH:	fSnapInterval2 = 1/1.0f;	break;
		case NOTE_8TH:	fSnapInterval2 = 1/2.0f;	break;
		case NOTE_12TH:	fSnapInterval2 = 1/3.0f;	break;
		case NOTE_16TH:	fSnapInterval2 = 1/4.0f;	break;
		case -1:		fSnapInterval2 = 10000;		break;	// nothing will ever snap to this.  That is good!
		default:	ASSERT( false );
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
	// count number of triplets
	int iNumChaosNotes = 0;
	for( int r=0; r<MAX_TAP_NOTE_ROWS; r++ )
	{
		if( !IsRowEmpty(r)  &&  GetNoteType(r) >= NOTE_12TH )
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


void NoteData::LoadTransformed( NoteData* pOriginal, int iNewNumTracks, int iNewToOriginalTrack[] )
{
	// init
	Init();
	
	m_iNumTracks = iNewNumTracks;

	// copy tracks
	for( int t=0; t<m_iNumTracks; t++ )
	{
		int iOriginalTrack = iNewToOriginalTrack[t];

		int i;

		memcpy( m_TapNotes[t], pOriginal->m_TapNotes[iOriginalTrack], MAX_TAP_NOTE_ROWS*sizeof(m_TapNotes[0][0]) );

		for( i=0; i<pOriginal->m_iNumHoldNotes; i++ )
		{
			HoldNote &hn = pOriginal->m_HoldNotes[i];
			if( hn.m_iTrack == iOriginalTrack )
			{
				hn.m_iTrack = t;
				this->AddHoldNote( hn );
			}
		}

	}

}


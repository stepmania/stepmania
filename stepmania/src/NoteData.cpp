#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: NoteData.cpp

 Desc: A pattern of ColorNotes that past Y==0.

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "NoteData.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "ThemeManager.h"
#include "ArrowEffects.h"
#include "PrefsManager.h"
#include "GameConstantsAndTypes.h"
#include "ErrorCatcher/ErrorCatcher.h"


NoteData::NoteData()
{
	for( int i=0; i<MAX_NOTE_TRACKS; i++ )
	{
		for( int j=0; j<MAX_TAP_NOTE_ELEMENTS; j++ )
			m_TapNotes[i][j] = '0';
	}
	m_iNumTracks = 0;

	m_iNumHoldNotes = 0;
}

void NoteData::ClearRange( NoteIndex iNoteIndexBegin, NoteIndex iNoteIndexEnd )
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
	
void NoteData::CopyRange( NoteData* pFrom, NoteIndex iNoteIndexBegin, NoteIndex iNoteIndexEnd )
{
	ASSERT( pFrom->m_iNumTracks == m_iNumTracks );

	int i;

	// copy recorded TapNotes
	for( i=iNoteIndexBegin; i<=iNoteIndexEnd; i++ )
	{
		for( int c=0; c<m_iNumTracks; c++ )
			m_TapNotes[c][i] = pFrom->m_TapNotes[c][i];
	}

	// copy recorded HoldNotes
	for( i=0; i<pFrom->m_iNumHoldNotes; i++ )
	{
		HoldNote hn = pFrom->m_HoldNotes[i];
		if( (hn.m_iStartIndex >= iNoteIndexBegin && hn.m_iStartIndex <= iNoteIndexEnd)  || 
			(hn.m_iEndIndex >= iNoteIndexBegin && hn.m_iEndIndex <= iNoteIndexEnd) )		// overlap (kinda)
		{
			this->AddHoldNote( hn );
		}
	}
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
	for( i=add.m_iStartIndex; i<=add.m_iEndIndex; i++ )
	{
		m_TapNotes[add.m_iTrack][i] = '0';
	};

	m_HoldNotes[m_iNumHoldNotes++] = add;
}

void NoteData::RemoveHoldNote( int index )
{
	ASSERT( index > 0  &&  index < m_iNumHoldNotes );

	for( int j=index; j<m_iNumHoldNotes-1; j++ )
	{
		m_HoldNotes[j] = m_HoldNotes[j+1];
	}
	m_iNumHoldNotes--;
}

float NoteData::GetLastBeat()			
{ 
	int iOldestIndexFoundSoFar = 0;
	
	int i;

	for( i=0; i<m_iNumHoldNotes; i++ )
	{
		if( m_HoldNotes[i].m_iEndIndex > iOldestIndexFoundSoFar )
			iOldestIndexFoundSoFar = m_HoldNotes[i].m_iEndIndex;
	}

	for( i=MAX_TAP_NOTE_ELEMENTS-1; i>=iOldestIndexFoundSoFar; i-- )		// iterate back to front
	{
		for( int t=0; t<m_iNumTracks; t++ )
		{
			if( m_TapNotes[t][i] != '0' )
			{
				iOldestIndexFoundSoFar = i;
				goto done_searching_tap_notes;
			}
		}
	}
done_searching_tap_notes:


	return NoteIndexToBeat( iOldestIndexFoundSoFar );
}

int NoteData::GetNumTapNotes()			
{ 
	int iNumSteps = 0;

	for( int i=0; i<MAX_TAP_NOTE_ELEMENTS; i++ )
	{
		for( int t=0; t<m_iNumTracks; t++ )
		{
			if( m_TapNotes[t][i] != '0' )
				iNumSteps++;
		}
	}
	
	iNumSteps += m_iNumHoldNotes;

	return iNumSteps;
}

int NoteData::GetNumHoldNotes()			
{ 
	return m_iNumHoldNotes;
}


void NoteData::CropToLeftSide()
{
	int iFirstRightSideColumn = 4;
	int iLastRightSideColumn = 7;

	// clear out the right half
	for( int c=iFirstRightSideColumn; c<=iLastRightSideColumn; c++ )
	{
		for( int i=0; i<MAX_TAP_NOTE_ELEMENTS; i++ ) // foreach TapNote
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
		for( int i=0; i<MAX_TAP_NOTE_ELEMENTS; i++ ) // foreach TapNote
			m_TapNotes[c][i] = '0';
	}

	// copy the right side into the left
	for( c=iFirstRightSideColumn; c<=iLastRightSideColumn; c++ )
	{
		for( int i=0; i<MAX_TAP_NOTE_ELEMENTS; i++ ) // foreach TapNote
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

	NoteData* pNewData = new NoteData;	// write into here as we tranform
	pNewData->m_iNumTracks = m_iNumTracks;

	// transform m_TapNotes 
	for( int i=0; i<MAX_TAP_NOTE_ELEMENTS; i++ ) 
	{
		for( int j=0; j<12; j++ )
		{
			int iOldCol = oldColToNewCol[j][0];
			int iNewCol = oldColToNewCol[j][1];
			
			pNewData->m_TapNotes[iNewCol][i] = m_TapNotes[iOldCol][i];
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
				pNewData->AddHoldNote( newHN );
			}
		}
	}

	//
	// copy note data from newData back into this
	//
	(*this) = *pNewData;
}

void NoteData::MakeLittle()
{
	// filter out all non-quarter notes
	for( int i=0; i<MAX_TAP_NOTE_ELEMENTS; i++ ) 
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
	// Note that a 3 or a 1 can end a HoldNote.  
	// A 1 can end a HoldNote because it's easier for parsing DWIs

	for( int col=0; col<m_iNumTracks; col++ )	// foreach column
	{
		for( int i=0; i<MAX_TAP_NOTE_ELEMENTS; i++ )	// foreach TapNote element
		{
			if( m_TapNotes[col][i] == '2' )	// this is a HoldNote begin marker
			{
				for( int j=i; j<MAX_TAP_NOTE_ELEMENTS; j++ )	// search for end of HoldNote
				{
					if( m_TapNotes[col][j] == '1'  ||
						m_TapNotes[col][j] == '3' )	// found it!
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

	int iNoteIndexBegin = BeatToNoteIndex( fBeginBeat );
	int iNoteIndexEnd = BeatToNoteIndex( fEndBeat );

	//ConvertHoldNotesTo2sAnd3s();

	// iterate over all TapNotes in the interval and snap them
	for( int i=iNoteIndexBegin; i<=iNoteIndexEnd; i++ )
	{
		int iOldIndex = i;
		float fOldBeat = NoteIndexToBeat( iOldIndex );
		float fNewBeat1 = froundf( fOldBeat, fSnapInterval1 );
		float fNewBeat2 = froundf( fOldBeat, fSnapInterval2 );

		bool bNewBeat1IsCloser = fabsf(fNewBeat1-fOldBeat) < fabsf(fNewBeat2-fOldBeat);
		float fNewBeat = bNewBeat1IsCloser ? fNewBeat1 : fNewBeat2;
		int iNewIndex = BeatToNoteIndex( fNewBeat );

		for( int c=0; c<m_iNumTracks; c++ )
		{
			TapNote note = m_TapNotes[c][iOldIndex];
			m_TapNotes[c][iOldIndex] = '0';
			m_TapNotes[c][iNewIndex] = max( m_TapNotes[c][iNewIndex], note );	// this causes HoldNotes to override TapNotes
		}
	}

	//Convert2sAnd3sToHoldNotes();
}


void NoteData::GetMeasureStrings( CStringArray &arrayMeasureStrings )
{
	ConvertHoldNotesTo2sAnd3s();

	float fLastBeat = this->GetLastBeat();
	int iLastMeasure = int( fLastBeat/BEATS_PER_MEASURE );

	for( int m=0; m<=iLastMeasure; m++ )	// foreach measure
	{
		CString sMeasureString;

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

		for( int i=iMeasureStartIndex; i<=iMeasureLastIndex; i+=iNoteIndexSpacing )
		{
			for( int c=0; c<m_iNumTracks; c++ )
				sMeasureString += m_TapNotes[c][i];
			sMeasureString += '\n';
		}

		arrayMeasureStrings.Add( sMeasureString );
	}

}

void NoteData::SetFromMeasureStrings( CStringArray &arrayMeasureStrings )
{
	for( int m=0; m<arrayMeasureStrings.GetSize(); m++ )
	{
		CString sMeasureString = arrayMeasureStrings[m];
		sMeasureString.TrimLeft();
		sMeasureString.TrimRight();

		CStringArray arrayNoteLines;
		split( sMeasureString, "\n", arrayNoteLines );

		int iNumNoteLines = arrayNoteLines.GetSize();

		for( int l=0; l<arrayNoteLines.GetSize(); l++ )
		{
			const float fPercentIntoMeasure = l/(float)arrayNoteLines.GetSize();
			const float fBeat = (m + fPercentIntoMeasure) * BEATS_PER_MEASURE;
			const int iIndex = BeatToNoteIndex( fBeat );

			CString sNoteLine = arrayNoteLines[l];
			sNoteLine.TrimRight();

			if( m_iNumTracks == 0 )
				m_iNumTracks = sNoteLine.GetLength();
			if( m_iNumTracks != sNoteLine.GetLength() )
				FatalError( "Line doesn't have right number of notes." );

			for( int c=0; c<sNoteLine.GetLength(); c++ )
			{
				m_TapNotes[c][iIndex] = sNoteLine[c];
			}
		}
	}

	ASSERT( m_iNumTracks != 0 );
}

void NoteData::LoadTransformed( NoteData* pOriginal, int iNewNumTracks, TrackNumber iNewToOriginalTrack[] )
{
	// init
	for( int i=0; i<MAX_NOTE_TRACKS; i++ )
	{
		for( int j=0; j<MAX_TAP_NOTE_ELEMENTS; j++ )
			m_TapNotes[i][j] = '0';
	}
	m_iNumHoldNotes = 0;
	
	
	m_iNumTracks = iNewNumTracks;


	// copy tracks
	for( int t=0; t<m_iNumTracks; t++ )
	{
		int iOriginalTrack = iNewToOriginalTrack[t];

		int i;

		for( i=0; i<MAX_TAP_NOTE_ELEMENTS; i++ )
			m_TapNotes[t][i] = pOriginal->m_TapNotes[iOriginalTrack][i];

		for( i=0; i<MAX_HOLD_NOTE_ELEMENTS; i++ )
		{
			HoldNote hn = pOriginal->m_HoldNotes[i];
			if( hn.m_iTrack == iOriginalTrack )
			{
				hn.m_iTrack = t;
				this->AddHoldNote( hn );
			}
		}

	}

}

#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: NoteDataWithScoring

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "NoteDataWithScoring.h"



NoteDataWithScoring::NoteDataWithScoring()
{
	Init();
}

void NoteDataWithScoring::Init()
{
	// init step elements
	for( int t=0; t<MAX_NOTE_TRACKS; t++ )
		for( int i=0; i<MAX_TAP_NOTE_ROWS; i++ )
			m_TapNoteScores[t][i] = TNS_NONE;

	for( int i=0; i<MAX_HOLD_NOTE_ELEMENTS; i++ )
		m_HoldNoteScores[i] = HNS_NONE;

}


int NoteDataWithScoring::GetNumSuccessfulTapNotes( const float fStartBeat, const float fEndBeat )			
{ 
	int iNumSuccessfulTapNotes = 0;

	int iStartIndex = BeatToNoteRow( fStartBeat );
	int iEndIndex = BeatToNoteRow( fEndBeat );

	for( int i=iStartIndex; i<min(iEndIndex, MAX_TAP_NOTE_ROWS); i++ )
	{
		for( int t=0; t<m_iNumTracks; t++ )
		{
			if( m_TapNotes[t][i] != '0'  &&  m_TapNoteScores[t][i] >= TNS_GREAT )
				iNumSuccessfulTapNotes++;
		}
	}
	
	return iNumSuccessfulTapNotes;
}

int NoteDataWithScoring::GetNumSuccessfulDoubles( const float fStartBeat, const float fEndBeat )			
{
	int iNumSuccessfulDoubles = 0;

	int iStartIndex = BeatToNoteRow( fStartBeat );
	int iEndIndex = BeatToNoteRow( fEndBeat );

	for( int i=iStartIndex; i<min(iEndIndex, MAX_TAP_NOTE_ROWS); i++ )
	{
		int iNumNotesThisIndex = 0;
		TapNoteScore	minTapNoteScore = TNS_PERFECT;
		for( int t=0; t<m_iNumTracks; t++ )
		{
			if( m_TapNotes[t][i] != '0' )
			{
				iNumNotesThisIndex++;
				minTapNoteScore = min( minTapNoteScore, m_TapNoteScores[t][i] );
			}
		}
		if( iNumNotesThisIndex >= 2  &&  minTapNoteScore >= TNS_GREAT )
			iNumSuccessfulDoubles++;
	}
	
	return iNumSuccessfulDoubles;
}

int NoteDataWithScoring::GetNumSuccessfulHoldNotes( const float fStartBeat, const float fEndBeat )			
{
	int iNumSuccessfulHolds = 0;

	int iStartIndex = BeatToNoteRow( fStartBeat );
	int iEndIndex = BeatToNoteRow( fEndBeat );

	for( int i=0; i<m_iNumHoldNotes; i++ )
	{
		HoldNote &hn = m_HoldNotes[i];
		if( iStartIndex <= hn.m_iStartIndex  &&  hn.m_iEndIndex <= iEndIndex  &&  m_HoldNoteScores[i] == HNS_OK )
			iNumSuccessfulHolds++;
	}
	return iNumSuccessfulHolds;
}


float NoteDataWithScoring::GetActualStreamRadarValue( float fSongSeconds )
{
	// density of steps
	int iNumSuccessfulNotes = GetNumSuccessfulTapNotes() + GetNumSuccessfulHoldNotes();
	float fNotesPerSecond = iNumSuccessfulNotes/fSongSeconds;
	float fReturn = fNotesPerSecond / 5;
	return min( fReturn, 1.2f );
}

float NoteDataWithScoring::GetActualVoltageRadarValue( float fSongSeconds )
{
	// peak density of steps
	float fMaxDensityPerSecSoFar = 0;

	for( int i=0; i<MAX_BEATS; i+=8 )
	{
		int iNumNotesThisMeasure = GetNumSuccessfulTapNotes((float)i,(float)i+8) + GetNumSuccessfulHoldNotes((float)i,(float)i+8);

		float fDensityThisMeasure = iNumNotesThisMeasure/2.0f;

		float fDensityPerSecThisMeasure = fDensityThisMeasure / fSongSeconds;

		if( fDensityPerSecThisMeasure > fMaxDensityPerSecSoFar )
			fMaxDensityPerSecSoFar = fDensityPerSecThisMeasure;
	}

	float fReturn = fMaxDensityPerSecSoFar*10;
	return min( fReturn, 1.2f );
}

float NoteDataWithScoring::GetActualAirRadarValue( float fSongSeconds )
{
	// number of doubles
	int iNumDoubles = GetNumSuccessfulDoubles();
	float fReturn = iNumDoubles / fSongSeconds * 2;
	return min( fReturn, 1.2f );
}

float NoteDataWithScoring::GetActualChaosRadarValue( float fSongSeconds )
{
	// irregularity of steps
	float fReturn = fabsf( GetActualStreamRadarValue(fSongSeconds) - GetActualVoltageRadarValue(fSongSeconds) ) * 2.5f;
	return min( fReturn, 1.2f );
}

float NoteDataWithScoring::GetActualFreezeRadarValue( float fSongSeconds )
{
	// number of hold steps
	float fReturn = GetNumSuccessfulHoldNotes() / fSongSeconds * 3;
	return min( fReturn, 1.2f );
}

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
	NoteData::Init();
	InitScoringData();
}

void NoteDataWithScoring::InitScoringData()
{
	for( int t=0; t<MAX_NOTE_TRACKS; t++ )
		for( int i=0; i<MAX_TAP_NOTE_ROWS; i++ )
			m_TapNoteScores[t][i] = TNS_NONE;

	for( int i=0; i<MAX_HOLD_NOTES; i++ )
	{
		m_HoldNoteScores[i] = HNS_NONE;
		m_fHoldNoteLife[i] = 1.0f;
	}
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
	float fReturn = fNotesPerSecond / 7;
	return min( fReturn, 1.0f );
}

float NoteDataWithScoring::GetActualVoltageRadarValue( float fSongSeconds )
{
	float fAvgBPS = GetLastBeat() / fSongSeconds;

	// peak density of steps
	float fMaxDensitySoFar = 0;

	const int BEAT_WINDOW = 8;

	for( int i=0; i<MAX_BEATS; i+=BEAT_WINDOW )
	{
		int iNumNotesThisWindow = GetNumSuccessfulTapNotes((float)i,(float)i+BEAT_WINDOW) + GetNumSuccessfulHoldNotes((float)i,(float)i+BEAT_WINDOW);
		float fDensityThisWindow = iNumNotesThisWindow/(float)BEAT_WINDOW;
		fMaxDensitySoFar = max( fMaxDensitySoFar, fDensityThisWindow );
	}

	float fReturn = fMaxDensitySoFar*fAvgBPS/10;
	return min( fReturn, 1.0f );
}

float NoteDataWithScoring::GetActualAirRadarValue( float fSongSeconds )
{
	// number of doubles
	int iNumDoubles = GetNumSuccessfulDoubles();
	float fReturn = iNumDoubles / fSongSeconds;
	return min( fReturn, 1.0f );
}

float NoteDataWithScoring::GetActualChaosRadarValue( float fSongSeconds )
{
	// count number of triplets
	int iNumChaosNotesCompleted = 0;
	for( int r=0; r<MAX_TAP_NOTE_ROWS; r++ )
	{
		if( !IsRowEmpty(r)  &&  IsRowComplete(r)  &&  GetNoteType(r) >= NOTE_12TH )
			iNumChaosNotesCompleted++;
	}

	float fReturn = iNumChaosNotesCompleted / fSongSeconds * 0.5f;
	return min( fReturn, 1.0f );
}

float NoteDataWithScoring::GetActualFreezeRadarValue( float fSongSeconds )
{
	// number of hold steps
	float fReturn = GetNumSuccessfulHoldNotes() / fSongSeconds;
	return min( fReturn, 1.0f );
}

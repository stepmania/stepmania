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

void NoteDataWithScoring::Init(int taps, int holds)
{
	NoteData::Init();

	for( int t=0; t<MAX_NOTE_TRACKS; t++ )
		for( int i=0; i<MAX_TAP_NOTE_ROWS; i++ )
			m_TapNoteScores[t][i] = TNS_NONE;

	m_HoldNoteScores.clear();
	m_HoldNoteScores.insert(m_HoldNoteScores.begin(), holds, HNS_NONE);

	m_fHoldNoteLife.clear();
	// start with full life
	m_fHoldNoteLife.insert(m_fHoldNoteLife.begin(), holds, 1.0f);
}

int NoteDataWithScoring::GetNumTapNotesWithScore( TapNoteScore tns, const float fStartBeat, const float fEndBeat )			
{ 
	int iNumSuccessfulTapNotes = 0;

	int iStartIndex = BeatToNoteRow( fStartBeat );
	int iEndIndex = BeatToNoteRow( fEndBeat );

	for( int i=iStartIndex; i<min(iEndIndex, MAX_TAP_NOTE_ROWS); i++ )
	{
		for( int t=0; t<m_iNumTracks; t++ )
		{
			if( this->GetTapNote(t, i) != TAP_EMPTY && m_TapNoteScores[t][i] == tns )
				iNumSuccessfulTapNotes++;
		}
	}
	
	return iNumSuccessfulTapNotes;
}

int NoteDataWithScoring::GetNumDoublesWithScore( TapNoteScore tns, const float fStartBeat, const float fEndBeat )			
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
			if( GetTapNote(t, i) != TAP_EMPTY )
			{
				iNumNotesThisIndex++;
				minTapNoteScore = min( minTapNoteScore, m_TapNoteScores[t][i] );
			}
		}
		if( iNumNotesThisIndex >= 2  &&  minTapNoteScore == tns )
			iNumSuccessfulDoubles++;
	}
	
	return iNumSuccessfulDoubles;
}

int NoteDataWithScoring::GetNumHoldNotesWithScore( HoldNoteScore hns, const float fStartBeat, const float fEndBeat )			
{
	int iNumSuccessfulHolds = 0;

	for( int i=0; i<GetNumHoldNotes(); i++ )
	{
		const HoldNote &hn = GetHoldNote(i);
		if( fStartBeat <= hn.m_fStartBeat  &&  hn.m_fEndBeat <= fEndBeat  &&  m_HoldNoteScores[i] == hns )
			iNumSuccessfulHolds++;
	}
	return iNumSuccessfulHolds;
}

bool NoteDataWithScoring::IsRowComplete( int index )
{
	for( int t=0; t<m_iNumTracks; t++ )
		if( GetTapNote(t, index) != TAP_EMPTY && m_TapNoteScores[t][index] < TNS_GREAT )
			return false;
	return true;
}

float NoteDataWithScoring::GetActualRadarValue( RadarCategory rv, float fSongSeconds )
{
	switch( rv )
	{
	case RADAR_STREAM:	return GetActualStreamRadarValue( fSongSeconds );	break;
	case RADAR_VOLTAGE:	return GetActualVoltageRadarValue( fSongSeconds );	break;
	case RADAR_AIR:		return GetActualAirRadarValue( fSongSeconds );		break;
	case RADAR_FREEZE:	return GetActualFreezeRadarValue( fSongSeconds );	break;
	case RADAR_CHAOS:	return GetActualChaosRadarValue( fSongSeconds );	break;
	default:	ASSERT(0);  return 0;
	}
}

float NoteDataWithScoring::GetActualStreamRadarValue( float fSongSeconds )
{
	// density of steps
	int iNumSuccessfulNotes = 
		GetNumTapNotesWithScore(TNS_PERFECT) + 
		GetNumTapNotesWithScore(TNS_GREAT)/2 + 
		GetNumHoldNotesWithScore(HNS_OK);
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
		int iNumNotesThisWindow = GetNumTapNotesWithScore(TNS_PERFECT,(float)i,(float)i+BEAT_WINDOW) + GetNumHoldNotesWithScore(HNS_OK,(float)i,(float)i+BEAT_WINDOW);
		float fDensityThisWindow = iNumNotesThisWindow/(float)BEAT_WINDOW;
		fMaxDensitySoFar = max( fMaxDensitySoFar, fDensityThisWindow );
	}

	float fReturn = fMaxDensitySoFar*fAvgBPS/10;
	return min( fReturn, 1.0f );
}

float NoteDataWithScoring::GetActualAirRadarValue( float fSongSeconds )
{
	// number of doubles
	int iNumDoubles = 
		GetNumDoublesWithScore(TNS_PERFECT) + 
		GetNumDoublesWithScore(TNS_GREAT)/2;
	float fReturn = iNumDoubles / fSongSeconds;
	return min( fReturn, 1.0f );
}

float NoteDataWithScoring::GetActualChaosRadarValue( float fSongSeconds )
{
	// count number of triplets
	int iNumChaosNotesCompleted = 0;
	for( int r=0; r<MAX_TAP_NOTE_ROWS; r++ )
	{
		if( !IsRowEmpty(r)  &&  IsRowComplete(r)  &&  GetNoteType(r) >= NOTE_TYPE_12TH )
			iNumChaosNotesCompleted++;
	}

	float fReturn = iNumChaosNotesCompleted / fSongSeconds * 0.5f;
	return min( fReturn, 1.0f );
}

float NoteDataWithScoring::GetActualFreezeRadarValue( float fSongSeconds )
{
	// number of hold steps
	float fReturn = GetNumHoldNotesWithScore(HNS_OK) / fSongSeconds;
	return min( fReturn, 1.0f );
}

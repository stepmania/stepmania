#include "global.h"
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

	for( int t=0; t<MAX_NOTE_TRACKS; t++ )
		m_TapNoteScores[t].clear();

	m_HoldNoteScores.clear();
	m_fHoldNoteLife.clear();
}

int NoteDataWithScoring::GetNumTapNotesWithScore( TapNoteScore tns, const float fStartBeat, float fEndBeat )
{ 
	int iNumSuccessfulTapNotes = 0;

	if(fEndBeat == -1)
		fEndBeat = GetMaxBeat()+1;

	unsigned iStartIndex = BeatToNoteRow( fStartBeat );
	unsigned iEndIndex = BeatToNoteRow( fEndBeat );

	for( unsigned i=iStartIndex; i<min(iEndIndex, m_TapNoteScores[0].size()); i++ )
	{
		for( int t=0; t<GetNumTracks(); t++ )
		{
			if( this->GetTapNote(t, i) != TAP_EMPTY && GetTapNoteScore(t, i) == tns )
				iNumSuccessfulTapNotes++;
		}
	}
	
	return iNumSuccessfulTapNotes;
}

int NoteDataWithScoring::GetNumDoublesWithScore( TapNoteScore tns, const float fStartBeat, float fEndBeat )
{
	int iNumSuccessfulDoubles = 0;

	if(fEndBeat == -1)
		fEndBeat = GetMaxBeat()+1;

	unsigned iStartIndex = BeatToNoteRow( fStartBeat );
	unsigned iEndIndex = BeatToNoteRow( fEndBeat );

	for( unsigned i=iStartIndex; i<min(iEndIndex, m_TapNoteScores[0].size()); i++ )
	{
		int iNumNotesThisIndex = 0;
		TapNoteScore	minTapNoteScore = TNS_MARVELOUS;
		for( int t=0; t<GetNumTracks(); t++ )
		{
			if( GetTapNote(t, i) != TAP_EMPTY )
			{
				iNumNotesThisIndex++;
				minTapNoteScore = min( minTapNoteScore, GetTapNoteScore(t, i) );
			}
		}
		if( iNumNotesThisIndex >= 2  &&  minTapNoteScore == tns )
			iNumSuccessfulDoubles++;
	}
	
	return iNumSuccessfulDoubles;
}

int NoteDataWithScoring::GetNumHoldNotesWithScore( HoldNoteScore hns, const float fStartBeat, float fEndBeat )
{
	int iNumSuccessfulHolds = 0;

	if(fEndBeat == -1)
		fEndBeat = GetMaxBeat()+1;

	for( int i=0; i<GetNumHoldNotes(); i++ )
	{
		const HoldNote &hn = GetHoldNote(i);
		if( fStartBeat <= hn.fStartBeat  &&  hn.fEndBeat <= fEndBeat  &&  m_HoldNoteScores[i] == hns )
			iNumSuccessfulHolds++;
	}
	return iNumSuccessfulHolds;
}

bool NoteDataWithScoring::IsRowComplete( int index )
{
	for( int t=0; t<GetNumTracks(); t++ )
		if( GetTapNote(t, index) != TAP_EMPTY && GetTapNoteScore(t, index) < TNS_GREAT )
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
		GetNumTapNotesWithScore(TNS_MARVELOUS) + 
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

	const int fEndBeat = (int) GetMaxBeat()+1;

	for( int i=0; i<fEndBeat; i+=BEAT_WINDOW )
	{
		int iNumNotesThisWindow = 0;
		iNumNotesThisWindow += GetNumTapNotesWithScore(TNS_MARVELOUS,(float)i,(float)i+BEAT_WINDOW);
		iNumNotesThisWindow += GetNumTapNotesWithScore(TNS_PERFECT,(float)i,(float)i+BEAT_WINDOW);
		iNumNotesThisWindow += GetNumHoldNotesWithScore(HNS_OK,(float)i,(float)i+BEAT_WINDOW);
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
		GetNumDoublesWithScore(TNS_MARVELOUS) + 
		GetNumDoublesWithScore(TNS_PERFECT) + 
		GetNumDoublesWithScore(TNS_GREAT)/2;
	float fReturn = iNumDoubles / fSongSeconds;
	return min( fReturn, 1.0f );
}

float NoteDataWithScoring::GetActualChaosRadarValue( float fSongSeconds )
{
	// count number of triplets
	int iNumChaosNotesCompleted = 0;
	for( unsigned r=0; r<m_TapNoteScores[0].size(); r++ )
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

template<class T>
void extend(vector<T> &v, T val, unsigned pos)
{
	int needed = pos - v.size() + 1;
	if(needed > 0)
	{
		needed += 100; /* optimization: give it a little more than it needs */
		v.insert(v.end(), needed, val);
	}
}

TapNoteScore NoteDataWithScoring::GetTapNoteScore(unsigned track, unsigned row) const
{
	if(row >= m_TapNoteScores[track].size())
		return TNS_NONE;
	return m_TapNoteScores[track][row];
}

void NoteDataWithScoring::SetTapNoteScore(unsigned track, unsigned row, TapNoteScore tns)
{
	extend(m_TapNoteScores[track], TNS_NONE, row);
	m_TapNoteScores[track][row] = tns;
}

HoldNoteScore NoteDataWithScoring::GetHoldNoteScore(unsigned h) const
{
	if(h >= m_HoldNoteScores.size())
		return HNS_NONE;
	return m_HoldNoteScores[h];
}

void NoteDataWithScoring::SetHoldNoteScore(unsigned h, HoldNoteScore hns)
{
	extend(m_HoldNoteScores, HNS_NONE, h);
	m_HoldNoteScores[h] = hns;
}

void NoteDataWithScoring::SetHoldNoteLife(unsigned h, float f)
{
	extend(m_fHoldNoteLife, 1.0f, h);
	m_fHoldNoteLife[h] = f;
}

float NoteDataWithScoring::GetHoldNoteLife(unsigned h) const
{
	// start with full life
	if(h >= m_fHoldNoteLife.size())
		return 1.0f;
	return m_fHoldNoteLife[h];
}

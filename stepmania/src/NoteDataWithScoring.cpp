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

int NoteDataWithScoring::GetMaxCombo() const
{
	int iNumSuccessfulTapNotes = 0;
	int fEndBeat = GetMaxBeat()+1;

	unsigned iStartIndex = BeatToNoteRow( (float)0 );
	unsigned iEndIndex = BeatToNoteRow( fEndBeat );

	int combo = 0;
	int maxcombo = 0;

	for( unsigned i=iStartIndex; i<min(float(iEndIndex), float(m_TapNoteScores[0].size())); i++ )
	{
		for( int t=0; t<GetNumTracks(); t++ )
		{
			if( this->GetTapNote(t, i) != TAP_EMPTY )
				if (GetTapNoteScore(t, i) >= TNS_GREAT)
					combo++;
				else
				{
					if (combo > maxcombo) maxcombo = combo;
					combo = 0;
				}
		}
	}
	
	return max(combo, maxcombo);
}

int NoteDataWithScoring::GetNumTapNotesWithScore( TapNoteScore tns, const float fStartBeat, float fEndBeat ) const
{ 
	int iNumSuccessfulTapNotes = 0;

	if(fEndBeat == -1)
		fEndBeat = GetMaxBeat()+1;

	unsigned iStartIndex = BeatToNoteRow( fStartBeat );
	unsigned iEndIndex = BeatToNoteRow( fEndBeat );

	for( unsigned i=iStartIndex; i<min(float(iEndIndex), float(m_TapNoteScores[0].size())); i++ )
	{
		for( int t=0; t<GetNumTracks(); t++ )
		{
			if( this->GetTapNote(t, i) != TAP_EMPTY && GetTapNoteScore(t, i) == tns )
				iNumSuccessfulTapNotes++;
		}
	}
	
	return iNumSuccessfulTapNotes;
}

int NoteDataWithScoring::GetNumDoublesWithScore( TapNoteScore tns, const float fStartBeat, float fEndBeat ) const
{
	int iNumSuccessfulDoubles = 0;

	if(fEndBeat == -1)
		fEndBeat = GetMaxBeat()+1;

	unsigned iStartIndex = BeatToNoteRow( fStartBeat );
	unsigned iEndIndex = BeatToNoteRow( fEndBeat );

	for( unsigned i=iStartIndex; i<min(static_cast<float>(iEndIndex), static_cast<float>(m_TapNoteScores[0].size())); i++ )
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

int NoteDataWithScoring::GetNumHoldNotesWithScore( HoldNoteScore hns, const float fStartBeat, float fEndBeat ) const
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

/* Return the minimum tap score of a row.  If the row isn't complete (not all
 * taps have been hit), return TNS_NONE or TNS_MISS. */
TapNoteScore NoteDataWithScoring::MinTapNoteScore(unsigned row) const
{
	TapNoteScore score = TNS_MARVELOUS;
	for( int t=0; t<GetNumTracks(); t++ )
	{
		/* If there's no tap note on this row, skip it, or else the score will always be TNS_NONE. */
		if(GetTapNote(t, row) == TAP_EMPTY) 
			continue;
		score = min( score, GetTapNoteScore(t, row) );
	}

	return score;
}

bool NoteDataWithScoring::IsRowCompletelyJudged(unsigned row) const
{
	return MinTapNoteScore(row) >= TNS_MISS;
}

/* Return the last tap score of a row: the grade of the tap that completed
 * the row.  If the row has no tap notes, return -1.  If any tap notes aren't
 * graded (any tap is TNS_NONE) or are missed (TNS_MISS), return it. */
int NoteDataWithScoring::LastTapNoteScoreTrack(unsigned row) const
{
	float scoretime = -9999;
	int best_track = -1;
	for( int t=0; t<GetNumTracks(); t++ )
	{
		/* If there's no tap note on this row, skip it; the score will always be TNS_NONE. */
		if(GetTapNote(t, row) == TAP_EMPTY) continue;

		TapNoteScore tns = GetTapNoteScore(t, row);
		if(tns == TNS_NONE || tns == TNS_MISS) return t;

		float tm = GetTapNoteOffset(t, row);
		if(tm < scoretime) continue;
		
		scoretime = tm;
		best_track = t;
	}

	return best_track;
}

TapNoteScore NoteDataWithScoring::LastTapNoteScore(unsigned row) const
{
	int track = LastTapNoteScoreTrack(row);
	if(track == -1) return TNS_NONE;
	return GetTapNoteScore(track, row);
}


float NoteDataWithScoring::GetActualRadarValue( RadarCategory rv, float fSongSeconds ) const
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

float NoteDataWithScoring::GetActualStreamRadarValue( float fSongSeconds ) const
{
/*	// density of steps
	int iNumSuccessfulNotes = 
		GetNumTapNotesWithScore(TNS_MARVELOUS) + 
		GetNumTapNotesWithScore(TNS_PERFECT) + 
		GetNumTapNotesWithScore(TNS_GREAT)/2 + 
		GetNumHoldNotesWithScore(HNS_OK);
	float fNotesPerSecond = iNumSuccessfulNotes/fSongSeconds;
	float fReturn = fNotesPerSecond / 7;
	return min( fReturn, 1.0f );
	*/

	int MaxCombo = GetMaxCombo();
	float TotalSteps = GetNumTapNotes();

	return min( (float)MaxCombo/TotalSteps, 1.0f);
}

float NoteDataWithScoring::GetActualVoltageRadarValue( float fSongSeconds ) const
{
	// voltage is essentialy perfects divided by # of steps
	float totalnotes = GetNumTapNotes();
	float perfects = GetNumTapNotesWithScore(TNS_PERFECT) + GetNumTapNotesWithScore(TNS_MARVELOUS);

	float result = perfects/totalnotes;
	return result;

/*	float fAvgBPS = GetLastBeat() / fSongSeconds;

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
	*/
}

float NoteDataWithScoring::GetActualAirRadarValue( float fSongSeconds ) const
{
	// number of doubles
	int iNumDoubles = 
		GetNumDoublesWithScore(TNS_MARVELOUS) + 
		GetNumDoublesWithScore(TNS_PERFECT) + 
		GetNumDoublesWithScore(TNS_GREAT)/2;
//	float fReturn = iNumDoubles / fSongSeconds;
	int iTotalDoubles = GetNumDoubles();

	if (iTotalDoubles == 0) return 1.0f;  // no jumps in song
	float fReturn = (float)iNumDoubles / iTotalDoubles;
	return min( fReturn, 1.0f );
}

float NoteDataWithScoring::GetActualChaosRadarValue( float fSongSeconds ) const
{
/*	// count number of triplets
	int iNumChaosNotesCompleted = 0;
	for( unsigned r=0; r<m_TapNoteScores[0].size(); r++ )
	{
		if( !IsRowEmpty(r) && MinTapNoteScore(r) >= TNS_GREAT && GetNoteType(r) >= NOTE_TYPE_12TH )
			iNumChaosNotesCompleted++;
	}

	float fReturn = iNumChaosNotesCompleted / fSongSeconds * 0.5f;
	return min( fReturn, 1.0f );
	*/

	float marvelous = GetNumTapNotesWithScore(TNS_MARVELOUS);
	float perfect = GetNumTapNotesWithScore(TNS_PERFECT);
	float great = GetNumTapNotesWithScore(TNS_GREAT);
	float good = GetNumTapNotesWithScore(TNS_GOOD);
	float boo = GetNumTapNotesWithScore(TNS_BOO);
	float miss = GetNumTapNotesWithScore(TNS_MISS);

	float ok = GetNumHoldNotesWithScore(HNS_OK);
	float holdnotes = GetNumHoldNotes();

	float DPEarned =  2 * (marvelous + perfect)
					+     (great)
					- 4 * (boo)
					- 8 * (miss)
					+ 6 * (ok);

	float TotalDP =	2 * (marvelous + perfect + great
						+ good + boo + miss)
						+ 6 * holdnotes;

	float fResult = DPEarned/TotalDP;

	return min( fResult, 1.0f);
}

float NoteDataWithScoring::GetActualFreezeRadarValue( float fSongSeconds ) const
{
	// number of hold steps
	float totalsteps = (float)GetNumHoldNotes();
	if (totalsteps == 0) return 1.0f;
	float fReturn = GetNumHoldNotesWithScore(HNS_OK) / totalsteps;
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

float NoteDataWithScoring::GetTapNoteOffset(unsigned track, unsigned row) const
{
	if(row >= m_TapNoteOffset[track].size())
		return 0;
	return m_TapNoteOffset[track][row];
}

void NoteDataWithScoring::SetTapNoteOffset(unsigned track, unsigned row, float offset)
{
	extend(m_TapNoteOffset[track], 0.f, row);
	m_TapNoteOffset[track][row] = offset;
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

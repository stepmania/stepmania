#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: NoteDataWithScoring

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Glenn Maynard
-----------------------------------------------------------------------------
*/

#include "NoteDataWithScoring.h"
#include "GameState.h"
#include "RageUtil.h"
#include "StageStats.h"

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
			if( this->GetTapNote(t, i) != TAP_EMPTY && GetTapNoteScore(t, i) >= tns )
				iNumSuccessfulTapNotes++;
		}
	}
	
	return iNumSuccessfulTapNotes;
}

int NoteDataWithScoring::GetNumNWithScore( TapNoteScore tns, int MinTaps, const float fStartBeat, float fEndBeat ) const
{
	if( fEndBeat == -1 )
		fEndBeat = GetMaxBeat();

	int iStartIndex = BeatToNoteRow( fStartBeat );
	int iEndIndex = BeatToNoteRow( fEndBeat );

	iStartIndex = max( iStartIndex, 0 );
	iEndIndex = min( iEndIndex, GetMaxRow()-1 );

	int iNumSuccessfulDoubles = 0;
	for( int i=iStartIndex; i<=iEndIndex; i++ )
	{
		int iNumNotesThisIndex = 0;
		TapNoteScore	minTapNoteScore = TNS_MARVELOUS;
		for( int t=0; t<GetNumTracks(); t++ )
		{
			switch( GetTapNote(t, i) )
			{
			case TAP_TAP:		
			case TAP_HOLD_HEAD: 
			case TAP_ADDITION:	
				iNumNotesThisIndex++;
				minTapNoteScore = min( minTapNoteScore, GetTapNoteScore(t, i) );
			}
		}
		if( iNumNotesThisIndex >= MinTaps && minTapNoteScore >= tns )
			iNumSuccessfulDoubles++;
	}
	
	return iNumSuccessfulDoubles;
}

int NoteDataWithScoring::GetNumHoldNotesWithScore( HoldNoteScore hns, const float fStartBeat, float fEndBeat ) const
{
	int iNumSuccessfulHolds = 0;

	if(fEndBeat == -1)
		fEndBeat = GetMaxBeat();

	int iStartIndex = BeatToNoteRow( fStartBeat );
	int iEndIndex = BeatToNoteRow( fEndBeat );

	for( int i=0; i<GetNumHoldNotes(); i++ )
	{
		const HoldNote &hn = GetHoldNote(i);
		if( iStartIndex > hn.iStartRow ||  hn.iEndRow > iEndIndex )
			continue;
		if( GetHoldNoteScore(hn) == hns )
			iNumSuccessfulHolds++;
	}
	return iNumSuccessfulHolds;
}

int NoteDataWithScoring::GetSuccessfulMines( float fStartBeat, float fEndBeat ) const
{
	if( fEndBeat == -1 )
		fEndBeat = GetMaxBeat();

	int iStartIndex = BeatToNoteRow( fStartBeat );
	int iEndIndex = BeatToNoteRow( fEndBeat );

	iStartIndex = max( iStartIndex, 0 );
	iEndIndex = min( iEndIndex, GetMaxRow()-1 );

	int iNumSuccessfulMinesNotes = 0;
	for( int i=iStartIndex; i<=iEndIndex; i++ )
	{
		for( int t=0; t<GetNumTracks(); t++ )
		{
			if( this->GetTapNote(t, i) == TAP_MINE && GetTapNoteScore(t, i) != TNS_HIT_MINE )
				iNumSuccessfulMinesNotes++;
		}
	}
	
	return iNumSuccessfulMinesNotes;
}

/* See NoteData::GetNumHands(). */
int NoteDataWithScoring::GetSuccessfulHands( float fStartBeat, float fEndBeat ) const
{
	if( fEndBeat == -1 )
		fEndBeat = GetMaxBeat();

	int iStartIndex = BeatToNoteRow( fStartBeat );
	int iEndIndex = BeatToNoteRow( fEndBeat );

	/* Clamp to known-good ranges. */
	iStartIndex = max( iStartIndex, 0 );
	iEndIndex = min( iEndIndex, GetMaxRow()-1 );

	int iNum = 0;
	for( int i=iStartIndex; i<=iEndIndex; i++ )
	{
		if( !RowNeedsHands(i) )
			continue;

		bool Missed = false;
		for( int t=0; t<GetNumTracks(); t++ )
		{
			TapNote tn = GetTapNoteX(t, i);
			if( tn == TAP_MINE || tn == TAP_EMPTY ) // mines don't count
				continue;
			if( GetTapNoteScore(t, i) <= TNS_BOO )
				Missed = true;
		}

		if( Missed )
			continue;

		/* Check hold scores. */
		for( int j=0; j<GetNumHoldNotes(); j++ )
		{
			const HoldNote &hn = GetHoldNote(j);
			if( hn.iStartRow+1 <= i && i <= hn.iEndRow && GetHoldNoteScore(hn) != HNS_OK )
				Missed = true;
		}

		if( !Missed )
			iNum++;
	}

	return iNum;
}

/* Return the minimum tap score of a row.  If the row isn't complete (not all
 * taps have been hit), return TNS_NONE or TNS_MISS. */
TapNoteScore NoteDataWithScoring::MinTapNoteScore(unsigned row) const
{
	TapNoteScore score = TNS_MARVELOUS;
	for( int t=0; t<GetNumTracks(); t++ )
	{
		/* Don't coun, or else the score 
		 * will always be TNS_NONE. */
		TapNote tn = GetTapNote(t, row);
		if( tn == TAP_EMPTY || tn == TAP_MINE ) 
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
		/* Skip empty tracks and mines */
		TapNote tn = GetTapNote(t, row);
		if( tn == TAP_EMPTY || tn == TAP_MINE ) 
			continue;

		TapNoteScore tns = GetTapNoteScore(t, row);
		
		if( tns == TNS_MISS || tns == TNS_NONE )
			return t;

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

/* From aaroninjapan.com (http://www.aaroninjapan.com/ddr2.html)
 *
 * Stream: The ratio of your number of Perfects to getting all Perfects 
 * Voltage: The ratio of your maximum combo to getting a Full Combo 
 * Air: The ratio of your number of Perfects on all your jumps (R+L, R+D, etc) to getting all Perfects on all Jumps 
 * Chaos: The ratio of your dance level compared to that of AAA (all Perfect) level 
 * Freeze: The ratio of your total number of "OK"s on all the Freeze arrows to getting all "OK" on all the Freeze arrows 
 *
 * I don't think chaos is correct, at least relative to DDREX. You can AA songs and get a full Chaos graph.
 * No idea what it actually could be, though.
 */

float NoteDataWithScoring::GetActualRadarValue( RadarCategory rv, PlayerNumber pn, float fSongSeconds ) const
{
	switch( rv )
	{
	case RADAR_STREAM:	return GetActualStreamRadarValue( fSongSeconds, pn );	break;
	case RADAR_VOLTAGE:	return GetActualVoltageRadarValue( fSongSeconds, pn );	break;
	case RADAR_AIR:		return GetActualAirRadarValue( fSongSeconds, pn );		break;
	case RADAR_FREEZE:	return GetActualFreezeRadarValue( fSongSeconds, pn );	break;
	case RADAR_CHAOS:	return GetActualChaosRadarValue( fSongSeconds, pn );	break;
	case RADAR_NUM_TAPS_AND_HOLDS: return (float) GetNumNWithScore( TNS_GOOD, 1 );
	case RADAR_NUM_JUMPS: return (float) GetNumNWithScore( TNS_GOOD, 2 );
	case RADAR_NUM_HOLDS: return (float) GetNumHoldNotesWithScore( HNS_OK );
	case RADAR_NUM_MINES: return (float) GetSuccessfulMines();
	case RADAR_NUM_HANDS: return (float) GetSuccessfulHands();
	default: ASSERT(0);   return 0;
	}
}

float NoteDataWithScoring::GetActualStreamRadarValue( float fSongSeconds, PlayerNumber pn ) const
{
	int TotalSteps = GetNumTapNotes();
	if( !TotalSteps )
		return 1;

	const int Perfects = GetNumTapNotesWithScore(TNS_PERFECT);
	return clamp( float(Perfects)/TotalSteps, 0.0f, 1.0f );
}

float NoteDataWithScoring::GetActualVoltageRadarValue( float fSongSeconds, PlayerNumber pn ) const
{
	/* g_CurStageStats.iMaxCombo is unrelated to GetNumTapNotes: m_bComboContinuesBetweenSongs
	 * might be on, and the way combo is counted varies depending on the mode and score
	 * keeper.  Instead, let's use the length of the longest recorded combo.  This is
	 * only subtly different: it's the percent of the song the longest combo took to get. */
	const StageStats::Combo_t MaxCombo = g_CurStageStats.GetMaxCombo( pn );
	float fComboPercent = SCALE( MaxCombo.size, 0, g_CurStageStats.fLastPos[pn]-g_CurStageStats.fFirstPos[pn], 0.0f, 1.0f );
	return clamp( fComboPercent, 0.0f, 1.0f );
}

float NoteDataWithScoring::GetActualAirRadarValue( float fSongSeconds, PlayerNumber pn ) const
{
	const int iTotalDoubles = GetNumDoubles();
	if (iTotalDoubles == 0)
		return 1;  // no jumps in song

	// number of doubles
	const int iNumDoubles = GetNumNWithScore( TNS_PERFECT, 2 );
	return clamp( (float)iNumDoubles / iTotalDoubles, 0.0f, 1.0f );
}

float NoteDataWithScoring::GetActualChaosRadarValue( float fSongSeconds, PlayerNumber pn ) const
{
	const int PossibleDP = g_CurStageStats.iPossibleDancePoints[pn];
	if ( PossibleDP == 0 )
		return 1;

	const int ActualDP = g_CurStageStats.iActualDancePoints[pn];
	return clamp( float(ActualDP)/PossibleDP, 0.0f, 1.0f );
}

float NoteDataWithScoring::GetActualFreezeRadarValue( float fSongSeconds, PlayerNumber pn ) const
{
	// number of hold steps
	const int TotalHolds = GetNumHoldNotes();
	if ( TotalHolds == 0 )
		return 1.0f;

	const int ActualHolds = GetNumHoldNotesWithScore(HNS_OK);
	return clamp( float(ActualHolds) / TotalHolds, 0.0f, 1.0f );
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

/* We use the end row to index hold notes, instead of the start row, because the start row
 * changes when hold notes are being stepped on, but end rows never change. */
HoldNoteScore NoteDataWithScoring::GetHoldNoteScore( const HoldNote &hn ) const
{
	map<RowTrack, HoldNoteScore>::const_iterator it = m_HoldNoteScores.find( RowTrack(hn) );
	if( it == m_HoldNoteScores.end() )
		return HNS_NONE;
	return it->second;
}

void NoteDataWithScoring::SetHoldNoteScore( const HoldNote &hn, HoldNoteScore hns )
{
	m_HoldNoteScores[RowTrack(hn)] = hns;
}

void NoteDataWithScoring::SetHoldNoteLife( const HoldNote &hn, float f )
{
	m_fHoldNoteLife[RowTrack(hn)] = f;
}

float NoteDataWithScoring::GetHoldNoteLife( const HoldNote &hn ) const
{
	map<RowTrack, float>::const_iterator it = m_fHoldNoteLife.find( RowTrack(hn) );
	if( it == m_fHoldNoteLife.end() )
		return 1.0f;
	return it->second;
}

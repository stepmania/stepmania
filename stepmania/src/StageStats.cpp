#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: StageStats

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "StageStats.h"
#include "GameState.h"
#include "RageLog.h"
#include "SongManager.h"
#include "RageUtil.h"

StageStats::StageStats()
{
	memset( this, 0, sizeof(StageStats) );
	for( int p=0; p<NUM_PLAYERS; p++ )
		for( int i = 0; i < LIFE_RECORD_RESOLUTION; ++i )
			fLifeRecord[p][i] = -1;
}

void StageStats::AddStats( const StageStats& other )
{
	pSong = NULL;		// meaningless
	StageType = STAGE_INVALID; // meaningless
	memset( fAliveSeconds, 0, sizeof(fAliveSeconds) );
	
	// weight long and marathon songs
	ASSERT( other.pSong );
	const int iLengthMultiplier = SongManager::GetNumStagesForSong( other.pSong );

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		pSteps[p] = NULL;
		iMeter[p] += other.iMeter[p] * iLengthMultiplier;
		fAliveSeconds[p] += other.fAliveSeconds[p];
		bFailed[p] |= other.bFailed[p];
		bFailedEarlier[p] |= other.bFailedEarlier[p];
		iPossibleDancePoints[p] += other.iPossibleDancePoints[p];
		iActualDancePoints[p] += other.iActualDancePoints[p];
		
		for( int t=0; t<NUM_TAP_NOTE_SCORES; t++ )
			iTapNoteScores[p][t] += other.iTapNoteScores[p][t];
		for( int h=0; h<NUM_HOLD_NOTE_SCORES; h++ )
			iHoldNoteScores[p][h] += other.iHoldNoteScores[p][h];
		iCurCombo[p] += other.iCurCombo[p];
		iMaxCombo[p] += other.iMaxCombo[p];
		iCurMissCombo[p] += other.iCurMissCombo[p];
		iScore[p] += other.iScore[p];
		for( int r=0; r<NUM_RADAR_CATEGORIES; r++ )
		{
			fRadarPossible[p][r] += other.fRadarPossible[p][r];
			fRadarActual[p][r] += other.fRadarActual[p][r];
		}
		fSecondsBeforeFail[p] += other.fSecondsBeforeFail[p];
		iSongsPassed[p] += other.iSongsPassed[p];
		iSongsPlayed[p] += other.iSongsPlayed[p];
		iTotalError[p] += other.iTotalError[p];

		for( int i = 0; i < LIFE_RECORD_RESOLUTION; ++i )
			fLifeRecord[p][i] = -1;
	}
}

Grade StageStats::GetGrade( PlayerNumber pn )
{
	ASSERT( GAMESTATE->IsPlayerEnabled(pn) );	// should be calling this is player isn't joined!

	if( bFailedEarlier[pn] )
		return GRADE_E;

	/* XXX: This entire calculation should be in ScoreKeeper, but final evaluation
	 * is tricky since at that point the ScoreKeepers no longer exist. 
	 *
	 * See http://www.aaroninjapan.com/ddr2.html ("Regular play scoring") */
	const float TapScoreValues[NUM_TAP_NOTE_SCORES] = { 0, -8, -4, 0, +1, +2, +2 };
	const float HoldScoreValues[NUM_HOLD_NOTE_SCORES] = { 0, 0, +6 };

	float Possible = 0, Actual = 0;
	int i;
	for( i = TNS_MISS; i < NUM_TAP_NOTE_SCORES; ++i )
	{
		Actual += iTapNoteScores[pn][i] * TapScoreValues[i];
		Possible += iTapNoteScores[pn][i] * TapScoreValues[TNS_MARVELOUS];
	}

	for( i = HNS_OK; i < NUM_HOLD_NOTE_SCORES; ++i )
	{
		Actual += iHoldNoteScores[pn][i] * HoldScoreValues[i];
		Possible += iHoldNoteScores[pn][i] * HoldScoreValues[HNS_OK];
	}

	float fPercentDancePoints = Actual / Possible;
	fPercentDancePoints = max( 0.f, fPercentDancePoints );
	LOG->Trace( "GetGrade: Actual: %f, Possible: %f, Percent: %f", Actual, Possible, fPercentDancePoints );

	/* check for "AAAA".  Check DP == 100%: if we're using eg. "LITTLE", we might have all
	 * marvelouses but still not qualify for a AAAA. */
	if( fPercentDancePoints > .9999 &&
		iTapNoteScores[pn][TNS_MARVELOUS] > 0 &&
		iTapNoteScores[pn][TNS_PERFECT] == 0 &&
		iTapNoteScores[pn][TNS_GREAT] == 0 &&
		iTapNoteScores[pn][TNS_GOOD] == 0 &&
		iTapNoteScores[pn][TNS_BOO] == 0 &&
		iTapNoteScores[pn][TNS_MISS] == 0 &&
		iHoldNoteScores[pn][HNS_NG] == 0 )
		return GRADE_AAAA;

	/* Based on the percentage of your total "Dance Points" to the maximum
	 * possible number, the following rank is assigned: 
	 *
	 * 100% - AAA
	 *  93% - AA
	 *  80% - A
	 *  65% - B
	 *  45% - C
	 * Less - D
	 * Fail - E
	 */

	if     ( fPercentDancePoints == 1.00 )		return GRADE_AAA;
	else if( fPercentDancePoints >= 0.93 )		return GRADE_AA;
	else if( fPercentDancePoints >= 0.80 )		return GRADE_A;
	else if( fPercentDancePoints >= 0.65 )		return GRADE_B;
	else if( fPercentDancePoints >= 0.45 )		return GRADE_C;
	else if( fPercentDancePoints >= 0    )		return GRADE_D;
	else										return GRADE_E;
}

bool StageStats::OnePassed() const
{
	for( int p=0; p<NUM_PLAYERS; p++ )
		if( GAMESTATE->IsHumanPlayer(p) && !bFailed[p] )
			return true;
	return false;
}

float StageStats::GetPercentDancePoints( PlayerNumber pn ) const
{
	if( iPossibleDancePoints[pn] == 0 )
		return 0; // div/0

	if( iActualDancePoints[pn] == iPossibleDancePoints[pn] )
		return 1;	// correct for rounding error

	float fPercentDancePoints =  iActualDancePoints[pn] / (float)iPossibleDancePoints[pn];
	return clamp( fPercentDancePoints, 0, 1 );
}

void StageStats::SetLifeRecord( PlayerNumber pn, float life, float pos )
{
	pos = clamp( pos, 0, 1 );

	int offset = (int) roundf( pos * LIFE_RECORD_RESOLUTION );
	for( int i = 0; i <= offset; ++i )
		if( fLifeRecord[pn][i] < 0 )
			fLifeRecord[pn][i] = life;
}

void StageStats::GetLifeRecord( PlayerNumber pn, float *life, int nout ) const
{
	for( int i = 0; i < nout; ++i )
	{
		int from = i * (LIFE_RECORD_RESOLUTION-1) / nout;
		life[i] = fLifeRecord[pn][from];
	}
}

void StageStats::UpdateComboList( PlayerNumber pn, float pos )
{
	pos = clamp( pos, 0, 1 );
	const int cnt = iCurCombo[pn];
	if( !cnt )
		return; /* no combo */

	if( ComboList[pn].size() == 0 || ComboList[pn].back().cnt >= cnt )
	{
		LOG->Trace("new combo, start %f", pos);
		/* This is a new combo. */
		Combo_t NewCombo;
		NewCombo.start = pos;
		ComboList[pn].push_back( NewCombo );
	}

	Combo_t &combo = ComboList[pn].back();
	combo.size = pos - combo.start;
	combo.cnt = cnt;
		LOG->Trace("combo pos %f, size %f, %i", pos, combo.size, combo.cnt );
}

/* SetLifeRecord and UpdateComboList take a percentage (0..1) in pos, but the values
 * will actually be somewhat off as they're based on Song::m_fFirstBeat and m_fLastBeat,
 * which are per-song, not per-Steps.  Scale and shift our records so that they're
 * really 0..1. */
void StageStats::Finish()
{
	for( int pn = 0; pn < NUM_PLAYERS; ++pn )
	{
		if( ComboList[pn].size() == 0 )
			continue;

		const float First = ComboList[pn].front().start;
		const float Last = ComboList[pn].back().start + ComboList[pn].back().size;
		if( fabsf(First-Last) < 0.0001f )
			continue;

		LOG->Trace("xxx %f ... %f", First, Last );//, Scale);
		unsigned i;
		for( i = 0; i < ComboList[pn].size(); ++i )
		{
			ComboList[pn][i].start = SCALE( ComboList[pn][i].start, First, Last, 0.0f, 1.0f );
			ComboList[pn][i].size /= Last - First;
		}

		float NewLifeRecord[LIFE_RECORD_RESOLUTION];
		for( i = 0; i < LIFE_RECORD_RESOLUTION; ++i )
		{
			int from = (int) roundf( SCALE( i, 0.0f, 1.0f, First, Last ) );
			from = clamp( from, 0, LIFE_RECORD_RESOLUTION-1 );
			NewLifeRecord[i] = fLifeRecord[pn][from];
		}
		memcpy( fLifeRecord[pn], NewLifeRecord, sizeof(fLifeRecord[pn]) );
	}
}


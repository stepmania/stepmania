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


StageStats::StageStats()
{
	memset( this, 0, sizeof(StageStats) );
}

void StageStats::operator+=( const StageStats& other )
{
	pSong = NULL;		// meaningless
	memset( fAliveSeconds, 0, sizeof(fAliveSeconds) );
	
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		iMeter[p] += other.iMeter[p];
		fAliveSeconds[p] += other.fAliveSeconds[p];
		bFailed[p] |= other.bFailed[p];
		iPossibleDancePoints[p] += other.iPossibleDancePoints[p];
		iActualDancePoints[p] += other.iActualDancePoints[p];
		
		for( int t=0; t<NUM_TAP_NOTE_SCORES; t++ )
			iTapNoteScores[p][t] += other.iTapNoteScores[p][t];
		for( int h=0; h<NUM_HOLD_NOTE_SCORES; h++ )
			iHoldNoteScores[p][h] += other.iHoldNoteScores[p][h];
		iCurCombo[p] += other.iCurCombo[p];
		iMaxCombo[p] += other.iMaxCombo[p];
		iScore[p] += other.iScore[p];
		for( int r=0; r<NUM_RADAR_CATEGORIES; r++ )
		{
			fRadarPossible[p][r] += other.fRadarPossible[p][r];
			fRadarActual[p][r] += other.fRadarActual[p][r];
		}
		fSecondsBeforeFail[p] += other.fSecondsBeforeFail[p];
		iSongsPlayed[p] += other.iSongsPlayed[p];
		iSongsPassed[p] += other.iSongsPassed[p];
	}
}

Grade StageStats::GetGrade( PlayerNumber pn )
{
	ASSERT( GAMESTATE->IsPlayerEnabled(pn) );	// should be calling this is player isn't joined!

	if( bFailed[pn] )
		return GRADE_E;

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
	float fPercentDancePoints = iActualDancePoints[pn] / (float)iPossibleDancePoints[pn];
	fPercentDancePoints = max( 0.f, fPercentDancePoints );
	LOG->Trace( "iActualDancePoints: %i", iActualDancePoints[pn] );
	LOG->Trace( "iPossibleDancePoints: %i", iPossibleDancePoints[pn] );
	LOG->Trace( "fPercentDancePoints: %f", fPercentDancePoints  );

	// check for "AAAA"
	if( iTapNoteScores[pn][TNS_MARVELOUS] > 0 &&
		iTapNoteScores[pn][TNS_PERFECT] == 0 &&
		iTapNoteScores[pn][TNS_GREAT] == 0 &&
		iTapNoteScores[pn][TNS_GOOD] == 0 &&
		iTapNoteScores[pn][TNS_BOO] == 0 &&
		iTapNoteScores[pn][TNS_MISS] == 0 &&
		iHoldNoteScores[pn][HNS_NG] == 0 )
		return GRADE_AAAA;

	if     ( fPercentDancePoints == 1.00 )		return GRADE_AAA;
	else if( fPercentDancePoints >= 0.93 )		return GRADE_AA;
	else if( fPercentDancePoints >= 0.80 )		return GRADE_A;
	else if( fPercentDancePoints >= 0.65 )		return GRADE_B;
	else if( fPercentDancePoints >= 0.45 )		return GRADE_C;
	else if( fPercentDancePoints >= 0    )		return GRADE_D;
	else										return GRADE_E;
}

#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: StageStats

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "StageStats.h"

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
		iActualDancePoints[p] += other.iPossibleDancePoints[p];
		
		for( int t=0; t<NUM_TAP_NOTE_SCORES; t++ )
			iTapNoteScores[p][t] += other.iTapNoteScores[p][t];
		for( int h=0; h<NUM_HOLD_NOTE_SCORES; h++ )
			iHoldNoteScores[p][h] += iHoldNoteScores[p][h];
		iMaxCombo[p] += other.iMaxCombo[p];
		fScore[p] += other.fScore[p];
		for( int r=0; r<NUM_RADAR_VALUES; r++ )
		{
			fRadarPossible[p][r] += other.fRadarPossible[p][r];
			fRadarActual[p][r] += other.fRadarActual[p][r];
		}
		fSecondsBeforeFail[p] += other.fSecondsBeforeFail[p];
		iSongsPassed[p] += other.iSongsPassed[p];
	}
}


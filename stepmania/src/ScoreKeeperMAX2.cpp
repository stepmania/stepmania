#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScoreKeeperMAX2

 Desc: 

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/
#include "ScoreKeeperMAX2.h"
#include "GameState.h"
#include "PrefsManager.h"
#include "Notes.h"

ScoreKeeperMAX2::ScoreKeeperMAX2(Notes *notes, PlayerNumber pn_):
	ScoreKeeper(pn_)
{
//	Stats_DoublesCount = true;

	NoteData noteData;
	notes->GetNoteData( &noteData );

	int Meter = notes? notes->GetMeter() : 5;
	Meter = min(Meter, 10);

	int N = noteData.GetNumRowsWithTaps() + noteData.GetNumHoldNotes();
	
	int sum = (N * (N + 1)) / 2;

	if(sum)
		m_fScoreMultiplier = float(Meter * 1000000) / sum;
	else /* avoid div/0 on empty songs */
		m_fScoreMultiplier = 0.f;

	ASSERT(m_fScoreMultiplier >= 0.0);

	m_iTapNotesHit = 0;
	m_lScore = 0;
}

void ScoreKeeperMAX2::AddScore( TapNoteScore score )
{
	int p;	// score multiplier 
	
	switch( score )
	{
	case TNS_MARVELOUS:	p = 10;		break;
	case TNS_PERFECT:	p = 10;		break;
	case TNS_GREAT:		p = 5;		break;
	default:			p = 0;		break;
	}

	m_lScore += p * ++m_iTapNotesHit;

	ASSERT(m_lScore >= 0);

	GAMESTATE->m_CurStageStats.fScore[m_PlayerNumber] = m_lScore * m_fScoreMultiplier;
}

void ScoreKeeperMAX2::HandleTapRowScore( TapNoteScore scoreOfLastTap, int iNumTapsInRow )
{
	ASSERT( iNumTapsInRow >= 1 );

	// update dance points
	GAMESTATE->m_CurStageStats.iActualDancePoints[m_PlayerNumber] += TapNoteScoreToDancePoints( scoreOfLastTap );
	GAMESTATE->m_CurStageStats.iTapNoteScores[m_PlayerNumber][scoreOfLastTap] += 1;

/*
  A single step's points are calculated as follows: 
  
  p = score multiplier (Perfect = 10, Great = 5, other = 0)
  N = total number of steps and freeze steps
  S = The sum of all integers from 1 to N (the total number of steps/freeze steps) 
  n = number of the current step or freeze step (varies from 1 to N)
  B = Base value of the song (1,000,000 X the number of feet difficulty) - All edit data is rated as 5 feet
  So, the score for one step is: 
  one_step_score = p * (B/S) * n 
  
  *IMPORTANT* : Double steps (U+L, D+R, etc.) count as two steps instead of one, so 
  if you get a double L+R on the 112th step of a song, you score is calculated with a
  Perfect/Great/whatever for both the 112th and 113th steps. Got it? Now, through simple
  algebraic manipulation 
  S = 1+...+N = (1+N)*N/2 (1 through N added together) 
  Okay, time for an example: 
  
  So, for example, suppose we wanted to calculate the step score of a "Great" on the 57th step of a 441 step, 8-foot difficulty song (I'm just making this one up): 
  
  S = (1 + 441)*441 / 2
  = 194,222 / 2
  = 97,461
  StepScore = p * (B/S) * n
  = 5 * (8,000,000 / 97,461) * 57
  = 5 * (82) * 57 (The 82 is rounded down from 82.08411...)
  = 23,370
  Remember this is just the score for the step, not the cumulative score up to the 57th step. Also, please note that I am currently checking into rounding errors with the system and if there are any, how they are resolved in the system. 
  
  Note: if you got all Perfect on this song, you would get (p=10)*B, which is 80,000,000. In fact, the maximum possible score for any song is the number of feet difficulty X 10,000,000. 
  3dfsux:
  I redid this code so it will store the score as a long, then correct the score for each song based on that value.
  		lScore == p * n
  		m_fScoreMultiplier = (B/S)
   keeping these seperate for as long as possible improves accuracy.

*/
//	for( int i=0; i<iNumTapsInRow; i++ )
	AddScore( scoreOfLastTap );		// only score once per row
}


void ScoreKeeperMAX2::HandleHoldScore( HoldNoteScore holdScore, TapNoteScore tapScore )
{
	// update dance points totals
	GAMESTATE->m_CurStageStats.iHoldNoteScores[m_PlayerNumber][holdScore] ++;
	GAMESTATE->m_CurStageStats.iActualDancePoints[m_PlayerNumber] += HoldNoteScoreToDancePoints( holdScore );

	if( holdScore == HNS_OK )
		AddScore( TNS_PERFECT );
}


int ScoreKeeperMAX2::GetPossibleDancePoints( const NoteData* pNoteData )
{
//Each song has a certain number of "Dance Points" assigned to it. For regular arrows, this is 2 per arrow. For freeze arrows, it is 6 per arrow. When you add this all up, you get the maximum number of possible "Dance Points". 
//
//Your "Dance Points" are calculated as follows: 
//
//A "Marvelous" is worth 3 points, but oniy when in oni mode
//A "Perfect" is worth 2 points
//A "Great" is worth 1 points
//A "Good" is worth 0 points
//A "Boo" will subtract 4 points
//A "Miss" will subtract 8 points
//An "OK" (Successful Freeze step) will add 6 points
//A "NG" (Unsuccessful Freeze step) is worth 0 points

	/* Note that, if Marvelous timing is disabled or not active (not course mode),
	 * PERFECT will be used instead. */

	TapNoteScore maxPossibleTapScore = PREFSMAN->m_bMarvelousTiming ? TNS_MARVELOUS : TNS_PERFECT;

	return pNoteData->GetNumRowsWithTaps()*TapNoteScoreToDancePoints(maxPossibleTapScore)+
	   pNoteData->GetNumHoldNotes()*HoldNoteScoreToDancePoints(HNS_OK);
}


int ScoreKeeperMAX2::TapNoteScoreToDancePoints( TapNoteScore tns )
{
	const bool bOni = GAMESTATE->IsCourseMode();

	if(!PREFSMAN->m_bMarvelousTiming && tns == TNS_MARVELOUS)
		tns = TNS_PERFECT;

	switch( tns )
	{
	case TNS_MARVELOUS:	return bOni ? +3 : +2;
	case TNS_PERFECT:	return +2;
	case TNS_GREAT:		return +1;
	case TNS_GOOD:		return +0;
	case TNS_BOO:		return bOni ? 0 : -4;
	case TNS_MISS:		return bOni ? 0 : -8;
	case TNS_NONE:		return 0;
	default:	ASSERT(0);	return 0;
	}
}

int ScoreKeeperMAX2::HoldNoteScoreToDancePoints( HoldNoteScore hns )
{
	switch( hns )
	{
	case HNS_OK:	return +6;
	case HNS_NG:	return +0;
	default:	ASSERT(0);	return 0;
	}
}


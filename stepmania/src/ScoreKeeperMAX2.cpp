#include "global.h"
#include "ScoreKeeperMAX2.h"
#include "GameState.h"

ScoreKeeperMAX2::ScoreKeeperMAX2(Notes *notes, NoteDataWithScoring &data, PlayerNumber pn_):
	ScoreKeeper(pn_)
{
//	Stats_DoublesCount = true;

	int Meter = notes? notes->GetMeter() : 5;
	Meter = min(Meter, 10);

	/* Hold notes count as two tap notes.  However, hold notes already count
	 * as 1 in GetNumTapNotes(), so only add 1*GetNumHoldNotes, not 2. */
	int iNumTapNotes = data.GetNumTapNotes() + data.GetNumHoldNotes();
	
	int sum = (iNumTapNotes * (iNumTapNotes + 1)) / 2;

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

void ScoreKeeperMAX2::HandleNoteScore( TapNoteScore score, int iNumTapsInRow )
{
	ScoreKeeper::HandleNoteScore(score, iNumTapsInRow);

	ASSERT( iNumTapsInRow >= 1 );

	// update dance points for Oni lifemeter
	GAMESTATE->m_CurStageStats.iActualDancePoints[m_PlayerNumber] += iNumTapsInRow * TapNoteScoreToDancePoints( score );
	GAMESTATE->m_CurStageStats.iTapNoteScores[m_PlayerNumber][score] += iNumTapsInRow;

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
	for( int i=0; i<iNumTapsInRow; i++ )
		AddScore( score );
}


void ScoreKeeperMAX2::HandleHoldNoteScore( HoldNoteScore score, TapNoteScore TapNoteScore )
{
	// update dance points totals
	GAMESTATE->m_CurStageStats.iHoldNoteScores[m_PlayerNumber][score] ++;
	GAMESTATE->m_CurStageStats.iActualDancePoints[m_PlayerNumber] += HoldNoteScoreToDancePoints( score );

	if( score == HNS_OK )
		AddScore( TNS_PERFECT );
}

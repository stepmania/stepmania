#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScoreKeeper5th

 Desc: Modified scoring for 5th mix, based on MAX2

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/
#include "ScoreKeeper5th.h"
#include "GameState.h"
#include "PrefsManager.h"
#include "Steps.h"
#include "PrefsManager.h"
#include "ScreenManager.h"
#include "ScreenGameplay.h"
#include "GameState.h"
#include "Course.h"
#include "UnlockSystem.h"
#include "SDL_utils.h"
#include "SongManager.h"
#include "NoteDataUtil.h"


ScoreKeeper5th::ScoreKeeper5th( const vector<Steps*>& apNotes_, const CStringArray &asModifiers, PlayerNumber pn_ ):
	ScoreKeeper(pn_), apNotes(apNotes_)
{
	//
	// Fill in m_CurStageStats, calculate multiplier
	//
	int iTotalPossibleDancePoints = 0;
	for( unsigned i=0; i<apNotes.size(); i++ )
	{
		Steps* pSteps = apNotes[i];
		NoteData notedata;
		pSteps->GetNoteData( &notedata );

		/* We might have been given lots of songs; don't keep them in memory uncompressed. */
		pSteps->Compress();

		const StyleDef* pStyleDef = GAMESTATE->GetCurrentStyleDef();
		NoteData playerNoteData;
		pStyleDef->GetTransformedNoteDataForStyle( pn_, &notedata, &playerNoteData );

		/* Apply transforms to find out how the notes will really look. 
		 *
		 * XXX: This is brittle: if we end up combining mods for a song differently
		 * than ScreenGameplay, we'll end up with the wrong data.  We should probably
		 * have eg. GAMESTATE->GetOptionsForCourse(po,so,pn) to get options based on
		 * the last call to StoreSelectedOptions and the modifiers list, but that'd
		 * mean moving the queues in ScreenGameplay to GameState ... */
		PlayerOptions ModsForThisSong( GAMESTATE->m_PlayerOptions[pn_] );
		ModsForThisSong.FromString( asModifiers[i] );

		NoteData playerNoteDataPostModifiers(playerNoteData);
		NoteDataUtil::TransformNoteData( playerNoteData, ModsForThisSong, GAMESTATE->GetCurrentStyleDef()->m_StepsType );
		 
		iTotalPossibleDancePoints += this->GetPossibleDancePoints( playerNoteData, playerNoteDataPostModifiers );
	}
	GAMESTATE->m_CurStageStats.iPossibleDancePoints[pn_] = iTotalPossibleDancePoints;

	if( !GAMESTATE->IsCourseMode() )
	{
		ASSERT( !apNotes.empty() );
		GAMESTATE->m_CurStageStats.pSong = GAMESTATE->m_pCurSong;
		GAMESTATE->m_CurStageStats.iMeter[pn_] = apNotes[0]->GetMeter();
	} else {
		GAMESTATE->m_CurStageStats.pSong = NULL;
	}



	m_iScore = 0;
	m_iCurToastyCombo = 0; 
	m_iMaxScoreSoFar = 0;
	m_iPointBonus = 0;
	m_iComboBonus = 0;
	m_iNumTapsAndHolds = 0;
	m_bIsLastSongInCourse = false;
}

void ScoreKeeper5th::OnNextSong( int iSongInCourseIndex, Steps* pNotes, NoteData* pNoteData )
{
/*
  http://www.aaroninjapan.com/ddr2.html

  Note on NONSTOP Mode scoring

  Nonstop mode requires the player to play 4 songs in succession, with the total maximum possible score for the four song set being 100,000,000. This comes from the sum of the four stages' maximum possible scores, which, regardless of song or difficulty is: 

  10,000,000 for the first song
  20,000,000 for the second song
  30,000,000 for the third song
  40,000,000 for the fourth song

  We extend this to work with nonstop courses of any length.

  We also keep track of this scoring type in endless, with 100mil per iteration
  of all songs, though this score isn't actually seen anywhere right now.
*/
	//
	// Calculate the score multiplier
	//
	m_iMaxPossiblePoints = 0;
	if( GAMESTATE->IsCourseMode() )
	{
		const int numSongsInCourse = apNotes.size();
		ASSERT( numSongsInCourse != 0 );

		const int iIndex = iSongInCourseIndex % numSongsInCourse;
		m_bIsLastSongInCourse = (iIndex+1 == numSongsInCourse);

		if( numSongsInCourse < 10 )
		{
			const int courseMult = (numSongsInCourse * (numSongsInCourse + 1)) / 2;
			ASSERT(courseMult >= 0);

			m_iMaxPossiblePoints = (100000000 * (iIndex+1)) / courseMult;
		}
		else
		{
			/* When we have lots of songs, the scale above biases too much: in a
			 * course with 50 songs, the first song is worth 80k, the last 4mil, which
			 * is too much of a difference.
			 *
			 * With this, each song in a 50-song course will be worth 2mil. */
			m_iMaxPossiblePoints = 100000000 / numSongsInCourse;
		}
	}
	else
	{
		const int iMeter = clamp( pNotes->GetMeter(), 1, 10 );

		// long ver and marathon ver songs have higher max possible scores
		int iLengthMultiplier = SongManager::GetNumStagesForSong( GAMESTATE->m_pCurSong );
		m_iMaxPossiblePoints = (1 + iMeter * iLengthMultiplier) * 5000000;
	}
	ASSERT( m_iMaxPossiblePoints >= 0 );
	m_iMaxScoreSoFar += m_iMaxPossiblePoints;

	m_iNumTapsAndHolds = pNoteData->GetNumRowsWithTaps() + pNoteData->GetNumHoldNotes();

	m_iPointBonus = m_iMaxPossiblePoints;

	ASSERT( m_iPointBonus >= 0 );

	m_iTapNotesHit = 0;
}

static int GetScore(int p, int B, int S, int n)
{
	/* There's a problem with the scoring system described below.  B/S is truncated
	 * to an int.  However, in some cases we can end up with very small base scores.
	 * Each song in a 50-song nonstop course will be worth 2mil, which is a base of
	 * 200k; B/S will end up being zero.
	 *
	 * If we rearrange the equation to (p*B*n) / S, this problem goes away.
	 * (To do that, we need to either use 64-bit ints or rearrange it a little
	 * more and use floats, since p*B*n won't fit a 32-bit int.)  However, this
	 * changes the scoring rules slightly.
	 */

#if 0
	/* This is the actual method described below. */
	return p * (B / S) * n;
#elif 1
	/* This doesn't round down B/S. */
	return int(Uint64(p) * n * B / S);
#else
	/* This also doesn't round down B/S. Use this if you don't have 64-bit ints. */
	return int(p * n * (float(B) / S));
#endif

}

void ScoreKeeper5th::AddScore( TapNoteScore score )
{
/*
  http://www.aaroninjapan.com/ddr2.html

  Regular scoring:

  Let p = score multiplier (Perfect = 10, Great = 5, other = 0)
  
  Note on NONSTOP Mode scoring

  Let p = score multiplier (Marvelous = 10, Perfect = 9, Great = 5, other = 0)

*/
	int p = 0;	// score multiplier 
	const bool MarvelousEnabled = GAMESTATE->IsCourseMode() && (PREFSMAN->m_iMarvelousTiming > 0);

	switch( score )
	{
	case TNS_MARVELOUS:	p = 10;		break;
	case TNS_PERFECT:	p = MarvelousEnabled? 9:10; break;
	case TNS_GREAT:		p = 5;		break;
	default:			p = 0;		break;
	}

	m_iTapNotesHit++;

	const int N = m_iNumTapsAndHolds;
	const int sum = (N * (N + 1)) / 2;
	const int B = m_iMaxPossiblePoints/10;

	int iScoreMultiplier = (m_iMaxPossiblePoints / (10*sum));
	ASSERT( iScoreMultiplier >= 0 );

	// What does this do?  "Don't use a multiplier if 
	// the player has failed"?
	// Also, why does this switch on score again instead
	// of just adding p?  -Chris
	if( GAMESTATE->m_CurStageStats.bFailed[m_PlayerNumber] )
		switch( score )
		{
			case TNS_MARVELOUS:	m_iScore += 10;		break;
			case TNS_PERFECT:	m_iScore += MarvelousEnabled? 9:10; break;
			case TNS_GREAT:		m_iScore += 5;		break;
		}
	else
	{
		m_iScore += GetScore(p, B, sum, m_iTapNotesHit);

		if (!GAMESTATE->IsCourseMode())
		{
			// handle combo bonus

			// combo hasn't been updated yet, its one bigger than it really is
			int m_iCurrentCombo = GAMESTATE->m_CurStageStats.iCurCombo[m_PlayerNumber] + 1; 

			switch( score )
			{
			case TNS_MARVELOUS:
			case TNS_PERFECT:
				m_iComboBonus += 55 * m_iCurrentCombo;
				break;
			case TNS_GREAT:
				m_iComboBonus += 33 * m_iCurrentCombo;
			}
			GAMESTATE->m_CurStageStats.iBonus[m_PlayerNumber] = m_iComboBonus;
		}
	}

	/* Subtract the maximum this step could have been worth from the bonus. */
	m_iPointBonus -= GetScore(10, B, sum, m_iTapNotesHit);

	if ( m_iTapNotesHit == m_iNumTapsAndHolds && score >= TNS_PERFECT )
	{
		if (!GAMESTATE->m_CurStageStats.bFailed[m_PlayerNumber])
			m_iScore += m_iPointBonus;
		if ( m_bIsLastSongInCourse )
		{
			m_iScore += 100000000 - m_iMaxScoreSoFar;

			/* If we're in Endless mode, we'll come around here again, so reset
			 * the bonus counter. */
			m_iMaxScoreSoFar = 0;
		}
	}

	ASSERT(m_iScore >= 0);

	printf( "score: %i\n", m_iScore );

	GAMESTATE->m_CurStageStats.iScore[m_PlayerNumber] = m_iScore;
}

void ScoreKeeper5th::HandleTapRowScore( TapNoteScore scoreOfLastTap, int iNumTapsInRow, int iNumAdditions )
{
	ASSERT( iNumTapsInRow >= 1 );
	int iNumTapsToScore = iNumTapsInRow-iNumAdditions;

	// Update dance points.  Additions don't count.
	if( iNumTapsToScore > 0 )
	{
		GAMESTATE->m_CurStageStats.iActualDancePoints[m_PlayerNumber] += TapNoteScoreToDancePoints( scoreOfLastTap );
	}

	// Do count additions in judge totals.
	GAMESTATE->m_CurStageStats.iTapNoteScores[m_PlayerNumber][scoreOfLastTap] += 1;

/*
  http://www.aaroninjapan.com/ddr2.html

  A single step's points are calculated as follows: 
  
  p = score multiplier (Perfect = 10, Great = 5, other = 0)
  N = total number of steps and freeze steps
  S = The sum of all integers from 1 to N (the total number of steps/freeze steps) 
  n = number of the current step or freeze step (varies from 1 to N)
  B = Base value of the song (1,000,000 X the number of feet difficulty) - All edit data is rated as 5 feet
  So, the score for one step is: 
  one_step_score = p * (B/S) * n 
  
  *IMPORTANT* : Double steps (U+L, D+R, etc.) count as two steps instead of one *for your combo count only*, 
  so if you get a double L+R on the 112th step of a song, you score is calculated for only one step, not two, 
  as the combo counter might otherwise imply.  
	
  Now, through simple algebraic manipulation:
  S = 1+...+N = (1+N)*N/2 (1 through N added together) 

  Okay, time for an example.  Suppose we wanted to calculate the step score of a "Great" on the 57th step of 
  a 441 step, 8-foot difficulty song (I'm just making this one up): 
  
  S = (1 + 441)*441 / 2
  = 194,222 / 2
  = 97,461
  StepScore = p * (B/S) * n
  = 5 * (8,000,000 / 97,461) * 57
  = 5 * (82) * 57 (The 82 is rounded down from 82.08411...)
  = 23,370
  
  Remember this is just the score for the step, not the cumulative score up to the 57th step. Also, please note that 
  I am currently checking into rounding errors with the system and if there are any, how they are resolved in the system. 
  
  Note: if you got all Perfect on this song, you would get (p=10)*B, which is 80,000,000. In fact, the maximum possible 
  score for any song is the number of feet difficulty X 10,000,000. 
*/
	AddScore( scoreOfLastTap );		// only score once per row


	//
	// handle combo logic
	//
#ifndef DEBUG
	if( PREFSMAN->m_bAutoPlay && !GAMESTATE->m_bDemonstrationOrJukebox )	// cheaters never prosper
	{
		m_iCurToastyCombo = 0;
		return;
	}
#endif //DEBUG

	//
	// Toasty combo
	//
	switch( scoreOfLastTap )
	{
	case TNS_MARVELOUS:
	case TNS_PERFECT:
		m_iCurToastyCombo += iNumTapsInRow;

		if( m_iCurToastyCombo==250 && !GAMESTATE->m_bDemonstrationOrJukebox )
		{
			SCREENMAN->PostMessageToTopScreen( SM_PlayToasty, 0 );
			GAMESTATE->m_pUnlockingSys->UnlockToasty();
		}
		break;
	default:
		m_iCurToastyCombo = 0;
		break;
	}

	//
	// Regular combo
	//
	int &iCurCombo = GAMESTATE->m_CurStageStats.iCurCombo[m_PlayerNumber];
	int iOldCombo = iCurCombo;
	
/*
  http://www.aaroninjapan.com/ddr2.html

  Note on ONI Mode scoring
  
  Your combo counter only increased with a "Marvelous/Perfect", and double Marvelous/Perfect steps (left and right, etc.)
  only add 1 to your combo instead of 2. The combo counter thus becomes a "Marvelous/Perfect" counter. 
*/
	switch( GAMESTATE->m_PlayMode )
	{
	case PLAY_MODE_ARCADE:
	case PLAY_MODE_BATTLE:
	case PLAY_MODE_RAVE:
	case PLAY_MODE_NONSTOP:
	case PLAY_MODE_ENDLESS:
		switch( scoreOfLastTap )
		{
		case TNS_MARVELOUS:
		case TNS_PERFECT:
		case TNS_GREAT:
			iCurCombo += iNumTapsInRow;
			break;
		}
		break;
	case PLAY_MODE_ONI:
		switch( scoreOfLastTap )
		{
		case TNS_MARVELOUS:
		case TNS_PERFECT:
			iCurCombo++;
			break;
		}
		break;
	default:
		ASSERT(0);
	}


#define CROSSED( x ) (iOldCombo<x && iCurCombo>=x)

	if ( CROSSED(100) )	
		SCREENMAN->PostMessageToTopScreen( SM_100Combo, 0 );
	else if( CROSSED(200) )	
		SCREENMAN->PostMessageToTopScreen( SM_200Combo, 0 );
	else if( CROSSED(300) )	
		SCREENMAN->PostMessageToTopScreen( SM_300Combo, 0 );
	else if( CROSSED(400) )	
		SCREENMAN->PostMessageToTopScreen( SM_400Combo, 0 );
	else if( CROSSED(500) )	
		SCREENMAN->PostMessageToTopScreen( SM_500Combo, 0 );
	else if( CROSSED(600) )	
		SCREENMAN->PostMessageToTopScreen( SM_600Combo, 0 );
	else if( CROSSED(700) )	
		SCREENMAN->PostMessageToTopScreen( SM_700Combo, 0 );
	else if( CROSSED(800) )	
		SCREENMAN->PostMessageToTopScreen( SM_800Combo, 0 );
	else if( CROSSED(900) )	
		SCREENMAN->PostMessageToTopScreen( SM_900Combo, 0 );
	else if( CROSSED(1000))	
		SCREENMAN->PostMessageToTopScreen( SM_1000Combo, 0 );

	// new max combo
	GAMESTATE->m_CurStageStats.iMaxCombo[m_PlayerNumber] = max(GAMESTATE->m_CurStageStats.iMaxCombo[m_PlayerNumber], iCurCombo);


	switch( scoreOfLastTap )
	{
	case TNS_GOOD:
	case TNS_BOO:
	case TNS_MISS:
		if( iCurCombo>50 )
			SCREENMAN->PostMessageToTopScreen( SM_ComboStopped, 0 );

		iCurCombo = 0;
		break;
	}
}


void ScoreKeeper5th::HandleHoldScore( HoldNoteScore holdScore, TapNoteScore tapScore )
{
	// update dance points totals
	GAMESTATE->m_CurStageStats.iHoldNoteScores[m_PlayerNumber][holdScore] ++;
	GAMESTATE->m_CurStageStats.iActualDancePoints[m_PlayerNumber] += HoldNoteScoreToDancePoints( holdScore );

	if( holdScore == HNS_OK )
		AddScore( TNS_MARVELOUS );
}


int ScoreKeeper5th::GetPossibleDancePoints( const NoteData &preNoteData, const NoteData &postNoteData )
{
	/* Note that, if Marvelous timing is disabled or not active (not course mode),
	 * PERFECT will be used instead. */

	/*
	 * The logic here is that if you use a modifier that adds notes, you should have to
	 * hit the new notes to get a high grade.  However, if you use one that removes notes,
	 * they should simply be counted as misses. */
	int NumTaps = max( preNoteData.GetNumRowsWithTaps(), postNoteData.GetNumRowsWithTaps() );
	int NumHolds = max( preNoteData.GetNumHoldNotes(), postNoteData.GetNumHoldNotes() );
	return NumTaps*TapNoteScoreToDancePoints(TNS_MARVELOUS)+
	   NumHolds*HoldNoteScoreToDancePoints(HNS_OK);
}



int ScoreKeeper5th::TapNoteScoreToDancePoints( TapNoteScore tns )
{
	if(!GAMESTATE->ShowMarvelous() && tns == TNS_MARVELOUS)
		tns = TNS_PERFECT;

/*
  http://www.aaroninjapan.com/ddr2.html

  Regular play scoring

  A "Perfect" is worth 2 points
  A "Great" is worth 1 points
  A "Good" is worth 0 points
  A "Boo" will subtract 4 points
  A "Miss" will subtract 8 points
  An "OK" (Successful Freeze step) will add 6 points
  A "NG" (Unsuccessful Freeze step) is worth 0 points

  Note on ONI Mode scoring

  The total number of Dance Points is calculated with Marvelous steps being worth 3 points, Perfects getting 
  2 points, OKs getting 3 points, Greats getting 1 point, and everything else is worth 0 points. (Note: The 
  "Marvelous" step rating is a new rating to DDR Extreme only used in Oni and Nonstop modes. They are rated 
  higher than "Perfect" steps). 

  Note on NONSTOP Mode scoring

  A "Marvelous" is worth 2 points
  A "Perfect" is also worth 2 points
  A "Great" is worth 1 points
  A "Good" is worth 0 points
  A "Boo" will subtract 4 points
  A "Miss" will subtract 8 points
  An "OK" (Successful Freeze step) will add 6 points
  A "NG" (Unsuccessful Freeze step) is worth 0 points	
*/

	switch( GAMESTATE->m_PlayMode )
	{
	case PLAY_MODE_ARCADE:
	case PLAY_MODE_BATTLE:
	case PLAY_MODE_RAVE:
	case PLAY_MODE_ENDLESS:
	case PLAY_MODE_NONSTOP:
		switch( tns )
		{
		case TNS_MARVELOUS:	return +2;
		case TNS_PERFECT:	return +2;
		case TNS_GREAT:		return +1;
		case TNS_GOOD:		return +0;
		case TNS_BOO:		return -4;
		case TNS_MISS:		return -8;
		}
		break;
	case PLAY_MODE_ONI:
		switch( tns )
		{
		case TNS_MARVELOUS:	return +3;
		case TNS_PERFECT:	return +2;
		case TNS_GREAT:		return +1;
		case TNS_GOOD:		return +0;
		case TNS_BOO:		return +0;
		case TNS_MISS:		return +0;
		}
		break;
	default:
		ASSERT(0);
	}	
	return +0;
}

int ScoreKeeper5th::HoldNoteScoreToDancePoints( HoldNoteScore hns )
{
	switch( GAMESTATE->m_PlayMode )
	{
	case PLAY_MODE_ARCADE:
	case PLAY_MODE_BATTLE:
	case PLAY_MODE_RAVE:
	case PLAY_MODE_ENDLESS:
	case PLAY_MODE_NONSTOP:
		switch( hns )
		{
		case HNS_OK:	return +6;
		case HNS_NG:	return +0;
		}
		break;
	case PLAY_MODE_ONI:
		switch( hns )
		{
		case HNS_OK:	return (PREFSMAN->m_iMarvelousTiming != 0)? +3:+2;
		case HNS_NG:	return +0;
		}
		break;
	default:
		ASSERT(0);
	}
	return +0;
}


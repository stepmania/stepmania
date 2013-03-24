#include "global.h"
#include "ScoreKeeperNormal.h"
#include "GameState.h"
#include "PrefsManager.h"
#include "GamePreferences.h"
#include "Steps.h"
#include "ScreenManager.h"
#include "GameState.h"
#include "Course.h"
#include "SongManager.h"
#include "NoteDataUtil.h"
#include "NoteData.h"
#include "RageLog.h"
#include "StageStats.h"
#include "ProfileManager.h"
#include "NetworkSyncManager.h"
#include "PlayerState.h"
#include "Game.h"
#include "Style.h"
#include "Song.h"
#include "TimingData.h"
#include "NoteDataWithScoring.h"


void PercentScoreWeightInit( size_t /*ScoreEvent*/ i, RString &sNameOut, int &defaultValueOut )
{
	sNameOut = "PercentScoreWeight" + ScoreEventToString( (ScoreEvent)i );
	switch( i )
	{
	default:
		FAIL_M(ssprintf("Invalid ScoreEvent: %i", i));
	case SE_W1:		defaultValueOut = 3;	break;
	case SE_W2:		defaultValueOut = 2;	break;
	case SE_W3:		defaultValueOut = 1;	break;
	case SE_W4:		defaultValueOut = 0;	break;
	case SE_W5:		defaultValueOut = 0;	break;
	case SE_Miss:		defaultValueOut = 0;	break;
	case SE_HitMine:	defaultValueOut = -2;	break;
	case SE_CheckpointHit:	defaultValueOut = 0;	break;
	case SE_CheckpointMiss:	defaultValueOut = 0;	break;
	case SE_Held:		defaultValueOut = 3;	break;
	case SE_LetGo:		defaultValueOut = 0;	break;
	}
}

void GradeWeightInit( size_t /*ScoreEvent*/ i, RString &sNameOut, int &defaultValueOut )
{
	sNameOut = "GradeWeight" + ScoreEventToString( (ScoreEvent)i );
	switch( i )
	{
	default:
		FAIL_M(ssprintf("Invalid ScoreEvent: %i", i));
	case SE_W1:		defaultValueOut = 2;	break;
	case SE_W2:		defaultValueOut = 2;	break;
	case SE_W3:		defaultValueOut = 1;	break;
	case SE_W4:		defaultValueOut = 0;	break;
	case SE_W5:		defaultValueOut = -4;	break;
	case SE_Miss:		defaultValueOut = -8;	break;
	case SE_HitMine:	defaultValueOut = -8;	break;
	case SE_CheckpointHit:	defaultValueOut = 0;	break;
	case SE_CheckpointMiss:	defaultValueOut = 0;	break;
	case SE_Held:		defaultValueOut = 6;	break;
	case SE_LetGo:		defaultValueOut = 0;	break;
	}
}

static Preference1D<int> g_iPercentScoreWeight( PercentScoreWeightInit, NUM_ScoreEvent );
static Preference1D<int> g_iGradeWeight( GradeWeightInit, NUM_ScoreEvent );

ScoreKeeperNormal::ScoreKeeperNormal( PlayerState *pPlayerState, PlayerStageStats *pPlayerStageStats ):
	ScoreKeeper(pPlayerState, pPlayerStageStats)
{
}

void ScoreKeeperNormal::Load(
		const vector<Song*>& apSongs,
		const vector<Steps*>& apSteps,
		const vector<AttackArray> &asModifiers )
{
	m_apSteps = apSteps;
	ASSERT( apSongs.size() == apSteps.size() );
	ASSERT( apSongs.size() == asModifiers.size() );

	// True if a jump is one to combo, false if combo is purely based on tap count.
	m_ComboIsPerRow.Load( "Gameplay", "ComboIsPerRow" );
	m_MissComboIsPerRow.Load( "Gameplay", "MissComboIsPerRow" );
	m_MinScoreToContinueCombo.Load( "Gameplay", "MinScoreToContinueCombo" );
	m_MinScoreToMaintainCombo.Load( "Gameplay", "MinScoreToMaintainCombo" );
	m_MaxScoreToIncrementMissCombo.Load( "Gameplay", "MaxScoreToIncrementMissCombo" );
	m_MineHitIncrementsMissCombo.Load( "Gameplay", "MineHitIncrementsMissCombo" );
	m_AvoidMineIncrementsCombo.Load( "Gameplay", "AvoidMineIncrementsCombo" );

	// Toasty triggers (idea from 3.9+)
	// Multiple toasty support doesn't seem to be working right now.
	// Since it's causing more problems than solutions, I'm going back to
	// the old way of a single toasty trigger for now.
	//m_vToastyTriggers.Load( "Gameplay", "ToastyTriggersAt" );
	m_ToastyTrigger.Load( "Gameplay", "ToastyTriggersAt" );

	// Fill in STATSMAN->m_CurStageStats, calculate multiplier
	int iTotalPossibleDancePoints = 0;
	int iTotalPossibleGradePoints = 0;
	for( unsigned i=0; i<apSteps.size(); i++ )
	{
		Song* pSong = apSongs[i];
		ASSERT( pSong != NULL );
		Steps* pSteps = apSteps[i];
		ASSERT( pSteps != NULL );
		const AttackArray &aa = asModifiers[i];
		NoteData ndTemp;
		pSteps->GetNoteData( ndTemp );

		// We might have been given lots of songs; don't keep them in memory uncompressed.
		pSteps->Compress();

		const Style* pStyle = GAMESTATE->GetCurrentStyle();
		NoteData nd;
		pStyle->GetTransformedNoteDataForStyle( m_pPlayerState->m_PlayerNumber, ndTemp, nd );

		/* Compute RadarValues before applying any user-selected mods. Apply
		 * Course mods and count them in the "pre" RadarValues because they're
		 * forced and not chosen by the user. */
		NoteDataUtil::TransformNoteData( nd, aa, pSteps->m_StepsType, pSong );
		RadarValues rvPre;
		GAMESTATE->SetProcessedTimingData(pSteps->GetTimingData());
		NoteDataUtil::CalculateRadarValues( nd, pSong->m_fMusicLengthSeconds, rvPre );

		/* Apply user transforms to find out how the notes will really look.
		 *
		 * XXX: This is brittle: if we end up combining mods for a song differently
		 * than ScreenGameplay, we'll end up with the wrong data.  We should probably
		 * have eg. GAMESTATE->GetOptionsForCourse(po,so,pn) to get options based on
		 * the last call to StoreSelectedOptions and the modifiers list, but that'd
		 * mean moving the queues in ScreenGameplay to GameState ... */
		NoteDataUtil::TransformNoteData( nd, m_pPlayerState->m_PlayerOptions.GetStage(), pSteps->m_StepsType );
		RadarValues rvPost;
		NoteDataUtil::CalculateRadarValues( nd, pSong->m_fMusicLengthSeconds, rvPost );
		GAMESTATE->SetProcessedTimingData(NULL);

		iTotalPossibleDancePoints += this->GetPossibleDancePoints( rvPre, rvPost );
		iTotalPossibleGradePoints += this->GetPossibleGradePoints( rvPre, rvPost );
	}

	m_pPlayerStageStats->m_iPossibleDancePoints = iTotalPossibleDancePoints;
	m_pPlayerStageStats->m_iPossibleGradePoints = iTotalPossibleGradePoints;

	m_iScoreRemainder = 0;
	m_iCurToastyCombo = 0;
	//m_iCurToastyTrigger = 0;
	//m_iNextToastyAt = 0;
	m_iMaxScoreSoFar = 0;
	m_iPointBonus = 0;
	m_iNumTapsAndHolds = 0;
	m_iNumNotesHitThisRow = 0;
	m_bIsLastSongInCourse = false;

	Message msg( "ScoreChanged" );
	msg.SetParam( "PlayerNumber", m_pPlayerState->m_PlayerNumber );
	msg.SetParam( "MultiPlayer", m_pPlayerState->m_mp );
	MESSAGEMAN->Broadcast( msg );

	memset( m_ComboBonusFactor, 0, sizeof(m_ComboBonusFactor) );
	m_iRoundTo = 1;
}

void ScoreKeeperNormal::OnNextSong( int iSongInCourseIndex, const Steps* pSteps, const NoteData* pNoteData )
{
/*
  Note on NONSTOP Mode scoring

  Nonstop mode requires the player to play 4 songs in succession, with the total maximum possible score for
  the four song set being 100,000,000. This comes from the sum of the four stages' maximum possible scores,
  which, regardless of song or difficulty is:

  10,000,000 for the first song
  20,000,000 for the second song
  30,000,000 for the third song
  40,000,000 for the fourth song

  We extend this to work with nonstop courses of any length.

  We also keep track of this scoring type in endless, with 100mil per iteration
  of all songs, though this score isn't actually seen anywhere right now.
*/
	// Calculate the score multiplier
	m_iMaxPossiblePoints = 0;
	if( GAMESTATE->IsCourseMode() )
	{
		const int numSongsInCourse = m_apSteps.size();
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
		// long ver and marathon ver songs have higher max possible scores
		int iLengthMultiplier = GameState::GetNumStagesMultiplierForSong( GAMESTATE->m_pCurSong );
		
		/* This is no longer just simple additive/subtractive scoring,
		 * but start with capping the score at the size of the score counter. */
		m_iMaxPossiblePoints = 10 * 10000000 * iLengthMultiplier;
	}
	ASSERT( m_iMaxPossiblePoints >= 0 );
	m_iMaxScoreSoFar += m_iMaxPossiblePoints;

	GAMESTATE->SetProcessedTimingData(const_cast<TimingData *>(pSteps->GetTimingData()));
	
	m_iNumTapsAndHolds = pNoteData->GetNumRowsWithTapOrHoldHead() + pNoteData->GetNumHoldNotes()
		+ pNoteData->GetNumRolls();

	m_iPointBonus = m_iMaxPossiblePoints;
	m_pPlayerStageStats->m_iMaxScore = m_iMaxScoreSoFar;

	/* MercifulBeginner shouldn't clamp weights in course mode, even if a beginner
	 * song is in a course, since that makes PlayerStageStats::GetGrade hard. */
	m_bIsBeginner = pSteps->GetDifficulty() == Difficulty_Beginner && !GAMESTATE->IsCourseMode();

	ASSERT( m_iPointBonus >= 0 );

	m_iTapNotesHit = 0;
	
	GAMESTATE->SetProcessedTimingData(NULL);
}

static int GetScore(int p, int Z, int S, int n)
{
	/* There's a problem with the scoring system described below. Z/S is truncated
	 * to an int. However, in some cases we can end up with very small base scores.
	 * Each song in a 50-song nonstop course will be worth 2mil, which is a base of
	 * 200k; Z/S will end up being zero.
	 *
	 * If we rearrange the equation to (p*Z*n) / S, this problem goes away.
	 * (To do that, we need to either use 64-bit ints or rearrange it a little
	 * more and use floats, since p*Z*n won't fit a 32-bit int.)  However, this
	 * changes the scoring rules slightly.
	 */

#if 0
	// This is the actual method described below.
	return p * (Z / S) * n;
#elif 1
	// This doesn't round down Z/S.
	return int(int64_t(p) * n * Z / S);
#else
	// This also doesn't round down Z/S. Use this if you don't have 64-bit ints.
	return int(p * n * (float(Z) / S));
#endif

}

void ScoreKeeperNormal::AddTapScore( TapNoteScore tns )
{
}

void ScoreKeeperNormal::AddHoldScore( HoldNoteScore hns )
{
	if( hns == HNS_Held )
		AddScoreInternal( TNS_W1 );
	else if ( hns == HNS_LetGo )
		AddScoreInternal( TNS_W4 ); // required for subtractive score display to work properly.
}

void ScoreKeeperNormal::AddTapRowScore( TapNoteScore score, const NoteData &nd, int iRow )
{
	AddScoreInternal( score );
}

extern ThemeMetric<bool> PENALIZE_TAP_SCORE_NONE;
void ScoreKeeperNormal::HandleTapScoreNone()
{
	if( PENALIZE_TAP_SCORE_NONE )
	{
		m_pPlayerStageStats->m_iCurCombo = 0;

		if( m_pPlayerState->m_PlayerNumber != PLAYER_INVALID )
			MESSAGEMAN->Broadcast( enum_add2(Message_CurrentComboChangedP1,m_pPlayerState->m_PlayerNumber) );
	}

	// TODO: networking code
}

void ScoreKeeperNormal::AddScoreInternal( TapNoteScore score )
{
	int &iScore = m_pPlayerStageStats->m_iScore;
	int &iCurMaxScore = m_pPlayerStageStats->m_iCurMaxScore;

	// See Aaron In Japan for more details about the scoring formulas.
	// Note: this assumes no custom scoring systems are in use.
	int p = 0;	// score multiplier

	switch( score )
	{
	case TNS_W1:	p = 10;	break;
	case TNS_W2:	p = GAMESTATE->ShowW1()? 9:10; break;
	case TNS_W3:	p = 5;	break;
	default:		p = 0;	break;
	}

	m_iTapNotesHit++;

	const int N = m_iNumTapsAndHolds;
	const int sum = (N * (N + 1)) / 2;
	const int Z = m_iMaxPossiblePoints/10;

	// Don't use a multiplier if the player has failed
	if( m_pPlayerStageStats->m_bFailed )
	{
		iScore += p;
		// make score evenly divisible by 5
		// only update this on the next step, to make it less *obvious*
		/* Round to the nearest 5, instead of always rounding down, so a base score
		* of 9 will round to 10, not 5. */
		if (p > 0)
			iScore = ((iScore+2) / 5) * 5;
	}
	else
	{
		iScore += GetScore(p, Z, sum, m_iTapNotesHit);
		const int &iCurrentCombo = m_pPlayerStageStats->m_iCurCombo;
		iScore += m_ComboBonusFactor[score] * iCurrentCombo;
	}

	// Subtract the maximum this step could have been worth from the bonus.
	m_iPointBonus -= GetScore(10, Z, sum, m_iTapNotesHit);
	// And add the maximum this step could have been worth to the max score up to now.
	iCurMaxScore += GetScore(10, Z, sum, m_iTapNotesHit);

	if ( m_iTapNotesHit == m_iNumTapsAndHolds && score >= TNS_W2 )
	{
		if( !m_pPlayerStageStats->m_bFailed )
			iScore += m_iPointBonus;
		if ( m_bIsLastSongInCourse )
		{
			iScore += 100000000 - m_iMaxScoreSoFar;
			iCurMaxScore += 100000000 - m_iMaxScoreSoFar;

			/* If we're in Endless mode, we'll come around here again, so reset
			* the bonus counter. */
			m_iMaxScoreSoFar = 0;
		}
		iCurMaxScore += m_iPointBonus;
	}

	ASSERT_M( iScore >= 0, "iScore < 0 before re-rounding" );

	// Undo rounding from the last tap, and re-round.
	iScore += m_iScoreRemainder;
	m_iScoreRemainder = (iScore % m_iRoundTo);
	iScore = iScore - m_iScoreRemainder;

	ASSERT_M( iScore >= 0, "iScore < 0 after re-rounding" );

	// LOG->Trace( "score: %i", iScore );
}

void ScoreKeeperNormal::HandleTapScore( const TapNote &tn )
{
	TapNoteScore tns = tn.result.tns;

	if( tn.type == TapNote::mine )
	{
		if( tns == TNS_HitMine )
		{
			if( !m_pPlayerStageStats->m_bFailed )
				m_pPlayerStageStats->m_iActualDancePoints += TapNoteScoreToDancePoints( TNS_HitMine );
			m_pPlayerStageStats->m_iTapNoteScores[TNS_HitMine] += 1;
			if( m_MineHitIncrementsMissCombo )
				HandleComboInternal( 0, 0, 1 );
			
		}
		
		if( tns == TNS_AvoidMine && m_AvoidMineIncrementsCombo )
			HandleComboInternal( 1, 0, 0 );

		NSMAN->ReportScore(
			m_pPlayerState->m_PlayerNumber,
			tns,
			m_pPlayerStageStats->m_iScore,
			m_pPlayerStageStats->m_iCurCombo,
			tn.result.fTapNoteOffset
		);
		Message msg( "ScoreChanged" );
		msg.SetParam( "PlayerNumber", m_pPlayerState->m_PlayerNumber );
		msg.SetParam( "MultiPlayer", m_pPlayerState->m_mp );
		MESSAGEMAN->Broadcast( msg );
	}

	AddTapScore( tns );
}

void ScoreKeeperNormal::HandleHoldCheckpointScore( const NoteData &nd, int iRow, int iNumHoldsHeldThisRow, int iNumHoldsMissedThisRow )
{
	HandleTapNoteScoreInternal(iNumHoldsMissedThisRow == 0 ? TNS_CheckpointHit:TNS_CheckpointMiss,
							   TNS_CheckpointHit, iRow);
	HandleComboInternal( iNumHoldsHeldThisRow, 0, iNumHoldsMissedThisRow, iRow );
}

void ScoreKeeperNormal::HandleTapNoteScoreInternal( TapNoteScore tns, TapNoteScore maximum, int row )
{
	// Update dance points.
	if( !m_pPlayerStageStats->m_bFailed )
		m_pPlayerStageStats->m_iActualDancePoints += TapNoteScoreToDancePoints( tns );

	// update judged row totals. Respect Combo segments here.
	TimingData &td = *GAMESTATE->m_pCurSteps[m_pPlayerState->m_PlayerNumber]->GetTimingData();
	ComboSegment *cs = td.GetComboSegmentAtRow(row);
	if (tns == TNS_CheckpointHit || tns >= m_MinScoreToContinueCombo)
	{
		m_pPlayerStageStats->m_iTapNoteScores[tns] += cs->GetCombo();
	}
	else if (tns == TNS_CheckpointMiss || tns < m_MinScoreToMaintainCombo)
	{
		m_pPlayerStageStats->m_iTapNoteScores[tns] += cs->GetMissCombo();
	}
	else
	{	
		m_pPlayerStageStats->m_iTapNoteScores[tns] += 1;
	}

	// increment the current total possible dance score
	m_pPlayerStageStats->m_iCurPossibleDancePoints += TapNoteScoreToDancePoints( maximum );
}

void ScoreKeeperNormal::HandleComboInternal( int iNumHitContinueCombo, int iNumHitMaintainCombo, int iNumBreakCombo, int iRow )
{
	// Regular combo
	if( m_ComboIsPerRow )
	{
		iNumHitContinueCombo = min( iNumHitContinueCombo, 1 );
		iNumHitMaintainCombo = min( iNumHitMaintainCombo, 1 );
		iNumBreakCombo = min( iNumBreakCombo, 1 );
	}

	if( iNumHitContinueCombo > 0 || iNumHitMaintainCombo > 0 )
	{
		m_pPlayerStageStats->m_iCurMissCombo = 0;
	}
	TimingData &td = *GAMESTATE->m_pCurSteps[m_pPlayerState->m_PlayerNumber]->GetTimingData();
	if( iNumBreakCombo == 0 )
	{
		int multiplier = ( iRow == -1 ? 1 : td.GetComboSegmentAtRow( iRow )->GetCombo() );
		m_pPlayerStageStats->m_iCurCombo += iNumHitContinueCombo * multiplier;
	}
	else
	{
		m_pPlayerStageStats->m_iCurCombo = 0;
		int multiplier = ( iRow == -1 ? 1 : td.GetComboSegmentAtRow(iRow)->GetMissCombo());
		m_pPlayerStageStats->m_iCurMissCombo += ( m_MissComboIsPerRow ? 1 : iNumBreakCombo ) * multiplier;
	}
}

void ScoreKeeperNormal::HandleRowComboInternal( TapNoteScore tns, int iNumTapsInRow, int iRow )
{
	if( m_ComboIsPerRow )
	{
		iNumTapsInRow = min( iNumTapsInRow, 1);
	}
	TimingData &td = *GAMESTATE->m_pCurSteps[m_pPlayerState->m_PlayerNumber]->GetTimingData();
	if ( tns >= m_MinScoreToContinueCombo )
	{
		m_pPlayerStageStats->m_iCurMissCombo = 0;
		int multiplier = ( iRow == -1 ? 1 : td.GetComboSegmentAtRow( iRow )->GetCombo() );
		m_pPlayerStageStats->m_iCurCombo += iNumTapsInRow * multiplier;
	}
	else if ( tns < m_MinScoreToMaintainCombo )
	{
		m_pPlayerStageStats->m_iCurCombo = 0;

		if( tns <= m_MaxScoreToIncrementMissCombo )
		{
			int multiplier = ( iRow == -1 ? 1 : td.GetComboSegmentAtRow(iRow)->GetMissCombo());
			m_pPlayerStageStats->m_iCurMissCombo += ( m_MissComboIsPerRow ? 1 : iNumTapsInRow ) * multiplier;
		}
	}
}

void ScoreKeeperNormal::GetRowCounts( const NoteData &nd, int iRow,
					  int &iNumHitContinueCombo, int &iNumHitMaintainCombo,
					  int &iNumBreakCombo )
{
	iNumHitContinueCombo = iNumHitMaintainCombo = iNumBreakCombo = 0;
	for( int track = 0; track < nd.GetNumTracks(); ++track )
	{
		const TapNote &tn = nd.GetTapNote( track, iRow );

		if( tn.type != TapNote::tap && tn.type != TapNote::hold_head && tn.type != TapNote::lift )
			continue;
		TapNoteScore tns = tn.result.tns;
		if( tns >= m_MinScoreToContinueCombo )
			++iNumHitContinueCombo;
		else if( tns >= m_MinScoreToMaintainCombo )
			++iNumHitMaintainCombo;
		else
			++iNumBreakCombo;
	}
}

void ScoreKeeperNormal::HandleTapRowScore( const NoteData &nd, int iRow )
{
	int iNumHitContinueCombo, iNumHitMaintainCombo, iNumBreakCombo;
	GetRowCounts( nd, iRow, iNumHitContinueCombo, iNumHitMaintainCombo, iNumBreakCombo );

	int iNumTapsInRow = iNumHitContinueCombo + iNumHitMaintainCombo + iNumBreakCombo;
	if( iNumTapsInRow <= 0 )
		return;

	m_iNumNotesHitThisRow = iNumTapsInRow;

	TapNoteScore scoreOfLastTap = NoteDataWithScoring::LastTapNoteWithResult( nd, iRow ).result.tns;
	HandleTapNoteScoreInternal( scoreOfLastTap, TNS_W1, iRow );
	
	if ( GAMESTATE->GetCurrentGame()->m_bCountNotesSeparately )
	{
		HandleComboInternal( iNumHitContinueCombo, iNumHitMaintainCombo, iNumBreakCombo, iRow );
	}
	else
	{
		HandleRowComboInternal( scoreOfLastTap, iNumTapsInRow, iRow ); //This should work?
	}

	if( m_pPlayerState->m_PlayerNumber != PLAYER_INVALID )
		MESSAGEMAN->Broadcast( enum_add2(Message_CurrentComboChangedP1,m_pPlayerState->m_PlayerNumber) );

	AddTapRowScore( scoreOfLastTap, nd, iRow );		// only score once per row

	// handle combo logic
#ifndef DEBUG
	if( (GamePreferences::m_AutoPlay != PC_HUMAN || m_pPlayerState->m_PlayerOptions.GetCurrent().m_fPlayerAutoPlay != 0)
		&& !GAMESTATE->m_bDemonstrationOrJukebox )	// cheaters always prosper >:D -aj comment edit
	{
		m_iCurToastyCombo = 0;
		return;
	}
#endif //DEBUG

	// Toasty combo
	//vector<int> iToastyMilestones;
	switch( scoreOfLastTap )
	{
	case TNS_W1:
	case TNS_W2:
		m_iCurToastyCombo += iNumTapsInRow;

		/*
		// compile the list of toasty triggers
		{
			Lua *L = LUA->Get();
			m_vToastyTriggers.PushSelf(L);
			LuaHelpers::ReadArrayFromTable(iToastyMilestones, L);
			lua_pop( L, 1 );
			LUA->Release(L);
		}
		// find out which one we're at.
		if(m_iCurToastyTrigger <= int(iToastyMilestones.size()))
		{
			m_iNextToastyAt = iToastyMilestones[m_iCurToastyTrigger];
		}
		else // out of index value? then don't make it toasty!
		{
			m_iNextToastyAt = -1;
		}
		*/

		if( m_iCurToastyCombo >= m_ToastyTrigger &&
			m_iCurToastyCombo - iNumTapsInRow < m_ToastyTrigger &&
			!GAMESTATE->m_bDemonstrationOrJukebox )
		{
			SCREENMAN->PostMessageToTopScreen( SM_PlayToasty, 0 );
			Message msg("ToastyAchieved");
			msg.SetParam( "PlayerNumber", m_pPlayerState->m_PlayerNumber );
			msg.SetParam( "ToastyCombo", m_iCurToastyCombo );
			MESSAGEMAN->Broadcast(msg);

			// TODO: keep a pointer to the Profile.  Don't index with m_PlayerNumber
			PROFILEMAN->IncrementToastiesCount( m_pPlayerState->m_PlayerNumber );

			//m_iCurToastyTrigger++;
		}
		break;
	default:
		m_iCurToastyCombo = 0;
		Message msg("ToastyDropped");
		msg.SetParam( "PlayerNumber", m_pPlayerState->m_PlayerNumber );
		MESSAGEMAN->Broadcast(msg);
		break;
	}

	// TODO: Remove indexing with PlayerNumber
	PlayerNumber pn = m_pPlayerState->m_PlayerNumber;
	float offset = NoteDataWithScoring::LastTapNoteWithResult( nd, iRow ).result.fTapNoteOffset;
	NSMAN->ReportScore( pn, scoreOfLastTap,
			m_pPlayerStageStats->m_iScore,
			m_pPlayerStageStats->m_iCurCombo, offset );
	Message msg( "ScoreChanged" );
	msg.SetParam( "PlayerNumber", m_pPlayerState->m_PlayerNumber );
	msg.SetParam( "MultiPlayer", m_pPlayerState->m_mp );
	msg.SetParam( "ToastyCombo", m_iCurToastyCombo );
	MESSAGEMAN->Broadcast( msg );
}


void ScoreKeeperNormal::HandleHoldScore( const TapNote &tn )
{
	HoldNoteScore holdScore = tn.HoldResult.hns;

	// update dance points totals
	if( !m_pPlayerStageStats->m_bFailed )
		m_pPlayerStageStats->m_iActualDancePoints += HoldNoteScoreToDancePoints( holdScore );
	m_pPlayerStageStats->m_iCurPossibleDancePoints += HoldNoteScoreToDancePoints( HNS_Held );
	m_pPlayerStageStats->m_iHoldNoteScores[holdScore] ++;

	// increment the current total possible dance score

	m_pPlayerStageStats->m_iCurPossibleDancePoints += HoldNoteScoreToDancePoints( HNS_Held );

	AddHoldScore( holdScore );

	// TODO: Remove indexing with PlayerNumber
	PlayerNumber pn = m_pPlayerState->m_PlayerNumber;
	NSMAN->ReportScore(
		pn,
		holdScore+TapNoteScore_Invalid,
		m_pPlayerStageStats->m_iScore,
		m_pPlayerStageStats->m_iCurCombo,
		tn.result.fTapNoteOffset );
	Message msg( "ScoreChanged" );
	msg.SetParam( "PlayerNumber", m_pPlayerState->m_PlayerNumber );
	msg.SetParam( "MultiPlayer", m_pPlayerState->m_mp );
	MESSAGEMAN->Broadcast( msg );
}


int ScoreKeeperNormal::GetPossibleDancePoints( const RadarValues& radars )
{
	/* Note: If W1 timing is disabled or not active (not course mode),
	 * W2 will be used instead. */

	int NumTaps = int(radars[RadarCategory_TapsAndHolds]);
	int NumHolds = int(radars[RadarCategory_Holds]);
	int NumRolls = int(radars[RadarCategory_Rolls]);
	return
		NumTaps*TapNoteScoreToDancePoints(TNS_W1, false) +
		NumHolds*HoldNoteScoreToDancePoints(HNS_Held, false) +
		NumRolls*HoldNoteScoreToDancePoints(HNS_Held, false);
}

int ScoreKeeperNormal::GetPossibleDancePoints( const RadarValues& fOriginalRadars, const RadarValues& fPostRadars )
{
	/* The logic here is that if you use a modifier that adds notes, you should
	 * have to hit the new notes to get a high grade. However, if you use one
	 * that removes notes, they should simply be counted as misses. */
	return max(
		GetPossibleDancePoints(fOriginalRadars),
		GetPossibleDancePoints(fPostRadars) );
}

int ScoreKeeperNormal::GetPossibleGradePoints( const RadarValues& radars )
{
	/* Note: if W1 timing is disabled or not active (not course mode),
	 * W2 will be used instead. */

	int NumTaps = int(radars[RadarCategory_TapsAndHolds]);
	int NumHolds = int(radars[RadarCategory_Holds]);
	int NumRolls = int(radars[RadarCategory_Rolls]);
	return
		NumTaps*TapNoteScoreToGradePoints(TNS_W1, false) +
		NumHolds*HoldNoteScoreToGradePoints(HNS_Held, false) +
		NumRolls*HoldNoteScoreToGradePoints(HNS_Held, false);
}

int ScoreKeeperNormal::GetPossibleGradePoints( const RadarValues& fOriginalRadars, const RadarValues& fPostRadars )
{
	/* The logic here is that if you use a modifier that adds notes, you should
	 * have to hit the new notes to get a high grade. However, if you use one
	 * that removes notes, they should simply be counted as misses. */
	return max(
		GetPossibleGradePoints(fOriginalRadars),
		GetPossibleGradePoints(fPostRadars) );
}

int ScoreKeeperNormal::TapNoteScoreToDancePoints( TapNoteScore tns ) const
{
	return TapNoteScoreToDancePoints( tns, m_bIsBeginner );
}

int ScoreKeeperNormal::HoldNoteScoreToDancePoints( HoldNoteScore hns ) const
{
	return HoldNoteScoreToDancePoints( hns, m_bIsBeginner );
}

int ScoreKeeperNormal::TapNoteScoreToGradePoints( TapNoteScore tns ) const
{
	return TapNoteScoreToGradePoints( tns, m_bIsBeginner );
}
int ScoreKeeperNormal::HoldNoteScoreToGradePoints( HoldNoteScore hns ) const
{
	return HoldNoteScoreToGradePoints( hns, m_bIsBeginner );
}

int ScoreKeeperNormal::TapNoteScoreToDancePoints( TapNoteScore tns, bool bBeginner )
{
	if( !GAMESTATE->ShowW1() && tns == TNS_W1 )
		tns = TNS_W2;

	/* This is used for Oni percentage displays. Grading values are currently in
	 * StageStats::GetGrade. */
	int iWeight = 0;
	switch( tns )
	{
	DEFAULT_FAIL( tns );
	case TNS_None:		iWeight = 0;									break;
	case TNS_HitMine:	iWeight = g_iPercentScoreWeight[SE_HitMine];	break;
	case TNS_Miss:		iWeight = g_iPercentScoreWeight[SE_Miss];		break;
	case TNS_W5:		iWeight = g_iPercentScoreWeight[SE_W5];			break;
	case TNS_W4:		iWeight = g_iPercentScoreWeight[SE_W4];			break;
	case TNS_W3:		iWeight = g_iPercentScoreWeight[SE_W3];			break;
	case TNS_W2:		iWeight = g_iPercentScoreWeight[SE_W2];			break;
	case TNS_W1:		iWeight = g_iPercentScoreWeight[SE_W1];			break;
	case TNS_CheckpointHit:	iWeight = g_iPercentScoreWeight[SE_CheckpointHit];	break;
	case TNS_CheckpointMiss:iWeight = g_iPercentScoreWeight[SE_CheckpointMiss];	break;
	}
	if( bBeginner && PREFSMAN->m_bMercifulBeginner )
		iWeight = max( 0, iWeight );
	return iWeight;
}

int ScoreKeeperNormal::HoldNoteScoreToDancePoints( HoldNoteScore hns, bool bBeginner )
{
	int iWeight = 0;
	switch( hns )
	{
	DEFAULT_FAIL( hns );
	case HNS_None:	iWeight = 0;									break;
	case HNS_LetGo:	iWeight = g_iPercentScoreWeight[SE_LetGo];	break;
	case HNS_Held:	iWeight = g_iPercentScoreWeight[SE_Held];		break;
	}
	if( bBeginner && PREFSMAN->m_bMercifulBeginner )
		iWeight = max( 0, iWeight );
	return iWeight;
}

int ScoreKeeperNormal::TapNoteScoreToGradePoints( TapNoteScore tns, bool bBeginner )
{
	if( !GAMESTATE->ShowW1() && tns == TNS_W1 )
		tns = TNS_W2;

	/* This is used for Oni percentage displays. Grading values are currently in
	 * StageStats::GetGrade. */
	int iWeight = 0;
	switch( tns )
	{
	DEFAULT_FAIL( tns );
	case TNS_None:		iWeight = 0;							break;
	case TNS_AvoidMine:	iWeight = 0;						break;
	case TNS_HitMine:	iWeight = g_iGradeWeight[SE_HitMine];	break;
	case TNS_Miss:		iWeight = g_iGradeWeight[SE_Miss];		break;
	case TNS_W5:		iWeight = g_iGradeWeight[SE_W5];		break;
	case TNS_W4:		iWeight = g_iGradeWeight[SE_W4];		break;
	case TNS_W3:		iWeight = g_iGradeWeight[SE_W3];		break;
	case TNS_W2:		iWeight = g_iGradeWeight[SE_W2];		break;
	case TNS_W1:		iWeight = g_iGradeWeight[SE_W1];		break;
	case TNS_CheckpointHit:	iWeight = g_iGradeWeight[SE_CheckpointHit];	break;
	case TNS_CheckpointMiss:iWeight = g_iGradeWeight[SE_CheckpointMiss];	break;
	}
	if( bBeginner && PREFSMAN->m_bMercifulBeginner )
		iWeight = max( 0, iWeight );
	return iWeight;
}

int ScoreKeeperNormal::HoldNoteScoreToGradePoints( HoldNoteScore hns, bool bBeginner )
{
	int iWeight = 0;
	switch( hns )
	{
	DEFAULT_FAIL( hns );
	case HNS_None:	iWeight = 0;							break;
	case HNS_LetGo:	iWeight = g_iGradeWeight[SE_LetGo];	break;
	case HNS_Held:	iWeight = g_iGradeWeight[SE_Held];		break;
	}
	if( bBeginner && PREFSMAN->m_bMercifulBeginner )
		iWeight = max( 0, iWeight );
	return iWeight;
}

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include "global.h"
#include "StageStats.h"
#include "GameState.h"
#include "RageLog.h"
#include "SongManager.h"
#include "RageUtil.h"
#include "PrefsManager.h"
#include "Foreach.h"
#include "Steps.h"
#include "song.h"

StageStats	g_CurStageStats;
vector<StageStats>	g_vPlayedStageStats;

void PlayerStageStats::Init()
{
	vpSteps.clear();
	fAliveSeconds = 0;
	bFailed = bFailedEarlier = false;
	iPossibleDancePoints = iActualDancePoints = 0;
	iCurCombo = iMaxCombo = iCurMissCombo = iScore = iBonus = iMaxScore = iMaxScoreToNow = 0;
	fSecondsBeforeFail = 0;
	iSongsPassed = iSongsPlayed = 0;
	iTotalError = 0;

	ZERO( iTapNoteScores );
	ZERO( iHoldNoteScores );
	radarPossible.Zero();
	radarActual.Zero();

	fFirstSecond = 999999;
	fLastSecond = 0;
}

void PlayerStageStats::AddStats( const PlayerStageStats& other )
{
	FOREACH_CONST( Steps*, other.vpSteps, s )
		vpSteps.push_back( *s );
	fAliveSeconds += other.fAliveSeconds;
	bFailed |= other.bFailed;
	bFailedEarlier |= other.bFailedEarlier;
	iPossibleDancePoints += other.iPossibleDancePoints;
	iActualDancePoints += other.iActualDancePoints;
	
	for( int t=0; t<NUM_TAP_NOTE_SCORES; t++ )
		iTapNoteScores[t] += other.iTapNoteScores[t];
	for( int h=0; h<NUM_HOLD_NOTE_SCORES; h++ )
		iHoldNoteScores[h] += other.iHoldNoteScores[h];
	iCurCombo += other.iCurCombo;
	iMaxCombo += other.iMaxCombo;
	iCurMissCombo += other.iCurMissCombo;
	iScore += other.iScore;
	iMaxScore += other.iMaxScore;
	iMaxScoreToNow += other.iMaxScoreToNow;
	radarPossible += other.radarPossible;
	radarActual += other.radarActual;
	fSecondsBeforeFail += other.fSecondsBeforeFail;
	iSongsPassed += other.iSongsPassed;
	iSongsPlayed += other.iSongsPlayed;
	iTotalError += other.iTotalError;

	const float fOtherFirstSecond = other.fFirstSecond + fLastSecond;
	const float fOtherLastSecond = other.fLastSecond + fLastSecond;
	fLastSecond = fOtherLastSecond;

	map<float,float>::const_iterator it;
	for( it = other.fLifeRecord.begin(); it != other.fLifeRecord.end(); ++it )
	{
		const float pos = it->first;
		const float life = it->second;
		fLifeRecord[fOtherFirstSecond+pos] = life;
	}

	for( unsigned i=0; i<other.ComboList.size(); ++i )
	{
		const Combo_t &combo = other.ComboList[i];

		Combo_t newcombo(combo);
		newcombo.fStartSecond += fOtherFirstSecond;
		ComboList.push_back( newcombo );
	}

	/* Merge identical combos.  This normally only happens in course mode, when a combo
	 * continues between songs. */
	for( unsigned i=1; i<ComboList.size(); ++i )
	{
		Combo_t &prevcombo = ComboList[i-1];
		Combo_t &combo = ComboList[i];
		const float PrevComboEnd = prevcombo.fStartSecond + prevcombo.fSizeSeconds;
		const float ThisComboStart = combo.fStartSecond;
		if( fabsf(PrevComboEnd - ThisComboStart) > 0.001 )
			continue;

		/* These are really the same combo. */
		prevcombo.fSizeSeconds += combo.fSizeSeconds;
		prevcombo.cnt += combo.cnt;
		ComboList.erase( ComboList.begin()+i );
		--i;
	}
}

Grade PlayerStageStats::GetGrade() const
{
	if( bFailedEarlier )
		return GRADE_FAILED;

	/* XXX: This entire calculation should be in ScoreKeeper, but final evaluation
	 * is tricky since at that point the ScoreKeepers no longer exist. 
	 *
	 * See http://www.aaroninjapan.com/ddr2.html ("Regular play scoring") */
	float Possible = 0, Actual = 0;
	FOREACH_TapNoteScore( tns )
	{
		int iTapScoreValue;
		switch( tns )
		{
		case TNS_NONE:		iTapScoreValue = 0;											break;
		case TNS_HIT_MINE:	iTapScoreValue = PREFSMAN->m_iGradeWeightHitMine;	break;
		case TNS_MISS:		iTapScoreValue = PREFSMAN->m_iGradeWeightMiss;		break;
		case TNS_BOO:		iTapScoreValue = PREFSMAN->m_iGradeWeightBoo;		break;
		case TNS_GOOD:		iTapScoreValue = PREFSMAN->m_iGradeWeightGood;		break;
		case TNS_GREAT:		iTapScoreValue = PREFSMAN->m_iGradeWeightGreat;		break;
		case TNS_PERFECT:	iTapScoreValue = PREFSMAN->m_iGradeWeightPerfect;	break;
		case TNS_MARVELOUS:	iTapScoreValue = PREFSMAN->m_iGradeWeightMarvelous;	break;
		default: FAIL_M( ssprintf("%i", tns) );											break;
		}
		Actual += iTapNoteScores[tns] * iTapScoreValue;
		Possible += iTapNoteScores[tns] * PREFSMAN->m_iGradeWeightMarvelous;
	}

	FOREACH_HoldNoteScore( hns )
	{
		int iHoldScoreValue;
		switch( hns )
		{
		case HNS_NONE:	iHoldScoreValue = 0;									break;
		case HNS_NG:	iHoldScoreValue = PREFSMAN->m_iGradeWeightNG;	break;
		case HNS_OK:	iHoldScoreValue = PREFSMAN->m_iGradeWeightOK;	break;
		default: FAIL_M( ssprintf("%i", hns) );									break;
		}
		Actual += iHoldNoteScores[hns] * iHoldScoreValue;
		Possible += iHoldNoteScores[hns] * PREFSMAN->m_iGradeWeightOK;
	}

	LOG->Trace( "GetGrade: Actual: %f, Possible: %f", Actual, Possible );

	Grade grade = GRADE_FAILED;

	float fPercent = (Possible == 0) ? 0 : Actual / Possible;

	FOREACH_Grade(g)
	{
		if( fPercent >= PREFSMAN->m_fGradePercent[g] )
		{
			grade = g;
			break;
		}
	}

	LOG->Trace( "GetGrade: Grade: %s, %i", GradeToString(grade).c_str(), PREFSMAN->m_bGradeTier02IsAllPerfects );
	if( PREFSMAN->m_bGradeTier02IsAllPerfects )
	{
		if(	iTapNoteScores[TNS_MARVELOUS] > 0 &&
			iTapNoteScores[TNS_PERFECT] == 0 &&
			iTapNoteScores[TNS_GREAT] == 0 &&
			iTapNoteScores[TNS_GOOD] == 0 &&
			iTapNoteScores[TNS_BOO] == 0 &&
			iTapNoteScores[TNS_MISS] == 0 &&
			iTapNoteScores[TNS_HIT_MINE] == 0 &&
			iHoldNoteScores[HNS_NG] == 0 )
			return GRADE_TIER_1;

		if( iTapNoteScores[TNS_PERFECT] > 0 &&
			iTapNoteScores[TNS_GREAT] == 0 &&
			iTapNoteScores[TNS_GOOD] == 0 &&
			iTapNoteScores[TNS_BOO] == 0 &&
			iTapNoteScores[TNS_MISS] == 0 &&
			iTapNoteScores[TNS_HIT_MINE] == 0 &&
			iHoldNoteScores[HNS_NG] == 0 )
			return GRADE_TIER_2;

		return max( grade, GRADE_TIER_3 );
	}

	return grade;
}


void StageStats::Init()
{
	playMode = PLAY_MODE_INVALID;
	pStyle = NULL;
	vpSongs.clear();
	StageType = STAGE_INVALID;
	fGameplaySeconds = 0;
}

void StageStats::AssertValid( PlayerNumber pn ) const
{
	if( vpSongs[0] )
		CHECKPOINT_M( vpSongs[0]->GetFullTranslitTitle() );
	ASSERT( m_player[pn].vpSteps[0] );
	ASSERT_M( playMode < NUM_PLAY_MODES, ssprintf("playmode %i", playMode) );
	ASSERT( pStyle != NULL );
	ASSERT_M( m_player[pn].vpSteps[0]->GetDifficulty() < NUM_DIFFICULTIES, ssprintf("difficulty %i", m_player[pn].vpSteps[0]->GetDifficulty()) );
	ASSERT( vpSongs.size() == m_player[pn].vpSteps.size() );
}


int StageStats::GetAverageMeter( PlayerNumber pn ) const
{
	int iTotalMeter = 0;
	int iTotalCount = 0;
	ASSERT( vpSongs.size() == m_player[pn].vpSteps.size() );

	for( unsigned i=0; i<vpSongs.size(); i++ )
	{
		Song* pSong = vpSongs[i];
		Steps* pSteps = m_player[pn].vpSteps[i];

		// weight long and marathon songs
		int iWeight = SongManager::GetNumStagesForSong( pSong );
		int iMeter = pSteps->GetMeter();
		
		iTotalMeter += iMeter;
		iTotalCount += iWeight;
	}
	return iTotalMeter / iTotalCount;	// round down
}

void StageStats::AddStats( const StageStats& other )
{
	ASSERT( !other.vpSongs.empty() );
	FOREACH_CONST( Song*, other.vpSongs, s )
		vpSongs.push_back( *s );
	StageType = STAGE_INVALID; // meaningless
	
	fGameplaySeconds += other.fGameplaySeconds;

	FOREACH_PlayerNumber( p )
	{
		m_player[p].AddStats( other.m_player[p] );
	}
}

bool StageStats::OnePassed() const
{
	FOREACH_PlayerNumber( p )
		if( GAMESTATE->IsHumanPlayer(p) && !m_player[p].bFailed )
			return true;
	return false;
}

bool StageStats::AllFailed() const
{
	FOREACH_EnabledPlayer( pn )
		if( !m_player[pn].bFailed )
			return false;
	return true;
}

bool StageStats::AllFailedEarlier() const
{
	FOREACH_EnabledPlayer( p )
		if( !m_player[p].bFailedEarlier )
			return false;
	return true;
}

float PlayerStageStats::GetPercentDancePoints() const
{
	if( iPossibleDancePoints == 0 )
		return 0; // div/0

	if( iActualDancePoints == iPossibleDancePoints )
		return 1;	// correct for rounding error

	/* This can happen in battle, with transform attacks. */
	//ASSERT_M( iActualDancePoints <= iPossibleDancePoints, ssprintf("%i/%i", iActualDancePoints, iPossibleDancePoints) );

	float fPercentDancePoints =  iActualDancePoints / (float)iPossibleDancePoints;
	
	return fPercentDancePoints;
}

void PlayerStageStats::SetLifeRecordAt( float fLife, float fSecond )
{
	if( fSecond < 0 )
		return;

	fFirstSecond = min( fSecond, fFirstSecond );
	fLastSecond = max( fSecond, fLastSecond );
	//LOG->Trace( "fLastSecond = %f", fLastSecond );

	if( !fLifeRecord.empty() )
	{
		const float old = GetLifeRecordAt( fSecond );
		if( fabsf(old-fSecond) < 0.001f )
			return; /* no change */
	}

	fLifeRecord[fSecond] = fLife;
}

float PlayerStageStats::GetLifeRecordAt( float fSecond ) const
{
	/* Find the first element whose key is not less than k. */
	map<float,float>::const_iterator it = fLifeRecord.lower_bound( fSecond );

	/* Find the first element whose key is less than k. */
	if( it != fLifeRecord.begin() )
		--it;

	return it->second;

}

float PlayerStageStats::GetLifeRecordLerpAt( float fSecond ) const
{
	/* Find the first element whose key is not less than k. */
	map<float,float>::const_iterator later = fLifeRecord.lower_bound( fSecond );

	/* Find the first element whose key is less than k. */
	map<float,float>::const_iterator earlier = later;
	if( earlier != fLifeRecord.begin() )
		--earlier;

	if( earlier->first == later->first )
		return earlier->second;

	/* earlier <= pos <= later */
	const float f = SCALE( fSecond, earlier->first, later->first, 1, 0 );
	return earlier->second * f + later->second * (1-f);
}


void PlayerStageStats::GetLifeRecord( float *fLifeOut, int iNumSamples ) const
{
	for( int i = 0; i < iNumSamples; ++i )
	{
		float from = SCALE( i, 0, (float)iNumSamples, fFirstSecond, fLastSecond );
		fLifeOut[i] = GetLifeRecordLerpAt( from );
	}
}

/* If "rollover" is true, we're being called before gameplay begins, so we can record
 * the amount of the first combo that comes from the previous song. */
void PlayerStageStats::UpdateComboList( float fSecond, bool rollover )
{
	if( fSecond < 0 )
		return;

	if( !rollover )
	{
		fFirstSecond = min( fSecond, fFirstSecond );
		fLastSecond = max( fSecond, fLastSecond );
		//LOG->Trace( "fLastSecond = %f", fLastSecond );
	}

	int cnt = iCurCombo;
	if( !cnt )
		return; /* no combo */

	if( ComboList.size() == 0 || ComboList.back().cnt >= cnt )
	{
		/* If the previous combo (if any) starts on -9999, then we rolled over some
		 * combo, but missed the first step.  Remove it. */
		if( ComboList.size() && ComboList.back().fStartSecond == -9999 )
			ComboList.erase( ComboList.begin()+ComboList.size()-1, ComboList.end() );

		/* This is a new combo. */
		Combo_t NewCombo;
		/* "start" is the position that the combo started within this song.  If we're
		 * recording rollover, the combo hasn't started yet (within this song), so put
		 * a placeholder in and set it on the next call.  (Otherwise, start will be less
		 * than fFirstPos.) */
		if( rollover )
			NewCombo.fStartSecond = -9999;
		else
			NewCombo.fStartSecond = fSecond;
		ComboList.push_back( NewCombo );
	}

	Combo_t &combo = ComboList.back();
	combo.fSizeSeconds = fSecond - combo.fStartSecond;
	combo.cnt = cnt;
	if( !rollover && combo.fStartSecond == -9999 )
		combo.fStartSecond = fSecond;

	if( rollover )
		combo.rollover = cnt;
}

/* This returns the largest combo contained within the song, as if
 * m_bComboContinuesBetweenSongs is turned off. */
PlayerStageStats::Combo_t PlayerStageStats::GetMaxCombo() const
{
	if( ComboList.size() == 0 )
		return Combo_t();

	int m = 0;
	for( unsigned i = 1; i < ComboList.size(); ++i )
	{
		if( ComboList[i].cnt > ComboList[m].cnt )
			m = i;
	}

	return ComboList[m];
}

int	PlayerStageStats::GetComboAtStartOfStage() const
{
	if( ComboList.empty() )
		return 0;
	else
		return ComboList[0].rollover;
}

bool PlayerStageStats::FullComboOfScore( TapNoteScore tnsAllGreaterOrEqual ) const
{
	ASSERT( tnsAllGreaterOrEqual >= TNS_GREAT );
	ASSERT( tnsAllGreaterOrEqual <= TNS_MARVELOUS );
	
	if( iHoldNoteScores[HNS_NG] > 0 )
		return false;

	for( int i=TNS_MISS; i<tnsAllGreaterOrEqual; i++ )
	{
		if( iTapNoteScores[i] > 0 )
			return false;
	}
	return true;
}

bool PlayerStageStats::SingleDigitsOfScore( TapNoteScore tnsAllGreaterOrEqual ) const
{
	return FullComboOfScore( tnsAllGreaterOrEqual ) &&
		iTapNoteScores[tnsAllGreaterOrEqual] < 10;
}

bool PlayerStageStats::OneOfScore( TapNoteScore tnsAllGreaterOrEqual ) const
{
	return FullComboOfScore( tnsAllGreaterOrEqual ) &&
		iTapNoteScores[tnsAllGreaterOrEqual] == 1;
}

int PlayerStageStats::GetTotalTaps() const
{
	int iTotalTaps = 0;
	for( int i=TNS_MISS; i<NUM_TAP_NOTE_SCORES; i++ )
	{
		iTotalTaps += iTapNoteScores[i];
	}
	return iTotalTaps;
}

float PlayerStageStats::GetPercentageOfTaps( TapNoteScore tns ) const
{
	int iTotalTaps = 0;
	for( int i=TNS_MISS; i<NUM_TAP_NOTE_SCORES; i++ )
	{
		iTotalTaps += iTapNoteScores[i];
	}
	return iTapNoteScores[tns] / (float)iTotalTaps;
}

static Grade GetBestGrade()
{
	Grade g = NUM_GRADES;
	FOREACH_EnabledPlayer( pn )
		g = min( g, g_CurStageStats.m_player[pn].GetGrade() );
	return g;
}

static Grade GetWorstGrade()
{
	Grade g = GRADE_TIER_1;
	FOREACH_EnabledPlayer( pn )
		g = max( g, g_CurStageStats.m_player[pn].GetGrade() );
	return g;
}

#include "LuaFunctions.h"
LuaFunction_NoArgs( GetStagesPlayed,		(int) g_vPlayedStageStats.size() );
LuaFunction_NoArgs( GetBestGrade,			GetBestGrade() );
LuaFunction_NoArgs( GetWorstGrade,			GetWorstGrade() );
LuaFunction_NoArgs( OnePassed,				g_CurStageStats.OnePassed() );
LuaFunction_PlayerNumber( FullCombo,		g_CurStageStats.m_player[pn].FullCombo() )
LuaFunction_PlayerNumber( MaxCombo,			g_CurStageStats.m_player[pn].GetMaxCombo().cnt )
LuaFunction_PlayerNumber( GetGrade,			g_CurStageStats.m_player[pn].GetGrade() )
LuaFunction_Str( Grade,						StringToGrade(str) );

const StageStats *GetStageStatsN( int n )
{
	if( n == (int) g_vPlayedStageStats.size() )
		return &g_CurStageStats;
	if( n > (int) g_vPlayedStageStats.size() )
		return NULL;
	return &g_vPlayedStageStats[n];
}

/* GetGrade(0) returns the first grade; GetGrade(1) returns the second grade, and
 * and so on.  GetGrade(GetStagesPlayed()) returns the current grade (from g_CurStageStats).
 * If beyond the current song played, return GRADE_NO_DATA. */
Grade GetGrade( int n, PlayerNumber pn )
{
	const StageStats *pStats = GetStageStatsN( n );
	if( pStats == NULL )
		return GRADE_NO_DATA;
	return pStats->m_player[pn].GetGrade();
}

bool OneGotGrade( int n, Grade g )
{
	FOREACH_HumanPlayer( pn )
		if( GetGrade( n, pn ) == g )
			return true;

	return false;
}


LuaFunction_IntInt( OneGotGrade, OneGotGrade( a1, (Grade) a2 ) );

Grade GetFinalGrade( PlayerNumber pn )
{
	if( !GAMESTATE->IsHumanPlayer(pn) )
		return GRADE_NO_DATA;
	StageStats stats;
	GAMESTATE->GetFinalEvalStats( stats );
	return stats.m_player[pn].GetGrade();
}
LuaFunction_PlayerNumber( GetFinalGrade, GetFinalGrade(pn) );

Grade GetBestFinalGrade()
{
	Grade top_grade = GRADE_FAILED;
	StageStats stats;
	GAMESTATE->GetFinalEvalStats( stats );
	FOREACH_PlayerNumber( p )
		if( GAMESTATE->IsHumanPlayer(p) )
			top_grade = min( top_grade, stats.m_player[p].GetGrade() );
	return top_grade;
}
LuaFunction_NoArgs( GetBestFinalGrade, GetBestFinalGrade() );

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

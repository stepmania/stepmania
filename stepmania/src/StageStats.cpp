#include "global.h"
#include "StageStats.h"
#include "GameState.h"
#include "RageLog.h"
#include "SongManager.h"
#include "RageUtil.h"
#include "PrefsManager.h"

StageStats	g_CurStageStats;
vector<StageStats>	g_vPlayedStageStats;

void StageStats::Init()
{
	playMode = PLAY_MODE_INVALID;
	pStyle = NULL;
	pSong = NULL;
	StageType = STAGE_INVALID;
	fGameplaySeconds = 0;

	FOREACH_PlayerNumber( p )
	{
		pSteps[p] = NULL;
		iMeter[p] = 0;
		fAliveSeconds[p] = 0;
		bFailed[p] = bFailedEarlier[p] = false;
		iPossibleDancePoints[p] = iActualDancePoints[p] = 0;
		iCurCombo[p] = iMaxCombo[p] = iCurMissCombo[p] = iScore[p] = iBonus[p] = 0;
		fSecondsBeforeFail[p] = 0;
		iSongsPassed[p] = iSongsPlayed[p] = 0;
		iTotalError[p] = 0;

		memset( iTapNoteScores[p], 0, sizeof(iTapNoteScores[p]) );
		memset( iHoldNoteScores[p], 0, sizeof(iHoldNoteScores[p]) );
		radarPossible[p].Init();
		radarActual[p].Init();

		fFirstSecond[p] = 999999;
		fLastSecond[p] = 0;
	}
}

void StageStats::AddStats( const StageStats& other )
{
	pSong = NULL;		// meaningless
	StageType = STAGE_INVALID; // meaningless
	memset( fAliveSeconds, 0, sizeof(fAliveSeconds) );	// why? -Chris
	
	// weight long and marathon songs
	ASSERT( other.pSong );
	const int iLengthMultiplier = SongManager::GetNumStagesForSong( other.pSong );

	fGameplaySeconds += other.fGameplaySeconds;

	FOREACH_PlayerNumber( p )
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
		radarPossible[p] += other.radarPossible[p];
		radarActual[p] += other.radarActual[p];
		fSecondsBeforeFail[p] += other.fSecondsBeforeFail[p];
		iSongsPassed[p] += other.iSongsPassed[p];
		iSongsPlayed[p] += other.iSongsPlayed[p];
		iTotalError[p] += other.iTotalError[p];

		const float fOtherFirstSecond = other.fFirstSecond[p] + fLastSecond[p];
		const float fOtherLastSecond = other.fLastSecond[p] + fLastSecond[p];
		fLastSecond[p] = fOtherLastSecond;

		map<float,float>::const_iterator it;
		for( it = other.fLifeRecord[p].begin(); it != other.fLifeRecord[p].end(); ++it )
		{
			const float pos = it->first;
			const float life = it->second;
			fLifeRecord[p][fOtherFirstSecond+pos] = life;
		}

		unsigned i;
		for( i=0; i<other.ComboList[p].size(); ++i )
		{
			const Combo_t &combo = other.ComboList[p][i];

			Combo_t newcombo(combo);
			newcombo.fStartSecond += fOtherFirstSecond;
			ComboList[p].push_back( newcombo );
		}

		/* Merge identical combos.  This normally only happens in course mode, when a combo
		 * continues between songs. */
		for( i=1; i<ComboList[p].size(); ++i )
		{
			Combo_t &prevcombo = ComboList[p][i-1];
			Combo_t &combo = ComboList[p][i];
			const float PrevComboEnd = prevcombo.fStartSecond + prevcombo.fSizeSeconds;
			const float ThisComboStart = combo.fStartSecond;
			if( fabsf(PrevComboEnd - ThisComboStart) > 0.001 )
				continue;

			/* These are really the same combo. */
			prevcombo.fSizeSeconds += combo.fSizeSeconds;
			prevcombo.cnt += combo.cnt;
			ComboList[p].erase( ComboList[p].begin()+i );
			--i;
		}
	}
}

Grade StageStats::GetGrade( PlayerNumber pn ) const
{
	ASSERT( GAMESTATE->IsPlayerEnabled(pn) );	// shouldn't be calling this is player isn't joined!

	if( bFailedEarlier[pn] )
		return GRADE_FAILED;

	/* XXX: This entire calculation should be in ScoreKeeper, but final evaluation
	 * is tricky since at that point the ScoreKeepers no longer exist. 
	 *
	 * See http://www.aaroninjapan.com/ddr2.html ("Regular play scoring") */
	int TapScoreValues[NUM_TAP_NOTE_SCORES] = 
	{ 
		0,
		PREFSMAN->m_iGradeWeightHitMine,
		PREFSMAN->m_iGradeWeightMiss,
		PREFSMAN->m_iGradeWeightBoo,
		PREFSMAN->m_iGradeWeightGood,
		PREFSMAN->m_iGradeWeightGreat,
		PREFSMAN->m_iGradeWeightPerfect,
		PREFSMAN->m_iGradeWeightMarvelous,
	};
	int HoldScoreValues[NUM_HOLD_NOTE_SCORES] =
	{
		0,
		PREFSMAN->m_iGradeWeightOK,
		PREFSMAN->m_iGradeWeightNG
	};

	float Possible = 0, Actual = 0;
	int i;
	for( i = 0; i < NUM_TAP_NOTE_SCORES; ++i )
	{
		Actual += iTapNoteScores[pn][i] * TapScoreValues[i];
		Possible += iTapNoteScores[pn][i] * TapScoreValues[TNS_MARVELOUS];
	}

	for( i = HNS_OK; i < NUM_HOLD_NOTE_SCORES; ++i )
	{
		Actual += iHoldNoteScores[pn][i] * HoldScoreValues[i];
		Possible += iHoldNoteScores[pn][i] * HoldScoreValues[HNS_OK];
	}

	LOG->Trace( "GetGrade: Actual: %f, Possible: %f", Actual, Possible );

#define ROUNDING_ERROR 0.00001f
	Grade grade = GRADE_FAILED;

	float fPercent = (Possible == 0) ? 0 : Actual / Possible;

	FOREACH_Grade(g)
	{
		if( fPercent >= PREFSMAN->m_fGradePercent[g]-ROUNDING_ERROR )
		{
			grade = g;
			break;
		}
	}

	LOG->Trace( "GetGrade: Grade: %s, %i", GradeToString(grade).c_str(), PREFSMAN->m_bGradeTier02IsAllPerfects );
	if( PREFSMAN->m_bGradeTier02IsAllPerfects )
	{
		if(	iTapNoteScores[pn][TNS_MARVELOUS] > 0 &&
			iTapNoteScores[pn][TNS_PERFECT] == 0 &&
			iTapNoteScores[pn][TNS_GREAT] == 0 &&
			iTapNoteScores[pn][TNS_GOOD] == 0 &&
			iTapNoteScores[pn][TNS_BOO] == 0 &&
			iTapNoteScores[pn][TNS_MISS] == 0 &&
			iTapNoteScores[pn][TNS_HIT_MINE] == 0 &&
			iHoldNoteScores[pn][HNS_NG] == 0 )
			return GRADE_TIER_1;

		if( iTapNoteScores[pn][TNS_PERFECT] > 0 &&
			iTapNoteScores[pn][TNS_GREAT] == 0 &&
			iTapNoteScores[pn][TNS_GOOD] == 0 &&
			iTapNoteScores[pn][TNS_BOO] == 0 &&
			iTapNoteScores[pn][TNS_MISS] == 0 &&
			iTapNoteScores[pn][TNS_HIT_MINE] == 0 &&
			iHoldNoteScores[pn][HNS_NG] == 0 )
			return GRADE_TIER_2;

		return max( grade, GRADE_TIER_3 );
	}

	return grade;
}

bool StageStats::OnePassed() const
{
	FOREACH_PlayerNumber( p )
		if( GAMESTATE->IsHumanPlayer(p) && !bFailed[p] )
			return true;
	return false;
}

bool StageStats::AllFailed() const
{
	FOREACH_PlayerNumber( pn )
		if( GAMESTATE->IsPlayerEnabled(PlayerNumber(pn)) )
			if( !bFailed[pn] )
				return false;
	return true;
}

bool StageStats::AllFailedEarlier() const
{
	FOREACH_PlayerNumber( p )
		if( GAMESTATE->IsPlayerEnabled(p) && !bFailedEarlier[p] )
			return false;
	return true;
}

float StageStats::GetPercentDancePoints( PlayerNumber pn ) const
{
	if( iPossibleDancePoints[pn] == 0 )
		return 0; // div/0

	if( iActualDancePoints[pn] == iPossibleDancePoints[pn] )
		return 1;	// correct for rounding error

	/* This can happen in battle, with transform attacks. */
	//ASSERT_M( iActualDancePoints[pn] <= iPossibleDancePoints[pn], ssprintf("%i/%i", iActualDancePoints[pn], iPossibleDancePoints[pn]) );

	float fPercentDancePoints =  iActualDancePoints[pn] / (float)iPossibleDancePoints[pn];
	
	return fPercentDancePoints;
}

void StageStats::SetLifeRecordAt( PlayerNumber pn, float fLife, float fSecond )
{
	if( fSecond < 0 )
		return;

	fFirstSecond[pn] = min( fSecond, fFirstSecond[pn] );
	fLastSecond[pn] = max( fSecond, fLastSecond[pn] );
	LOG->Trace( "fLastSecond = %f", fLastSecond[pn] );

	if( !fLifeRecord[pn].empty() )
	{
		const float old = GetLifeRecordAt( pn, fSecond );
		if( fabsf(old-fSecond) < 0.001f )
			return; /* no change */
	}

	fLifeRecord[pn][fSecond] = fLife;
}

float	StageStats::GetLifeRecordAt( PlayerNumber pn, float fSecond ) const
{
	/* Find the first element whose key is not less than k. */
	map<float,float>::const_iterator it = fLifeRecord[pn].lower_bound( fSecond );

	/* Find the first element whose key is less than k. */
	if( it != fLifeRecord[pn].begin() )
		--it;

	return it->second;

}

float StageStats::GetLifeRecordLerpAt( PlayerNumber pn, float fSecond ) const
{
	/* Find the first element whose key is not less than k. */
	map<float,float>::const_iterator later = fLifeRecord[pn].lower_bound( fSecond );

	/* Find the first element whose key is less than k. */
	map<float,float>::const_iterator earlier = later;
	if( earlier != fLifeRecord[pn].begin() )
		--earlier;

	if( earlier->first == later->first )
		return earlier->second;

	/* earlier <= pos <= later */
	const float f = SCALE( fSecond, earlier->first, later->first, 1, 0 );
	return earlier->second * f + later->second * (1-f);
}


void StageStats::GetLifeRecord( PlayerNumber pn, float *fLifeOut, int iNumSamples ) const
{
	for( int i = 0; i < iNumSamples; ++i )
	{
		float from = SCALE( i, 0, (float)iNumSamples, fFirstSecond[pn], fLastSecond[pn] );
		fLifeOut[i] = GetLifeRecordLerpAt( pn, from );
	}
}

/* If "rollover" is true, we're being called before gameplay begins, so we can record
 * the amount of the first combo that comes from the previous song. */
void StageStats::UpdateComboList( PlayerNumber pn, float fSecond, bool rollover )
{
	if( fSecond < 0 )
		return;

	if( !rollover )
	{
		fFirstSecond[pn] = min( fSecond, fFirstSecond[pn] );
		fLastSecond[pn] = max( fSecond, fLastSecond[pn] );
		LOG->Trace( "fLastSecond = %f", fLastSecond[pn] );
	}

	int cnt = iCurCombo[pn];
	if( !cnt )
		return; /* no combo */

	if( ComboList[pn].size() == 0 || ComboList[pn].back().cnt >= cnt )
	{
		/* If the previous combo (if any) starts on -9999, then we rolled over some
		 * combo, but missed the first step.  Remove it. */
		if( ComboList[pn].size() && ComboList[pn].back().fStartSecond == -9999 )
			ComboList[pn].erase( ComboList[pn].begin()+ComboList[pn].size()-1, ComboList[pn].end() );

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
		ComboList[pn].push_back( NewCombo );
	}

	Combo_t &combo = ComboList[pn].back();
	combo.fSizeSeconds = fSecond - combo.fStartSecond;
	combo.cnt = cnt;
	if( !rollover && combo.fStartSecond == -9999 )
		combo.fStartSecond = fSecond;

	if( rollover )
		combo.rollover = cnt;
}

/* This returns the largest combo contained within the song, as if
 * m_bComboContinuesBetweenSongs is turned off. */
StageStats::Combo_t StageStats::GetMaxCombo( PlayerNumber pn ) const
{
	if( ComboList[pn].size() == 0 )
		return Combo_t();

	int m = 0;
	for( unsigned i = 1; i < ComboList[pn].size(); ++i )
	{
		if( ComboList[pn][i].cnt > ComboList[pn][m].cnt )
			m = i;
	}

	return ComboList[pn][m];
}

int	StageStats::GetComboAtStartOfStage( PlayerNumber pn ) const
{
	if( ComboList[pn].empty() )
		return 0;
	else
		return ComboList[pn][0].rollover;
}

bool StageStats::FullComboOfScore( PlayerNumber pn, TapNoteScore tnsAllGreaterOrEqual ) const
{
	ASSERT( tnsAllGreaterOrEqual >= TNS_GREAT );
	ASSERT( tnsAllGreaterOrEqual <= TNS_MARVELOUS );
	
	if( iHoldNoteScores[pn][HNS_NG] > 0 )
		return false;

	for( int i=TNS_MISS; i<tnsAllGreaterOrEqual; i++ )
	{
		if( iTapNoteScores[pn][i] > 0 )
			return false;
	}
	return true;
}

bool StageStats::SingleDigitsOfScore( PlayerNumber pn, TapNoteScore tnsAllGreaterOrEqual ) const
{
	return FullComboOfScore( pn, tnsAllGreaterOrEqual ) &&
		iTapNoteScores[pn][tnsAllGreaterOrEqual] < 10;
}

bool StageStats::OneOfScore( PlayerNumber pn, TapNoteScore tnsAllGreaterOrEqual ) const
{
	return FullComboOfScore( pn, tnsAllGreaterOrEqual ) &&
		iTapNoteScores[pn][tnsAllGreaterOrEqual] == 1;
}

int StageStats::GetTotalTaps( PlayerNumber pn ) const
{
	int iTotalTaps = 0;
	for( int i=TNS_MISS; i<NUM_TAP_NOTE_SCORES; i++ )
	{
		iTotalTaps += iTapNoteScores[pn][i];
	}
	return iTotalTaps;
}

float StageStats::GetPercentageOfTaps( PlayerNumber pn, TapNoteScore tns ) const
{
	int iTotalTaps = 0;
	for( int i=TNS_MISS; i<NUM_TAP_NOTE_SCORES; i++ )
	{
		iTotalTaps += iTapNoteScores[pn][i];
	}
	return iTapNoteScores[pn][tns] / (float)iTotalTaps;
}

static Grade GetBestGrade()
{
	Grade g = NUM_GRADES;
	for( unsigned pn=0; pn<NUM_PLAYERS; ++pn )
	{
		if( !GAMESTATE->IsPlayerEnabled(pn) )
			continue;
		g = min( g, g_CurStageStats.GetGrade( (PlayerNumber)pn ) );
	}
	return g;
}

static Grade GetWorstGrade()
{
	Grade g = GRADE_TIER_1;
	for( unsigned pn=0; pn<NUM_PLAYERS; ++pn )
	{
		if( !GAMESTATE->IsPlayerEnabled(pn) )
			continue;
		g = max( g, g_CurStageStats.GetGrade( (PlayerNumber)pn ) );
	}
	return g;
}

#include "LuaFunctions.h"
LuaFunction_NoArgs( GetStagesPlayed,		(int) g_vPlayedStageStats.size() );
LuaFunction_NoArgs( GetBestGrade,			GetBestGrade() );
LuaFunction_NoArgs( GetWorstGrade,			GetWorstGrade() );
LuaFunction_NoArgs( OnePassed,				g_CurStageStats.OnePassed() );
LuaFunction_PlayerNumber( FullCombo,		g_CurStageStats.FullCombo(pn) )
LuaFunction_PlayerNumber( MaxCombo,			g_CurStageStats.GetMaxCombo(pn).cnt )
LuaFunction_PlayerNumber( GetGrade,			g_CurStageStats.GetGrade(pn) )
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
	return pStats->GetGrade( pn );
}

bool OneGotGrade( int n, Grade g )
{
	FOREACH_HumanPlayer( pn )
		if( GetGrade( n, (PlayerNumber)pn ) == g )
			return true;

	return false;
}


LuaFunction_IntInt( OneGotGrade, OneGotGrade( a1, (Grade) a2 ) );

Grade GetFinalGrade( PlayerNumber pn )
{
	if( !GAMESTATE->IsHumanPlayer(pn) )
		return GRADE_NO_DATA;
	vector<Song*> vSongs;
	StageStats stats;
	GAMESTATE->GetFinalEvalStatsAndSongs( stats, vSongs );
	return stats.GetGrade( pn );
}
LuaFunction_PlayerNumber( GetFinalGrade, GetFinalGrade( pn ) );

Grade GetBestFinalGrade()
{
	Grade top_grade = GRADE_FAILED;
	vector<Song*> vSongs;
	StageStats stats;
	GAMESTATE->GetFinalEvalStatsAndSongs( stats, vSongs );
	FOREACH_PlayerNumber( p )
		if( GAMESTATE->IsHumanPlayer(p) )
			top_grade = min( top_grade, stats.GetGrade((PlayerNumber)p) );
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

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
#include "PrefsManager.h"

StageStats	g_CurStageStats;
vector<StageStats>	g_vPlayedStageStats;

StageStats::StageStats()
{
	playMode = PLAY_MODE_INVALID;
	style = STYLE_INVALID;
	pSong = NULL;
	StageType = STAGE_INVALID;
	fGameplaySeconds = 0;

	for( int p=0; p<NUM_PLAYERS; p++ )
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
		fFirstPos[p] = fLastPos[p] = 0;

		memset( iTapNoteScores[p], 0, sizeof(iTapNoteScores[p]) );
		memset( iHoldNoteScores[p], 0, sizeof(iHoldNoteScores[p]) );
		memset( fRadarPossible[p], 0, sizeof(fRadarPossible[p]) );
		memset( fRadarActual[p], 0, sizeof(fRadarActual[p]) );

		fFirstPos[p] = 999999;
		fLastPos[p] = 0;
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

		const float fOtherFirst = other.fFirstPos[p] + fLastPos[p];
		const float fOtherLast = other.fLastPos[p] + fLastPos[p];

		map<float,float>::const_iterator it;
		for( it = other.fLifeRecord[p].begin(); it != other.fLifeRecord[p].end(); ++it )
		{
			const float pos = it->first;
			const float life = it->second;
			fLifeRecord[p][fOtherFirst+pos] = life;
		}

		unsigned i;
		for( i=0; i<other.ComboList[p].size(); ++i )
		{
			const Combo_t &combo = other.ComboList[p][i];

			Combo_t newcombo(combo);
			newcombo.start += fOtherFirst;
			ComboList[p].push_back( newcombo );
		}

		/* Merge identical combos.  This normally only happens in course mode, when a combo
		 * continues between songs. */
		for( i=1; i<ComboList[p].size(); ++i )
		{
			Combo_t &prevcombo = ComboList[p][i-1];
			Combo_t &combo = ComboList[p][i];
			const float PrevComboEnd = prevcombo.start + prevcombo.size;
			const float ThisComboStart = combo.start;
			if( fabsf(PrevComboEnd - ThisComboStart) > 0.001 )
				continue;

			/* These are really the same combo. */
			prevcombo.size += combo.size;
			prevcombo.cnt += combo.cnt;
			ComboList[p].erase( ComboList[p].begin()+i );
			--i;
		}
		
		fLastPos[p] = fOtherLast;
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
	Grade grade = GRADE_NO_DATA;

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
	for( int p=0; p<NUM_PLAYERS; p++ )
		if( GAMESTATE->IsHumanPlayer(p) && !bFailed[p] )
			return true;
	return false;
}

bool StageStats::AllFailed() const
{
	for( int pn=0; pn<NUM_PLAYERS; pn++ )
		if( GAMESTATE->IsPlayerEnabled(PlayerNumber(pn)) )
			if( !bFailed[pn] )
				return false;
	return true;
}

bool StageStats::AllFailedEarlier() const
{
	for( int p=0; p<NUM_PLAYERS; p++ )
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
	//RAGE_ASSERT_M( iActualDancePoints[pn] <= iPossibleDancePoints[pn], ssprintf("%i/%i", iActualDancePoints[pn], iPossibleDancePoints[pn]) );

	float fPercentDancePoints =  iActualDancePoints[pn] / (float)iPossibleDancePoints[pn];
	
	return fPercentDancePoints;
}

void StageStats::SetLifeRecord( PlayerNumber pn, float life, float pos )
{
	if( pos < 0 )
		return;

	fFirstPos[pn] = min( pos, fFirstPos[pn] );
	fLastPos[pn] = max( pos, fLastPos[pn] );

	if( !fLifeRecord[pn].empty() )
	{
		const float old = GetLifeRecordAt( pn, pos );
		if( fabsf(old-pos) < 0.001f )
			return; /* no change */
	}

	fLifeRecord[pn][pos] = life;
}

float	StageStats::GetLifeRecordAt( PlayerNumber pn, float pos ) const
{
	/* Find the first element whose key is not less than k. */
	map<float,float>::const_iterator it = fLifeRecord[pn].lower_bound( pos );

	/* Find the first element whose key is less than k. */
	if( it != fLifeRecord[pn].begin() )
		--it;

	return it->second;

}

float StageStats::GetLifeRecordLerpAt( PlayerNumber pn, float pos ) const
{
	/* Find the first element whose key is not less than k. */
	map<float,float>::const_iterator later = fLifeRecord[pn].lower_bound( pos );

	/* Find the first element whose key is less than k. */
	map<float,float>::const_iterator earlier = later;
	if( earlier != fLifeRecord[pn].begin() )
		--earlier;

	if( earlier->first == later->first )
		return earlier->second;

	/* earlier <= pos <= later */
	const float f = SCALE( pos, earlier->first, later->first, 1, 0 );
	return earlier->second * f + later->second * (1-f);
}


void StageStats::GetLifeRecord( PlayerNumber pn, float *life, int nout ) const
{
	for( int i = 0; i < nout; ++i )
	{
		float from = SCALE( i, 0, (float)nout, fFirstPos[pn], fLastPos[pn] );
		life[i] = GetLifeRecordLerpAt( (PlayerNumber)pn, from );
	}
}

/* If "rollover" is true, we're being called before gameplay begins, so we can record
 * the amount of the first combo that comes from the previous song. */
void StageStats::UpdateComboList( PlayerNumber pn, float pos, bool rollover )
{
	if( pos < 0 )
		return;

	if( !rollover )
	{
		fFirstPos[pn] = min( pos, fFirstPos[pn] );
		fLastPos[pn] = max( pos, fLastPos[pn] );
	}

	int cnt = iCurCombo[pn];
	if( !cnt )
		return; /* no combo */

	if( ComboList[pn].size() == 0 || ComboList[pn].back().cnt >= cnt )
	{
		/* If the previous combo (if any) starts on -9999, then we rolled over some
		 * combo, but missed the first step.  Remove it. */
		if( ComboList[pn].size() && ComboList[pn].back().start == -9999 )
			ComboList[pn].erase( ComboList[pn].begin()+ComboList[pn].size()-1, ComboList[pn].end() );

		/* This is a new combo. */
		Combo_t NewCombo;
		/* "start" is the position that the combo started within this song.  If we're
		 * recording rollover, the combo hasn't started yet (within this song), so put
		 * a placeholder in and set it on the next call.  (Otherwise, start will be less
		 * than fFirstPos.) */
		if( rollover )
			NewCombo.start = -9999;
		else
			NewCombo.start = pos;
		ComboList[pn].push_back( NewCombo );
	}

	Combo_t &combo = ComboList[pn].back();
	combo.size = pos - combo.start;
	combo.cnt = cnt;
	if( !rollover && combo.start == -9999 )
		combo.start = pos;

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
LuaFunction_NoArgs( GetBestGrade,			GetBestGrade() );
LuaFunction_NoArgs( GetWorstGrade,			GetWorstGrade() );
LuaFunction_NoArgs( OnePassed,				g_CurStageStats.OnePassed() );
LuaFunction_PlayerNumber( FullCombo,		g_CurStageStats.FullCombo(pn) )
LuaFunction_PlayerNumber( MaxCombo,			g_CurStageStats.GetMaxCombo(pn).cnt )
LuaFunction_PlayerNumber( GetGrade,			g_CurStageStats.GetGrade(pn) )
LuaFunction_Str( Grade,						StringToGrade(str) );

/* PrevGrade(0) returns the last grade; PrevGrade(1) returns the one before that,
 * and so on.  If you go back beyond the first song played, return GRADE_NO_DATA. */
Grade GetPrevGrade( int n, PlayerNumber pn )
{
	int stage = int(g_vPlayedStageStats.size()) - n - 1;
	if( stage < 0 || stage >= (int) g_vPlayedStageStats.size() )
		return GRADE_NO_DATA;
	return g_vPlayedStageStats[stage].GetGrade( pn );
}

bool OneGotGrade( int n, Grade g )
{
	for( unsigned pn=0; pn<NUM_PLAYERS; pn++ )
		if( GAMESTATE->IsHumanPlayer((PlayerNumber)pn) )
			if( GetPrevGrade( n, (PlayerNumber)pn ) == g )
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
	for( int p=0; p<NUM_PLAYERS; p++ )
		if( GAMESTATE->IsHumanPlayer(p) )
			top_grade = min( top_grade, stats.GetGrade((PlayerNumber)p) );
	return top_grade;
}
LuaFunction_NoArgs( GetBestFinalGrade, GetBestFinalGrade() );

#include "global.h"
#include "PlayerStageStats.h"
#include "RageLog.h"
#include "ThemeManager.h"
#include "Foreach.h"
#include "LuaFunctions.h"
#include "LuaManager.h"
#include <float.h>
#include "GameState.h"
#include "Course.h"
#include "Steps.h"
#include "ScoreKeeperNormal.h"

#define GRADE_PERCENT_TIER(i)	THEME->GetMetricF("PlayerStageStats",ssprintf("GradePercent%s",GradeToString((Grade)i).c_str()))
#define GRADE_TIER02_IS_ALL_W2S	THEME->GetMetricB("PlayerStageStats","GradeTier02IsAllW2s")

const float LESSON_PASS_THRESHOLD = 0.8f;

void PlayerStageStats::Init()
{
	vpPlayedSteps.clear();
	vpPossibleSteps.clear();
	fAliveSeconds = 0;
	bFailed = false;
	bFailedEarlier = false;
	iPossibleDancePoints = iCurPossibleDancePoints = iActualDancePoints = 0;
	iPossibleGradePoints = 0;
	iCurCombo = iMaxCombo = iCurMissCombo = iScore = iBonus = iMaxScore = iCurMaxScore = 0;
	iSongsPassed = iSongsPlayed = 0;
	iTotalError = 0;
	fCaloriesBurned = 0;
	iTotalError = 0;
	fLifeRemainingSeconds = 0;

	ZERO( iTapNoteScores );
	ZERO( iHoldNoteScores );
	radarPossible.Zero();
	radarActual.Zero();

	fFirstSecond = FLT_MAX;
	fLastSecond = 0;

	m_pdaToShow = PER_DIFFICULTY_AWARD_INVALID;
	m_pcaToShow = PEAK_COMBO_AWARD_INVALID;
	m_iPersonalHighScoreIndex = -1;
	m_iMachineHighScoreIndex = -1;
	m_rc = RANKING_INVALID;
	m_HighScore = HighScore();
}

void PlayerStageStats::AddStats( const PlayerStageStats& other )
{
	FOREACH_CONST( Steps*, other.vpPlayedSteps, s )
		vpPlayedSteps.push_back( *s );
	FOREACH_CONST( Steps*, other.vpPossibleSteps, s )
		vpPossibleSteps.push_back( *s );
	fAliveSeconds += other.fAliveSeconds;
	bFailed |= other.bFailed;
	bFailedEarlier |= other.bFailedEarlier;
	iPossibleDancePoints += other.iPossibleDancePoints;
	iActualDancePoints += other.iActualDancePoints;
	iCurPossibleDancePoints += other.iCurPossibleDancePoints;
	iPossibleGradePoints += other.iPossibleGradePoints;
	
	for( int t=0; t<NUM_TapNoteScore; t++ )
		iTapNoteScores[t] += other.iTapNoteScores[t];
	for( int h=0; h<NUM_HoldNoteScore; h++ )
		iHoldNoteScores[h] += other.iHoldNoteScores[h];
	iCurCombo += other.iCurCombo;
	iMaxCombo += other.iMaxCombo;
	iCurMissCombo += other.iCurMissCombo;
	iScore += other.iScore;
	iMaxScore += other.iMaxScore;
	iCurMaxScore += other.iCurMaxScore;
	radarPossible += other.radarPossible;
	radarActual += other.radarActual;
	iSongsPassed += other.iSongsPassed;
	iSongsPlayed += other.iSongsPlayed;
	iTotalError += other.iTotalError;
	fCaloriesBurned += other.fCaloriesBurned;
	fLifeRemainingSeconds = other.fLifeRemainingSeconds;	// don't accumulate

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

Grade PlayerStageStats::GetGradeFromPercent( float fPercent )
{
	Grade grade = Grade_Failed;

	FOREACH_Grade(g)
	{
		if( fPercent >= GRADE_PERCENT_TIER(g) )
		{
			grade = g;
			break;
		}
	}
	return grade;
}

Grade PlayerStageStats::GetGrade() const
{
	if( bFailedEarlier )
		return Grade_Failed;

	/* XXX: This entire calculation should be in ScoreKeeper, but final evaluation
	 * is tricky since at that point the ScoreKeepers no longer exist. */
	float fActual = 0;

	bool bIsBeginner = false;
	if( vpPlayedSteps.size() && !GAMESTATE->IsCourseMode() )
		bIsBeginner = vpPlayedSteps[0]->GetDifficulty() == DIFFICULTY_BEGINNER;

	FOREACH_TapNoteScore( tns )
	{
		int iTapScoreValue = ScoreKeeperNormal::TapNoteScoreToGradePoints( tns, bIsBeginner );
		fActual += iTapNoteScores[tns] * iTapScoreValue;
		LOG->Trace( "GetGrade actual: %i * %i", iTapNoteScores[tns], iTapScoreValue );
	}

	FOREACH_HoldNoteScore( hns )
	{
		int iHoldScoreValue = ScoreKeeperNormal::HoldNoteScoreToGradePoints( hns, bIsBeginner );
		fActual += iHoldNoteScores[hns] * iHoldScoreValue;
		LOG->Trace( "GetGrade actual: %i * %i", iHoldNoteScores[hns], iHoldScoreValue );
	}

	LOG->Trace( "GetGrade: fActual: %f, fPossible: %d", fActual, iPossibleGradePoints );


	float fPercent = (iPossibleGradePoints == 0) ? 0 : fActual / iPossibleGradePoints;

	Grade grade = GetGradeFromPercent( fPercent );

	LOG->Trace( "GetGrade: Grade: %s, %i", GradeToString(grade).c_str(), GRADE_TIER02_IS_ALL_W2S );
	if( GRADE_TIER02_IS_ALL_W2S )
	{
		if(	iTapNoteScores[TNS_W1] > 0 &&
			iTapNoteScores[TNS_W2] == 0 &&
			iTapNoteScores[TNS_W3] == 0 &&
			iTapNoteScores[TNS_W4] == 0 &&
			iTapNoteScores[TNS_W5] == 0 &&
			iTapNoteScores[TNS_Miss] == 0 &&
			iTapNoteScores[TNS_HitMine] == 0 &&
			iHoldNoteScores[HNS_LetGo] == 0 )
			return Grade_Tier01;

		if( iTapNoteScores[TNS_W2] > 0 &&
			iTapNoteScores[TNS_W3] == 0 &&
			iTapNoteScores[TNS_W4] == 0 &&
			iTapNoteScores[TNS_W5] == 0 &&
			iTapNoteScores[TNS_Miss] == 0 &&
			iTapNoteScores[TNS_HitMine] == 0 &&
			iHoldNoteScores[HNS_LetGo] == 0 )
			return Grade_Tier02;

		return max( grade, Grade_Tier03 );
	}

	return grade;
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

float PlayerStageStats::GetCurMaxPercentDancePoints() const
{
	if ( iPossibleDancePoints == 0 )
		return 0; // div/0

	if ( iCurPossibleDancePoints == iPossibleDancePoints )
		return 1; // correct for rounding error

	float fCurMaxPercentDancePoints = iCurPossibleDancePoints / (float)iPossibleDancePoints;

	return fCurMaxPercentDancePoints;
}

int PlayerStageStats::GetLessonScoreActual() const
{
	int iScore = 0;

	FOREACH_TapNoteScore( tns )
	{
		switch( tns )
		{
		case TNS_AvoidMine:
		case TNS_W5:
		case TNS_W4:
		case TNS_W3:
		case TNS_W2:
		case TNS_W1:
			iScore += iTapNoteScores[tns];
			break;
		}
	}

	FOREACH_HoldNoteScore( hns )
	{
		switch( hns )
		{
		case HNS_Held:
			iScore += iHoldNoteScores[hns];
			break;
		}
	}

	return iScore;
}

int PlayerStageStats::GetLessonScoreNeeded() const
{
	float fScore = 0;

	FOREACH_CONST( Steps*, vpPossibleSteps, steps )
		fScore += (*steps)->GetRadarValues().m_Values.v.fNumTapsAndHolds;

	return lrintf( fScore * LESSON_PASS_THRESHOLD );
}

void PlayerStageStats::ResetScoreForLesson()
{
	iCurPossibleDancePoints = 0;
	iActualDancePoints = 0;
	FOREACH_TapNoteScore( tns )
		iTapNoteScores[tns] = 0;
	FOREACH_HoldNoteScore( hns )
		iHoldNoteScores[hns] = 0;
	iCurCombo = 0;
	iMaxCombo = 0;
	iCurMissCombo = 0;
	iScore = 0;
	iCurMaxScore = 0;
	iMaxScore = 0;
}

void PlayerStageStats::SetLifeRecordAt( float fLife, float fStepsSecond )
{
	// Don't save life stats in endless courses, or could run OOM in a few hours.
	if( GAMESTATE->m_pCurCourse && GAMESTATE->m_pCurCourse->IsEndless() )
		return;
	
	if( fStepsSecond < 0 )
		return;

	fFirstSecond = min( fStepsSecond, fFirstSecond );
	fLastSecond = max( fStepsSecond, fLastSecond );
	//LOG->Trace( "fLastSecond = %f", fLastSecond );

	// fSecond will always be greater than any value already in the map.
	fLifeRecord[fStepsSecond] = fLife;

	//
	// Memory optimization:
	// If we have three consecutive records A, B, and C all with the same fLife,
	// we can eliminate record B without losing data.  Only check the last three 
	// records in the map since we're only inserting at the end, and we know all 
	// earlier redundant records have already been removed.
	//
	map<float,float>::iterator C = fLifeRecord.end();
	--C;
	if( C == fLifeRecord.begin() ) // no earlier records left
		return;

	map<float,float>::iterator B = C;
	--B;
	if( B == fLifeRecord.begin() ) // no earlier records left
		return;

	map<float,float>::iterator A = B;
	--A;

	if( A->second == B->second && B->second == C->second )
		fLifeRecord.erase(B);
}

float PlayerStageStats::GetLifeRecordAt( float fStepsSecond ) const
{
	if( fLifeRecord.empty() )
		return 0;
	
	/* Find the first element whose key is greater than k. */
	map<float,float>::const_iterator it = fLifeRecord.upper_bound( fStepsSecond );

	/* Find the last element whose key is less than or equal to k. */
	if( it != fLifeRecord.begin() )
		--it;

	return it->second;

}

float PlayerStageStats::GetLifeRecordLerpAt( float fStepsSecond ) const
{
	if( fLifeRecord.empty() )
		return 0;
	
	/* Find the first element whose key is greater than k. */
	map<float,float>::const_iterator later = fLifeRecord.upper_bound( fStepsSecond );

	/* Find the last element whose key is less than or equal to k. */
	map<float,float>::const_iterator earlier = later;
	if( earlier != fLifeRecord.begin() )
		--earlier;

	if( earlier->first == later->first )	// two samples from the same time.  Don't divide by zero in SCALE
		return earlier->second;

	if( later == fLifeRecord.end() )
		return earlier->second;

	/* earlier <= pos <= later */
	return SCALE( fStepsSecond, earlier->first, later->first, earlier->second, later->second );
}


void PlayerStageStats::GetLifeRecord( float *fLifeOut, int iNumSamples, float fStepsEndSecond ) const
{
	for( int i = 0; i < iNumSamples; ++i )
	{
		float from = SCALE( i, 0, (float)iNumSamples, 0.0f, fStepsEndSecond );
		fLifeOut[i] = GetLifeRecordLerpAt( from );
	}
}

/* If "rollover" is true, we're being called before gameplay begins, so we can record
 * the amount of the first combo that comes from the previous song. */
void PlayerStageStats::UpdateComboList( float fSecond, bool bRollover )
{
	// Don't save combo stats in endless courses, or could run OOM in a few hours.
	if( GAMESTATE->m_pCurCourse && GAMESTATE->m_pCurCourse->IsEndless() )
		return;
	
	if( fSecond < 0 )
		return;

	if( !bRollover )
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
		if( bRollover )
			NewCombo.fStartSecond = -9999;
		else
			NewCombo.fStartSecond = fSecond;
		ComboList.push_back( NewCombo );
	}

	Combo_t &combo = ComboList.back();
	combo.fSizeSeconds = fSecond - combo.fStartSecond;
	combo.cnt = cnt;
	if( !bRollover && combo.fStartSecond == -9999 )
		combo.fStartSecond = fSecond;

	if( bRollover )
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
	ASSERT( tnsAllGreaterOrEqual >= TNS_W3 );
	ASSERT( tnsAllGreaterOrEqual <= TNS_W1 );

	// If missed any holds, then it's not a full combo
	if( iHoldNoteScores[HNS_LetGo] > 0 )
		return false;

	// If has any of the judgments below, then not a full combo
	for( int i=TNS_Miss; i<tnsAllGreaterOrEqual; i++ )
	{
		if( iTapNoteScores[i] > 0 )
			return false;
	}

	// If has at least one of the judgments equal to or above, then is a full combo.
	for( int i=tnsAllGreaterOrEqual; i<NUM_TapNoteScore; i++ )
	{
		if( iTapNoteScores[i] > 0 )
			return true;
	}

	return false;
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
	for( int i=TNS_Miss; i<NUM_TapNoteScore; i++ )
	{
		iTotalTaps += iTapNoteScores[i];
	}
	return iTotalTaps;
}

float PlayerStageStats::GetPercentageOfTaps( TapNoteScore tns ) const
{
	int iTotalTaps = 0;
	for( int i=TNS_Miss; i<NUM_TapNoteScore; i++ )
	{
		iTotalTaps += iTapNoteScores[i];
	}
	return iTapNoteScores[tns] / (float)iTotalTaps;
}

void PlayerStageStats::CalcAwards( PlayerNumber p, bool bGaveUp, bool bUsedAutoplay )
{
	LOG->Trace( "hand out awards" );
	
	m_pcaToShow = PEAK_COMBO_AWARD_INVALID;

	if( bGaveUp || bUsedAutoplay )
		return;
	
	deque<PerDifficultyAward> &vPdas = GAMESTATE->m_vLastPerDifficultyAwards[p];

	LOG->Trace( "per difficulty awards" );

	// per-difficulty awards
	// don't give per-difficutly awards if using easy mods
	if( !GAMESTATE->IsDisqualified(p) )
	{
		if( FullComboOfScore( TNS_W3 ) )
			vPdas.push_back( AWARD_FULL_COMBO_W3 );
		if( SingleDigitsOfScore( TNS_W3 ) )
			vPdas.push_back( AWARD_SINGLE_DIGIT_W3 );
		if( FullComboOfScore( TNS_W2 ) )
			vPdas.push_back( AWARD_FULL_COMBO_W2 );
		if( SingleDigitsOfScore( TNS_W2 ) )
			vPdas.push_back( AWARD_SINGLE_DIGIT_W2 );
		if( FullComboOfScore( TNS_W1 ) )
			vPdas.push_back( AWARD_FULL_COMBO_W1 );
		
		if( OneOfScore( TNS_W3 ) )
			vPdas.push_back( AWARD_ONE_W3 );
		if( OneOfScore( TNS_W2 ) )
			vPdas.push_back( AWARD_ONE_W2 );

		float fPercentW3s = GetPercentageOfTaps( TNS_W3 );
		if( fPercentW3s >= 0.8f )
			vPdas.push_back( AWARD_PERCENT_80_W3 );
		if( fPercentW3s >= 0.9f )
			vPdas.push_back( AWARD_PERCENT_90_W3 );
		if( fPercentW3s >= 1.f )
			vPdas.push_back( AWARD_PERCENT_100_W3 );
	}

	// Max one PDA per stage
	if( !vPdas.empty() )
		vPdas.erase( vPdas.begin(), vPdas.end()-1 );
	
	if( !vPdas.empty() )
		m_pdaToShow = vPdas.back();
	else
		m_pdaToShow = PER_DIFFICULTY_AWARD_INVALID;

	LOG->Trace( "done with per difficulty awards" );

	// DO give peak combo awards if using easy mods
	int iComboAtStartOfStage = GetComboAtStartOfStage();
	int iPeakCombo = GetMaxCombo().cnt;

	FOREACH_PeakComboAward( pca )
	{
		int iLevel = 1000 * (pca+1);
		bool bCrossedLevel = iComboAtStartOfStage < iLevel && iPeakCombo >= iLevel;
		LOG->Trace( "pca = %d, iLevel = %d, bCrossedLevel = %d", pca, iLevel, bCrossedLevel );
		if( bCrossedLevel )
			GAMESTATE->m_vLastPeakComboAwards[p].push_back( pca );
	}

	if( !GAMESTATE->m_vLastPeakComboAwards[p].empty() )
		m_pcaToShow = GAMESTATE->m_vLastPeakComboAwards[p].back();
	else
		m_pcaToShow = PEAK_COMBO_AWARD_INVALID;

	LOG->Trace( "done with per combo awards" );

}


LuaFunction( GetGradeFromPercent,	PlayerStageStats::GetGradeFromPercent( FArg(1) ) )


// lua start
#include "LuaBinding.h"

class LunaPlayerStageStats: public Luna<PlayerStageStats>
{
public:
	LunaPlayerStageStats() { LUA->Register( Register ); }

	DEFINE_METHOD( GetCaloriesBurned,			fCaloriesBurned )
	DEFINE_METHOD( GetLifeRemainingSeconds,		fLifeRemainingSeconds )
	DEFINE_METHOD( GetSurvivalSeconds,			GetSurvivalSeconds() )
	DEFINE_METHOD( FullCombo,					FullCombo() )
	DEFINE_METHOD( MaxCombo,					GetMaxCombo().cnt )
	DEFINE_METHOD( GetGrade,					GetGrade() )
	DEFINE_METHOD( GetLessonScoreActual,		GetLessonScoreActual() )
	DEFINE_METHOD( GetLessonScoreNeeded,		GetLessonScoreNeeded() )
	DEFINE_METHOD( GetPersonalHighScoreIndex,	m_iPersonalHighScoreIndex )
	DEFINE_METHOD( GetMachineHighScoreIndex,	m_iMachineHighScoreIndex )
	DEFINE_METHOD( GetPerDifficultyAward,		m_pdaToShow )
	DEFINE_METHOD( GetPeakComboAward,			m_pcaToShow )

	static void Register(lua_State *L)
	{
		ADD_METHOD( GetCaloriesBurned );
		ADD_METHOD( GetLifeRemainingSeconds );
		ADD_METHOD( GetSurvivalSeconds );
		ADD_METHOD( FullCombo );
		ADD_METHOD( MaxCombo );
		ADD_METHOD( GetGrade );
		ADD_METHOD( GetLessonScoreActual );
		ADD_METHOD( GetLessonScoreNeeded );
		ADD_METHOD( GetPersonalHighScoreIndex );
		ADD_METHOD( GetMachineHighScoreIndex );
		ADD_METHOD( GetPerDifficultyAward );
		ADD_METHOD( GetPeakComboAward );

		Luna<T>::Register( L );
	}
};

LUA_REGISTER_CLASS( PlayerStageStats )
// lua end


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

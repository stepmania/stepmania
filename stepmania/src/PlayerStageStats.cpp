#include "global.h"
#include "PlayerStageStats.h"
#include "RageLog.h"
#include "PrefsManager.h"
#include "ThemeManager.h"
#include "Foreach.h"
#include <float.h>

#define GRADE_PERCENT_TIER(i)			THEME->GetMetricF("PlayerStageStats",ssprintf("GradePercent%s",GradeToString((Grade)i).c_str()))
#define GRADE_TIER_02_IS_ALL_PERFECTS	THEME->GetMetricB("PlayerStageStats","GradeTier02IsAllPerfects")

void PlayerStageStats::Init()
{
	vpSteps.clear();
	fAliveSeconds = 0;
	bFailed = bFailedEarlier = false;
	iPossibleDancePoints = iActualDancePoints = 0;
	iCurCombo = iMaxCombo = iCurMissCombo = iScore = iBonus = iMaxScore = iCurMaxScore = 0;
	fSecondsBeforeFail = 0;
	iSongsPassed = iSongsPlayed = 0;
	iTotalError = 0;
	fCaloriesBurned = 0;

	ZERO( iTapNoteScores );
	ZERO( iHoldNoteScores );
	radarPossible.Zero();
	radarActual.Zero();

	fFirstSecond = FLT_MAX;
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
	iCurMaxScore += other.iCurMaxScore;
	radarPossible += other.radarPossible;
	radarActual += other.radarActual;
	fSecondsBeforeFail += other.fSecondsBeforeFail;
	iSongsPassed += other.iSongsPassed;
	iSongsPlayed += other.iSongsPlayed;
	iTotalError += other.iTotalError;
	fCaloriesBurned += other.fCaloriesBurned;

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
	 * is tricky since at that point the ScoreKeepers no longer exist. */
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
		if( tns != TNS_HIT_MINE )
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
		if( fPercent >= GRADE_PERCENT_TIER(g) )
		{
			grade = g;
			break;
		}
	}

	LOG->Trace( "GetGrade: Grade: %s, %i", GradeToString(grade).c_str(), GRADE_TIER_02_IS_ALL_PERFECTS );
	if( GRADE_TIER_02_IS_ALL_PERFECTS )
	{
		if(	iTapNoteScores[TNS_MARVELOUS] > 0 &&
			iTapNoteScores[TNS_PERFECT] == 0 &&
			iTapNoteScores[TNS_GREAT] == 0 &&
			iTapNoteScores[TNS_GOOD] == 0 &&
			iTapNoteScores[TNS_BOO] == 0 &&
			iTapNoteScores[TNS_MISS] == 0 &&
			iTapNoteScores[TNS_HIT_MINE] == 0 &&
			iHoldNoteScores[HNS_NG] == 0 )
			return GRADE_TIER_01;

		if( iTapNoteScores[TNS_PERFECT] > 0 &&
			iTapNoteScores[TNS_GREAT] == 0 &&
			iTapNoteScores[TNS_GOOD] == 0 &&
			iTapNoteScores[TNS_BOO] == 0 &&
			iTapNoteScores[TNS_MISS] == 0 &&
			iTapNoteScores[TNS_HIT_MINE] == 0 &&
			iHoldNoteScores[HNS_NG] == 0 )
			return GRADE_TIER_02;

		return max( grade, GRADE_TIER_03 );
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


// lua start
#include "LuaBinding.h"

template<class T>
class LunaPlayerStageStats : public Luna<T>
{
public:
	LunaPlayerStageStats() { LUA->Register( Register ); }

	static int GetCaloriesBurned( T* p, lua_State *L )		{ lua_pushnumber(L, p->fCaloriesBurned ); return 1; }

	static void Register(lua_State *L)
	{
		ADD_METHOD( GetCaloriesBurned )
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

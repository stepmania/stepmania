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
		PREFSMAN->m_iGradeHitMineWeight,
		PREFSMAN->m_iGradeMissWeight,
		PREFSMAN->m_iGradeBooWeight,
		PREFSMAN->m_iGradeGoodWeight,
		PREFSMAN->m_iGradeGreatWeight,
		PREFSMAN->m_iGradePerfectWeight,
		PREFSMAN->m_iGradeMarvelousWeight,
	};
	int HoldScoreValues[NUM_HOLD_NOTE_SCORES] =
	{
		0,
		PREFSMAN->m_iGradeNGWeight,
		PREFSMAN->m_iGradeOKWeight,
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

	float fPercentDancePoints = Actual / Possible;
	fPercentDancePoints = max( 0.f, fPercentDancePoints );
	LOG->Trace( "GetGrade: Actual: %f, Possible: %f, Percent: %f", Actual, Possible, fPercentDancePoints );

	/* check for "AAAA".  Check DP == 100%: if we're using eg. "LITTLE", we might have all
	 * marvelouses but still not qualify for a AAAA. */
	if( PREFSMAN->m_bGradeTier1IsAllMarvelouses &&
		fPercentDancePoints > .9999 &&
		iTapNoteScores[pn][TNS_MARVELOUS] > 0 &&
		iTapNoteScores[pn][TNS_PERFECT] == 0 &&
		iTapNoteScores[pn][TNS_GREAT] == 0 &&
		iTapNoteScores[pn][TNS_GOOD] == 0 &&
		iTapNoteScores[pn][TNS_BOO] == 0 &&
		iTapNoteScores[pn][TNS_MISS] == 0 &&
		iHoldNoteScores[pn][HNS_NG] == 0 )
		return GRADE_TIER_1;

	if( PREFSMAN->m_bGradeTier2IsAllPerfects &&
		fPercentDancePoints > .9999 &&
		iTapNoteScores[pn][TNS_PERFECT] > 0 &&
		iTapNoteScores[pn][TNS_GREAT] == 0 &&
		iTapNoteScores[pn][TNS_GOOD] == 0 &&
		iTapNoteScores[pn][TNS_BOO] == 0 &&
		iTapNoteScores[pn][TNS_MISS] == 0 &&
		iHoldNoteScores[pn][HNS_NG] == 0 )
		return GRADE_TIER_2;


	// Start checking at TIER_3 if m_bGradeTier2IsAllPerfects is true.
	// Start checking at TIER_2 if m_bGradeTier1IsAllMarvelouses is true.
	int g;
	if( PREFSMAN->m_bGradeTier2IsAllPerfects )
		g = 2;
	else if( PREFSMAN->m_bGradeTier1IsAllMarvelouses )
		g = 1;
	else
		g = 0;
	for( ; g<NUM_GRADE_TIERS; g++ )
		if( fPercentDancePoints >= PREFSMAN->m_fGradePercentTier[g] )
			return (Grade)(GRADE_TIER_1+g);

	return (Grade)(GRADE_TIER_1+NUM_GRADE_TIERS-1);
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

	ASSERT( iActualDancePoints[pn] <= iPossibleDancePoints[pn] );

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
		/* This is a new combo. */
		Combo_t NewCombo;
		NewCombo.start = pos;
		ComboList[pn].push_back( NewCombo );
	}

	Combo_t &combo = ComboList[pn].back();
	combo.size = pos - combo.start;
	combo.cnt = cnt;

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

bool StageStats::FullCombo( PlayerNumber pn ) const
{
	if( ComboList[pn].size() != 1 )
	{
		LOG->Trace("FullCombo(%i): %i != 1", pn, (int)ComboList[pn].size());
		return false;
	}

	const float ComboStart = ComboList[pn][0].start;
	const float ComboEnd = ComboList[pn][0].start + ComboList[pn][0].size;

	const bool ComboStartsAtBeginning = fabs( ComboStart - fFirstPos[pn] ) < 0.001f;
	const bool ComboEndsAtEnd = fabs( ComboEnd - fLastPos[pn] ) < 0.001f;
	
	LOG->Trace("FullCombo(%i): %f .. %f, %f .. %f, %i, %i",
		pn, ComboStart, ComboEnd, fFirstPos[pn], fLastPos[pn], ComboStartsAtBeginning, ComboEndsAtEnd );
	return ComboStartsAtBeginning && ComboEndsAtEnd;
}


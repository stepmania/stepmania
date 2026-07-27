#include "global.h"
#include "PlayerStageStats.h"
#include "RageLog.h"
#include "ThemeManager.h"
#include "LuaManager.h"
#include "GameState.h"
#include "Course.h"
#include "Steps.h"
#include "ScoreKeeperNormal.h"
#include "PrefsManager.h"
#include "CommonMetrics.h"

#include <cfloat>
#include <cmath>
#include <cstddef>
#include <numeric>

#define GRADE_PERCENT_TIER(i)	THEME->GetMetricF("PlayerStageStats",ssprintf("GradePercent%s",GradeToString((Grade)i).c_str()))
// deprecated, but no solution to replace them exists yet:
#define GRADE_TIER02_IS_ALL_W2S	THEME->GetMetricB("PlayerStageStats","GradeTier02IsAllW2s")
#define GRADE_TIER01_IS_ALL_W2S THEME->GetMetricB("PlayerStageStats","GradeTier01IsAllW2s")
#define GRADE_TIER02_IS_FULL_COMBO THEME->GetMetricB("PlayerStageStats","GradeTier02IsFullCombo")

static ThemeMetric<TapNoteScore> g_MinScoreToMaintainCombo( "Gameplay", "MinScoreToMaintainCombo" );
static ThemeMetric<bool> g_MineHitIncrementsMissCombo( "Gameplay", "MineHitIncrementsMissCombo" );

const float LESSON_PASS_THRESHOLD = 0.8f;

Grade GetGradeFromPercent( float fPercent );

void PlayerStageStats::InternalInit()
{
	m_pStyle= nullptr;
	m_for_multiplayer= false;
	m_player_number= PLAYER_1;
	m_multiplayer_number= MultiPlayer_P1;

  m_bPlayerCanAchieveFullCombo = true;
	m_bJoined = false;
	m_vpPossibleSteps.clear();
	m_iStepsPlayed = 0;
	m_fAliveSeconds = 0;
	m_bFailed = false;
	m_iPossibleDancePoints = 0;
	m_iCurPossibleDancePoints = 0;
	m_iActualDancePoints = 0;
	m_iPossibleGradePoints = 0;
	m_iCurCombo = 0;
	m_iMaxCombo = 0;
	m_iCurMissCombo = 0;
	m_iCurScoreMultiplier = 1;
	m_iScore = 0;
	m_iMaxScore = 0;
	m_iCurMaxScore = 0;
	m_iSongsPassed = 0;
	m_iSongsPlayed = 0;
	m_fLifeRemainingSeconds = 0;
	m_iNumControllerSteps = 0;
	m_fCaloriesBurned = 0;

	ZERO( m_iTapNoteScores );
	ZERO( m_iHoldNoteScores );
	m_radarPossible.Zero();
	m_radarActual.Zero();

	m_fFirstSecond = FLT_MAX;
	m_fLastSecond = 0;

	m_StageAward = StageAward_Invalid;
	m_PeakComboAward = PeakComboAward_Invalid;
	m_iPersonalHighScoreIndex = -1;
	m_iMachineHighScoreIndex = -1;
	m_bDisqualified = false;
	m_rc = RankingCategory_Invalid;
	m_HighScore = HighScore();
}

void PlayerStageStats::Init(PlayerNumber pn)
{
	m_for_multiplayer= false;
	m_player_number= pn;
}

void PlayerStageStats::Init(MultiPlayer pn)
{
	m_for_multiplayer= true;
	m_multiplayer_number= pn;
}

void PlayerStageStats::AddStats( const PlayerStageStats& other )
{
	m_pStyle= other.m_pStyle;
	m_bJoined = other.m_bJoined;
	for (Steps *s : other.m_vpPossibleSteps)
		m_vpPossibleSteps.push_back( s );
	m_iStepsPlayed += other.m_iStepsPlayed;
	m_fAliveSeconds += other.m_fAliveSeconds;
	m_bFailed |= other.m_bFailed;
	m_iPossibleDancePoints += other.m_iPossibleDancePoints;
	m_iActualDancePoints += other.m_iActualDancePoints;
	m_iCurPossibleDancePoints += other.m_iCurPossibleDancePoints;
	m_iPossibleGradePoints += other.m_iPossibleGradePoints;

	for( int t=0; t<NUM_TapNoteScore; t++ )
		m_iTapNoteScores[t] += other.m_iTapNoteScores[t];
	for( int h=0; h<NUM_HoldNoteScore; h++ )
		m_iHoldNoteScores[h] += other.m_iHoldNoteScores[h];
	m_iCurCombo += other.m_iCurCombo;
	m_iMaxCombo += other.m_iMaxCombo;
	m_iCurMissCombo += other.m_iCurMissCombo;
	m_iScore += other.m_iScore;
	m_iMaxScore += other.m_iMaxScore;
	m_iCurMaxScore += other.m_iCurMaxScore;
	m_radarPossible += other.m_radarPossible;
	m_radarActual += other.m_radarActual;
	m_iSongsPassed += other.m_iSongsPassed;
	m_iSongsPlayed += other.m_iSongsPlayed;
	m_iNumControllerSteps += other.m_iNumControllerSteps;
	m_fCaloriesBurned += other.m_fCaloriesBurned;
	m_fLifeRemainingSeconds = other.m_fLifeRemainingSeconds;	// don't accumulate
	m_bDisqualified |= other.m_bDisqualified;

	// FirstSecond is always 0, and last second is the time of the last step,
	// so add 1 second between the stages so that the last element of this
	// stage's record isn't overwritten by the first element of the other
	// stage's record. -Kyz
	const float fOtherFirstSecond = other.m_fFirstSecond + m_fLastSecond + 1.0f;
	const float fOtherLastSecond = other.m_fLastSecond + m_fLastSecond + 1.0f;
	m_fLastSecond = fOtherLastSecond;

	std::map<float, float>::const_iterator it;
	for( it = other.m_fLifeRecord.begin(); it != other.m_fLifeRecord.end(); ++it )
	{
		const float pos = it->first;
		const float life = it->second;
		m_fLifeRecord[fOtherFirstSecond+pos] = life;
	}

	for( unsigned i=0; i<other.m_ComboList.size(); ++i )
	{
		const Combo_t &combo = other.m_ComboList[i];

		Combo_t newcombo(combo);
		newcombo.m_fStartSecond += fOtherFirstSecond;
		m_ComboList.push_back( newcombo );
	}

	/* Merge identical combos. This normally only happens in course mode, when
	 * a combo continues between songs. */
	for( unsigned i=1; i<m_ComboList.size(); ++i )
	{
		Combo_t &prevcombo = m_ComboList[i-1];
		Combo_t &combo = m_ComboList[i];
		const float PrevComboEnd = prevcombo.m_fStartSecond + prevcombo.m_fSizeSeconds;
		const float ThisComboStart = combo.m_fStartSecond;
		if( std::abs(PrevComboEnd - ThisComboStart) > 0.001 )
			continue;

		// These are really the same combo.
		prevcombo.m_fSizeSeconds += combo.m_fSizeSeconds;
		prevcombo.m_cnt += combo.m_cnt;
		m_ComboList.erase( m_ComboList.begin()+i );
		--i;
	}
}

Grade GetGradeFromPercent( float fPercent )
{
	Grade grade = Grade_Failed;

	FOREACH_ENUM( Grade,g)
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
	if( m_bFailed )
		return Grade_Failed;

	/* XXX: This entire calculation should be in ScoreKeeper, but final evaluation
	 * is tricky since at that point the ScoreKeepers no longer exist. */
	float fActual = 0;

	bool bIsBeginner = false;
	if( m_iStepsPlayed > 0 && !GAMESTATE->IsCourseMode() )
		bIsBeginner = m_vpPossibleSteps[0]->GetDifficulty() == Difficulty_Beginner;

	FOREACH_ENUM( TapNoteScore, tns )
	{
		int iTapScoreValue = ScoreKeeperNormal::TapNoteScoreToGradePoints( tns, bIsBeginner );
		fActual += m_iTapNoteScores[tns] * iTapScoreValue;
		//LOG->Trace( "GetGrade actual: %i * %i", m_iTapNoteScores[tns], iTapScoreValue );
	}

	FOREACH_ENUM( HoldNoteScore, hns )
	{
		int iHoldScoreValue = ScoreKeeperNormal::HoldNoteScoreToGradePoints( hns, bIsBeginner );
		fActual += m_iHoldNoteScores[hns] * iHoldScoreValue;
		//LOG->Trace( "GetGrade actual: %i * %i", m_iHoldNoteScores[hns], iHoldScoreValue );
	}

	//LOG->Trace( "GetGrade: fActual: %f, fPossible: %d", fActual, m_iPossibleGradePoints );

	float fPercent = (m_iPossibleGradePoints == 0) ? 0 : fActual / m_iPossibleGradePoints;

	Grade grade = GetGradeFromPercent( fPercent );

	//LOG->Trace( "GetGrade: Grade: %s, %i", GradeToString(grade).c_str(), GRADE_TIER02_IS_ALL_W2S );

	// TODO: Change these conditions to use Lua instead. -aj
	if( GRADE_TIER02_IS_ALL_W2S )
	{
		if( FullComboOfScore(TNS_W1) )
			return Grade_Tier01;

		if( FullComboOfScore(TNS_W2) )
			return Grade_Tier02;

		grade = std::max( grade, Grade_Tier03 );
	}

	if( GRADE_TIER01_IS_ALL_W2S )
	{
		if( FullComboOfScore(TNS_W2) )
			return Grade_Tier01;
		grade = std::max( grade, Grade_Tier02 );
	}

	if( GRADE_TIER02_IS_FULL_COMBO )
	{
		if( FullComboOfScore(g_MinScoreToMaintainCombo) )
			return Grade_Tier02;
		grade = std::max( grade, Grade_Tier03 );
	}

	return grade;
}

float PlayerStageStats::MakePercentScore( int iActual, int iPossible )
{
	if( iPossible == 0 )
		return 0; // div/0

	if( iActual == iPossible )
		return 1;	// correct for rounding error

	// This can happen in battle, with transform attacks.
	//ASSERT_M( iActual <= iPossible, ssprintf("%i/%i", iActual, iPossible) );

	float fPercent =  iActual / (float)iPossible;

	// don't allow negative
	fPercent = std::max(0.0f, fPercent);

	int iPercentTotalDigits = 3 + CommonMetrics::PERCENT_SCORE_DECIMAL_PLACES;	// "100" + "." + "00"

	// TRICKY: printf will round, but we want to truncate. Otherwise, we may display
	// a percent score that's too high and doesn't match up with the calculated grade.
	float fTruncInterval = std::pow( 0.1f, (float)iPercentTotalDigits-1 );

	// TRICKY: ftruncf is rounding 1.0000000 to 0.99990004. Give a little boost
	// to fPercentDancePoints to correct for this.
	fPercent += 0.000001f;

	fPercent = ftruncf( fPercent, fTruncInterval );
	return fPercent;
}

RString PlayerStageStats::FormatPercentScore( float fPercentDancePoints )
{
	int iPercentTotalDigits = 3 + CommonMetrics::PERCENT_SCORE_DECIMAL_PLACES;	// "100" + "." + "00"

	RString s = ssprintf( "%*.*f%%", iPercentTotalDigits,
			     (int)CommonMetrics::PERCENT_SCORE_DECIMAL_PLACES,
			     fPercentDancePoints*100 );
	return s;
}

float PlayerStageStats::GetPercentDancePoints() const
{
	return MakePercentScore( m_iActualDancePoints, m_iPossibleDancePoints );
}

float PlayerStageStats::GetCurMaxPercentDancePoints() const
{
	if ( m_iPossibleDancePoints == 0 )
		return 0; // div/0

	if ( m_iCurPossibleDancePoints == m_iPossibleDancePoints )
		return 1; // correct for rounding error

	float fCurMaxPercentDancePoints = m_iCurPossibleDancePoints / (float)m_iPossibleDancePoints;

	return fCurMaxPercentDancePoints;
}

// TODO: Make this use lua. Let more judgments be possible. -Wolfman2000
int PlayerStageStats::GetLessonScoreActual() const
{
	int iScore = 0;

	FOREACH_ENUM( TapNoteScore, tns )
	{
		switch( tns )
		{
			case TNS_AvoidMine:
			case TNS_W5:
			case TNS_W4:
			case TNS_W3:
			case TNS_W2:
			case TNS_W1:
				iScore += m_iTapNoteScores[tns];
			default:
				break;
		}
	}

	FOREACH_ENUM( HoldNoteScore, hns )
	{
		switch( hns )
		{
			case HNS_Held:
				iScore += m_iHoldNoteScores[hns];
			default:
				break;
		}
	}

	return iScore;
}

int PlayerStageStats::GetLessonScoreNeeded() const
{
	float fScore = std::accumulate(m_vpPossibleSteps.begin(), m_vpPossibleSteps.end(), 0.f,
		[](float total, Steps const *steps) { return total + steps->GetRadarValues(PLAYER_1)[RadarCategory_TapsAndHolds]; });
	return std::lrint( fScore * LESSON_PASS_THRESHOLD );
}

void PlayerStageStats::ResetScoreForLesson()
{
	m_iCurPossibleDancePoints = 0;
	m_iActualDancePoints = 0;
	FOREACH_ENUM( TapNoteScore, tns )
		m_iTapNoteScores[tns] = 0;
	FOREACH_ENUM( HoldNoteScore, hns )
		m_iHoldNoteScores[hns] = 0;
	m_iCurCombo = 0;
	m_iMaxCombo = 0;
	m_iCurMissCombo = 0;
	m_iScore = 0;
	m_iCurMaxScore = 0;
	m_iMaxScore = 0;
}

void PlayerStageStats::SetLifeRecordAt( float fLife, float fStepsSecond )
{
	// Don't save life stats in endless courses, or could run OOM in a few hours.
	if( GAMESTATE->m_pCurCourse && GAMESTATE->m_pCurCourse->IsEndless() )
		return;

	if( fStepsSecond < 0 )
		return;

	m_fFirstSecond = std::min( fStepsSecond, m_fFirstSecond );
	m_fLastSecond = std::max( fStepsSecond, m_fLastSecond );
	//LOG->Trace( "fLastSecond = %f", m_fLastSecond );

	// fStepsSecond will usually be greater than any value already in the map,
	// but if a tap and a hold both set the life on the same frame, it won't.
	// Check whether an entry already exists for the current time, and move it
	// back a tiny bit if it does and the new value is not the same as the old.
	// Otherwise, you get the rare bug where the life graph shows a gradual
	// decline when the lifebar was actually full up to a miss.  This occurs
	// because the first call has full life, and removes the previous full life
	// entry.  Then the second call of the frame occurs and sets the life for
	// the current time to a lower value.
	// -Kyz
	std::map<float, float>::iterator curr= m_fLifeRecord.find(fStepsSecond);
	if(curr != m_fLifeRecord.end())
	{
		if(curr->second != fLife)
		{
			// 2^-8
			m_fLifeRecord[fStepsSecond - 0.00390625]= curr->second;
		}
	}
	m_fLifeRecord[fStepsSecond] = fLife;

	Message msg(static_cast<MessageID>(Message_LifeMeterChangedP1+Enum::to_integral(m_player_number)));
	msg.SetParam("Life", fLife);
	msg.SetParam("StepsSecond", fStepsSecond);
	MESSAGEMAN->Broadcast(msg);

	// Memory optimization:
	// If we have three consecutive records A, B, and C all with the same fLife,
	// we can eliminate record B without losing data. Only check the last three
	// records in the map since we're only inserting at the end, and we know all
	// earlier redundant records have already been removed.
	std::map<float, float>::iterator C = m_fLifeRecord.end();
	--C;
	if( C == m_fLifeRecord.begin() ) // no earlier records left
		return;

	std::map<float, float>::iterator B = C;
	--B;
	if( B == m_fLifeRecord.begin() ) // no earlier records left
		return;

	std::map<float, float>::iterator A = B;
	--A;

	if( A->second == B->second && B->second == C->second )
		m_fLifeRecord.erase(B);
}

float PlayerStageStats::GetLifeRecordAt( float fStepsSecond ) const
{
	if( m_fLifeRecord.empty() )
		return 0;

	// Find the first element whose key is greater than k.
	std::map<float, float>::const_iterator it = m_fLifeRecord.upper_bound( fStepsSecond );

	// Find the last element whose key is less than or equal to k.
	if( it != m_fLifeRecord.begin() )
		--it;

	return it->second;

}

float PlayerStageStats::GetLifeRecordLerpAt( float fStepsSecond ) const
{
	if( m_fLifeRecord.empty() )
		return 0;

	// Find the first element whose key is greater than k.
	std::map<float, float>::const_iterator later = m_fLifeRecord.upper_bound( fStepsSecond );

	// Find the last element whose key is less than or equal to k.
	std::map<float, float>::const_iterator earlier = later;
	if( earlier != m_fLifeRecord.begin() )
		--earlier;

	if( later == m_fLifeRecord.end() )
		return earlier->second;

	if( earlier->first == later->first ) // two samples from the same time.  Don't divide by zero in SCALE
		return earlier->second;

	// earlier <= pos <= later
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

float PlayerStageStats::GetCurrentLife() const
{
	if( m_fLifeRecord.empty() )
		return 0;
	std::map<float, float>::const_iterator iter = m_fLifeRecord.end();
	--iter;
	return iter->second;
}

/* If bRollover is true, we're being called before gameplay begins, so we can
 * record the amount of the first combo that comes from the previous song. */
void PlayerStageStats::UpdateComboList( float fSecond, bool bRollover )
{
	// Don't save combo stats in endless courses, or could run OOM in a few hours.
	if( GAMESTATE->m_pCurCourse && GAMESTATE->m_pCurCourse->IsEndless() )
		return;

	if( fSecond < 0 )
		return;

	if( !bRollover )
	{
		m_fFirstSecond = std::min( fSecond, m_fFirstSecond );
		m_fLastSecond = std::max( fSecond, m_fLastSecond );
		//LOG->Trace( "fLastSecond = %f", fLastSecond );
	}

	int cnt = m_iCurCombo;
	if( !cnt )
		return; // no combo

	if( m_ComboList.size() == 0 || m_ComboList.back().m_cnt >= cnt )
	{
		/* If the previous combo (if any) starts on -9999, then we rolled over
		 * some combo, but missed the first step. Remove it. */
		if( m_ComboList.size() && m_ComboList.back().m_fStartSecond == -9999 )
			m_ComboList.erase( m_ComboList.begin()+m_ComboList.size()-1, m_ComboList.end() );

		// This is a new combo.
		Combo_t NewCombo;
		/* "start" is the position that the combo started within this song.
		 * If we're recording rollover, the combo hasn't started yet (within
		 * this song), so put a placeholder in and set it on the next call.
		 * (Otherwise, start will be less than fFirstPos.) */
		if( bRollover )
			NewCombo.m_fStartSecond = -9999;
		else
			NewCombo.m_fStartSecond = fSecond;
		m_ComboList.push_back( NewCombo );
	}

	Combo_t &combo = m_ComboList.back();
	if( !bRollover && combo.m_fStartSecond == -9999 )
		combo.m_fStartSecond = 0;

	combo.m_fSizeSeconds = fSecond - combo.m_fStartSecond;
	combo.m_cnt = cnt;

	if( bRollover )
		combo.m_rollover = cnt;
}

/* This returns the largest combo contained within the song, as if
 * m_bComboContinuesBetweenSongs is turned off. */
PlayerStageStats::Combo_t PlayerStageStats::GetMaxCombo() const
{
	if( m_ComboList.size() == 0 )
		return Combo_t();

	int m = 0;
	for( unsigned i = 1; i < m_ComboList.size(); ++i )
	{
		if( m_ComboList[i].m_cnt > m_ComboList[m].m_cnt )
			m = i;
	}

	return m_ComboList[m];
}

int PlayerStageStats::GetComboAtStartOfStage() const
{
	if( m_ComboList.empty() )
		return 0;
	else
		return m_ComboList[0].m_rollover;
}

bool PlayerStageStats::FullComboOfScore( TapNoteScore tnsAllGreaterOrEqual ) const
{
	ASSERT( tnsAllGreaterOrEqual >= TNS_W5 );
	ASSERT( tnsAllGreaterOrEqual <= TNS_W1 );

  //if we've set MissCombo to anything besides 0, it's not a full combo
  if( !m_bPlayerCanAchieveFullCombo )
    return false;

	// If missed any holds, then it's not a full combo
	if( m_iHoldNoteScores[HNS_LetGo] > 0 )
		return false;

	//if any checkpoints were missed, it's not a full combo	either
	if( m_iTapNoteScores[TNS_CheckpointMiss] > 0 )
		return false;

	// If has any of the judgments below, then not a full combo
	for( int i=TNS_Miss; i<tnsAllGreaterOrEqual; i++ )
	{
		if( m_iTapNoteScores[i] > 0 )
			return false;
	}

	// hit any mines when they increment the miss combo? It's not a full combo.
	if( g_MineHitIncrementsMissCombo && m_iTapNoteScores[TNS_HitMine] > 0 )
		return false;

	// If has at least one of the judgments equal to or above, then is a full combo.
	for( int i=tnsAllGreaterOrEqual; i<NUM_TapNoteScore; i++ )
	{
		if( m_iTapNoteScores[i] > 0 )
			return true;
	}

	return false;
}

TapNoteScore PlayerStageStats::GetBestFullComboTapNoteScore() const
{
	// Optimization opportunity: ...
	// (seriously? -aj)
	for( TapNoteScore i=TNS_W1; i >= TNS_W5; enum_add(i,-1))
	{
		if( FullComboOfScore(i) )
			return i;
	}
	return TapNoteScore_Invalid;
}

bool PlayerStageStats::SingleDigitsOfScore( TapNoteScore tnsAllGreaterOrEqual ) const
{
	return FullComboOfScore( tnsAllGreaterOrEqual ) &&
		m_iTapNoteScores[tnsAllGreaterOrEqual] < 10;
}

bool PlayerStageStats::OneOfScore( TapNoteScore tnsAllGreaterOrEqual ) const
{
	return FullComboOfScore( tnsAllGreaterOrEqual ) &&
		m_iTapNoteScores[tnsAllGreaterOrEqual] == 1;
}

int PlayerStageStats::GetTotalTaps() const
{
	int iTotalTaps = 0;
	for( int i=TNS_Miss; i<NUM_TapNoteScore; i++ )
	{
		iTotalTaps += m_iTapNoteScores[i];
	}
	return iTotalTaps;
}

float PlayerStageStats::GetPercentageOfTaps( TapNoteScore tns ) const
{
	int iTotalTaps = 0;
	for( int i=TNS_Miss; i<NUM_TapNoteScore; i++ )
	{
		iTotalTaps += m_iTapNoteScores[i];
	}
	return m_iTapNoteScores[tns] / (float)iTotalTaps;
}

void PlayerStageStats::CalcAwards( PlayerNumber p, bool bGaveUp, bool bUsedAutoplay )
{
	//LOG->Trace( "hand out awards" );

	m_PeakComboAward = PeakComboAward_Invalid;

	if( bGaveUp || bUsedAutoplay )
		return;

	std::deque<StageAward> &vPdas = GAMESTATE->m_vLastStageAwards[p];

	//LOG->Trace( "per difficulty awards" );

	// per-difficulty awards
	// don't give per-difficutly awards if using easy mods
	if( !IsDisqualified() )
	{
		if( FullComboOfScore( TNS_W3 ) )
			vPdas.push_back( StageAward_FullComboW3 );
		if( SingleDigitsOfScore( TNS_W3 ) )
			vPdas.push_back( StageAward_SingleDigitW3 );
		if( FullComboOfScore( TNS_W2 ) )
			vPdas.push_back( StageAward_FullComboW2 );
		if( SingleDigitsOfScore( TNS_W2 ) )
			vPdas.push_back( StageAward_SingleDigitW2 );
		if( FullComboOfScore( TNS_W1 ) )
			vPdas.push_back( StageAward_FullComboW1 );

		if( OneOfScore( TNS_W3 ) )
			vPdas.push_back( StageAward_OneW3 );
		if( OneOfScore( TNS_W2 ) )
			vPdas.push_back( StageAward_OneW2 );

		float fPercentW3s = GetPercentageOfTaps( TNS_W3 );
		if( fPercentW3s >= 0.8f )
			vPdas.push_back( StageAward_80PercentW3 );
		if( fPercentW3s >= 0.9f )
			vPdas.push_back( StageAward_90PercentW3 );
		if( fPercentW3s >= 1.f )
			vPdas.push_back( StageAward_100PercentW3 );
	}

	// Max one PDA per stage
	if( !vPdas.empty() )
		vPdas.erase( vPdas.begin(), vPdas.end()-1 );

	if( !vPdas.empty() )
		m_StageAward = vPdas.back();
	else
		m_StageAward = StageAward_Invalid;

	//LOG->Trace( "done with per difficulty awards" );

	// DO give peak combo awards if using easy mods
	int iComboAtStartOfStage = GetComboAtStartOfStage();
	int iPeakCombo = GetMaxCombo().m_cnt;

	FOREACH_ENUM( PeakComboAward,pca )
	{
		int iLevel = 1000 * (pca+1);
		bool bCrossedLevel = iComboAtStartOfStage < iLevel && iPeakCombo >= iLevel;
		//LOG->Trace( "pca = %d, iLevel = %d, bCrossedLevel = %d", pca, iLevel, bCrossedLevel );
		if( bCrossedLevel )
			GAMESTATE->m_vLastPeakComboAwards[p].push_back( pca );
	}

	if( !GAMESTATE->m_vLastPeakComboAwards[p].empty() )
		m_PeakComboAward = GAMESTATE->m_vLastPeakComboAwards[p].back();
	else
		m_PeakComboAward = PeakComboAward_Invalid;

	//LOG->Trace( "done with per combo awards" );

}

bool PlayerStageStats::IsDisqualified() const
{
	if( !PREFSMAN->m_bDisqualification )
		return false;
	return m_bDisqualified;
}

LuaFunction( GetGradeFromPercent,	GetGradeFromPercent( FArg(1) ) )
LuaFunction( FormatPercentScore,	PlayerStageStats::FormatPercentScore( FArg(1) ) )


// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the PlayerStageStats. */
class LunaPlayerStageStats: public Luna<PlayerStageStats>
{
public:
	DEFINE_METHOD( GetCaloriesBurned,			m_fCaloriesBurned )
	DEFINE_METHOD( GetNumControllerSteps,		m_iNumControllerSteps )
	DEFINE_METHOD( GetLifeRemainingSeconds,		m_fLifeRemainingSeconds )
	DEFINE_METHOD( GetSurvivalSeconds,			GetSurvivalSeconds() )
	DEFINE_METHOD( GetCurrentCombo,				m_iCurCombo )
	DEFINE_METHOD( GetCurrentMissCombo,			m_iCurMissCombo )
	DEFINE_METHOD( GetCurrentScoreMultiplier,	m_iCurScoreMultiplier )
	DEFINE_METHOD( GetScore,					m_iScore )
	DEFINE_METHOD( GetCurMaxScore,				m_iCurMaxScore )
	DEFINE_METHOD( GetTapNoteScores,			m_iTapNoteScores[Enum::Check<TapNoteScore>(L, 1)] )
	DEFINE_METHOD( GetHoldNoteScores,			m_iHoldNoteScores[Enum::Check<HoldNoteScore>(L, 1)] )
	DEFINE_METHOD( FullCombo,					FullCombo() )
	DEFINE_METHOD( FullComboOfScore,			FullComboOfScore( Enum::Check<TapNoteScore>(L, 1) ) )
	DEFINE_METHOD( MaxCombo,					GetMaxCombo().m_cnt )
	DEFINE_METHOD( GetCurrentLife,				GetCurrentLife() )
	DEFINE_METHOD( GetGrade,					GetGrade() )
	DEFINE_METHOD( GetActualDancePoints,		m_iActualDancePoints )
	DEFINE_METHOD( GetPossibleDancePoints,		m_iPossibleDancePoints )
	DEFINE_METHOD( GetCurrentPossibleDancePoints,		m_iCurPossibleDancePoints )
	DEFINE_METHOD( GetPercentDancePoints,		GetPercentDancePoints() )
	DEFINE_METHOD( GetLessonScoreActual,		GetLessonScoreActual() )
	DEFINE_METHOD( GetLessonScoreNeeded,		GetLessonScoreNeeded() )
	DEFINE_METHOD( GetPersonalHighScoreIndex,	m_iPersonalHighScoreIndex )
	DEFINE_METHOD( GetMachineHighScoreIndex,	m_iMachineHighScoreIndex )
	DEFINE_METHOD( GetStageAward,				m_StageAward )
	DEFINE_METHOD( GetPeakComboAward,			m_PeakComboAward )
	DEFINE_METHOD( IsDisqualified,				IsDisqualified() )
	DEFINE_METHOD( GetAliveSeconds,				m_fAliveSeconds )
	DEFINE_METHOD( GetPercentageOfTaps,			GetPercentageOfTaps( Enum::Check<TapNoteScore>(L, 1) ) )
	DEFINE_METHOD( GetBestFullComboTapNoteScore, GetBestFullComboTapNoteScore() )
	DEFINE_METHOD( GetFailed, 					m_bFailed )
	DEFINE_METHOD( GetSongsPassed, 					m_iSongsPassed )
	DEFINE_METHOD( GetSongsPlayed, 					m_iSongsPlayed )

	static int GetHighScore( T* p, lua_State *L )
	{
		p->m_HighScore.PushSelf(L);
		return 1;
	}

	static int GetPlayedSteps( T* p, lua_State *L )
	{
		lua_newtable(L);
		for( int i = 0; i < (int) std::min(p->m_iStepsPlayed, (int) p->m_vpPossibleSteps.size()); ++i )
		{
			p->m_vpPossibleSteps[i]->PushSelf(L);
			lua_rawseti( L, -2, i+1 );
		}
		return 1;
	}
	static int GetPossibleSteps( T* p, lua_State *L )
	{
		lua_newtable(L);
		for( int i = 0; i < (int) p->m_vpPossibleSteps.size(); ++i )
		{
			p->m_vpPossibleSteps[i]->PushSelf(L);
			lua_rawseti( L, -2, i+1 );
		}
		return 1;
	}
	static int GetComboList( T* p, lua_State *L )
	{
		lua_createtable(L, p->m_ComboList.size(), 0);
		for( std::size_t i= 0; i < p->m_ComboList.size(); ++i)
		{
			lua_createtable(L, 0, 6);
			lua_pushstring(L, "StartSecond");
			lua_pushnumber(L, p->m_ComboList[i].m_fStartSecond);
			lua_rawset(L, -3);
			lua_pushstring(L, "SizeSeconds");
			lua_pushnumber(L, p->m_ComboList[i].m_fSizeSeconds);
			lua_rawset(L, -3);
			lua_pushstring(L, "Count");
			lua_pushnumber(L, p->m_ComboList[i].m_cnt);
			lua_rawset(L, -3);
			lua_pushstring(L, "Rollover");
			lua_pushnumber(L, p->m_ComboList[i].m_rollover);
			lua_rawset(L, -3);
			lua_pushstring(L, "StageCount");
			lua_pushnumber(L, p->m_ComboList[i].GetStageCnt());
			lua_rawset(L, -3);
			lua_pushstring(L, "IsZero");
			lua_pushnumber(L, p->m_ComboList[i].IsZero());
			lua_rawset(L, -3);
			lua_rawseti(L, -2, i+1);
		}
		return 1;
	}
	static int GetLifeRecord( T* p, lua_State *L )
	{
		float last_second= FArg(1);
		int samples= 100;
		if (lua_gettop(L) >= 2 && !lua_isnil(L,2))
		{
			samples= IArg(2);
			if(samples <= 0)
			{
				LOG->Trace("PlayerStageStats:GetLifeRecord requires an integer greater than 0.  Defaulting to 100.");
				samples= 100;
			}
		}
		lua_createtable(L, samples, 0);
		for(int i= 0; i < samples; ++i)
		{
			// The scale from range is [0, samples-1] because that is i's range.
			float from= SCALE(i, 0, (float)samples-1.0f, 0.0f, last_second);
			float curr= p->GetLifeRecordLerpAt(from);
			lua_pushnumber(L, curr);
			lua_rawseti(L, -2, i+1);
		}
		return 1;
	}

	static int GetRadarPossible( T* p, lua_State *L ) { p->m_radarPossible.PushSelf(L); return 1; }
	static int GetRadarActual( T* p, lua_State *L ) { p->m_radarActual.PushSelf(L); return 1; }
	static int SetScore( T* p, lua_State *L )
	{
		if( IArg(1) >= 0 )
		{
			p->m_iScore = IArg(1);
			return 1;
		}
		COMMON_RETURN_SELF;
	}
	static int SetCurMaxScore( T* p, lua_State *L )
	{
		if( IArg(1) >= 0 )
		{
			p->m_iCurMaxScore = IArg(1);
			return 1;
		}
		COMMON_RETURN_SELF;
	}
	static int SetDancePointLimits( T* p, lua_State *L )
	{
		int actual = IArg(1);
		int possible = IArg(2);
		if( actual >= 0 && possible > 0 )
		{
			p->m_iPossibleDancePoints = possible;
			if( actual <= possible )
			{
				p->m_iActualDancePoints = actual;
			}
			else
			{
				p->m_iActualDancePoints = possible;
			}
			return 1;
		}
		COMMON_RETURN_SELF;
	}

	static int FailPlayer( T* p, lua_State *L )
	{
		p->m_bFailed = true;
		COMMON_RETURN_SELF;
	}

	LunaPlayerStageStats()
	{
		ADD_METHOD( GetCaloriesBurned );
		ADD_METHOD( GetNumControllerSteps );
		ADD_METHOD( GetLifeRemainingSeconds );
		ADD_METHOD( GetSurvivalSeconds );
		ADD_METHOD( GetCurrentCombo );
		ADD_METHOD( GetCurrentMissCombo );
		ADD_METHOD( GetCurrentScoreMultiplier );
		ADD_METHOD( GetScore );
		ADD_METHOD( GetCurMaxScore );
		ADD_METHOD( GetTapNoteScores );
		ADD_METHOD( GetHoldNoteScores );
		ADD_METHOD( FullCombo );
		ADD_METHOD( FullComboOfScore );
		ADD_METHOD( MaxCombo );
		ADD_METHOD( GetCurrentLife );
		ADD_METHOD( GetGrade );
		ADD_METHOD( GetHighScore );
		ADD_METHOD( GetActualDancePoints );
		ADD_METHOD( GetPossibleDancePoints );
		ADD_METHOD( GetCurrentPossibleDancePoints );
		ADD_METHOD( GetPercentDancePoints );
		ADD_METHOD( GetLessonScoreActual );
		ADD_METHOD( GetLessonScoreNeeded );
		ADD_METHOD( GetPersonalHighScoreIndex );
		ADD_METHOD( GetMachineHighScoreIndex );
		ADD_METHOD( GetStageAward );
		ADD_METHOD( GetPeakComboAward );
		ADD_METHOD( IsDisqualified );
		ADD_METHOD( GetPlayedSteps );
		ADD_METHOD( GetPossibleSteps );
		ADD_METHOD( GetComboList );
		ADD_METHOD( GetLifeRecord );
		ADD_METHOD( GetAliveSeconds );
		ADD_METHOD( GetPercentageOfTaps );
		ADD_METHOD( GetRadarActual );
		ADD_METHOD( GetRadarPossible );
		ADD_METHOD( GetBestFullComboTapNoteScore );
		ADD_METHOD( GetFailed );
		ADD_METHOD( SetScore );
		ADD_METHOD( GetCurMaxScore );
		ADD_METHOD( SetCurMaxScore );
		ADD_METHOD( SetDancePointLimits );
		ADD_METHOD( FailPlayer );
		ADD_METHOD( GetSongsPassed );
		ADD_METHOD( GetSongsPlayed );
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

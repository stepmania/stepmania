#include "global.h"
#include "LifeMeterBar.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "RageTimer.h"
#include "GameState.h"
#include "RageMath.h"
#include "ThemeManager.h"
#include "Song.h"
#include "StatsManager.h"
#include "ThemeMetric.h"
#include "PlayerState.h"
#include "Quad.h"
#include "ActorUtil.h"
#include "StreamDisplay.h"
#include "Steps.h"
#include "Course.h"

#include <cstddef>

static RString LIFE_PERCENT_CHANGE_NAME( std::size_t i )   { return "LifePercentChange" + ScoreEventToString( (ScoreEvent)i ); }

LifeMeterBar::LifeMeterBar()
{
	DANGER_THRESHOLD.Load	("LifeMeterBar","DangerThreshold");
	INITIAL_VALUE.Load	("LifeMeterBar","InitialValue");
	HOT_VALUE.Load		("LifeMeterBar","HotValue");
	LIFE_MULTIPLIER.Load	( "LifeMeterBar","LifeMultiplier");
	MIN_STAY_ALIVE.Load	("LifeMeterBar","MinStayAlive");
	FORCE_LIFE_DIFFICULTY_ON_EXTRA_STAGE.Load ("LifeMeterBar","ForceLifeDifficultyOnExtraStage");
	EXTRA_STAGE_LIFE_DIFFICULTY.Load	("LifeMeterBar","ExtraStageLifeDifficulty");
	m_fLifePercentChange.Load( "LifeMeterBar", LIFE_PERCENT_CHANGE_NAME, NUM_ScoreEvent );

	m_pPlayerState = nullptr;

	const RString sType = "LifeMeterBar";

	m_fPassingAlpha = 0;
	m_fHotAlpha = 0;

	m_fBaseLifeDifficulty = PREFSMAN->m_fLifeDifficultyScale;
	m_fLifeDifficulty = m_fBaseLifeDifficulty;

	// set up progressive lifebar
	m_iProgressiveLifebar = PREFSMAN->m_iProgressiveLifebar;
	m_iMissCombo = 0;

	// set up combotoregainlife
	m_iComboToRegainLife = 0;

	bool bExtra = GAMESTATE->IsAnExtraStage();
	RString sExtra = bExtra ? "extra " : "";

	m_sprUnder.Load( THEME->GetPathG(sType,sExtra+"Under") );
	m_sprUnder->SetName( "Under" );
	ActorUtil::LoadAllCommandsAndSetXY( m_sprUnder, sType );
	this->AddChild( m_sprUnder );

	m_sprDanger.Load( THEME->GetPathG(sType,sExtra+"Danger") );
	m_sprDanger->SetName( "Danger" );
	ActorUtil::LoadAllCommandsAndSetXY( m_sprDanger, sType );
	this->AddChild( m_sprDanger );

	m_pStream = new StreamDisplay;
	m_pStream->Load( bExtra ? "StreamDisplayExtra" : "StreamDisplay" );
	m_pStream->SetName( "Stream" );
	ActorUtil::LoadAllCommandsAndSetXY( m_pStream, sType );
	this->AddChild( m_pStream );

	m_sprOver.Load( THEME->GetPathG(sType,sExtra+"Over") );
	m_sprOver->SetName( "Over" );
	ActorUtil::LoadAllCommandsAndSetXY( m_sprOver, sType );
	this->AddChild( m_sprOver );
}

LifeMeterBar::~LifeMeterBar()
{
	SAFE_DELETE( m_pStream );
}

void LifeMeterBar::Load( const PlayerState *pPlayerState, PlayerStageStats *pPlayerStageStats )
{
	LifeMeter::Load( pPlayerState, pPlayerStageStats );

	PlayerNumber pn = pPlayerState->m_PlayerNumber;

	DrainType dtype = pPlayerState->m_PlayerOptions.GetStage().m_DrainType;
	switch( dtype )
	{
		case DrainType_Normal:
			m_fLifePercentage = INITIAL_VALUE;
			break;
			/* These types only go down, so they always start at full. */
		case DrainType_NoRecover:
		case DrainType_SuddenDeath:
			m_fLifePercentage = 1.0f;	break;
		default:
			FAIL_M(ssprintf("Invalid DrainType: %i", dtype));
	}

	// Change life difficulty to really easy if merciful beginner on
	m_bMercifulBeginnerInEffect =
		GAMESTATE->m_PlayMode == PLAY_MODE_REGULAR  &&
		GAMESTATE->IsPlayerEnabled( pPlayerState )  &&
		GAMESTATE->m_pCurSteps[pn]->GetDifficulty() == Difficulty_Beginner  &&
		PREFSMAN->m_bMercifulBeginner;

	AfterLifeChanged();
}

void LifeMeterBar::ChangeLife( TapNoteScore score )
{
	float fDeltaLife=0.f;
	switch( score )
	{
	DEFAULT_FAIL( score );
	case TNS_W1:		fDeltaLife = m_fLifePercentChange.GetValue(SE_W1);	break;
	case TNS_W2:		fDeltaLife = m_fLifePercentChange.GetValue(SE_W2);	break;
	case TNS_W3:		fDeltaLife = m_fLifePercentChange.GetValue(SE_W3);	break;
	case TNS_W4:		fDeltaLife = m_fLifePercentChange.GetValue(SE_W4);	break;
	case TNS_W5:		fDeltaLife = m_fLifePercentChange.GetValue(SE_W5);	break;
	case TNS_Miss:		fDeltaLife = m_fLifePercentChange.GetValue(SE_Miss);	break;
	case TNS_HitMine:	fDeltaLife = m_fLifePercentChange.GetValue(SE_HitMine);	break;
	case TNS_None:		fDeltaLife = m_fLifePercentChange.GetValue(SE_Miss);	break;
	case TNS_CheckpointHit:	fDeltaLife = m_fLifePercentChange.GetValue(SE_CheckpointHit);	break;
	case TNS_CheckpointMiss:fDeltaLife = m_fLifePercentChange.GetValue(SE_CheckpointMiss);	break;
	}

	// this was previously if( IsHot()  &&  score < TNS_GOOD ) in 3.9... -freem
	if(PREFSMAN->m_HarshHotLifePenalty && IsHot()  &&  fDeltaLife < 0)
		fDeltaLife = std::min( fDeltaLife, -0.10f );		// make it take a while to get back to "hot"

	switch(m_pPlayerState->m_PlayerOptions.GetSong().m_DrainType)
	{
	DEFAULT_FAIL(m_pPlayerState->m_PlayerOptions.GetSong().m_DrainType);
	case DrainType_Normal:
		break;
	case DrainType_NoRecover:
		fDeltaLife = std::min( fDeltaLife, 0.0f );
		break;
	case DrainType_SuddenDeath:
		if( score < MIN_STAY_ALIVE )
			fDeltaLife = -1.0f;
		else
			fDeltaLife = 0;
		break;
	}

	ChangeLife( fDeltaLife );
}

void LifeMeterBar::ChangeLife( HoldNoteScore score, TapNoteScore tscore )
{
	float fDeltaLife=0.f;
	DrainType dtype = m_pPlayerState->m_PlayerOptions.GetSong().m_DrainType;
	switch( dtype )
	{
	case DrainType_Normal:
		switch( score )
		{
		case HNS_Held:		fDeltaLife = m_fLifePercentChange.GetValue(SE_Held);	break;
		case HNS_LetGo:	fDeltaLife = m_fLifePercentChange.GetValue(SE_LetGo);	break;
		case HNS_Missed:	fDeltaLife = m_fLifePercentChange.GetValue(SE_Missed);	break;
		default:
			FAIL_M(ssprintf("Invalid HoldNoteScore: %i", score));
		}
		if(PREFSMAN->m_HarshHotLifePenalty && IsHot()  &&  score == HNS_LetGo)
			fDeltaLife = -0.10f;		// make it take a while to get back to "hot"
		break;
	case DrainType_NoRecover:
		switch( score )
		{
		case HNS_Held:		fDeltaLife = +0.000f;	break;
		case HNS_LetGo:	fDeltaLife = m_fLifePercentChange.GetValue(SE_LetGo);	break;
		case HNS_Missed:		fDeltaLife = +0.000f;	break;
		default:
			FAIL_M(ssprintf("Invalid HoldNoteScore: %i", score));
		}
		break;
	case DrainType_SuddenDeath:
		switch( score )
		{
		case HNS_Held:		fDeltaLife = +0;	break;
		case HNS_LetGo:	fDeltaLife = -1.0f;	break;
		case HNS_Missed:	fDeltaLife = +0;	break;
		default:
			FAIL_M(ssprintf("Invalid HoldNoteScore: %i", score));
		}
		break;
	default:
		FAIL_M(ssprintf("Invalid DrainType: %i", dtype));
	}

	ChangeLife( fDeltaLife );
}

void LifeMeterBar::ChangeLife( float fDeltaLife )
{
	bool bUseMercifulDrain = m_bMercifulBeginnerInEffect || PREFSMAN->m_bMercifulDrain;
	if( bUseMercifulDrain  &&  fDeltaLife < 0 )
		fDeltaLife *= SCALE( m_fLifePercentage, 0.f, 1.f, 0.5f, 1.f);

	// handle progressiveness and ComboToRegainLife here
	if( fDeltaLife >= 0 )
	{
		m_iMissCombo = 0;
		m_iComboToRegainLife = std::max( m_iComboToRegainLife-1, 0 );
		if ( m_iComboToRegainLife > 0 )
			fDeltaLife = 0.0f;
	}
	else
	{
		fDeltaLife *= 1 + (float)m_iProgressiveLifebar/8 * m_iMissCombo;
		// do this after; only successive W5/miss will increase the amount of life lost.
		m_iMissCombo++;
		/* Increase by m_iRegenComboAfterMiss; never push it beyond m_iMaxRegenComboAfterMiss
		 * but don't reduce it if it's already past. */
		const int NewComboToRegainLife = std::min(
			 (int)PREFSMAN->m_iMaxRegenComboAfterMiss,
			 m_iComboToRegainLife + PREFSMAN->m_iRegenComboAfterMiss );

		m_iComboToRegainLife = std::max( m_iComboToRegainLife, NewComboToRegainLife );
	}

	// If we've already failed, there's no point in letting them fill up the bar again.
	if( m_pPlayerStageStats->m_bFailed )
		return;

	switch(m_pPlayerState->m_PlayerOptions.GetSong().m_DrainType)
	{
		case DrainType_Normal:
		case DrainType_NoRecover:
			if( fDeltaLife > 0 )
				fDeltaLife *= m_fLifeDifficulty;
			else
				fDeltaLife /= m_fLifeDifficulty;
			break;
		case DrainType_SuddenDeath:
			// This should always -1.0f;
			if( fDeltaLife < 0 )
				fDeltaLife = -1.0f;
			else
				fDeltaLife = 0;
			break;
		default:
		break;
	}

	m_fLifePercentage += fDeltaLife;
	CLAMP( m_fLifePercentage, 0, LIFE_MULTIPLIER );
	AfterLifeChanged();
}

void LifeMeterBar::SetLife(float value)
{
	m_fLifePercentage= value;
	CLAMP( m_fLifePercentage, 0, LIFE_MULTIPLIER );
	AfterLifeChanged();
}

extern ThemeMetric<bool> PENALIZE_TAP_SCORE_NONE;
void LifeMeterBar::HandleTapScoreNone()
{
	if( PENALIZE_TAP_SCORE_NONE )
		ChangeLife( TNS_None );
}

void LifeMeterBar::AfterLifeChanged()
{
	m_pStream->SetPercent( m_fLifePercentage );

	Message msg( "LifeChanged" );
	msg.SetParam( "Player", m_pPlayerState->m_PlayerNumber );
	msg.SetParam( "LifeMeter", LuaReference::CreateFromPush(*this) );
	MESSAGEMAN->Broadcast( msg );
}

bool LifeMeterBar::IsHot() const
{
	return m_fLifePercentage >= HOT_VALUE;
}

bool LifeMeterBar::IsInDanger() const
{
	return m_fLifePercentage < DANGER_THRESHOLD;
}

bool LifeMeterBar::IsFailing() const
{
	return m_fLifePercentage <= m_pPlayerState->m_PlayerOptions.GetCurrent().m_fPassmark;
}


void LifeMeterBar::Update( float fDeltaTime )
{
	LifeMeter::Update( fDeltaTime );

	m_fPassingAlpha += !IsFailing() ? +fDeltaTime*2 : -fDeltaTime*2;
	CLAMP( m_fPassingAlpha, 0, 1 );

	m_fHotAlpha  += IsHot() ? + fDeltaTime*2 : -fDeltaTime*2;
	CLAMP( m_fHotAlpha, 0, 1 );

	m_pStream->SetPassingAlpha( m_fPassingAlpha );
	m_pStream->SetHotAlpha( m_fHotAlpha );

	if( m_pPlayerState->m_HealthState == HealthState_Danger )
		m_sprDanger->SetVisible( true );
	else
		m_sprDanger->SetVisible( false );
}


void LifeMeterBar::UpdateNonstopLifebar()
{
	int iCleared, iTotal, iProgressiveLifebarDifficulty;

	switch( GAMESTATE->m_PlayMode )
	{
	case PLAY_MODE_REGULAR:
		if( GAMESTATE->IsEventMode() || GAMESTATE->m_bDemonstrationOrJukebox )
			return;

		iCleared = GAMESTATE->m_iCurrentStageIndex;
		iTotal = PREFSMAN->m_iSongsPerPlay;
		iProgressiveLifebarDifficulty = PREFSMAN->m_iProgressiveStageLifebar;
		break;
	case PLAY_MODE_NONSTOP:
		iCleared = GAMESTATE->GetCourseSongIndex();
		iTotal = GAMESTATE->m_pCurCourse->GetEstimatedNumStages();
		iProgressiveLifebarDifficulty = PREFSMAN->m_iProgressiveNonstopLifebar;
		break;
	default:
		return;
	}

//	if (iCleared > iTotal) iCleared = iTotal; // clear/iTotal <= 1
//	if (iTotal == 0) iTotal = 1;  // no division by 0

	if( GAMESTATE->IsAnExtraStage() && FORCE_LIFE_DIFFICULTY_ON_EXTRA_STAGE )
	{
		// extra stage is its own thing, should not be progressive
		// and it should be as difficult as life 4
		// (e.g. it should not depend on life settings)

		m_iProgressiveLifebar = 0;
		m_fLifeDifficulty = EXTRA_STAGE_LIFE_DIFFICULTY;
		return;
	}

	if( iTotal > 1 )
		m_fLifeDifficulty = m_fBaseLifeDifficulty - 0.2f * (int)(iProgressiveLifebarDifficulty * iCleared / (iTotal - 1));
	else
		m_fLifeDifficulty = m_fBaseLifeDifficulty - 0.2f * iProgressiveLifebarDifficulty;

	if( m_fLifeDifficulty >= 0.4f )
		return;

	/* Approximate deductions for a miss
	 * Life 1 :    5   %
	 * Life 2 :    5.7 %
	 * Life 3 :    6.6 %
	 * Life 4 :    8   %
	 * Life 5 :   10   %
	 * Life 6 :   13.3 %
	 * Life 7 :   20   %
	 * Life 8 :   26.6 %
	 * Life 9 :   32   %
	 * Life 10:   40   %
	 * Life 11:   50   %
	 * Life 12:   57.1 %
	 * Life 13:   66.6 %
	 * Life 14:   80   %
	 * Life 15:  100   %
	 * Life 16+: 200   %
	 *
	 * Note there is 200%, because boos take off 1/2 as much as
	 * a miss, and a W5 would suck up half of your lifebar.
	 *
	 * Everything past 7 is intended mainly for nonstop mode.
	 */

	// the lifebar is pretty harsh at 0.4 already (you lose
	// about 20% of your lifebar); at 0.2 it would be 40%, which
	// is too harsh at one difficulty level higher.  Override.
	int iLifeDifficulty = int( (1.8f - m_fLifeDifficulty)/0.2f );

	// first eight values don't matter
	float fDifficultyValues[16] = {0,0,0,0,0,0,0,0,
		0.3f, 0.25f, 0.2f, 0.16f, 0.14f, 0.12f, 0.10f, 0.08f};

	if( iLifeDifficulty >= 16 )
	{
		// judge 16 or higher
		m_fLifeDifficulty = 0.04f;
		return;
	}

	m_fLifeDifficulty = fDifficultyValues[iLifeDifficulty];
	return;
}

void LifeMeterBar::FillForHowToPlay( int NumW2s, int NumMisses )
{
	m_iProgressiveLifebar = 0;  // disable progressive lifebar

	float AmountForW2	= NumW2s * m_fLifeDifficulty * 0.008f;
	float AmountForMiss	= NumMisses / m_fLifeDifficulty * 0.08f;

	m_fLifePercentage = AmountForMiss - AmountForW2;
	CLAMP( m_fLifePercentage, 0.0f, 1.0f );
	AfterLifeChanged();
}

/*
 * (c) 2001-2004 Chris Danford
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

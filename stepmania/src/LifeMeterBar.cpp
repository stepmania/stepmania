#include "global.h"
#include "LifeMeterBar.h"
#include "PrefsManager.h"
#include "RageException.h"
#include "RageTimer.h"
#include "GameState.h"
#include "RageMath.h"
#include "ThemeManager.h"
#include "song.h"
#include "StatsManager.h"
#include "ThemeMetric.h"
#include "PlayerState.h"
#include "Quad.h"
#include "ActorUtil.h"
#include "StreamDisplay.h"


static ThemeMetric<float> METER_WIDTH		("LifeMeterBar","MeterWidth");
static ThemeMetric<float> METER_HEIGHT		("LifeMeterBar","MeterHeight");
static ThemeMetric<float> DANGER_THRESHOLD	("LifeMeterBar","DangerThreshold");
static ThemeMetric<int> NUM_CHAMBERS		("LifeMeterBar","NumChambers");
static ThemeMetric<int> NUM_STRIPS			("LifeMeterBar","NumStrips");
static ThemeMetric<float> INITIAL_VALUE		("LifeMeterBar","InitialValue");

const float FAIL_THRESHOLD = 0;


LifeMeterBar::LifeMeterBar()
{
	switch( GAMESTATE->m_SongOptions.m_DrainType )
	{
	case SongOptions::DRAIN_NORMAL:
		m_fLifePercentage = INITIAL_VALUE;
		break;

	/* These types only go down, so they always start at full. */
	case SongOptions::DRAIN_NO_RECOVER:
	case SongOptions::DRAIN_SUDDEN_DEATH:
		m_fLifePercentage = 1.0f;	break;
	default:	ASSERT(0);
	}

	const CString sType = "LifeMeterBar";

	m_fPassingAlpha = 0;
	m_fHotAlpha = 0;
	m_bFailedEarlier = false;

	m_fBaseLifeDifficulty = PREFSMAN->m_fLifeDifficultyScale;
	m_fLifeDifficulty = m_fBaseLifeDifficulty;

	// set up progressive lifebar
	m_iProgressiveLifebar = PREFSMAN->m_iProgressiveLifebar;
	m_iMissCombo = 0;

	// set up combotoregainlife
	m_iComboToRegainLife = 0;

	m_sprBackground.Load( THEME->GetPathG(sType,"background") );
	m_sprBackground->SetName( "Background" );
	m_sprBackground->ZoomToWidth( METER_WIDTH );
	m_sprBackground->ZoomToHeight( METER_HEIGHT );
	this->AddChild( m_sprBackground );

	m_quadDangerGlow.ZoomToWidth( METER_WIDTH );
	m_quadDangerGlow.ZoomToHeight( METER_HEIGHT );
	m_quadDangerGlow.SetEffectDiffuseShift();
	m_quadDangerGlow.SetEffectColor1( RageColor(1,0,0,0.8f) );
	m_quadDangerGlow.SetEffectColor2( RageColor(1,0,0,0) );
	m_quadDangerGlow.SetEffectClock( Actor::CLOCK_BGM_BEAT );
	this->AddChild( &m_quadDangerGlow );

	m_pStream = new StreamDisplay;
	bool bExtra = GAMESTATE->IsExtraStage()||GAMESTATE->IsExtraStage2();
	CString sExtra = bExtra ? "extra " : "";
	m_pStream->Load(
		METER_WIDTH,
		METER_HEIGHT,
		NUM_STRIPS,
		NUM_CHAMBERS,		
		THEME->GetPathG(sType,sExtra+"normal"),
		THEME->GetPathG(sType,sExtra+"hot"),
		THEME->GetPathG(sType,sExtra+"passing"),
		THEME->GetMetricA(sType,"StreamNormalOnCommand"),
		THEME->GetMetricA(sType,"StreamHotOnCommand"),
		THEME->GetMetricA(sType,"StreamPassingOnCommand")
		);
	this->AddChild( m_pStream );

	m_sprFrame.Load( THEME->GetPathG(sType,sExtra+"frame") );
	m_sprFrame->SetName( "Frame" );
	this->AddChild( m_sprFrame );

	AfterLifeChanged();
}

LifeMeterBar::~LifeMeterBar()
{
	SAFE_DELETE( m_pStream );
}

void LifeMeterBar::Load( PlayerNumber pn )
{
	LifeMeter::Load( pn );
}

void LifeMeterBar::ChangeLife( TapNoteScore score )
{
	float fDeltaLife=0.f;
	switch( GAMESTATE->m_SongOptions.m_DrainType )
	{
	case SongOptions::DRAIN_NORMAL:
		switch( score )
		{
		case TNS_MARVELOUS:	fDeltaLife = PREFSMAN->m_fLifeDeltaPercentChangeMarvelous;	break;
		case TNS_PERFECT:	fDeltaLife = PREFSMAN->m_fLifeDeltaPercentChangePerfect;	break;
		case TNS_GREAT:		fDeltaLife = PREFSMAN->m_fLifeDeltaPercentChangeGreat;		break;
		case TNS_GOOD:		fDeltaLife = PREFSMAN->m_fLifeDeltaPercentChangeGood;		break;
		case TNS_BOO:		fDeltaLife = PREFSMAN->m_fLifeDeltaPercentChangeBoo;		break;
		case TNS_MISS:		fDeltaLife = PREFSMAN->m_fLifeDeltaPercentChangeMiss;		break;
		case TNS_HIT_MINE:	fDeltaLife = PREFSMAN->m_fLifeDeltaPercentChangeHitMine;	break;
		default:
			ASSERT(0);
		}
		if( IsHot()  &&  score < TNS_GOOD )
			fDeltaLife = -0.10f;		// make it take a while to get back to "doing great"
		break;
	case SongOptions::DRAIN_NO_RECOVER:
		switch( score )
		{
		case TNS_MARVELOUS:	fDeltaLife = +0.000f;	break;
		case TNS_PERFECT:	fDeltaLife = +0.000f;	break;
		case TNS_GREAT:		fDeltaLife = +0.000f;	break;
		case TNS_GOOD:		fDeltaLife = +0.000f;	break;
		case TNS_BOO:		fDeltaLife = PREFSMAN->m_fLifeDeltaPercentChangeBoo;	break;
		case TNS_MISS:		fDeltaLife = PREFSMAN->m_fLifeDeltaPercentChangeMiss;	break;
		case TNS_HIT_MINE:	fDeltaLife = PREFSMAN->m_fLifeDeltaPercentChangeHitMine;	break;
		default:
			ASSERT(0);
		}
		break;
	case SongOptions::DRAIN_SUDDEN_DEATH:
		switch( score )
		{
		case TNS_MARVELOUS:	fDeltaLife = +0;	break;
		case TNS_PERFECT:	fDeltaLife = +0;	break;
		case TNS_GREAT:		fDeltaLife = +0;	break;
		case TNS_GOOD:		fDeltaLife = -1.0;	break;
		case TNS_BOO:		fDeltaLife = -1.0;	break;
		case TNS_MISS:		fDeltaLife = -1.0;	break;
		case TNS_HIT_MINE:	fDeltaLife = -1.0;	break;
		default:
			ASSERT(0);
		}
		break;
	default:
		ASSERT(0);
	}

	ChangeLife( fDeltaLife );
}

void LifeMeterBar::ChangeLife( HoldNoteScore score, TapNoteScore tscore )
{
	/* The initial tap note score (which we happen to have in have in
	 * tscore) has already been reported to the above function.  If the
	 * hold end result was an NG, count it as a miss; if the end result
	 * was an OK, count a perfect.  (Remember, this is just life meter
	 * computation, not scoring.) */
	float fDeltaLife=0.f;
	switch( GAMESTATE->m_SongOptions.m_DrainType )
	{
	case SongOptions::DRAIN_NORMAL:
		switch( score )
		{
		case HNS_OK:	fDeltaLife = PREFSMAN->m_fLifeDeltaPercentChangeOK;	break;
		case HNS_NG:	fDeltaLife = PREFSMAN->m_fLifeDeltaPercentChangeNG;	break;
		default:
			ASSERT(0);
		}
		if( IsHot()  &&  score == HNS_NG )
			fDeltaLife = -0.10f;		// make it take a while to get back to "doing great"
		break;
	case SongOptions::DRAIN_NO_RECOVER:
		switch( score )
		{
		case HNS_OK:	fDeltaLife = +0.000f;	break;
		case HNS_NG:	fDeltaLife = PREFSMAN->m_fLifeDeltaPercentChangeNG;	break;
		default:
			ASSERT(0);
		}
		break;
	case SongOptions::DRAIN_SUDDEN_DEATH:
		switch( score )
		{
		case HNS_OK:		fDeltaLife = +0;	break;
		case HNS_NG:		fDeltaLife = -1.0;	break;
		default:
			ASSERT(0);
		}
		break;
	default:
		ASSERT(0);
	}

	ChangeLife( fDeltaLife );
}

void LifeMeterBar::ChangeLife( float fDeltaLife )
{
	if( PREFSMAN->m_bMercifulDrain  &&  fDeltaLife < 0 )
		fDeltaLife *= SCALE( m_fLifePercentage, 0.f, 1.f, 0.5f, 1.f);

	// handle progressiveness and ComboToRegainLife here
	if( fDeltaLife >= 0 )
	{
		m_iMissCombo = 0;
		m_iComboToRegainLife = max( m_iComboToRegainLife-1, 0 );
		if ( m_iComboToRegainLife > 0 )
			fDeltaLife = 0.0f;
	}
	else
	{
		fDeltaLife *= 1 + (float)m_iProgressiveLifebar/8 * m_iMissCombo;
		// do this after; only successive boo/miss will
		// increase the amount of life lost.
		m_iMissCombo++;
		/* Increase by m_iRegenComboAfterMiss; never push it beyond m_iMaxRegenComboAfterMiss
		 * but don't reduce it if it's already past. */
		const int NewComboToRegainLife = min( 
			(int)PREFSMAN->m_iMaxRegenComboAfterMiss,
			m_iComboToRegainLife + PREFSMAN->m_iRegenComboAfterMiss );
		m_iComboToRegainLife = max( m_iComboToRegainLife, NewComboToRegainLife );
	}

	/* If we've already failed, there's no point in letting them fill up the bar again.  */
	if( STATSMAN->m_CurStageStats.m_player[m_PlayerNumber].bFailed )
		fDeltaLife = 0;

	switch( GAMESTATE->m_SongOptions.m_DrainType )
	{
	case SongOptions::DRAIN_NORMAL:
	case SongOptions::DRAIN_NO_RECOVER:
		if( fDeltaLife > 0 )
			fDeltaLife *= m_fLifeDifficulty;
		else
			fDeltaLife /= m_fLifeDifficulty;
		break;
	}

	// check if this step would cause a fail
	if( m_fLifePercentage + fDeltaLife <= FAIL_THRESHOLD  &&  m_fLifePercentage > FAIL_THRESHOLD )
	{
		/* Increase by m_iRegenComboAfterFail; never push it beyond m_iMaxRegenComboAfterFail
		 * but don't reduce it if it's already past. */
		const int NewComboToRegainLife = min( 
			(int)PREFSMAN->m_iMaxRegenComboAfterFail,
			m_iComboToRegainLife + PREFSMAN->m_iRegenComboAfterFail );
		m_iComboToRegainLife = max( m_iComboToRegainLife, NewComboToRegainLife );
	}
	
	m_fLifePercentage += fDeltaLife;
	CLAMP( m_fLifePercentage, 0, 1 );
	AfterLifeChanged();

	if( m_fLifePercentage <= FAIL_THRESHOLD )
		STATSMAN->m_CurStageStats.m_player[m_PlayerNumber].bFailedEarlier = true;
}

void LifeMeterBar::AfterLifeChanged()
{
	m_pStream->SetPercent( m_fLifePercentage );
}

bool LifeMeterBar::IsPastPassmark() const
{
    if( GAMESTATE->m_pPlayerState[m_PlayerNumber]->m_PlayerOptions.m_fPassmark > 0 )
    {
		return m_fLifePercentage >= GAMESTATE->m_pPlayerState[m_PlayerNumber]->m_PlayerOptions.m_fPassmark;
    }
    else
    {
		return false;
    }
}

bool LifeMeterBar::IsHot() const
{ 
	return m_fLifePercentage >= 1; 
}

bool LifeMeterBar::IsInDanger() const
{ 
	return m_fLifePercentage < DANGER_THRESHOLD; 
}

bool LifeMeterBar::IsFailing() const
{ 
	return m_fLifePercentage <= 0; 
}


void LifeMeterBar::Update( float fDeltaTime )
{
	LifeMeter::Update( fDeltaTime );
	
	m_fPassingAlpha += IsPastPassmark() ? +fDeltaTime*2 : -fDeltaTime*2;
	CLAMP( m_fPassingAlpha, 0, 1 );

	m_fHotAlpha  += IsHot() ? + fDeltaTime*2 : -fDeltaTime*2;
	CLAMP( m_fHotAlpha, 0, 1 );

	m_pStream->SetPassingAlpha( m_fPassingAlpha );
	m_pStream->SetHotAlpha( m_fHotAlpha );

	if( m_pStream->GetTrailingLifePercent() < DANGER_THRESHOLD && !GAMESTATE->IsPlayerDead(m_PlayerNumber) )
		m_quadDangerGlow.SetDiffuseAlpha( 1 );
	else
		m_quadDangerGlow.SetDiffuseAlpha( 0 );
}


void LifeMeterBar::DrawPrimitives()
{
	ActorFrame::DrawPrimitives();
}
#include "RageLog.h"
void LifeMeterBar::UpdateNonstopLifebar(const int cleared, 
		const int total, int ProgressiveLifebarDifficulty)
{
//	if (cleared > total) cleared = total; // clear/total <= 1
//	if (total == 0) total = 1;  // no division by 0

	if (GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2())
	{   // extra stage is its own thing, should not be progressive
	    // and it should be as difficult as life 4
		// (e.g. it should not depend on life settings)

		m_iProgressiveLifebar = 0;
		m_fLifeDifficulty = 1.0f;
		return;
	}

	// should be checked before calling function, but in case
	// it isn't, do so here
	/* No, wait: if we're playing nonstop, event mode just means that we can play another
	 * nonstop course later, so it shouldn't affect life difficulty. */
/*	if (GAMESTATE->GetEventMode())
	{
		m_fLifeDifficulty = m_fBaseLifeDifficulty;
		return;
	} */

	if (total > 1)
		m_fLifeDifficulty = m_fBaseLifeDifficulty - 0.2f * (int)(ProgressiveLifebarDifficulty * cleared / (total - 1));
	else
		m_fLifeDifficulty = m_fBaseLifeDifficulty - 0.2f * ProgressiveLifebarDifficulty;

	if (m_fLifeDifficulty >= 0.4) return;

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
	 * a miss, and a boo would suck up half of your lifebar.
	 *
	 * Everything past 7 is intended mainly for nonstop mode.
     */


	// the lifebar is pretty harsh at 0.4 already (you lose
	// about 20% of your lifebar); at 0.2 it would be 40%, which
	// is too harsh at one difficulty level higher.  Override.

	int m_iLifeDifficulty = int((1.8f - m_fLifeDifficulty)/0.2f);
	
	// first eight values don't matter
	float DifficultyValues[16] = {0,0,0,0,0,0,0,0, 
		0.3f, 0.25f, 0.2f, 0.16f, 0.14f, 0.12f, 0.10f, 0.08f};

	if (m_iLifeDifficulty >= 16)
	{
		// judge 16 or higher
		m_fLifeDifficulty = 0.04f;
		return;
	}

	m_fLifeDifficulty = DifficultyValues[m_iLifeDifficulty];
	return;
}

void LifeMeterBar::FillForHowToPlay(int NumPerfects, int NumMisses)
{
	m_iProgressiveLifebar = 0;  // disable progressive lifebar

	float AmountForPerfect	= NumPerfects * m_fLifeDifficulty * 0.008f;
	float AmountForMiss		= NumMisses / m_fLifeDifficulty * 0.08f;

	m_fLifePercentage = AmountForMiss - AmountForPerfect;
	CLAMP( m_fLifePercentage, 0.0f, 1.0f );
	AfterLifeChanged();
}

void LifeMeterBar::ForceFail()
{
	m_fLifePercentage = 0;
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

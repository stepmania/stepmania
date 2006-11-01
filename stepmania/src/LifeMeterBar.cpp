#include "global.h"
#include "LifeMeterBar.h"
#include "PrefsManager.h"
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
#include "Steps.h"


static void LifePercentChangeInit( size_t /*ScoreEvent*/ i, RString &sNameOut, float &defaultValueOut )
{
	sNameOut = "LifePercentChange" + ScoreEventToString( (ScoreEvent)i );
	switch( i )
	{
	DEFAULT_FAIL( int(i) );
	case SE_W1:		defaultValueOut = +0.008f;	break;
	case SE_W2:		defaultValueOut = +0.008f;	break;
	case SE_W3:		defaultValueOut = +0.004f;	break;
	case SE_W4:		defaultValueOut = +0.000f;	break;
	case SE_W5:		defaultValueOut = -0.040f;	break;
	case SE_Miss:		defaultValueOut = -0.080f;	break;
	case SE_HitMine:	defaultValueOut = -0.160f;	break;
	case SE_Held:		defaultValueOut = +0.008f;	break;
	case SE_LetGo:		defaultValueOut = -0.080f;	break;
	}
}

static Preference1D<float> g_fLifePercentChange( LifePercentChangeInit, NUM_ScoreEvent );

static ThemeMetric<float> METER_WIDTH		("LifeMeterBar","MeterWidth");
static ThemeMetric<float> METER_HEIGHT		("LifeMeterBar","MeterHeight");
static ThemeMetric<float> DANGER_THRESHOLD	("LifeMeterBar","DangerThreshold");
static ThemeMetric<int>   NUM_CHAMBERS		("LifeMeterBar","NumChambers");
static ThemeMetric<int>   NUM_STRIPS		("LifeMeterBar","NumStrips");
static ThemeMetric<float> INITIAL_VALUE		("LifeMeterBar","InitialValue");
static ThemeMetric<TapNoteScore>   MIN_STAY_ALIVE	("LifeMeterBar","MinStayAlive");

const float FAIL_THRESHOLD = 0;


LifeMeterBar::LifeMeterBar()
{
	switch( GAMESTATE->m_SongOptions.GetStage().m_DrainType )
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

	const RString sType = "LifeMeterBar";

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
	bool bExtra = GAMESTATE->IsAnExtraStage();
	RString sExtra = bExtra ? "extra " : "";
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

void LifeMeterBar::Load( const PlayerState *pPlayerState, PlayerStageStats *pPlayerStageStats )
{
	LifeMeter::Load( pPlayerState, pPlayerStageStats );

	PlayerNumber pn = pPlayerState->m_PlayerNumber;

	// Change life difficulty to really easy if merciful beginner on
	bool bMercifulBeginnerInEffect = 
		GAMESTATE->m_PlayMode == PLAY_MODE_REGULAR  &&  
		GAMESTATE->IsPlayerEnabled( pPlayerState )  &&
		GAMESTATE->m_pCurSteps[pn]->GetDifficulty() == DIFFICULTY_BEGINNER  &&
		PREFSMAN->m_bMercifulBeginner;
	if( bMercifulBeginnerInEffect )
	{
		m_fBaseLifeDifficulty = 5.0f;
		m_fLifeDifficulty = m_fBaseLifeDifficulty;
	}
}

void LifeMeterBar::ChangeLife( TapNoteScore score )
{
	float fDeltaLife=0.f;
	switch( score )
	{
	DEFAULT_FAIL( score );
	case TNS_W1:		fDeltaLife = g_fLifePercentChange[SE_W1];	break;
	case TNS_W2:		fDeltaLife = g_fLifePercentChange[SE_W2];	break;
	case TNS_W3:		fDeltaLife = g_fLifePercentChange[SE_W3];	break;
	case TNS_W4:		fDeltaLife = g_fLifePercentChange[SE_W4];	break;
	case TNS_W5:		fDeltaLife = g_fLifePercentChange[SE_W5];	break;
	case TNS_Miss:		fDeltaLife = g_fLifePercentChange[SE_Miss];	break;
	case TNS_HitMine:	fDeltaLife = g_fLifePercentChange[SE_HitMine];	break;
	}
	if( IsHot()  &&  score < TNS_W4 )
		fDeltaLife = -0.10f;		// make it take a while to get back to "hot"

	switch( GAMESTATE->m_SongOptions.GetSong().m_DrainType )
	{
	DEFAULT_FAIL( GAMESTATE->m_SongOptions.GetSong().m_DrainType );
	case SongOptions::DRAIN_NORMAL:
		break;
	case SongOptions::DRAIN_NO_RECOVER:
		fDeltaLife = min( fDeltaLife, 0 );
		break;
	case SongOptions::DRAIN_SUDDEN_DEATH:
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
	switch( GAMESTATE->m_SongOptions.GetSong().m_DrainType )
	{
	case SongOptions::DRAIN_NORMAL:
		switch( score )
		{
		case HNS_Held:	fDeltaLife = g_fLifePercentChange[SE_Held];	break;
		case HNS_LetGo:	fDeltaLife = g_fLifePercentChange[SE_LetGo];	break;
		default:
			ASSERT(0);
		}
		if( IsHot()  &&  score == HNS_LetGo )
			fDeltaLife = -0.10f;		// make it take a while to get back to "hot"
		break;
	case SongOptions::DRAIN_NO_RECOVER:
		switch( score )
		{
		case HNS_Held:	fDeltaLife = +0.000f;	break;
		case HNS_LetGo:	fDeltaLife = g_fLifePercentChange[SE_LetGo];	break;
		default:
			ASSERT(0);
		}
		break;
	case SongOptions::DRAIN_SUDDEN_DEATH:
		switch( score )
		{
		case HNS_Held:		fDeltaLife = +0;	break;
		case HNS_LetGo:		fDeltaLife = -1.0;	break;
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
		// do this after; only successive W5/miss will
		// increase the amount of life lost.
		m_iMissCombo++;
		/* Increase by m_iRegenComboAfterMiss; never push it beyond m_iMaxRegenComboAfterMiss
		 * but don't reduce it if it's already past. */
		int iNewComboToRegainLife = m_iComboToRegainLife + PREFSMAN->m_iRegenComboAfterMiss;
		iNewComboToRegainLife = min( (int)PREFSMAN->m_iMaxRegenComboAfterMiss, iNewComboToRegainLife );
		m_iComboToRegainLife = max( m_iComboToRegainLife, iNewComboToRegainLife );
	}

	/* If we've already failed, there's no point in letting them fill up the bar again.  */
	if( m_pPlayerStageStats->bFailed )
		fDeltaLife = 0;

	switch( GAMESTATE->m_SongOptions.GetSong().m_DrainType )
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
		int iNewComboToRegainLife = m_iComboToRegainLife + PREFSMAN->m_iRegenComboAfterFail;
		iNewComboToRegainLife = min( (int)PREFSMAN->m_iMaxRegenComboAfterFail, iNewComboToRegainLife );
		m_iComboToRegainLife = max( m_iComboToRegainLife, iNewComboToRegainLife );
	}
	
	m_fLifePercentage += fDeltaLife;
	CLAMP( m_fLifePercentage, 0, 1 );
	AfterLifeChanged();

	if( m_fLifePercentage <= FAIL_THRESHOLD )
		m_pPlayerStageStats->bFailedEarlier = true;
}

void LifeMeterBar::AfterLifeChanged()
{
	m_pStream->SetPercent( m_fLifePercentage );
}

bool LifeMeterBar::IsPastPassmark() const
{
	if( m_pPlayerState->m_PlayerOptions.GetSong().m_fPassmark > 0 )
		return m_fLifePercentage >= m_pPlayerState->m_PlayerOptions.GetSong().m_fPassmark;
	else
		return false;
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

	if( m_fLifePercentage < DANGER_THRESHOLD && !GAMESTATE->IsPlayerDead(m_pPlayerState) )
		m_quadDangerGlow.SetDiffuseAlpha( 1 );
	else
		m_quadDangerGlow.SetDiffuseAlpha( 0 );
}


void LifeMeterBar::DrawPrimitives()
{
	ActorFrame::DrawPrimitives();
}
#include "RageLog.h"
void LifeMeterBar::UpdateNonstopLifebar( const int cleared, const int total, int ProgressiveLifebarDifficulty )
{
//	if (cleared > total) cleared = total; // clear/total <= 1
//	if (total == 0) total = 1;  // no division by 0

	if( GAMESTATE->IsAnExtraStage() )
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

	if( total > 1 )
		m_fLifeDifficulty = m_fBaseLifeDifficulty - 0.2f * (int)(ProgressiveLifebarDifficulty * cleared / (total - 1));
	else
		m_fLifeDifficulty = m_fBaseLifeDifficulty - 0.2f * ProgressiveLifebarDifficulty;

	if( m_fLifeDifficulty >= 0.4f )
		return;

	/*
	 * Approximate deductions for a miss
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

	int m_iLifeDifficulty = int( (1.8f - m_fLifeDifficulty)/0.2f );
	
	// first eight values don't matter
	float DifficultyValues[16] = {0,0,0,0,0,0,0,0, 
		0.3f, 0.25f, 0.2f, 0.16f, 0.14f, 0.12f, 0.10f, 0.08f};

	if( m_iLifeDifficulty >= 16 )
	{
		// judge 16 or higher
		m_fLifeDifficulty = 0.04f;
		return;
	}

	m_fLifeDifficulty = DifficultyValues[m_iLifeDifficulty];
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

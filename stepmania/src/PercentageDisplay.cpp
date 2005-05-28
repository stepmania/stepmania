#include "global.h"

#include "PercentageDisplay.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "PrefsManager.h"
#include "ActorUtil.h"
#include "RageLog.h"
#include "StageStats.h"
#include "PlayerState.h"
#include "LuaFunctions.h"

ThemeMetric<int> PERCENT_DECIMAL_PLACES	( "PercentageDisplay", "PercentDecimalPlaces" );
ThemeMetric<int> PERCENT_TOTAL_SIZE		( "PercentageDisplay", "PercentTotalSize" );

PercentageDisplay::PercentageDisplay()
{
	m_pSource = NULL;
}

void PercentageDisplay::Load( PlayerNumber pn, PlayerStageStats* pSource, const CString &sMetricsGroup, bool bAutoRefresh )
{
	m_PlayerNumber = pn;
	m_pSource = pSource;
	m_bAutoRefresh = bAutoRefresh;
	m_Last = -1;
	m_LastMax = -1;

	DANCE_POINT_DIGITS.Load( sMetricsGroup, "DancePointsDigits" );
	PERCENT_USE_REMAINDER.Load( sMetricsGroup, "PercentUseRemainder" );
	APPLY_SCORE_DISPLAY_OPTIONS.Load( sMetricsGroup, "ApplyScoreDisplayOptions" );


	if( PREFSMAN->m_bDancePointsForOni )
		m_textPercent.SetName( ssprintf("DancePointsP%i", pn+1) );
	else
		m_textPercent.SetName( ssprintf("PercentP%i", pn+1) );

	m_textPercent.LoadFromFont( THEME->GetPathF(sMetricsGroup,"text") );
	ActorUtil::SetXYAndOnCommand( m_textPercent, sMetricsGroup );
	ASSERT( m_textPercent.HasCommand("Off") );
	this->AddChild( &m_textPercent );

	if( !PREFSMAN->m_bDancePointsForOni && (bool)PERCENT_USE_REMAINDER )
	{
		m_textPercentRemainder.SetName( ssprintf("PercentRemainderP%d",pn+1) );
		m_textPercentRemainder.LoadFromFont( THEME->GetPathF(sMetricsGroup,"remainder") );
		ActorUtil::SetXYAndOnCommand( m_textPercentRemainder, sMetricsGroup );
		ASSERT( m_textPercentRemainder.HasCommand("Off") );
		m_textPercentRemainder.SetText( "456" );
		this->AddChild( &m_textPercentRemainder );
	}

	Refresh();
}

void PercentageDisplay::TweenOffScreen()
{
	m_textPercent.PlayCommand( "Off" );
	if( !PREFSMAN->m_bDancePointsForOni && (bool)PERCENT_USE_REMAINDER )
		m_textPercentRemainder.PlayCommand( "Off" );
}

void PercentageDisplay::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	if( m_bAutoRefresh )
		Refresh();
}

void PercentageDisplay::Refresh()
{
	const int iActualDancePoints = m_pSource->iActualDancePoints;
	const int iCurPossibleDancePoints = m_pSource->iCurPossibleDancePoints;

	if( iActualDancePoints == m_Last && iCurPossibleDancePoints == m_LastMax )
		return;

	m_Last = iActualDancePoints;
	m_LastMax = iCurPossibleDancePoints;

	CString sNumToDisplay;

	if( PREFSMAN->m_bDancePointsForOni )
	{
		sNumToDisplay = ssprintf( "%*d", (int) DANCE_POINT_DIGITS, max( 0, iActualDancePoints ) );
	}
	else
	{
		float fPercentDancePoints = m_pSource->GetPercentDancePoints();
		float fCurMaxPercentDancePoints = m_pSource->GetCurMaxPercentDancePoints();
		
		if ( APPLY_SCORE_DISPLAY_OPTIONS )
		{
			switch( GAMESTATE->m_pPlayerState[m_PlayerNumber]->m_CurrentPlayerOptions.m_ScoreDisplay )
			{
			case PlayerOptions::SCORING_ADD:
				// nothing to do
				break;
			case PlayerOptions::SCORING_SUBTRACT:
				fPercentDancePoints = 1.0f - ( fCurMaxPercentDancePoints - fPercentDancePoints );
				break;
			case PlayerOptions::SCORING_AVERAGE:
				if( fCurMaxPercentDancePoints == 0.0f ) // don't divide by zero fats
					fPercentDancePoints = 0.0f;
				else
					fPercentDancePoints = fPercentDancePoints / fCurMaxPercentDancePoints;
				break;
			}
		}

		// clamp percentage - feedback is that negative numbers look weird here.
		CLAMP( fPercentDancePoints, 0.f, 1.f );

		if( PERCENT_USE_REMAINDER )
		{
			int iPercentWhole = int(fPercentDancePoints*100);
			int iPercentRemainder = int( (fPercentDancePoints*100 - int(fPercentDancePoints*100)) * 10 );
			sNumToDisplay = ssprintf( "%2d", iPercentWhole );
			m_textPercentRemainder.SetText( ssprintf(".%01d%%", iPercentRemainder) );
		}
		else
		{		
			sNumToDisplay = FormatPercentScore( fPercentDancePoints );
			
			// HACK: Use the last frame in the numbers texture as '-'
			sNumToDisplay.Replace('-','x');
		}
	}

	m_textPercent.SetText( sNumToDisplay );
}

CString PercentageDisplay::FormatPercentScore( float fPercentDancePoints )
{
	// TRICKY: printf will round, but we want to truncate.  Otherwise, we may display a percent
	// score that's too high and doesn't match up with the calculated grade.
	float fTruncInterval = powf( 0.1f, (float)PERCENT_TOTAL_SIZE-1 );
	
	// TRICKY: ftruncf is rounding 1.0000000 to 0.99990004.  Give a little boost to 
	// fPercentDancePoints to correct for this.
	fPercentDancePoints += 0.000001f;

	fPercentDancePoints = ftruncf( fPercentDancePoints, fTruncInterval );
	
	CString s = ssprintf( "%*.*f%%", (int)PERCENT_TOTAL_SIZE, (int)PERCENT_DECIMAL_PLACES, fPercentDancePoints*100 );
	return s;
}

LuaFunction_Float( FormatPercentScore,	PercentageDisplay::FormatPercentScore(a1) )


/*
 * (c) 2001-2003 Chris Danford
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

#include "global.h"
#include "ScoreDisplayNormal.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "PrefsManager.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "PlayerState.h"
#include "StageStats.h"


const float SCORE_TWEEN_TIME = 0.2f;
const int NUM_SCORE_DIGITS	=	9;


ScoreDisplayNormal::ScoreDisplayNormal()
{
	LOG->Trace( "ScoreDisplayNormal::ScoreDisplayNormal()" );

	m_sprFrame.Load( THEME->GetPathToG("ScoreDisplayNormal frame") );
	this->AddChild( &m_sprFrame );

	// init the text
	m_text.LoadFromFont( THEME->GetPathToF("ScoreDisplayNormal") );
	m_text.SetShadowLength( 0 );

	m_iScore = 0;

	m_iTrailingScore = 0;
	m_iScoreVelocity = 0;

	CString s;
	for( int i=0; i<NUM_SCORE_DIGITS; i++ )
		s += ' ';
	m_text.SetText( s );
	this->AddChild( &m_text );
}

void ScoreDisplayNormal::Init( const PlayerState* pPlayerState ) 
{
	ScoreDisplay::Init( pPlayerState );

	// TODO: Remove use of PlayerNumber.
	PlayerNumber pn = pPlayerState->m_PlayerNumber;
	
	m_text.SetDiffuse( PlayerToColor(pn) );
}

void ScoreDisplayNormal::SetScore( int iNewScore ) 
{
	m_iScore = iNewScore;

	// play some games to display the correct score -- the actual internal score does not change at all
	// but the displayed one can (ie: displayed score for subtracrive is MaxScore - score)

	// TODO: Remove use of PlayerNumber.
	PlayerNumber pn = m_pPlayerState->m_PlayerNumber;


	if ( m_pPlayerState->m_CurrentPlayerOptions.m_fScoreDisplay == PlayerOptions::SCORING_SUBTRACT )
	{
		if ( iNewScore == 0 && g_CurStageStats.m_player[pn].iMaxScoreToNow == 0 )
		{
			m_iScore = g_CurStageStats.m_player[pn].iMaxScore;
		}
		else 
		{
			m_iScore = g_CurStageStats.m_player[pn].iMaxScore - ( g_CurStageStats.m_player[pn].iMaxScoreToNow - iNewScore );
		}
	}
	else if ( m_pPlayerState->m_CurrentPlayerOptions.m_fScoreDisplay == PlayerOptions::SCORING_AVERAGE )
	{
		if ( g_CurStageStats.m_player[pn].iMaxScoreToNow == 0 ) // don't divide by zero fats
			m_iScore = 0;
		else
		{
			float scoreRatio = ( (float) iNewScore / (float) g_CurStageStats.m_player[pn].iMaxScoreToNow );
			m_iScore = (int) ( scoreRatio * g_CurStageStats.m_player[pn].iMaxScore );
		}
	}
	
	int iDelta = m_iScore - m_iTrailingScore;

	m_iScoreVelocity = int(float(iDelta) / SCORE_TWEEN_TIME);	// in score units per second
}

void ScoreDisplayNormal::SetText( CString s ) 
{ 
	m_text.SetText( s );
}

void ScoreDisplayNormal::Update( float fDeltaTime )
{
	ScoreDisplay::Update( fDeltaTime );

	if( m_iTrailingScore != m_iScore )
	{
		// adjust for when player fails
		int increment = int(m_iScoreVelocity * fDeltaTime);
		if (m_iScoreVelocity != 0 && increment == 0) increment = 1;

		int iDeltaBefore = m_iScore - m_iTrailingScore;
		m_iTrailingScore += increment;
		int iDeltaAfter = m_iScore - m_iTrailingScore;

		if( (iDeltaBefore < 0 && iDeltaAfter > 0) ||
			(iDeltaBefore > 0 && iDeltaAfter < 0) )	// the sign changed
		{
			m_iTrailingScore = m_iScore;
			m_iScoreVelocity = 0;
		}

		m_text.SetText( ssprintf("%*.0i", NUM_SCORE_DIGITS, m_iTrailingScore) );
	}
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

#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScoreDisplayNormal.h

 Desc: A graphic displayed in the ScoreDisplayNormal during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScoreDisplayNormal.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "GameState.h"
#include "ThemeManager.h"


const float SCORE_TWEEN_TIME = 0.2f;
const int NUM_SCORE_DIGITS	=	9;


ScoreDisplayNormal::ScoreDisplayNormal()
{
	LOG->Trace( "ScoreDisplayNormal::ScoreDisplayNormal()" );

	// init the text
	m_text.LoadFromNumbers( THEME->GetPathTo("Numbers","gameplay score numbers") );
	m_text.TurnShadowOff();

	m_fScore = 0;
	m_fTrailingScore = 0;
	m_fScoreVelocity = 0;

	CString s;
	for( int i=0; i<NUM_SCORE_DIGITS; i++ )
		s += ' ';
	m_text.SetText( s );
	this->AddChild( &m_text );
}

void ScoreDisplayNormal::Init( PlayerNumber pn ) 
{
	ScoreDisplay::Init( pn );
	m_text.SetDiffuse( PlayerToColor(pn) );
}

void ScoreDisplayNormal::SetScore( float fNewScore ) 
{ 
	m_fScore = fNewScore;

	float fDelta = (float)m_fScore - m_fTrailingScore;

	m_fScoreVelocity = fDelta / SCORE_TWEEN_TIME;	// in score units per second
}

void ScoreDisplayNormal::SetText( CString s ) 
{ 
	m_text.SetText( s );
}

void ScoreDisplayNormal::Update( float fDeltaTime )
{
	ScoreDisplay::Update( fDeltaTime );

	if( m_fTrailingScore != m_fScore )
	{
		float fDeltaBefore = (float)m_fScore - m_fTrailingScore;
		m_fTrailingScore += m_fScoreVelocity * fDeltaTime;
		float fDeltaAfter = (float)m_fScore - m_fTrailingScore;

		if( fDeltaBefore * fDeltaAfter < 0 )	// the sign changed
		{
			m_fTrailingScore = (float)m_fScore;
			m_fScoreVelocity = 0;
		}

		m_text.SetText( ssprintf("%*.0f", NUM_SCORE_DIGITS, m_fTrailingScore) );
	}
}


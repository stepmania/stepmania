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
#include "GameState.h"
#include "ThemeManager.h"


const float SCORE_TWEEN_TIME = 0.2f;
const int NUM_SCORE_DIGITS	=	9;


ScoreDisplayNormal::ScoreDisplayNormal()
{
	LOG->Trace( "ScoreDisplayNormal::ScoreDisplayNormal()" );

	m_sprFrame.Load( THEME->GetPathToG("ScoreDisplayNormal frame") );
	this->AddChild( &m_sprFrame );

	// init the text
	m_text.LoadFromNumbers( THEME->GetPathToN("ScoreDisplayNormal") );
	m_text.EnableShadow( false );

	m_iScore = 0;
	m_iTrailingScore = 0;
	m_iScoreVelocity = 0;

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

void ScoreDisplayNormal::SetScore( int iNewScore ) 
{
	m_iScore = iNewScore;

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
		int iDeltaBefore = m_iScore - m_iTrailingScore;
		m_iTrailingScore += int(m_iScoreVelocity * fDeltaTime);
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


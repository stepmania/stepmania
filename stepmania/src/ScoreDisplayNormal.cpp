#include "stdafx.h"
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


const float SCORE_TWEEN_TIME = 0.2f;


ScoreDisplayNormal::ScoreDisplayNormal()
{
	LOG->Trace( "ScoreDisplayNormal::ScoreDisplayNormal()" );

	// init the text
	BitmapText::LoadFromNumbers( THEME->GetPathTo("Numbers","gameplay score numbers") );
	TurnShadowOff();

	m_fScore = 0;
	m_fTrailingScore = 0;
	m_fScoreVelocity = 0;

	CString s;
	for( int i=0; i<NUM_SCORE_DIGITS; i++ )
		s += ' ';
	SetText( s );
}


void ScoreDisplayNormal::Init( PlayerNumber pn )
{
	m_PlayerNumber = pn;
}

void ScoreDisplayNormal::SetScore( float fNewScore ) 
{ 
	m_fScore = fNewScore;

	float fDelta = (float)m_fScore - m_fTrailingScore;

	m_fScoreVelocity = fDelta / SCORE_TWEEN_TIME;	// in score units per second
}

void ScoreDisplayNormal::Update( float fDeltaTime )
{
	BitmapText::Update( fDeltaTime );

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

		SetText( ssprintf("%*f", NUM_SCORE_DIGITS, m_fTrailingScore) );
	}
}

void ScoreDisplayNormal::Draw()
{
	BitmapText::Draw();
}

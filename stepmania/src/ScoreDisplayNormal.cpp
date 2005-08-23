#include "global.h"
#include "ScoreDisplayNormal.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "PrefsManager.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "PlayerState.h"
#include "StatsManager.h"
#include "CommonMetrics.h"


ScoreDisplayNormal::ScoreDisplayNormal()
{
	LOG->Trace( "ScoreDisplayNormal::ScoreDisplayNormal()" );

	m_sprFrame.Load( THEME->GetPathG("ScoreDisplayNormal","frame") );
	this->AddChild( &m_sprFrame );

	// init the text
	m_text.LoadFromFont( THEME->GetPathF("ScoreDisplayNormal","numbers") );
	m_text.SetShadowLength( 0 );
	m_text.UpdateText();
	this->AddChild( &m_text );
}

void ScoreDisplayNormal::Init( const PlayerState* pPlayerState, const PlayerStageStats* pPlayerStageStats ) 
{
	ScoreDisplay::Init( pPlayerState, pPlayerStageStats );

	// TODO: Remove use of PlayerNumber.
	PlayerNumber pn = pPlayerState->m_PlayerNumber;
	
	m_text.SetDiffuse( PLAYER_COLOR.GetValue(pn) );
}

void ScoreDisplayNormal::SetScore( int iNewScore ) 
{
	float fScore = (float)iNewScore;

	// TODO: Remove use of PlayerNumber.
	PlayerNumber pn = m_pPlayerState->m_PlayerNumber;

	// Play some games to display the correct score -- the actual internal 
	// score does not change at all but the displayed one can (ie: displayed 
	// score for subtracrive is MaxScore - score).

	
	int iMaxScore = STATSMAN->m_CurStageStats.m_player[pn].iMaxScore;
	int iCurMaxScore = STATSMAN->m_CurStageStats.m_player[pn].iCurMaxScore;

	switch( m_pPlayerState->m_CurrentPlayerOptions.m_ScoreDisplay )
	{
	case PlayerOptions::SCORING_ADD:
		// nothing to do
		break;
	case PlayerOptions::SCORING_SUBTRACT:
		fScore = iMaxScore - ( iCurMaxScore - fScore );
		break;
	case PlayerOptions::SCORING_AVERAGE:
		if( iCurMaxScore == 0 ) // don't divide by zero fats
		{
			fScore = 0;
		}
		else
		{
			float fScoreRatio = fScore / (float)iCurMaxScore;
			fScore = fScoreRatio * iMaxScore;
		}
	}

	m_text.SetTargetNumber( fScore );
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

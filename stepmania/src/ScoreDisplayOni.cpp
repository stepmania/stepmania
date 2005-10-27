#include "global.h"
#include "ScoreDisplayOni.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "StatsManager.h"
#include "PlayerState.h"
#include "CommonMetrics.h"


ScoreDisplayOni::ScoreDisplayOni()
{
	LOG->Trace( "ScoreDisplayOni::ScoreDisplayOni()" );

	m_sprFrame.Load( THEME->GetPathG("ScoreDisplayOni","frame") );
	this->AddChild( &m_sprFrame );

	// init the text
	m_text.LoadFromFont( THEME->GetPathF("ScoreDisplayOni","numbers") );
	m_text.SetShadowLength( 0 );
	this->AddChild( &m_text );
}

void ScoreDisplayOni::Init( const PlayerState* pPlayerState, const PlayerStageStats* pPlayerStageStats ) 
{
	ScoreDisplay::Init( pPlayerState, pPlayerStageStats );

	// TODO: Remove use of PlayerNumber.
	PlayerNumber pn = pPlayerState->m_PlayerNumber;

	m_text.RunCommands( CommonMetrics::PLAYER_COLOR.GetValue(pn) );
}


void ScoreDisplayOni::Update( float fDelta )
{
	ScoreDisplay::Update( fDelta );

	// TODO: Remove use of PlayerNumber.
	PlayerNumber pn = m_pPlayerState->m_PlayerNumber;

	float fSecsIntoPlay = 0;
	if( GAMESTATE->IsPlayerEnabled(pn) )
		fSecsIntoPlay = STATSMAN->m_CurStageStats.m_player[pn].fAliveSeconds;

	m_text.SetText( SecondsToMMSSMsMs(fSecsIntoPlay) );
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

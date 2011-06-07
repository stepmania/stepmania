#include "global.h"
#include "ScoreDisplayNormal.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "PlayerState.h"
#include "StatsManager.h"
#include "CommonMetrics.h"
#include "ActorUtil.h"

ScoreDisplayNormal::ScoreDisplayNormal()
{
	LOG->Trace( "ScoreDisplayNormal::ScoreDisplayNormal()" );

	RString sType = "ScoreDisplayNormal";

	m_sprFrame.Load( THEME->GetPathG(sType,"Frame") );
	m_sprFrame->SetName( "Frame" );
	ActorUtil::LoadAllCommandsAndSetXY( m_sprFrame, sType );
	this->AddChild( m_sprFrame );

	// init the text
	m_text.LoadFromFont( THEME->GetPathF("ScoreDisplayNormal","Text") );
	m_text.Load( "RollingNumbers" );
	m_text.SetName( "Text" );
	m_text.UpdateText();
	ActorUtil::LoadAllCommandsAndSetXY( m_text, sType );
	this->AddChild( &m_text );
}

void ScoreDisplayNormal::Init( const PlayerState* pPlayerState, const PlayerStageStats* pPlayerStageStats ) 
{
	ScoreDisplay::Init( pPlayerState, pPlayerStageStats );

	PlayerState* pPlayerState_ = const_cast<PlayerState*>(pPlayerState);
	PlayerStageStats* pPlayerStageStats_ = const_cast<PlayerStageStats*>(pPlayerStageStats);

	Message msg("Load");
	msg.SetParam( "PlayerState", LuaReference::CreateFromPush(*pPlayerState_) );
	msg.SetParam( "PlayerStageStats", LuaReference::CreateFromPush(*pPlayerStageStats_) );
	this->HandleMessage( msg );
}

void ScoreDisplayNormal::SetScore( int iNewScore ) 
{
	float fScore = (float)iNewScore;

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

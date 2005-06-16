#include "global.h"
#include "ScoreDisplayAliveTime.h"
#include "RageUtil.h"
#include "StageStats.h"
#include "XmlFile.h"
#include "GameState.h"
#include "StatsManager.h"
#include "ActorUtil.h"

// lua start
LUA_REGISTER_CLASS( ScoreDisplayAliveTime )
// lua end
REGISTER_ACTOR_CLASS( ScoreDisplayAliveTime )


ScoreDisplayAliveTime::ScoreDisplayAliveTime()
{
	m_PlayerNumber = PLAYER_INVALID;
}

ScoreDisplayAliveTime::~ScoreDisplayAliveTime()
{
}

void ScoreDisplayAliveTime::LoadFromNode( const CString& sDir, const XNode* pNode )
{
	BitmapText::LoadFromNode( sDir, pNode );

	{
		CString sPlayerNumber;
		bool b = pNode->GetAttrValue( "PlayerNumber", sPlayerNumber );
		ASSERT( b );
		m_PlayerNumber = (PlayerNumber) LuaHelpers::RunExpressionI(sPlayerNumber);
	}
}

void ScoreDisplayAliveTime::Update( float fDelta )
{
	UpdateNumber();
	
	BitmapText::Update( fDelta );
}

void ScoreDisplayAliveTime::PlayCommand( const CString &sCommandName, Actor* pParent )
{
	// TODO: Add handling of GoalComplete message
	
	BitmapText::PlayCommand( sCommandName, pParent );
}

void ScoreDisplayAliveTime::UpdateNumber()
{
	float fSecsIntoPlay = 0;
	ASSERT( m_PlayerNumber != PLAYER_INVALID );
	if( GAMESTATE->IsPlayerEnabled(m_PlayerNumber) )
		fSecsIntoPlay = 
			STATSMAN->GetAccumStageStats().m_player[m_PlayerNumber].fAliveSeconds +
			STATSMAN->m_CurStageStats.m_player[m_PlayerNumber].fAliveSeconds;

	SetText( SecondsToMMSSMsMs(fSecsIntoPlay) );
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

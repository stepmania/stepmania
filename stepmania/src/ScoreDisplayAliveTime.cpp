#include "global.h"
#include "ScoreDisplayAliveTime.h"
#include "RageUtil.h"
#include "StageStats.h"
#include "XmlFile.h"
#include "GameState.h"
#include "StatsManager.h"
#include "ActorUtil.h"
#include "LuaManager.h"

REGISTER_ACTOR_CLASS( ScoreDisplayAliveTime )


ScoreDisplayAliveTime::ScoreDisplayAliveTime()
{
	m_PlayerNumber = PLAYER_INVALID;
}

ScoreDisplayAliveTime::~ScoreDisplayAliveTime()
{
}

void ScoreDisplayAliveTime::LoadFromNode( const XNode* pNode )
{
	BitmapText::LoadFromNode( pNode );

	{
		Lua *L = LUA->Get();
		bool b = pNode->PushAttrValue( L, "PlayerNumber" );
		ASSERT( b );
		LuaHelpers::Pop( L, m_PlayerNumber );
		LUA->Release( L );
	}
}

void ScoreDisplayAliveTime::Update( float fDelta )
{
	UpdateNumber();
	
	BitmapText::Update( fDelta );
}

void ScoreDisplayAliveTime::PlayCommand( const RString &sCommandName )
{
	// TODO: Add handling of GoalComplete message
	
	BitmapText::PlayCommand( sCommandName );
}

void ScoreDisplayAliveTime::UpdateNumber()
{
	float fSecsIntoPlay = 0;
	ASSERT( m_PlayerNumber != PLAYER_INVALID );
	if( GAMESTATE->IsPlayerEnabled(m_PlayerNumber) )
		fSecsIntoPlay = 
			STATSMAN->GetAccumPlayedStageStats().m_player[m_PlayerNumber].m_fAliveSeconds +
			STATSMAN->m_CurStageStats.m_player[m_PlayerNumber].m_fAliveSeconds;

	SetText( SecondsToMMSSMsMs(fSecsIntoPlay) );
}

// lua start
#include "LuaBinding.h"

class LunaScoreDisplayAliveTime: public Luna<ScoreDisplayAliveTime>
{
public:
	LunaScoreDisplayAliveTime()
	{
	}
};

LUA_REGISTER_DERIVED_CLASS( ScoreDisplayAliveTime, BitmapText )

// lua end

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

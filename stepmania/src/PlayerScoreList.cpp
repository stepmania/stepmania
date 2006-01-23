#include "global.h"
#include "PlayerScoreList.h"
#include "GameCommand.h"
#include "PlayerState.h"


//
// PlayerScoreItem
//
void PlayerScoreItem::Init( const PlayerState *pPlayerState, const PlayerStageStats *pPlayerStageStats )
{
	m_sprFrame.Load( THEME->GetPathG("PlayerScoreItem","Frame") );
	this->AddChild( m_sprFrame );

	m_score.Load( pPlayerState, pPlayerStageStats, "PlayerScoreItem Percent", true );
	this->AddChild( &m_score );
}


//
// PlayerScoreList
//
PlayerScoreList::PlayerScoreList()
{
	m_timerRefreshCountdown.Touch();
}

static void PositionItem( LuaExpression &expr, Actor *pSelf, int iItemIndex, int iNumItems )
{
	Lua *L = LUA->Get();
	expr.PushSelf( L );
	ASSERT( !lua_isnil(L, -1) );
	pSelf->PushSelf( L );
	LuaHelpers::Push( iItemIndex, L );
	LuaHelpers::Push( iNumItems, L );
	lua_call( L, 3, 0 ); // 3 args, 0 results
	LUA->Release(L);
}

void PlayerScoreList::Init( vector<const PlayerState*> vpPlayerState, vector<const PlayerStageStats*> vpPlayerStageStats )
{
	ASSERT( !vpPlayerState.empty() );
	ASSERT( !vpPlayerStageStats.empty() );

	m_vpPlayerState = vpPlayerState;
	m_vpPlayerStageStats = vpPlayerStageStats;

	m_vsprBullet.resize( vpPlayerState.size() );
	m_vScoreItem.resize( vpPlayerState.size() );

	// 
	{
		LuaExpression expr;
		expr.SetFromExpression( THEME->GetMetric("PlayerScoreList","ItemPositionCommandFunction") );
		Actor a;
		FOREACH( const PlayerState*, m_vpPlayerState, iter )
		{
			int i = iter - m_vpPlayerState.begin();

			PositionItem( expr, &a, i, vpPlayerState.size() );
			m_vtsPositions.push_back( a.DestTweenState() );
		}
	}

	GameCommand gc;
	FOREACH( const PlayerState*, m_vpPlayerState, iter )
	{
		int i = iter - m_vpPlayerState.begin();

		gc.m_iIndex = i;
		gc.m_MultiPlayer = (*iter)->m_mp;

		Lua *L = LUA->Get();
		gc.PushSelf( L );
		lua_setglobal( L, "ThisGameCommand" );
		LUA->Release( L );

		m_vsprBullet[i].Load( THEME->GetPathG("PlayerScoreList","Bullet") );
		m_vsprBullet[i]->DestTweenState() = m_vtsPositions[i];
		this->AddChild( m_vsprBullet[i] );

		m_vScoreItem[i].Init( vpPlayerState[i], vpPlayerStageStats[i] );
		this->AddChild( &m_vScoreItem[i] );

		LUA->UnsetGlobal( "ThisGameCommand" );
	}
}

void PlayerScoreList::Update( float fDelta ) 
{
	ActorFrame::Update( fDelta ); 

	bool bTimeToRefresh = m_timerRefreshCountdown.PeekDeltaTime() > 0.5f;
	if( bTimeToRefresh )
		m_timerRefreshCountdown.Touch();

	if( bTimeToRefresh )
		Refresh();
}

struct EnabledPlayerIndexAndScore
{
	int iEnabledPlayerIndex;
	float fScore;

	bool operator<( const EnabledPlayerIndexAndScore &other ) const { return fScore > other.fScore; }
};

void PlayerScoreList::Refresh()
{
	vector<EnabledPlayerIndexAndScore> v;

	FOREACH( const PlayerStageStats*, m_vpPlayerStageStats, iter )
	{
		int i = iter - m_vpPlayerStageStats.begin();
		EnabledPlayerIndexAndScore t = { i, (*iter)->GetPercentDancePoints() };
		v.push_back( t );
	}

	stable_sort( v.begin(), v.end() );

	FOREACH( EnabledPlayerIndexAndScore, v, iter )
	{
		int i = iter - v.begin();
		PlayerScoreItem &psi = m_vScoreItem[iter->iEnabledPlayerIndex];
		psi.SetDrawOrder( -i );
		psi.BeginTweening( 0.5f, TWEEN_DECELERATE );
		psi.DestTweenState() = m_vtsPositions[i];
	}

	this->SortByDrawOrder();
}

/*
 * (c) 2004 Chris Danford
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

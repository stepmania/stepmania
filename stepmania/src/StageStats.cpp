#include "global.h"
#include "StageStats.h"
#include "GameState.h"
#include "Foreach.h"
#include "Steps.h"
#include "song.h"

StageStats::StageStats()
{
	playMode = PLAY_MODE_INVALID;
	pStyle = NULL;
	vpSongs.clear();
	StageType = STAGE_INVALID;
	fGameplaySeconds = 0;
}

void StageStats::Init()
{
	*this = StageStats();
}

void StageStats::AssertValid( PlayerNumber pn ) const
{
	ASSERT( vpSongs.size() != 0 );
	if( vpSongs[0] )
		CHECKPOINT_M( vpSongs[0]->GetFullTranslitTitle() );
	ASSERT( m_player[pn].vpSteps.size() != 0 );
	ASSERT( m_player[pn].vpSteps[0] );
	ASSERT_M( playMode < NUM_PLAY_MODES, ssprintf("playmode %i", playMode) );
	ASSERT( pStyle != NULL );
	ASSERT_M( m_player[pn].vpSteps[0]->GetDifficulty() < NUM_DIFFICULTIES, ssprintf("difficulty %i", m_player[pn].vpSteps[0]->GetDifficulty()) );
	ASSERT( vpSongs.size() == m_player[pn].vpSteps.size() );
}


int StageStats::GetAverageMeter( PlayerNumber pn ) const
{
	int iTotalMeter = 0;
	ASSERT( vpSongs.size() == m_player[pn].vpSteps.size() );

	for( unsigned i=0; i<vpSongs.size(); i++ )
	{
		const Steps* pSteps = m_player[pn].vpSteps[i];
		iTotalMeter += pSteps->GetMeter();
	}
	return iTotalMeter / vpSongs.size();	// round down
}

void StageStats::AddStats( const StageStats& other )
{
	ASSERT( !other.vpSongs.empty() );
	FOREACH_CONST( Song*, other.vpSongs, s )
		vpSongs.push_back( *s );
	StageType = STAGE_INVALID; // meaningless
	
	fGameplaySeconds += other.fGameplaySeconds;

	FOREACH_PlayerNumber( p )
	{
		m_player[p].AddStats( other.m_player[p] );
	}
}

bool StageStats::OnePassed() const
{
	FOREACH_PlayerNumber( p )
		if( GAMESTATE->IsHumanPlayer(p) && !m_player[p].bFailed )
			return true;
	return false;
}

bool StageStats::AllFailed() const
{
	FOREACH_EnabledPlayer( pn )
		if( !m_player[pn].bFailed )
			return false;
	return true;
}

bool StageStats::AllFailedEarlier() const
{
	FOREACH_EnabledPlayer( p )
		if( !m_player[p].bFailedEarlier )
			return false;
	return true;
}


// lua start
#include "LuaBinding.h"

template<class T>
class LunaStageStats : public Luna<T>
{
public:
	LunaStageStats() { LUA->Register( Register ); }

	static int GetPlayerStageStats( T* p, lua_State *L )	{ p->m_player[IArg(1)].PushSelf(L); return 1; }
	static int GetGameplaySeconds( T* p, lua_State *L )		{ lua_pushnumber(L, p->fGameplaySeconds); return 1; }

	static void Register(lua_State *L)
	{
		ADD_METHOD( GetPlayerStageStats )
		ADD_METHOD( GetGameplaySeconds )
		Luna<T>::Register( L );
	}
};

LUA_REGISTER_CLASS( StageStats )
// lua end


/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
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

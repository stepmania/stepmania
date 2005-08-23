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
	vpPlayedSongs.clear();
	vpPossibleSongs.clear();
	StageType = STAGE_INVALID;
	fGameplaySeconds = 0;
	fStepsSeconds = 0;
}

void StageStats::Init()
{
	*this = StageStats();
}

void StageStats::AssertValid( PlayerNumber pn ) const
{
	ASSERT( vpPlayedSongs.size() != 0 );
	ASSERT( vpPossibleSongs.size() != 0 );
	if( vpPlayedSongs[0] )
		CHECKPOINT_M( vpPlayedSongs[0]->GetTranslitFullTitle() );
	ASSERT( m_player[pn].vpPlayedSteps.size() != 0 );
	ASSERT( m_player[pn].vpPlayedSteps[0] );
	ASSERT_M( playMode < NUM_PLAY_MODES, ssprintf("playmode %i", playMode) );
	ASSERT( pStyle != NULL );
	ASSERT_M( m_player[pn].vpPlayedSteps[0]->GetDifficulty() < NUM_DIFFICULTIES, ssprintf("difficulty %i", m_player[pn].vpPlayedSteps[0]->GetDifficulty()) );
	ASSERT( vpPlayedSongs.size() == m_player[pn].vpPlayedSteps.size() );
	ASSERT( vpPossibleSongs.size() == m_player[pn].vpPossibleSteps.size() );
}

void StageStats::AssertValid( MultiPlayer pn ) const
{
	ASSERT( vpPlayedSongs.size() != 0 );
	ASSERT( vpPossibleSongs.size() != 0 );
	if( vpPlayedSongs[0] )
		CHECKPOINT_M( vpPlayedSongs[0]->GetTranslitFullTitle() );
	ASSERT( m_multiPlayer[pn].vpPlayedSteps.size() != 0 );
	ASSERT( m_multiPlayer[pn].vpPlayedSteps[0] );
	ASSERT_M( playMode < NUM_PLAY_MODES, ssprintf("playmode %i", playMode) );
	ASSERT( pStyle != NULL );
	ASSERT_M( m_player[pn].vpPlayedSteps[0]->GetDifficulty() < NUM_DIFFICULTIES, ssprintf("difficulty %i", m_player[pn].vpPlayedSteps[0]->GetDifficulty()) );
	ASSERT( vpPlayedSongs.size() == m_player[pn].vpPlayedSteps.size() );
	ASSERT( vpPossibleSongs.size() == m_player[pn].vpPossibleSteps.size() );
}


int StageStats::GetAverageMeter( PlayerNumber pn ) const
{
	AssertValid( pn );

	// TODO: This isn't correct for courses.
	
	int iTotalMeter = 0;

	for( unsigned i=0; i<vpPlayedSongs.size(); i++ )
	{
		const Steps* pSteps = m_player[pn].vpPlayedSteps[i];
		iTotalMeter += pSteps->GetMeter();
	}
	return iTotalMeter / vpPlayedSongs.size();	// round down
}

void StageStats::AddStats( const StageStats& other )
{
	ASSERT( !other.vpPlayedSongs.empty() );
	FOREACH_CONST( Song*, other.vpPlayedSongs, s )
		vpPlayedSongs.push_back( *s );
	FOREACH_CONST( Song*, other.vpPossibleSongs, s )
		vpPossibleSongs.push_back( *s );
	StageType = STAGE_INVALID; // meaningless
	
	fGameplaySeconds += other.fGameplaySeconds;
	fStepsSeconds += other.fStepsSeconds;

	FOREACH_EnabledMultiPlayer( p )
		m_player[p].AddStats( other.m_player[p] );
}

bool StageStats::OnePassed() const
{
	FOREACH_EnabledMultiPlayer( p )
		if( !m_player[p].bFailed )
			return true;
	return false;
}

bool StageStats::AllFailed() const
{
	FOREACH_EnabledMultiPlayer( pn )
		if( !m_player[pn].bFailed )
			return false;
	return true;
}

bool StageStats::AllFailedEarlier() const
{
	FOREACH_EnabledMultiPlayer( p )
		if( !m_player[p].bFailedEarlier )
			return false;
	return true;
}

float StageStats::GetTotalPossibleStepsSeconds() const
{
	float fSecs = 0;
	FOREACH_CONST( Song*, vpPossibleSongs, s )
		fSecs += (*s)->GetStepsSeconds();
	return fSecs;
}


// lua start
#include "LuaBinding.h"

class LunaStageStats: public Luna<StageStats>
{
public:
	LunaStageStats() { LUA->Register( Register ); }

	static int GetPlayerStageStats( T* p, lua_State *L )	{ p->m_player[IArg(1)].PushSelf(L); return 1; }
	static int GetGameplaySeconds( T* p, lua_State *L )		{ lua_pushnumber(L, p->fGameplaySeconds); return 1; }
	static int OnePassed( T* p, lua_State *L )				{ lua_pushboolean(L, p->OnePassed()); return 1; }
	static int AllFailed( T* p, lua_State *L )				{ lua_pushboolean(L, p->AllFailed()); return 1; }

	static void Register(lua_State *L)
	{
		ADD_METHOD( GetPlayerStageStats )
		ADD_METHOD( GetGameplaySeconds )
		ADD_METHOD( OnePassed )
		ADD_METHOD( AllFailed )
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

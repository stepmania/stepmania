#include "global.h"
#include "StageStats.h"
#include "GameState.h"
#include "Foreach.h"
#include "Steps.h"
#include "song.h"

StageStats	g_CurStageStats;
vector<StageStats>	g_vPlayedStageStats;

void StageStats::Init()
{
	playMode = PLAY_MODE_INVALID;
	pStyle = NULL;
	vpSongs.clear();
	StageType = STAGE_INVALID;
	fGameplaySeconds = 0;
}

void StageStats::AssertValid( PlayerNumber pn ) const
{
	if( vpSongs[0] )
		CHECKPOINT_M( vpSongs[0]->GetFullTranslitTitle() );
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

	static void Register(lua_State *L)
	{
		ADD_METHOD( GetPlayerStageStats )
		Luna<T>::Register( L );
	}
};

LUA_REGISTER_CLASS( StageStats )
// lua end


//
// Old Lua
//

static Grade GetBestGrade()
{
	Grade g = NUM_GRADES;
	FOREACH_EnabledPlayer( pn )
		g = min( g, g_CurStageStats.m_player[pn].GetGrade() );
	return g;
}

static Grade GetWorstGrade()
{
	Grade g = GRADE_TIER_1;
	FOREACH_EnabledPlayer( pn )
		g = max( g, g_CurStageStats.m_player[pn].GetGrade() );
	return g;
}

#include "LuaFunctions.h"
LuaFunction_NoArgs( GetStagesPlayed,		(int) g_vPlayedStageStats.size() );
LuaFunction_NoArgs( GetBestGrade,			GetBestGrade() );
LuaFunction_NoArgs( GetWorstGrade,			GetWorstGrade() );
LuaFunction_NoArgs( OnePassed,				g_CurStageStats.OnePassed() );
LuaFunction_NoArgs( AllFailed,				g_CurStageStats.AllFailed() );
LuaFunction_PlayerNumber( FullCombo,		g_CurStageStats.m_player[pn].FullCombo() )
LuaFunction_PlayerNumber( MaxCombo,			g_CurStageStats.m_player[pn].GetMaxCombo().cnt )
LuaFunction_PlayerNumber( GetGrade,			g_CurStageStats.m_player[pn].GetGrade() )
LuaFunction_Str( Grade,						StringToGrade(str) );

const StageStats *GetStageStatsN( int n )
{
	if( n == (int) g_vPlayedStageStats.size() )
		return &g_CurStageStats;
	if( n > (int) g_vPlayedStageStats.size() )
		return NULL;
	return &g_vPlayedStageStats[n];
}

/* GetGrade(0) returns the first grade; GetGrade(1) returns the second grade, and
 * and so on.  GetGrade(GetStagesPlayed()) returns the current grade (from g_CurStageStats).
 * If beyond the current song played, return GRADE_NO_DATA. */
Grade GetGrade( int n, PlayerNumber pn )
{
	const StageStats *pStats = GetStageStatsN( n );
	if( pStats == NULL )
		return GRADE_NO_DATA;
	return pStats->m_player[pn].GetGrade();
}

bool OneGotGrade( int n, Grade g )
{
	FOREACH_HumanPlayer( pn )
		if( GetGrade( n, pn ) == g )
			return true;

	return false;
}


LuaFunction_IntInt( OneGotGrade, OneGotGrade( a1, (Grade) a2 ) );

Grade GetFinalGrade( PlayerNumber pn )
{
	if( !GAMESTATE->IsHumanPlayer(pn) )
		return GRADE_NO_DATA;
	StageStats stats;
	GAMESTATE->GetFinalEvalStats( stats );
	return stats.m_player[pn].GetGrade();
}
LuaFunction_PlayerNumber( GetFinalGrade, GetFinalGrade(pn) );

Grade GetBestFinalGrade()
{
	Grade top_grade = GRADE_FAILED;
	StageStats stats;
	GAMESTATE->GetFinalEvalStats( stats );
	FOREACH_HumanPlayer( p )
		top_grade = min( top_grade, stats.m_player[p].GetGrade() );
	return top_grade;
}
LuaFunction_NoArgs( GetBestFinalGrade, GetBestFinalGrade() );

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

#include "global.h"
#include "StatsManager.h"
#include "GameState.h"

StatsManager*	STATSMAN = NULL;	// global object accessable from anywhere in the program


StatsManager::StatsManager()
{
}

// lua start
#include "LuaBinding.h"

template<class T>
class LunaStatsManager : public Luna<T>
{
public:
	LunaStatsManager() { LUA->Register( Register ); }

	static int GetCurStageStats( T* p, lua_State *L )	{ p->m_CurStageStats.PushSelf(L); return 1; }

	static void Register(lua_State *L)
	{
		ADD_METHOD( GetCurStageStats )
		Luna<T>::Register( L );

		// Add global singleton if constructed already.  If it's not constructed yet,
		// then we'll register it later when we reinit Lua just before 
		// initializing the display.
		if( STATSMAN )
		{
			lua_pushstring(L, "STATSMAN");
			STATSMAN->PushSelf( LUA->L );
			lua_settable(L, LUA_GLOBALSINDEX);
		}
	}
};

LUA_REGISTER_CLASS( StatsManager )
// lua end


//
// Old Lua
//

static Grade GetBestGrade()
{
	Grade g = NUM_GRADES;
	FOREACH_EnabledPlayer( pn )
		g = min( g, STATSMAN->m_CurStageStats.m_player[pn].GetGrade() );
	return g;
}

static Grade GetWorstGrade()
{
	Grade g = GRADE_TIER_1;
	FOREACH_EnabledPlayer( pn )
		g = max( g, STATSMAN->m_CurStageStats.m_player[pn].GetGrade() );
	return g;
}

#include "LuaFunctions.h"
LuaFunction_NoArgs( GetStagesPlayed,		(int) STATSMAN->m_vPlayedStageStats.size() );
LuaFunction_NoArgs( GetBestGrade,			GetBestGrade() );
LuaFunction_NoArgs( GetWorstGrade,			GetWorstGrade() );
LuaFunction_NoArgs( OnePassed,				STATSMAN->m_CurStageStats.OnePassed() );
LuaFunction_NoArgs( AllFailed,				STATSMAN->m_CurStageStats.AllFailed() );
LuaFunction_PlayerNumber( FullCombo,		STATSMAN->m_CurStageStats.m_player[pn].FullCombo() )
LuaFunction_PlayerNumber( MaxCombo,			STATSMAN->m_CurStageStats.m_player[pn].GetMaxCombo().cnt )
LuaFunction_PlayerNumber( GetGrade,			STATSMAN->m_CurStageStats.m_player[pn].GetGrade() )
LuaFunction_Str( Grade,						StringToGrade(str) );

const StageStats *GetStageStatsN( int n )
{
	if( n == (int) STATSMAN->m_vPlayedStageStats.size() )
		return &STATSMAN->m_CurStageStats;
	if( n > (int) STATSMAN->m_vPlayedStageStats.size() )
		return NULL;
	return &STATSMAN->m_vPlayedStageStats[n];
}

/* GetGrade(0) returns the first grade; GetGrade(1) returns the second grade, and
 * and so on.  GetGrade(GetStagesPlayed()) returns the current grade (from STATSMAN->m_CurStageStats).
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

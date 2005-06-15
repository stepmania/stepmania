#include "global.h"
#include "StatsManager.h"
#include "GameState.h"
#include "Foreach.h"
#include "ProfileManager.h"
#include "PrefsManager.h"
#include "Steps.h"

StatsManager*	STATSMAN = NULL;	// global object accessable from anywhere in the program


StatsManager::StatsManager()
{
}

void StatsManager::Reset()
{
	m_CurStageStats.Init();
	m_vPlayedStageStats.clear();
	m_AccumStageStats.Init();
	
	CalcAccumStageStats();
}

StageStats AccumStageStats( const vector<StageStats>& vss )
{
	StageStats ssreturn;

	if( !vss.empty() )
	{
		ssreturn.pStyle = vss[0].pStyle;
		ssreturn.playMode = vss[0].playMode;
	}

	FOREACH_CONST( StageStats, vss, ss )
		ssreturn.AddStats( *ss );

	unsigned uNumSongs = ssreturn.vpPlayedSongs.size();

	if( uNumSongs == 0 )
		return ssreturn;	// don't divide by 0 below

	/* Scale radar percentages back down to roughly 0..1.  Don't scale RADAR_NUM_TAPS_AND_HOLDS
	 * and the rest, which are counters. */
	// FIXME: Weight each song by the number of stages it took to account for 
	// long, marathon.
	FOREACH_EnabledPlayer( pn )
	{
		for( int r = 0; r < RADAR_NUM_TAPS_AND_HOLDS; r++)
		{
			ssreturn.m_player[pn].radarPossible[r] /= uNumSongs;
			ssreturn.m_player[pn].radarActual[r] /= uNumSongs;
		}
	}

	return ssreturn;
}

void StatsManager::GetFinalEvalStageStats( StageStats& statsOut ) const
{
	statsOut.Init();

	vector<StageStats> vssToCount;

	// Show stats only for the latest 3 normal songs + passed extra stages
	int PassedRegularSongsLeft = 3;
	for( int i = (int)m_vPlayedStageStats.size()-1; i >= 0; --i )
	{
		const StageStats &ss = m_vPlayedStageStats[i];

		if( !ss.OnePassed() )
			continue;

		if( ss.StageType == StageStats::STAGE_NORMAL )
		{
			if( PassedRegularSongsLeft == 0 )
				break;

			--PassedRegularSongsLeft;
		}

		vssToCount.push_back( ss );
	}

	statsOut = AccumStageStats( vssToCount );
}


void StatsManager::CalcAccumStageStats()
{
	m_AccumStageStats = AccumStageStats( m_vPlayedStageStats );
}

/* This data is added to each player profile, and to the machine profile per-player. */
void AddPlayerStatsToProfile( Profile *pProfile, const StageStats &ss, PlayerNumber pn )
{
	ss.AssertValid( pn );
	CHECKPOINT;

	StyleID sID;
	sID.FromStyle( ss.pStyle );

	ASSERT( ss.vpPlayedSongs.size() == ss.m_player[pn].vpPlayedSteps.size() );
	for( unsigned i=0; i<ss.vpPlayedSongs.size(); i++ )
	{
		Steps *pSteps = ss.m_player[pn].vpPlayedSteps[i];

		pProfile->m_iNumSongsPlayedByPlayMode[ss.playMode]++;
		pProfile->m_iNumSongsPlayedByStyle[sID] ++;
		pProfile->m_iNumSongsPlayedByDifficulty[pSteps->GetDifficulty()] ++;

		int iMeter = clamp( pSteps->GetMeter(), 0, MAX_METER );
		pProfile->m_iNumSongsPlayedByMeter[iMeter] ++;
	}
	
	pProfile->m_iTotalDancePoints += ss.m_player[pn].iActualDancePoints;

	if( ss.StageType == StageStats::STAGE_EXTRA || ss.StageType == StageStats::STAGE_EXTRA2 )
	{
		if( ss.m_player[pn].bFailed )
			++pProfile->m_iNumExtraStagesFailed;
		else
			++pProfile->m_iNumExtraStagesPassed;
	}

	// If you fail in a course, you passed all but the final song.
	// FIXME: Not true.  If playing with 2 players, one player could have failed earlier.
	if( !ss.m_player[pn].bFailed )
	{
		pProfile->m_iNumStagesPassedByPlayMode[ss.playMode] ++;
		pProfile->m_iNumStagesPassedByGrade[ss.m_player[pn].GetGrade()] ++;
	}
}


void StatsManager::CommitStatsToProfiles()
{
	//
	// Add step totals.  Use radarActual, since the player might have failed part way
	// through the song, in which case we don't want to give credit for the rest of the
	// song.
	//
	FOREACH_HumanPlayer( pn )
	{
		int iNumTapsAndHolds	= (int) m_CurStageStats.m_player[pn].radarActual[RADAR_NUM_TAPS_AND_HOLDS];
		int iNumJumps			= (int) m_CurStageStats.m_player[pn].radarActual[RADAR_NUM_JUMPS];
		int iNumHolds			= (int) m_CurStageStats.m_player[pn].radarActual[RADAR_NUM_HOLDS];
		int iNumRolls			= (int) m_CurStageStats.m_player[pn].radarActual[RADAR_NUM_ROLLS];
		int iNumMines			= (int) m_CurStageStats.m_player[pn].radarActual[RADAR_NUM_MINES];
		int iNumHands			= (int) m_CurStageStats.m_player[pn].radarActual[RADAR_NUM_HANDS];
		float fCaloriesBurned	= m_CurStageStats.m_player[pn].fCaloriesBurned;
		PROFILEMAN->AddStepTotals( pn, iNumTapsAndHolds, iNumJumps, iNumHolds, iNumRolls, iNumMines, iNumHands, fCaloriesBurned );
	}

	// Update profile stats
	Profile* pMachineProfile = PROFILEMAN->GetMachineProfile();

	int iGameplaySeconds = (int)truncf(m_CurStageStats.fGameplaySeconds);

	pMachineProfile->m_iTotalGameplaySeconds += iGameplaySeconds;
	pMachineProfile->m_iCurrentCombo = 0;
	pMachineProfile->m_iNumTotalSongsPlayed += m_CurStageStats.vpPlayedSongs.size();

	CHECKPOINT;
	FOREACH_HumanPlayer( pn )
	{
		CHECKPOINT;

		Profile* pPlayerProfile = PROFILEMAN->GetProfile( pn );
		if( pPlayerProfile )
		{
			pPlayerProfile->m_iTotalGameplaySeconds += iGameplaySeconds;
			pPlayerProfile->m_iCurrentCombo = 
				PREFSMAN->m_bComboContinuesBetweenSongs ? 
				m_CurStageStats.m_player[pn].iCurCombo : 
				0;
			pPlayerProfile->m_iNumTotalSongsPlayed += m_CurStageStats.vpPlayedSongs.size();
		}

		AddPlayerStatsToProfile( pMachineProfile, m_CurStageStats, pn );

		if( pPlayerProfile )
			AddPlayerStatsToProfile( pPlayerProfile, m_CurStageStats, pn );

		CHECKPOINT;
	}
}


// lua start
#include "LuaBinding.h"

template<class T>
class LunaStatsManager : public Luna<T>
{
public:
	LunaStatsManager() { LUA->Register( Register ); }

	static int GetCurStageStats( T* p, lua_State *L )	{ p->m_CurStageStats.PushSelf(L); return 1; }
	static int GetAccumStageStats( T* p, lua_State *L )	{ p->GetAccumStageStats().PushSelf(L); return 1; }
	static int Reset( T* p, lua_State *L )				{ p->Reset(); return 0; }
	static int GetFinalGrade( T* p, lua_State *L )
	{
		PlayerNumber pn = (PlayerNumber)IArg(1);

		if( !GAMESTATE->IsHumanPlayer(pn) )
			lua_pushnumber( L, GRADE_NO_DATA );
		else
		{
			StageStats stats;
			p->GetFinalEvalStageStats( stats );
			lua_pushnumber( L, stats.m_player[pn].GetGrade() );
		}
		return 1;
	}
	static int GetStagesPlayed( T* p, lua_State *L )				{ lua_pushnumber( L, p->m_vPlayedStageStats.size() ); return 1; }

	static int GetBestGrade( T* p, lua_State *L )
	{
		Grade g = NUM_GRADES;
		FOREACH_EnabledPlayer( pn )
			g = min( g, STATSMAN->m_CurStageStats.m_player[pn].GetGrade() );
		lua_pushnumber( L, g );
		return 1;
	}

	static int GetWorstGrade( T* p, lua_State *L )
	{
		Grade g = GRADE_TIER01;
		FOREACH_EnabledPlayer( pn )
			g = max( g, STATSMAN->m_CurStageStats.m_player[pn].GetGrade() );
		lua_pushnumber( L, g );
		return 1;
	}


	static void Register(lua_State *L)
	{
		ADD_METHOD( GetCurStageStats )
		ADD_METHOD( GetAccumStageStats )
		ADD_METHOD( Reset )
		ADD_METHOD( GetFinalGrade )
		ADD_METHOD( GetStagesPlayed )
		ADD_METHOD( GetBestGrade )
		ADD_METHOD( GetWorstGrade )

		Luna<T>::Register( L );

		// Add global singleton if constructed already.  If it's not constructed yet,
		// then we'll register it later when we reinit Lua just before 
		// initializing the display.
		if( STATSMAN )
		{
			lua_pushstring(L, "STATSMAN");
			STATSMAN->PushSelf( L );
			lua_settable(L, LUA_GLOBALSINDEX);
		}
	}
};

LUA_REGISTER_CLASS( StatsManager )
// lua end


//
// Old Lua
//

#include "LuaFunctions.h"
LuaFunction_NoArgs( OnePassed,				STATSMAN->m_CurStageStats.OnePassed() );
LuaFunction_NoArgs( AllFailed,				STATSMAN->m_CurStageStats.AllFailed() );
LuaFunction( Grade,							StringToGrade( SArg(1) ) );

const StageStats *GetStageStatsN( int n )
{
	if( n == (int) STATSMAN->m_vPlayedStageStats.size() )
		return &STATSMAN->m_CurStageStats;
	if( n > (int) STATSMAN->m_vPlayedStageStats.size() )
		return NULL;
	return &STATSMAN->m_vPlayedStageStats[n];
}

Grade GetBestFinalGrade()
{
	Grade top_grade = GRADE_FAILED;
	StageStats stats;
	STATSMAN->GetFinalEvalStageStats( stats );
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

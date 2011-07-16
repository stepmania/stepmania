#include "global.h"
#include "StatsManager.h"
#include "RageFileManager.h"
#include "GameState.h"
#include "Foreach.h"
#include "ProfileManager.h"
#include "Profile.h"
#include "PrefsManager.h"
#include "Steps.h"
#include "StyleUtil.h"
#include "LuaManager.h"
#include "Profile.h"
#include "XmlFile.h"
#include "CryptManager.h"
#include "XmlFileUtil.h"

StatsManager*	STATSMAN = NULL;	// global object accessable from anywhere in the program

void AddPlayerStatsToProfile( Profile *pProfile, const StageStats &ss, PlayerNumber pn );
XNode* MakeRecentScoreNode( const StageStats &ss, Trail *pTrail, const PlayerStageStats &pss, MultiPlayer mp );

StatsManager::StatsManager()
{
	// Register with Lua.
	{
		Lua *L = LUA->Get();
		lua_pushstring( L, "STATSMAN" );
		this->PushSelf( L );
		lua_settable(L, LUA_GLOBALSINDEX);
		LUA->Release( L );
	}
}

StatsManager::~StatsManager()
{
	// Unregister with Lua.
	LUA->UnsetGlobal( "STATSMAN" );
}

void StatsManager::Reset()
{
	m_CurStageStats.Init();
	m_vPlayedStageStats.clear();
	m_AccumPlayedStageStats.Init();
	
	CalcAccumPlayedStageStats();
}

static StageStats AccumPlayedStageStats( const vector<StageStats>& vss )
{
	StageStats ssreturn;

	if( !vss.empty() )
	{
		ssreturn.m_pStyle = vss[0].m_pStyle;
		ssreturn.m_playMode = vss[0].m_playMode;
	}

	FOREACH_CONST( StageStats, vss, ss )
		ssreturn.AddStats( *ss );

	unsigned uNumSongs = ssreturn.m_vpPlayedSongs.size();

	if( uNumSongs == 0 )
		return ssreturn;	// don't divide by 0 below

	/* Scale radar percentages back down to roughly 0..1.  Don't scale RadarCategory_TapsAndHolds
	 * and the rest, which are counters. */
	// FIXME: Weight each song by the number of stages it took to account for 
	// long, marathon.
	FOREACH_EnabledPlayer( p )
	{
		for( int r = 0; r < RadarCategory_TapsAndHolds; r++)
		{
			ssreturn.m_player[p].m_radarPossible[r] /= uNumSongs;
			ssreturn.m_player[p].m_radarActual[r] /= uNumSongs;
		}
	}
	FOREACH_EnabledMultiPlayer( p )
	{
		for( int r = 0; r < RadarCategory_TapsAndHolds; r++)
		{
			ssreturn.m_multiPlayer[p].m_radarPossible[r] /= uNumSongs;
			ssreturn.m_multiPlayer[p].m_radarActual[r] /= uNumSongs;
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

		if( ss.m_Stage != Stage_Extra1 && ss.m_Stage != Stage_Extra2 )
		{
			if( PassedRegularSongsLeft == 0 )
				break;

			--PassedRegularSongsLeft;
		}

		vssToCount.push_back( ss );
	}

	statsOut = AccumPlayedStageStats( vssToCount );
}


void StatsManager::CalcAccumPlayedStageStats()
{
	m_AccumPlayedStageStats = AccumPlayedStageStats( m_vPlayedStageStats );
}

/* This data is added to each player profile, and to the machine profile per-player. */
void AddPlayerStatsToProfile( Profile *pProfile, const StageStats &ss, PlayerNumber pn )
{
	ss.AssertValid( pn );
	CHECKPOINT;

	StyleID sID;
	sID.FromStyle( ss.m_pStyle );

	ASSERT( (int) ss.m_vpPlayedSongs.size() == ss.m_player[pn].m_iStepsPlayed );
	for( int i=0; i<ss.m_player[pn].m_iStepsPlayed; i++ )
	{
		Steps *pSteps = ss.m_player[pn].m_vpPossibleSteps[i];

		pProfile->m_iNumSongsPlayedByPlayMode[ss.m_playMode]++;
		pProfile->m_iNumSongsPlayedByStyle[sID] ++;
		pProfile->m_iNumSongsPlayedByDifficulty[pSteps->GetDifficulty()] ++;

		int iMeter = clamp( pSteps->GetMeter(), 0, MAX_METER );
		pProfile->m_iNumSongsPlayedByMeter[iMeter] ++;
	}
	
	pProfile->m_iTotalDancePoints += ss.m_player[pn].m_iActualDancePoints;

	if( ss.m_Stage == Stage_Extra1 || ss.m_Stage == Stage_Extra2 )
	{
		if( ss.m_player[pn].m_bFailed )
			++pProfile->m_iNumExtraStagesFailed;
		else
			++pProfile->m_iNumExtraStagesPassed;
	}

	// If you fail in a course, you passed all but the final song.
	// FIXME: Not true.  If playing with 2 players, one player could have failed earlier.
	if( !ss.m_player[pn].m_bFailed )
	{
		pProfile->m_iNumStagesPassedByPlayMode[ss.m_playMode] ++;
		pProfile->m_iNumStagesPassedByGrade[ss.m_player[pn].GetGrade()] ++;
	}
}

XNode* MakeRecentScoreNode( const StageStats &ss, Trail *pTrail, const PlayerStageStats &pss, MultiPlayer mp )
{
	XNode* pNode = NULL;
	if( GAMESTATE->IsCourseMode() )
	{
		pNode = new XNode( "HighScoreForACourseAndTrail" );

		CourseID courseID;
		courseID.FromCourse(GAMESTATE->m_pCurCourse );
		pNode->AppendChild( courseID.CreateNode() );

		TrailID trailID;
		trailID.FromTrail( pTrail );
		pNode->AppendChild( trailID.CreateNode() );

	}
	else
	{
		pNode = new XNode( "HighScoreForASongAndSteps" );

		SongID songID;
		songID.FromSong( ss.m_vpPossibleSongs[0] );
		pNode->AppendChild( songID.CreateNode() );

		StepsID stepsID;
		stepsID.FromSteps( pss.m_vpPossibleSteps[0] );
		pNode->AppendChild( stepsID.CreateNode() );
	}

	XNode* pHighScore = pss.m_HighScore.CreateNode();
	pHighScore->AppendChild("Pad", mp);
	pHighScore->AppendChild("StageGuid", GAMESTATE->m_sStageGUID);
	pHighScore->AppendChild("Guid", CryptManager::GenerateRandomUUID());

	pNode->AppendChild( pHighScore );

	return pNode;
}

void StatsManager::CommitStatsToProfiles( const StageStats *pSS )
{
	// Add step totals.  Use radarActual, since the player might have failed part way
	// through the song, in which case we don't want to give credit for the rest of the
	// song.
	FOREACH_HumanPlayer( pn )
	{
		int iNumTapsAndHolds	= (int) pSS->m_player[pn].m_radarActual[RadarCategory_TapsAndHolds];
		int iNumJumps		= (int) pSS->m_player[pn].m_radarActual[RadarCategory_Jumps];
		int iNumHolds		= (int) pSS->m_player[pn].m_radarActual[RadarCategory_Holds];
		int iNumRolls		= (int) pSS->m_player[pn].m_radarActual[RadarCategory_Rolls];
		int iNumMines		= (int) pSS->m_player[pn].m_radarActual[RadarCategory_Mines];
		int iNumHands		= (int) pSS->m_player[pn].m_radarActual[RadarCategory_Hands];
		int iNumLifts		= (int) pSS->m_player[pn].m_radarActual[RadarCategory_Lifts];
		float fCaloriesBurned	= pSS->m_player[pn].m_fCaloriesBurned;
		PROFILEMAN->AddStepTotals( pn, iNumTapsAndHolds, iNumJumps, iNumHolds, iNumRolls, iNumMines, iNumHands, iNumLifts, fCaloriesBurned );
	}

	// Update profile stats
	Profile* pMachineProfile = PROFILEMAN->GetMachineProfile();

	int iGameplaySeconds = (int)truncf(pSS->m_fGameplaySeconds);

	pMachineProfile->m_iTotalGameplaySeconds += iGameplaySeconds;
	pMachineProfile->m_iNumTotalSongsPlayed += pSS->m_vpPlayedSongs.size();

	CHECKPOINT;
	if( !GAMESTATE->m_bMultiplayer )	// FIXME
	{
		FOREACH_HumanPlayer( pn )
		{
			CHECKPOINT;

			Profile* pPlayerProfile = PROFILEMAN->GetProfile( pn );
			if( pPlayerProfile )
			{
				pPlayerProfile->m_iTotalGameplaySeconds += iGameplaySeconds;
				pPlayerProfile->m_iNumTotalSongsPlayed += pSS->m_vpPlayedSongs.size();
			}

			LOG->Trace("Adding stats to machine profile...");
			AddPlayerStatsToProfile( pMachineProfile, *pSS, pn );

			if( pPlayerProfile )
			{
				LOG->Trace("Adding stats to player profile...");
				AddPlayerStatsToProfile( pPlayerProfile, *pSS, pn );
			}

			CHECKPOINT;
		}
	}

	// Save recent scores
	{
		auto_ptr<XNode> xml( new XNode("Stats") );
		xml->AppendChild( "MachineGuid",  PROFILEMAN->GetMachineProfile()->m_sGuid );

		XNode *recent = NULL;
		if( GAMESTATE->IsCourseMode() )
			recent = xml->AppendChild( new XNode("RecentCourseScores") );
		else
			recent = xml->AppendChild( new XNode("RecentSongScores") );

		if(!GAMESTATE->m_bMultiplayer)
		{
			FOREACH_HumanPlayer( p )
			{
				if( pSS->m_player[p].m_HighScore.IsEmpty() )
					continue;
				recent->AppendChild( MakeRecentScoreNode( *pSS, GAMESTATE->m_pCurTrail[p], pSS->m_player[p], MultiPlayer_Invalid ) );
			}
		}
		else
		{
			FOREACH_EnabledMultiPlayer( mp )
			{
				if( pSS->m_multiPlayer[mp].m_HighScore.IsEmpty() )
					continue;
				recent->AppendChild( MakeRecentScoreNode( *pSS, GAMESTATE->m_pCurTrail[GAMESTATE->GetMasterPlayerNumber()], pSS->m_multiPlayer[mp], mp ) );
			}
		}

		RString sDate = DateTime::GetNowDate().GetString();
		sDate.Replace(":","-");

		const RString UPLOAD_DIR = "/Save/Upload/";
		RString sFileNameNoExtension = Profile::MakeUniqueFileNameNoExtension(UPLOAD_DIR, sDate + " " );
		RString fn = UPLOAD_DIR + sFileNameNoExtension + ".xml";
		
		bool bSaved = XmlFileUtil::SaveToFile( xml.get(), fn, "", false );
		
		if( bSaved )
		{
			RString sStatsXmlSigFile = fn + SIGNATURE_APPEND;
			CryptManager::SignFileToFile(fn, sStatsXmlSigFile);
		}
	}

	//FileCopy( "Data/TempTestGroups.xml", "Save/Upload/data.xml" );
}

void StatsManager::UnjoinPlayer( PlayerNumber pn )
{
	/* A player has been unjoined.  Clear his data from m_vPlayedStageStats, and
	 * purge any m_vPlayedStageStats that no longer have any player data because
	 * all of the players that were playing at the time have been unjoined. */
	FOREACH( StageStats, m_vPlayedStageStats, ss )
		ss->m_player[pn] = PlayerStageStats();

	for( int i = 0; i < (int) m_vPlayedStageStats.size(); ++i )
	{
		StageStats &ss = m_vPlayedStageStats[i];
		bool bIsActive = false;
		FOREACH_PlayerNumber( p )
			if( ss.m_player[p].m_bJoined )
				bIsActive = true;
		FOREACH_MultiPlayer( mp )
			if( ss.m_multiPlayer[mp].m_bJoined )
				bIsActive = true;
		if( bIsActive )
			continue;

		m_vPlayedStageStats.erase( m_vPlayedStageStats.begin()+i );
		--i;
	}
}

void StatsManager::GetStepsInUse( set<Steps*> &apInUseOut ) const
{
	for( int i = 0; i < (int) m_vPlayedStageStats.size(); ++i )
	{
		FOREACH_PlayerNumber( pn )
		{
			const PlayerStageStats &pss = m_vPlayedStageStats[i].m_player[pn];
			apInUseOut.insert( pss.m_vpPossibleSteps.begin(), pss.m_vpPossibleSteps.end() );
		}

		FOREACH_MultiPlayer( mp )
		{
			const PlayerStageStats &pss = m_vPlayedStageStats[i].m_multiPlayer[mp];
			apInUseOut.insert( pss.m_vpPossibleSteps.begin(), pss.m_vpPossibleSteps.end() );
		}
	}
}

// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the StatsManager. */ 
class LunaStatsManager: public Luna<StatsManager>
{
public:
	static int GetCurStageStats( T* p, lua_State *L )	{ p->m_CurStageStats.PushSelf(L); return 1; }
	static int GetPlayedStageStats( T* p, lua_State *L )
	{
		int iAgo = IArg(1);
		int iIndex = p->m_vPlayedStageStats.size() - iAgo;
		if( iIndex < 0 || iIndex >= (int) p->m_vPlayedStageStats.size() )
			return 0;

		p->m_vPlayedStageStats[iIndex].PushSelf(L);
		return 1;
	}
	static int Reset( T* p, lua_State *L )			{ p->Reset(); return 0; }
	static int GetAccumPlayedStageStats( T* p, lua_State *L )	{ p->GetAccumPlayedStageStats().PushSelf(L); return 1; }
	static int GetFinalGrade( T* p, lua_State *L )
	{
		PlayerNumber pn = Enum::Check<PlayerNumber>(L, 1);

		if( !GAMESTATE->IsHumanPlayer(pn) )
			lua_pushnumber( L, Grade_NoData );
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
		Grade g = NUM_Grade;
		FOREACH_EnabledPlayer( pn )
			g = min( g, STATSMAN->m_CurStageStats.m_player[pn].GetGrade() );
		lua_pushnumber( L, g );
		return 1;
	}

	static int GetWorstGrade( T* p, lua_State *L )
	{
		Grade g = Grade_Tier01;
		FOREACH_EnabledPlayer( pn )
			g = max( g, STATSMAN->m_CurStageStats.m_player[pn].GetGrade() );
		lua_pushnumber( L, g );
		return 1;
	}

	static int GetBestFinalGrade( T* t, lua_State *L )
	{
		Grade top_grade = Grade_Failed;
		StageStats stats;
		t->GetFinalEvalStageStats( stats );
		FOREACH_HumanPlayer( p )
		{
			// If this player failed any stage, then their final grade is an F.
			bool bPlayerFailedOneStage = false;
			FOREACH_CONST( StageStats, STATSMAN->m_vPlayedStageStats, ss )
			{
				if( ss->m_player[p].m_bFailed )
				{
					bPlayerFailedOneStage = true;
					break;
				}
			}

			if( bPlayerFailedOneStage )
				continue;

			top_grade = min( top_grade, stats.m_player[p].GetGrade() );
		}

		Enum::Push( L, top_grade );
		return 1;
	}

	LunaStatsManager()
	{
		ADD_METHOD( GetCurStageStats );
		ADD_METHOD( GetPlayedStageStats );
		ADD_METHOD( GetAccumPlayedStageStats );
		ADD_METHOD( Reset );
		ADD_METHOD( GetFinalGrade );
		ADD_METHOD( GetStagesPlayed );
		ADD_METHOD( GetBestGrade );
		ADD_METHOD( GetWorstGrade );
		ADD_METHOD( GetBestFinalGrade );
	}
};

LUA_REGISTER_CLASS( StatsManager )
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

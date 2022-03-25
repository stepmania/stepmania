#include "global.h"
#include "StageStats.h"
#include "GameState.h"
#include "Steps.h"
#include "Song.h"
#include "RageLog.h"
#include "PrefsManager.h"
#include "PlayerState.h"
#include "Style.h"
#include "Profile.h"
#include "ProfileManager.h"

/* Arcade:	for the current stage (one song).  
 * Nonstop/Oni/Endless:	 for current course (which usually contains multiple songs)
 */

StageStats::StageStats()
{
	m_playMode = PlayMode_Invalid;
	m_Stage = Stage_Invalid;
	m_iStageIndex = -1;
	m_vpPlayedSongs.clear();
	m_vpPossibleSongs.clear();
	m_EarnedExtraStage = EarnedExtraStage_No;
	m_bGaveUp = false;
	m_bUsedAutoplay = false;
	m_fGameplaySeconds = 0;
	m_fStepsSeconds = 0;
	m_fMusicRate = 1;
	FOREACH_PlayerNumber(pn)
	{
		m_player[pn].Init(pn);
	}
	FOREACH_MultiPlayer(pn)
	{
		m_multiPlayer[pn].Init(pn);
	}
}

void StageStats::Init()
{
	*this = StageStats();
}

void StageStats::AssertValid( PlayerNumber pn ) const
{
	ASSERT( m_vpPlayedSongs.size() != 0 );
	ASSERT( m_vpPossibleSongs.size() != 0 );
	if( m_vpPlayedSongs[0] )
		CHECKPOINT_M( m_vpPlayedSongs[0]->GetTranslitFullTitle() );
	ASSERT( m_player[pn].m_iStepsPlayed > 0 );
	ASSERT( m_player[pn].m_vpPossibleSteps.size() != 0 );
	ASSERT( m_player[pn].m_vpPossibleSteps[0] != nullptr );
	ASSERT_M( m_playMode < NUM_PlayMode, ssprintf("playmode %i", m_playMode) );
	ASSERT_M( m_player[pn].m_vpPossibleSteps[0]->GetDifficulty() < NUM_Difficulty, ssprintf("Invalid Difficulty %i", m_player[pn].m_vpPossibleSteps[0]->GetDifficulty()) );
	ASSERT_M( (int) m_vpPlayedSongs.size() == m_player[pn].m_iStepsPlayed, ssprintf("%i Songs Played != %i Steps Played for player %i", (int)m_vpPlayedSongs.size(), (int)m_player[pn].m_iStepsPlayed, pn) );
	ASSERT_M( m_vpPossibleSongs.size() == m_player[pn].m_vpPossibleSteps.size(), ssprintf("%i Possible Songs != %i Possible Steps for player %i", (int)m_vpPossibleSongs.size(), (int)m_player[pn].m_vpPossibleSteps.size(), pn) );
}

void StageStats::AssertValid( MultiPlayer pn ) const
{
	ASSERT( m_vpPlayedSongs.size() != 0 );
	ASSERT( m_vpPossibleSongs.size() != 0 );
	if( m_vpPlayedSongs[0] )
		CHECKPOINT_M( m_vpPlayedSongs[0]->GetTranslitFullTitle() );
	ASSERT( m_multiPlayer[pn].m_vpPossibleSteps.size() != 0 );
	ASSERT( m_multiPlayer[pn].m_vpPossibleSteps[0] != nullptr );
	ASSERT_M( m_playMode < NUM_PlayMode, ssprintf("playmode %i", m_playMode) );
	ASSERT_M( m_player[pn].m_vpPossibleSteps[0]->GetDifficulty() < NUM_Difficulty, ssprintf("difficulty %i", m_player[pn].m_vpPossibleSteps[0]->GetDifficulty()) );
	ASSERT( (int) m_vpPlayedSongs.size() == m_player[pn].m_iStepsPlayed );
	ASSERT( m_vpPossibleSongs.size() == m_player[pn].m_vpPossibleSteps.size() );
}


int StageStats::GetAverageMeter( PlayerNumber pn ) const
{
	AssertValid( pn );

	// TODO: This isn't correct for courses.
	int iTotalMeter = 0;

	for( unsigned i=0; i<m_vpPlayedSongs.size(); i++ )
	{
		const Steps* pSteps = m_player[pn].m_vpPossibleSteps[i];
		iTotalMeter += pSteps->GetMeter();
	}
	return iTotalMeter / m_vpPlayedSongs.size();	// round down
}

void StageStats::AddStats( const StageStats& other )
{
	ASSERT( !other.m_vpPlayedSongs.empty() );
	for (Song *s : other.m_vpPlayedSongs)
		m_vpPlayedSongs.push_back( s );
	for (Song *s : other.m_vpPossibleSongs)
		m_vpPossibleSongs.push_back( s );
	m_Stage = Stage_Invalid; // meaningless
	m_iStageIndex = -1; // meaningless

	m_bGaveUp |= other.m_bGaveUp;
	m_bUsedAutoplay |= other.m_bUsedAutoplay;

	m_fGameplaySeconds += other.m_fGameplaySeconds;
	m_fStepsSeconds += other.m_fStepsSeconds;

	FOREACH_EnabledPlayer( p )
		m_player[p].AddStats( other.m_player[p] );
}

bool StageStats::OnePassed() const
{
	FOREACH_EnabledPlayer( p )
		if( !m_player[p].m_bFailed )
			return true;
	return false;
}

bool StageStats::AllFailed() const
{
	FOREACH_EnabledPlayer( p )
		if( !m_player[p].m_bFailed )
			return false;
	return true;
}

float StageStats::GetTotalPossibleStepsSeconds() const
{
	float fSecs = 0;
	for (Song const *s : m_vpPlayedSongs)
		fSecs += s->GetStepsSeconds();
	return fSecs / m_fMusicRate;
}

static HighScore FillInHighScore( const PlayerStageStats &pss, const PlayerState &ps, RString sRankingToFillInMarker, RString sPlayerGuid )
{
	HighScore hs;
	hs.SetName( sRankingToFillInMarker );
	hs.SetGrade( pss.GetGrade() );
	hs.SetScore( pss.m_iScore );
	hs.SetPercentDP( pss.GetPercentDancePoints() );
	hs.SetAliveSeconds( pss.m_fAliveSeconds );
	hs.SetMaxCombo( pss.GetMaxCombo().m_cnt );
	hs.SetStageAward( pss.m_StageAward );
	hs.SetPeakComboAward( pss.m_PeakComboAward );

	vector<RString> asModifiers;
	{
		RString sPlayerOptions = ps.m_PlayerOptions.GetStage().GetString();
		if( !sPlayerOptions.empty() )
			asModifiers.push_back( sPlayerOptions );
		RString sSongOptions = GAMESTATE->m_SongOptions.GetStage().GetString();
		if( !sSongOptions.empty() )
			asModifiers.push_back( sSongOptions );
	}
	hs.SetModifiers( join(", ", asModifiers) );

	hs.SetDateTime( DateTime::GetNowDateTime() );
	hs.SetPlayerGuid( sPlayerGuid );
	hs.SetMachineGuid( PROFILEMAN->GetMachineProfile()->m_sGuid );
	hs.SetProductID( PREFSMAN->m_iProductID );
	FOREACH_ENUM( TapNoteScore, tns )
		hs.SetTapNoteScore( tns, pss.m_iTapNoteScores[tns] );
	FOREACH_ENUM( HoldNoteScore, hns )
		hs.SetHoldNoteScore( hns, pss.m_iHoldNoteScores[hns] );
	hs.SetRadarValues( pss.m_radarActual );
	hs.SetLifeRemainingSeconds( pss.m_fLifeRemainingSeconds );
	hs.SetDisqualified( pss.IsDisqualified() );

	return hs;
}

void StageStats::FinalizeScores( bool bSummary )
{
	switch( GAMESTATE->m_PlayMode )
	{
		case PLAY_MODE_BATTLE:
		case PLAY_MODE_RAVE:
			return; // don't save scores in battle
		default: break;
	}

	if( PREFSMAN->m_sTestInitialScreen.Get() != "" )
	{
		FOREACH_PlayerNumber( pn )
		{
			m_player[pn].m_iPersonalHighScoreIndex = 0;
			m_player[pn].m_iMachineHighScoreIndex = 0;
		}
	}

	// don't save scores if the player chose not to
	if( !GAMESTATE->m_SongOptions.GetCurrent().m_bSaveScore )
		return;

	LOG->Trace( "saving stats and high scores" );

	// generate a HighScore for each player

	// whether or not to save scores when the stage was failed depends on if this
	// is a course or not... it's handled below in the switch.
	FOREACH_HumanPlayer( p )
	{
		RString sPlayerGuid = PROFILEMAN->IsPersistentProfile(p) ? PROFILEMAN->GetProfile(p)->m_sGuid : RString("");
		m_player[p].m_HighScore = FillInHighScore( m_player[p], *GAMESTATE->m_pPlayerState[p], RANKING_TO_FILL_IN_MARKER[p], sPlayerGuid );
	}
	FOREACH_EnabledMultiPlayer( mp )
	{
		RString sPlayerGuid = "00000000-0000-0000-0000-000000000000";	// FIXME
		m_multiPlayer[mp].m_HighScore = FillInHighScore( m_multiPlayer[mp], *GAMESTATE->m_pMultiPlayerState[mp], "", sPlayerGuid );
	}

	FOREACH_HumanPlayer( p )
	{
		const HighScore &hs = m_player[p].m_HighScore;
		StepsType st = GAMESTATE->GetCurrentStyle(p)->m_StepsType;

		const Song* pSong = GAMESTATE->m_pCurSong;
		const Steps* pSteps = GAMESTATE->m_pCurSteps[p];

		// Don't save DQ'd scores
		if( hs.GetDisqualified() )
			continue;

		// Don't save autoplay scores
		if( m_bUsedAutoplay )
			continue;

		if( bSummary )
		{
			// don't save scores if any stage was failed
			if( m_player[p].m_bFailed ) 
				continue;

			int iAverageMeter = GetAverageMeter(p);
			m_player[p].m_rc = AverageMeterToRankingCategory( iAverageMeter );

			PROFILEMAN->AddCategoryScore( st, m_player[p].m_rc, p, hs, m_player[p].m_iPersonalHighScoreIndex, m_player[p].m_iMachineHighScoreIndex );

			// TRICKY: Increment play count here, and not on ScreenGameplay like the others.
			PROFILEMAN->IncrementCategoryPlayCount( st, m_player[p].m_rc, p );
		}
		else if( GAMESTATE->IsCourseMode() )
		{
			// Save this stage to recent scores
			Course* pCourse = GAMESTATE->m_pCurCourse;
			ASSERT( pCourse != nullptr );
			Trail* pTrail = GAMESTATE->m_pCurTrail[p];

			PROFILEMAN->AddCourseScore( pCourse, pTrail, p, hs, m_player[p].m_iPersonalHighScoreIndex, m_player[p].m_iMachineHighScoreIndex );
		}
		else
		{
			ASSERT( pSteps != nullptr );

			PROFILEMAN->AddStepsScore( pSong, pSteps, p, hs, m_player[p].m_iPersonalHighScoreIndex, m_player[p].m_iMachineHighScoreIndex );
		}
	}

	// If both players get a machine high score in the same HighScoreList,
	// then one player's score may have bumped the other player. Look in 
	// the HighScoreList and re-get the high score index.
	FOREACH_HumanPlayer( p )
	{
		if( m_player[p].m_iMachineHighScoreIndex == -1 )	// no record
			continue;	// skip

		HighScore &hs = m_player[p].m_HighScore;
		Profile* pProfile = PROFILEMAN->GetMachineProfile();
		StepsType st = GAMESTATE->GetCurrentStyle(p)->m_StepsType;

		const HighScoreList *pHSL = nullptr;
		if( bSummary )
		{
			pHSL = &pProfile->GetCategoryHighScoreList( st, m_player[p].m_rc );
		}
		else if( GAMESTATE->IsCourseMode() )
		{
			Course* pCourse = GAMESTATE->m_pCurCourse;
			ASSERT( pCourse != nullptr );
			Trail *pTrail = GAMESTATE->m_pCurTrail[p];
			ASSERT( pTrail != nullptr );
			pHSL = &pProfile->GetCourseHighScoreList( pCourse, pTrail );
		}
		else
		{
			Song* pSong = GAMESTATE->m_pCurSong;
			Steps* pSteps = GAMESTATE->m_pCurSteps[p];
			pHSL = &pProfile->GetStepsHighScoreList( pSong, pSteps );
		}

		vector<HighScore>::const_iterator iter = find( pHSL->vHighScores.begin(), pHSL->vHighScores.end(), hs );
		if( iter == pHSL->vHighScores.end() )
			m_player[p].m_iMachineHighScoreIndex = -1;
		else
			m_player[p].m_iMachineHighScoreIndex = iter - pHSL->vHighScores.begin();
	}

	LOG->Trace( "done saving stats and high scores" );
}

bool StageStats::PlayerHasHighScore( PlayerNumber pn ) const
{
	const Song *pSong = m_vpPlayedSongs[0];
	const Steps *pSteps = m_player[pn].m_vpPossibleSteps[0];
	const Course *pCourse = GAMESTATE->m_pCurCourse;
	const Trail *pTrail = GAMESTATE->m_pCurTrail[pn];

	// Don't show high scores for tutorial songs.
	if( pSong->IsTutorial() == Song::SHOW_NEVER )
		return false;

	const HighScoreList &hsl = 
		GAMESTATE->IsCourseMode() ?
		PROFILEMAN->GetMachineProfile()->GetCourseHighScoreList(pCourse, pTrail) :
		PROFILEMAN->GetMachineProfile()->GetStepsHighScoreList(pSong, pSteps);

	int iScore = m_player[pn].m_iScore;
	float fPercentDP = m_player[pn].GetPercentDancePoints();
	for( int h=0; h<(int)hsl.vHighScores.size() && h<PREFSMAN->m_iMaxHighScoresPerListForMachine; ++h )
	{
		const HighScore &hs = hsl.vHighScores[h];
		if( hs.GetName() == RANKING_TO_FILL_IN_MARKER[pn]  &&
			hs.GetPercentDP() == fPercentDP  && 
			hs.GetScore() == iScore )
		{
			return true;
		}
	}

	return false;
}

unsigned int StageStats::GetMinimumMissCombo() const
{
	unsigned int iMin = INT_MAX;
	FOREACH_HumanPlayer( p )
		iMin = min( iMin, m_player[p].m_iCurMissCombo );
	return iMin;
}

// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the StageStats. */ 
class LunaStageStats: public Luna<StageStats>
{
public:
	static int GetPlayerStageStats( T* p, lua_State *L )		{ p->m_player[Enum::Check<PlayerNumber>(L, 1)].PushSelf(L); return 1; }
	static int GetMultiPlayerStageStats( T* p, lua_State *L )	{ p->m_multiPlayer[Enum::Check<MultiPlayer>(L, 1)].PushSelf(L); return 1; }
	static int GetPlayedSongs( T* p, lua_State *L )
	{
		lua_newtable(L);
		for( int i = 0; i < (int) p->m_vpPlayedSongs.size(); ++i )
		{
			p->m_vpPlayedSongs[i]->PushSelf(L);
			lua_rawseti( L, -2, i+1 );
		}
		return 1;
	}
	static int GetPossibleSongs( T* p, lua_State *L )
	{
		lua_newtable(L);
		for( int i = 0; i < (int) p->m_vpPossibleSongs.size(); ++i )
		{
			p->m_vpPossibleSongs[i]->PushSelf(L);
			lua_rawseti( L, -2, i+1 );
		}
		return 1;

	}
	static int GetGameplaySeconds( T* p, lua_State *L )	{ lua_pushnumber(L, p->m_fGameplaySeconds); return 1; }
	static int OnePassed( T* p, lua_State *L )		{ lua_pushboolean(L, p->OnePassed()); return 1; }
	static int AllFailed( T* p, lua_State *L )		{ lua_pushboolean(L, p->AllFailed()); return 1; }
	static int GetStage( T* p, lua_State *L )		{ LuaHelpers::Push( L, p->m_Stage ); return 1; }
	DEFINE_METHOD( GetStageIndex,				m_iStageIndex )
	DEFINE_METHOD(GetStepsSeconds, m_fStepsSeconds)
	static int PlayerHasHighScore( T* p, lua_State *L )
	{
		lua_pushboolean(L, p->PlayerHasHighScore(Enum::Check<PlayerNumber>(L, 1)));
		return 1;
	}
	DEFINE_METHOD( GetEarnedExtraStage,			m_EarnedExtraStage )
	static int GaveUp( T* p, lua_State *L )		{ LuaHelpers::Push( L, p->m_bGaveUp ); return 1; }

	LunaStageStats()
	{
		ADD_METHOD( GetPlayerStageStats );
		ADD_METHOD( GetMultiPlayerStageStats );
		ADD_METHOD( GetPlayedSongs );
		ADD_METHOD( GetPossibleSongs );
		ADD_METHOD( GetGameplaySeconds );
		ADD_METHOD( OnePassed );
		ADD_METHOD( AllFailed );
		ADD_METHOD( GetStage );
		ADD_METHOD( GaveUp );
		ADD_METHOD( GetStageIndex );
		ADD_METHOD( GetStepsSeconds );
		ADD_METHOD( PlayerHasHighScore );
		ADD_METHOD( GetEarnedExtraStage );
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

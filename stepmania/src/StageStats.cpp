#include "global.h"
#include "StageStats.h"
#include "GameState.h"
#include "Foreach.h"
#include "Steps.h"
#include "song.h"
#include "RageLog.h"
#include "PrefsManager.h"
#include "PlayerState.h"
#include "Style.h"
#include "Profile.h"
#include "ProfileManager.h"

/*
 * Arcade:	for the current stage (one song).  
 * Nonstop/Oni/Endless:	 for current course (which usually contains multiple songs)
 */

StageStats::StageStats()
{
	m_playMode = PlayMode_Invalid;
	m_pStyle = NULL;
	m_vpPlayedSongs.clear();
	m_vpPossibleSongs.clear();
	m_StageType = Stage_Invalid;
	m_bGaveUp = false;
	m_bUsedAutoplay = false;
	m_fGameplaySeconds = 0;
	m_fStepsSeconds = 0;
	m_fMusicRate = 1;
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
	ASSERT( m_player[pn].m_vpPlayedSteps.size() != 0 );
	ASSERT( m_player[pn].m_vpPlayedSteps[0] );
	ASSERT_M( m_playMode < NUM_PlayMode, ssprintf("playmode %i", m_playMode) );
	ASSERT( m_pStyle != NULL );
	ASSERT_M( m_player[pn].m_vpPlayedSteps[0]->GetDifficulty() < NUM_Difficulty, ssprintf("difficulty %i", m_player[pn].m_vpPlayedSteps[0]->GetDifficulty()) );
	ASSERT( m_vpPlayedSongs.size() == m_player[pn].m_vpPlayedSteps.size() );
	ASSERT( m_vpPossibleSongs.size() == m_player[pn].m_vpPossibleSteps.size() );
}

void StageStats::AssertValid( MultiPlayer pn ) const
{
	ASSERT( m_vpPlayedSongs.size() != 0 );
	ASSERT( m_vpPossibleSongs.size() != 0 );
	if( m_vpPlayedSongs[0] )
		CHECKPOINT_M( m_vpPlayedSongs[0]->GetTranslitFullTitle() );
	ASSERT( m_multiPlayer[pn].m_vpPlayedSteps.size() != 0 );
	ASSERT( m_multiPlayer[pn].m_vpPlayedSteps[0] );
	ASSERT_M( m_playMode < NUM_PlayMode, ssprintf("playmode %i", m_playMode) );
	ASSERT( m_pStyle != NULL );
	ASSERT_M( m_player[pn].m_vpPlayedSteps[0]->GetDifficulty() < NUM_Difficulty, ssprintf("difficulty %i", m_player[pn].m_vpPlayedSteps[0]->GetDifficulty()) );
	ASSERT( m_vpPlayedSongs.size() == m_player[pn].m_vpPlayedSteps.size() );
	ASSERT( m_vpPossibleSongs.size() == m_player[pn].m_vpPossibleSteps.size() );
}


int StageStats::GetAverageMeter( PlayerNumber pn ) const
{
	AssertValid( pn );

	// TODO: This isn't correct for courses.
	
	int iTotalMeter = 0;

	for( unsigned i=0; i<m_vpPlayedSongs.size(); i++ )
	{
		const Steps* pSteps = m_player[pn].m_vpPlayedSteps[i];
		iTotalMeter += pSteps->GetMeter();
	}
	return iTotalMeter / m_vpPlayedSongs.size();	// round down
}

void StageStats::AddStats( const StageStats& other )
{
	ASSERT( !other.m_vpPlayedSongs.empty() );
	FOREACH_CONST( Song*, other.m_vpPlayedSongs, s )
		m_vpPlayedSongs.push_back( *s );
	FOREACH_CONST( Song*, other.m_vpPossibleSongs, s )
		m_vpPossibleSongs.push_back( *s );
	m_StageType = Stage_Invalid; // meaningless

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
	FOREACH_CONST( Song*, m_vpPossibleSongs, s )
		fSecs += (*s)->GetStepsSeconds();
	return fSecs / m_fMusicRate;
}

void StageStats::CommitScores( bool bSummary )
{
	switch( GAMESTATE->m_PlayMode )
	{
	case PLAY_MODE_BATTLE:
	case PLAY_MODE_RAVE:
		return; /* don't save scores in battle */
	}

	if( PREFSMAN->m_bScreenTestMode )
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

	FOREACH_HumanPlayer( p )
	{
		// don't save scores if the player is disqualified
		if( GAMESTATE->IsDisqualified(p) )
			continue;

		// whether or not to save scores when the stage was failed
		// depends on if this is a course or not ... it's handled
		// below in the switch

		HighScore &hs = m_player[p].m_HighScore;
		hs.SetName( RANKING_TO_FILL_IN_MARKER[p] );
		hs.SetGrade( m_player[p].GetGrade() );
		hs.SetScore( m_player[p].m_iScore );
		hs.SetPercentDP( m_player[p].GetPercentDancePoints() );
		hs.SetSurviveSeconds( m_player[p].m_fAliveSeconds );
		hs.SetModifiers( GAMESTATE->m_pPlayerState[p]->m_PlayerOptions.GetStage().GetString() );
		hs.SetDateTime( DateTime::GetNowDateTime() );
		hs.SetPlayerGuid( PROFILEMAN->IsPersistentProfile(p) ? PROFILEMAN->GetProfile(p)->m_sGuid : RString("") );
		hs.SetMachineGuid( PROFILEMAN->GetMachineProfile()->m_sGuid );
		hs.SetProductID( PREFSMAN->m_iProductID );
		FOREACH_TapNoteScore( tns )
			hs.SetTapNoteScore( tns, m_player[p].m_iTapNoteScores[tns] );
		FOREACH_HoldNoteScore( hns )
			hs.SetHoldNoteScore( hns, m_player[p].m_iHoldNoteScores[hns] );
		hs.SetRadarValues( m_player[p].m_radarActual );
		hs.SetLifeRemainingSeconds( m_player[p].m_fLifeRemainingSeconds );
	}

	FOREACH_HumanPlayer( p )
	{
		const HighScore &hs = m_player[p].m_HighScore;
		StepsType st = GAMESTATE->GetCurrentStyle()->m_StepsType;

		const Song* pSong = GAMESTATE->m_pCurSong;
		const Steps* pSteps = GAMESTATE->m_pCurSteps[p];

		if( bSummary )
		{
			// don't save scores if any stage was failed
			if( m_player[p].m_bFailed ) 
				continue;

			int iAverageMeter = GetAverageMeter(p);
			m_player[p].m_rc = AverageMeterToRankingCategory( iAverageMeter );

			PROFILEMAN->AddCategoryScore( st, m_player[p].m_rc, p, hs, m_player[p].m_iPersonalHighScoreIndex, m_player[p].m_iMachineHighScoreIndex );
			
			// TRICKY:  Increment play count here, and not on ScreenGameplay like the others.
			PROFILEMAN->IncrementCategoryPlayCount( st, m_player[p].m_rc, p );
		}
		else if( GAMESTATE->IsCourseMode() )
		{
			Course* pCourse = GAMESTATE->m_pCurCourse;
			ASSERT( pCourse );
			Trail* pTrail = GAMESTATE->m_pCurTrail[p];

			PROFILEMAN->AddCourseScore( pCourse, pTrail, p, hs, m_player[p].m_iPersonalHighScoreIndex, m_player[p].m_iMachineHighScoreIndex );
		}
		else
		{
			// don't save scores for a failed song
			if( m_player[p].m_bFailed )
				continue;

			ASSERT( pSteps );

			PROFILEMAN->AddStepsScore( pSong, pSteps, p, hs, m_player[p].m_iPersonalHighScoreIndex, m_player[p].m_iMachineHighScoreIndex );
		}
	}

	LOG->Trace( "done saving stats and high scores" );

	// If both players get a machine high score in the same HighScoreList,
	// then one player's score may have bumped the other player.  Look in 
	// the HighScoreList and re-get the high score index.
	FOREACH_HumanPlayer( p )
	{
		if( m_player[p].m_iMachineHighScoreIndex == -1 )	// no record
			continue;	// skip

		HighScore &hs = m_player[p].m_HighScore;
		Profile* pProfile = PROFILEMAN->GetMachineProfile();
		StepsType st = GAMESTATE->GetCurrentStyle()->m_StepsType;

		const HighScoreList *pHSL = NULL;
		if( bSummary )
		{
			pHSL = &pProfile->GetCategoryHighScoreList( st, m_player[p].m_rc );
		}
		else if( GAMESTATE->IsCourseMode() )
		{
			Course* pCourse = GAMESTATE->m_pCurCourse;
			ASSERT( pCourse );
			Trail *pTrail = GAMESTATE->m_pCurTrail[p];
			ASSERT( pTrail );
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


}

// lua start
#include "LuaBinding.h"

class LunaStageStats: public Luna<StageStats>
{
public:
	static int GetPlayerStageStats( T* p, lua_State *L )		{ p->m_player[Enum::Check<PlayerNumber>(L, 1)].PushSelf(L); return 1; }
	static int GetMultiPlayerStageStats( T* p, lua_State *L )	{ p->m_multiPlayer[Enum::Check<MultiPlayer>(L, 1)].PushSelf(L); return 1; }
	static int GetGameplaySeconds( T* p, lua_State *L )		{ lua_pushnumber(L, p->m_fGameplaySeconds); return 1; }
	static int OnePassed( T* p, lua_State *L )			{ lua_pushboolean(L, p->OnePassed()); return 1; }
	static int AllFailed( T* p, lua_State *L )			{ lua_pushboolean(L, p->AllFailed()); return 1; }

	LunaStageStats()
	{
		ADD_METHOD( GetPlayerStageStats );
		ADD_METHOD( GetMultiPlayerStageStats );
		ADD_METHOD( GetGameplaySeconds );
		ADD_METHOD( OnePassed );
		ADD_METHOD( AllFailed );
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

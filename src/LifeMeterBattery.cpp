#include "global.h"
#include "LifeMeterBattery.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "Steps.h"
#include "PlayerState.h"
#include "Course.h"
#include "ActorUtil.h"

#include <cmath>

LifeMeterBattery::LifeMeterBattery()
{
	m_iLivesLeft= 4;
	m_iTrailingLivesLeft = m_iLivesLeft;

	m_soundGainLife.Load( THEME->GetPathS("LifeMeterBattery","gain") );
	m_soundLoseLife.Load( THEME->GetPathS("LifeMeterBattery","lose"),true );
}

void LifeMeterBattery::Load( const PlayerState *pPlayerState, PlayerStageStats *pPlayerStageStats )
{
	LifeMeter::Load( pPlayerState, pPlayerStageStats );

	m_iLivesLeft = m_pPlayerState->m_PlayerOptions.GetStage().m_BatteryLives;
	m_iTrailingLivesLeft = m_iLivesLeft;

	const RString sType = "LifeMeterBattery";
	PlayerNumber pn = pPlayerState->m_PlayerNumber;

	MIN_SCORE_TO_KEEP_LIFE.Load(sType, "MinScoreToKeepLife");
	MAX_LIVES.Load(sType, "MaxLives");
	DANGER_THRESHOLD.Load(sType, "DangerThreshold");
	SUBTRACT_LIVES.Load(sType, "SubtractLives");
	MINES_SUBTRACT_LIVES.Load(sType, "MinesSubtractLives");
	HELD_ADD_LIVES.Load(sType, "HeldAddLives");
	LET_GO_SUBTRACT_LIVES.Load(sType, "LetGoSubtractLives");
	COURSE_SONG_REWARD_LIVES.Load(sType, "CourseSongRewardLives");

	LIVES_FORMAT.Load(sType, "NumLivesFormat");

	bool bPlayerEnabled = GAMESTATE->IsPlayerEnabled( pPlayerState );
	LuaThreadVariable var( "Player", LuaReference::Create(m_pPlayerState->m_PlayerNumber) );

	m_sprFrame.Load( THEME->GetPathG(sType,"frame") );
	this->AddChild( m_sprFrame );

	m_sprBattery.Load( THEME->GetPathG(sType,"lives") );
	m_sprBattery->SetName( ssprintf("BatteryP%i",int(pn+1)) );
	if( bPlayerEnabled )
	{
		ActorUtil::LoadAllCommandsAndSetXY( m_sprBattery, sType );
		this->AddChild( m_sprBattery );
	}

	m_textNumLives.LoadFromFont( THEME->GetPathF(sType, "lives") );
	m_textNumLives.SetName( ssprintf("NumLivesP%i",int(pn+1)) );
	if( bPlayerEnabled )
	{
		ActorUtil::LoadAllCommandsAndSetXY( m_textNumLives, sType );
		this->AddChild( &m_textNumLives );
	}
	// old hardcoded commands:
	/*
	m_sprFrame.SetZoomX( pn==PLAYER_1 ? 1.0f : -1.0f );
	m_sprBattery.SetZoomX( pn==PLAYER_1 ? 1.0f : -1.0f );
	m_sprBattery.SetX( BATTERY_X[pn] );
	m_textNumLives.SetX( NUM_X[pn] );
	m_textNumLives.SetY( NUM_Y );
	*/

	if( bPlayerEnabled )
	{
		m_Percent.Load( pPlayerState, pPlayerStageStats, "LifeMeterBattery Percent", true );
		// old hardcoded commands (this is useful, but let the themer decide
		// what they want to do, please -aj)
		//m_Percent.SetZoomX( pn==PLAYER_1 ? 1.0f : -1.0f );
		this->AddChild( &m_Percent );
	}

	Refresh();
}

void LifeMeterBattery::OnSongEnded()
{
	if( m_pPlayerStageStats->m_bFailed || m_iLivesLeft == 0 )
		return;

	if( m_iLivesLeft < m_pPlayerState->m_PlayerOptions.GetSong().m_BatteryLives )
	{
		m_iTrailingLivesLeft = m_iLivesLeft;
		PlayerNumber pn = m_pPlayerState->m_PlayerNumber;
		const Course *pCourse = GAMESTATE->m_pCurCourse;

		if( pCourse && pCourse->m_vEntries[GAMESTATE->GetCourseSongIndex()].iGainLives > -1 )
			m_iLivesLeft += pCourse->m_vEntries[GAMESTATE->GetCourseSongIndex()].iGainLives;
		else
		{
			Lua *L= LUA->Get();
			COURSE_SONG_REWARD_LIVES.PushSelf(L);
			PushSelf(L);
			LuaHelpers::Push(L, pn);
			RString error= "Error running CourseSongRewardLives callback: ";
			LuaHelpers::RunScriptOnStack(L, error, 2, 1, true);
			m_iLivesLeft += (int)luaL_optnumber(L, -1, 0);
			lua_settop(L, 0);
			LUA->Release(L);
		}
		m_iLivesLeft = std::min( m_iLivesLeft, m_pPlayerState->m_PlayerOptions.GetSong().m_BatteryLives );

		if( m_iTrailingLivesLeft < m_iLivesLeft )
			m_soundGainLife.Play(false);
	}

	Refresh();
}

void LifeMeterBattery::SubtractLives( int iLives )
{
	if( iLives <= 0 )
		return;

	m_iTrailingLivesLeft = m_iLivesLeft;
	m_iLivesLeft -= iLives;
	m_soundLoseLife.Play(false);
	m_textNumLives.PlayCommand("LoseLife");

	Refresh();
}

void LifeMeterBattery::AddLives( int iLives )
{
	if( iLives <= 0 )
		return;
	if( MAX_LIVES != 0 && m_iLivesLeft >= MAX_LIVES )
		return;

	m_iTrailingLivesLeft = m_iLivesLeft;
	m_iLivesLeft += iLives;
	m_soundGainLife.Play(false);
	m_textNumLives.PlayCommand("GainLife");

	Refresh();
}

void LifeMeterBattery::ChangeLives(int iLifeDiff)
{
	if( iLifeDiff < 0 )
		SubtractLives( std::abs(iLifeDiff) );
	else if( iLifeDiff > 0 )
		AddLives(iLifeDiff);
}

void LifeMeterBattery::ChangeLife( TapNoteScore score )
{
	if( m_iLivesLeft == 0 )
		return;

	bool bSubtract = false;
	// this probably doesn't handle hold checkpoints. -aj
	if( score == TNS_HitMine && MINES_SUBTRACT_LIVES > 0 )
	{
		SubtractLives(MINES_SUBTRACT_LIVES);
		bSubtract = true;
	}
	else
	{
		if( score < MIN_SCORE_TO_KEEP_LIFE && score > TNS_CheckpointMiss && SUBTRACT_LIVES > 0 )
		{
			SubtractLives(SUBTRACT_LIVES);
			bSubtract = true;
		}
	}
	BroadcastLifeChanged(bSubtract);
}

void LifeMeterBattery::ChangeLife( HoldNoteScore score, TapNoteScore tscore )
{
	if( m_iLivesLeft == 0 )
		return;

	bool bSubtract = false;
	if( score == HNS_Held && HELD_ADD_LIVES > 0 )
		AddLives(HELD_ADD_LIVES);
	if( score == HNS_LetGo && LET_GO_SUBTRACT_LIVES > 0 )
	{
		SubtractLives(LET_GO_SUBTRACT_LIVES);
		bSubtract = true;
	}
	BroadcastLifeChanged(bSubtract);
}

void LifeMeterBattery::SetLife(float value)
{
	int new_lives= static_cast<int>(value);
	bool lost= (new_lives < m_iLivesLeft);
	m_iLivesLeft= new_lives;
	BroadcastLifeChanged(lost);
}

void LifeMeterBattery::BroadcastLifeChanged(bool lost_life)
{
	Message msg("LifeChanged");
	msg.SetParam("Player", m_pPlayerState->m_PlayerNumber);
	msg.SetParam("LifeMeter", LuaReference::CreateFromPush(*this));
	msg.SetParam("LivesLeft", GetLivesLeft());
	msg.SetParam("LostLife", lost_life);
	MESSAGEMAN->Broadcast(msg);
}

void LifeMeterBattery::HandleTapScoreNone()
{
	// do nothing
}

void LifeMeterBattery::ChangeLife( float fDeltaLifePercent )
{
}

bool LifeMeterBattery::IsInDanger() const
{
	return m_iLivesLeft < DANGER_THRESHOLD;
}

bool LifeMeterBattery::IsHot() const
{
	return m_iLivesLeft == m_pPlayerState->m_PlayerOptions.GetSong().m_BatteryLives;
}

bool LifeMeterBattery::IsFailing() const
{
	return m_iLivesLeft == 0;
}

float LifeMeterBattery::GetLife() const
{
	if(!m_pPlayerState->m_PlayerOptions.GetSong().m_BatteryLives)
		return 1;

	return float(m_iLivesLeft) / m_pPlayerState->m_PlayerOptions.GetSong().m_BatteryLives;
}
int LifeMeterBattery::GetRemainingLives() const
{
	if( !m_pPlayerState->m_PlayerOptions.GetSong().m_BatteryLives )
		return 1;

	return m_iLivesLeft;
}
void LifeMeterBattery::Refresh()
{
	m_textNumLives.SetText( ssprintf(LIVES_FORMAT.GetValue(), m_iLivesLeft) );
	if( m_iLivesLeft < 0 )
	{
		// hide text to avoid showing -1
		m_textNumLives.SetVisible(false);
	}
	//update m_sprBattery
}

int LifeMeterBattery::GetTotalLives()
{
	return m_pPlayerState->m_PlayerOptions.GetSong().m_BatteryLives;
}

void LifeMeterBattery::Update( float fDeltaTime )
{
	LifeMeter::Update( fDeltaTime );
}

// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the LifeMeterBattery. */
class LunaLifeMeterBattery: public Luna<LifeMeterBattery>
{
public:
	static int GetLivesLeft( T* p, lua_State *L )	{ lua_pushnumber( L, p->GetLivesLeft() ); return 1; }
	static int GetTotalLives( T* p, lua_State *L )	{ lua_pushnumber(L, p->GetTotalLives()); return 1; }
	static int ChangeLives( T* p, lua_State *L )	{ p->ChangeLives(IArg(1)); COMMON_RETURN_SELF; }

	LunaLifeMeterBattery()
	{
		ADD_METHOD( GetLivesLeft );
		ADD_METHOD( GetTotalLives );
		ADD_METHOD( ChangeLives );
	}
};

LUA_REGISTER_DERIVED_CLASS( LifeMeterBattery, LifeMeter )

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

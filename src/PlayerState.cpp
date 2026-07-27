#include "global.h"
#include "PlayerState.h"
#include "GameState.h"
#include "RageLog.h"
#include "RadarValues.h"
#include "Steps.h"
#include "Song.h"

#include <vector>


PlayerState::PlayerState()
{
	m_PlayerNumber = PLAYER_INVALID;
	m_mp = MultiPlayer_Invalid;
	Reset();
}

void PlayerState::Reset()
{
	m_NotefieldZoom= 1.0f;
	m_PlayerOptions.Init();

	m_fLastDrawnBeat = -100;

	m_HealthState = HealthState_Alive;

	m_PlayerController = PC_HUMAN;

	m_iCpuSkill = 5;

	m_iLastPositiveSumOfAttackLevels = 0;
	m_fSecondsUntilAttacksPhasedOut = 0;
	m_bAttackBeganThisUpdate = false;
	m_bAttackEndedThisUpdate = false;
	m_ActiveAttacks.clear();
	m_ModsToApply.clear();

	m_iTapsHitSinceLastHasteUpdate = 0;
	m_iTapsMissedSinceLastHasteUpdate = 0;

	m_fSuperMeter = 0;	// between 0 and NUM_ATTACK_LEVELS
	m_fSuperMeterGrowthScale = 1;

	for( int i=0; i<NUM_INVENTORY_SLOTS; i++ )
		m_Inventory[i].MakeBlank();

}

void PlayerState::Update( float fDelta )
{
	// TRICKY: GAMESTATE->Update is run before any of the Screen update's,
	// so we'll clear these flags here and let them get turned on later
	m_bAttackBeganThisUpdate = false;
	m_bAttackEndedThisUpdate = false;

	bool bRebuildPlayerOptions = false;

	// See if any delayed attacks are starting or ending.
	for( unsigned s=0; s<m_ActiveAttacks.size(); s++ )
	{
		Attack &attack = m_ActiveAttacks[s];

		// You must add sattack by calling GameState::LaunchAttack,
		// or else the sentinel value won't be
		// converted into the current music time.
		ASSERT( attack.fStartSecond != ATTACK_STARTS_NOW );

		bool bCurrentlyEnabled =
			attack.bGlobal ||
			( attack.fStartSecond < m_Position.m_fMusicSeconds &&
			m_Position.m_fMusicSeconds < attack.fStartSecond+attack.fSecsRemaining );

		if( m_ActiveAttacks[s].bOn == bCurrentlyEnabled )
			continue; // OK

		if( m_ActiveAttacks[s].bOn && !bCurrentlyEnabled )
			m_bAttackEndedThisUpdate = true;
		else if( !m_ActiveAttacks[s].bOn && bCurrentlyEnabled )
			m_bAttackBeganThisUpdate = true;

		bRebuildPlayerOptions = true;

		m_ActiveAttacks[s].bOn = bCurrentlyEnabled;
	}

	if( bRebuildPlayerOptions )
		RebuildPlayerOptionsFromActiveAttacks();

	// Update after enabling attacks, so we approach the new state.
	m_PlayerOptions.Update( fDelta );

	if( m_fSecondsUntilAttacksPhasedOut > 0 )
		m_fSecondsUntilAttacksPhasedOut = std::max( 0.0f, m_fSecondsUntilAttacksPhasedOut - fDelta );
}

void PlayerState::SetPlayerNumber(PlayerNumber pn)
{
	m_PlayerNumber = pn;
	FOREACH_ENUM(ModsLevel, ml)
	{
		m_PlayerOptions.Get(ml).m_pn= pn;
	}
}

void PlayerState::ResetToDefaultPlayerOptions( ModsLevel l )
{
	PlayerOptions po;
	GAMESTATE->GetDefaultPlayerOptions( po );
	m_PlayerOptions.Assign( l, po );
}

/* This is called to launch an attack, or to queue an attack if a.fStartSecond
 * is set.  This is also called by GameState::Update when activating a queued attack. */
void PlayerState::LaunchAttack( const Attack& a )
{
	LOG->Trace( "Launch attack '%s' against P%d at %f", a.sModifiers.c_str(), m_PlayerNumber+1, a.fStartSecond );

	Attack attack = a;

	/* If fStartSecond is the sentinel, it means "launch as soon as possible". For m_ActiveAttacks,
	 * mark the real time it's starting (now), so Update() can know when the attack
	 * started so it can be removed later.  For m_ModsToApply, leave the sentinel in,
	 * so Player::Update knows to apply attack transforms correctly. (yuck) */
	m_ModsToApply.push_back( attack );
	if( attack.fStartSecond == ATTACK_STARTS_NOW )
		attack.fStartSecond = m_Position.m_fMusicSeconds;
	m_ActiveAttacks.push_back( attack );

	RebuildPlayerOptionsFromActiveAttacks();
}

void PlayerState::RemoveActiveAttacks( AttackLevel al )
{
	for( unsigned s=0; s<m_ActiveAttacks.size(); s++ )
	{
		if( al != NUM_ATTACK_LEVELS && al != m_ActiveAttacks[s].level )
			continue;
		m_ActiveAttacks.erase( m_ActiveAttacks.begin()+s, m_ActiveAttacks.begin()+s+1 );
		--s;
	}
	RebuildPlayerOptionsFromActiveAttacks();
}

void PlayerState::EndActiveAttacks()
{
	for (Attack &a : m_ActiveAttacks)
		a.fSecsRemaining = 0;
}

void PlayerState::RemoveAllInventory()
{
	for( int s=0; s<NUM_INVENTORY_SLOTS; s++ )
	{
		m_Inventory[s].fSecsRemaining = 0;
		m_Inventory[s].sModifiers = "";
	}
}

void PlayerState::RebuildPlayerOptionsFromActiveAttacks()
{
	// rebuild player options
	PlayerOptions po = m_PlayerOptions.GetStage();
	SongOptions so = GAMESTATE->m_SongOptions.GetStage();
	for( unsigned s=0; s<m_ActiveAttacks.size(); s++ )
	{
		if( !m_ActiveAttacks[s].bOn )
			continue; /* hasn't started yet */
		po.FromString( m_ActiveAttacks[s].sModifiers );
		so.FromString( m_ActiveAttacks[s].sModifiers );
	}
	m_PlayerOptions.Assign( ModsLevel_Song, po );
	if( m_PlayerNumber == GAMESTATE->GetMasterPlayerNumber() )
		GAMESTATE->m_SongOptions.Assign( ModsLevel_Song, so );

	int iSumOfAttackLevels = GetSumOfActiveAttackLevels();
	if( iSumOfAttackLevels > 0 )
	{
		m_iLastPositiveSumOfAttackLevels = iSumOfAttackLevels;
		m_fSecondsUntilAttacksPhasedOut = 10000;	// any positive number that won't run out before the attacks
	}
	else
	{
		// don't change!  m_iLastPositiveSumOfAttackLevels[p] = iSumOfAttackLevels;
		m_fSecondsUntilAttacksPhasedOut = 2;	// 2 seconds to phase out
	}
}

int PlayerState::GetSumOfActiveAttackLevels() const
{
	int iSum = 0;

	for( unsigned s=0; s<m_ActiveAttacks.size(); s++ )
		if( m_ActiveAttacks[s].fSecsRemaining > 0 && m_ActiveAttacks[s].level != NUM_ATTACK_LEVELS )
			iSum += m_ActiveAttacks[s].level;

	return iSum;
}

const SongPosition &PlayerState::GetDisplayedPosition() const
{
	if( GAMESTATE->m_bIsUsingStepTiming )
		return m_Position;
	return GAMESTATE->m_Position;
}

const TimingData &PlayerState::GetDisplayedTiming() const
{
	Steps *steps = GAMESTATE->m_pCurSteps[m_PlayerNumber];
	if( steps == nullptr )
		return GAMESTATE->m_pCurSong->m_SongTiming;
	return *steps->GetTimingData();
}


// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the PlayerState. */
class LunaPlayerState: public Luna<PlayerState>
{
public:
	static int ApplyPreferredOptionsToOtherLevels(T* p, lua_State* L)
	{
		p->m_PlayerOptions.Assign(ModsLevel_Preferred,
			p->m_PlayerOptions.Get(ModsLevel_Preferred));
		return 0;
	}
	DEFINE_METHOD( GetPlayerNumber, m_PlayerNumber );
	static int GetSongPosition( T* p, lua_State *L )
	{
		p->m_Position.PushSelf(L);
		return 1;
	}
	DEFINE_METHOD( GetMultiPlayerNumber, m_mp );
	DEFINE_METHOD( GetPlayerController, m_PlayerController );
	static int SetPlayerOptions( T* p, lua_State *L )
	{
		ModsLevel m = Enum::Check<ModsLevel>( L, 1 );
		PlayerOptions po;
		po.FromString( SArg(2) );
		p->m_PlayerOptions.Assign( m, po );
		return 0;
	}
	static int GetPlayerOptions( T* p, lua_State *L )
	{
		ModsLevel m = Enum::Check<ModsLevel>( L, 1 );
		p->m_PlayerOptions.Get(m).PushSelf(L);
		return 1;
	}
	static int GetPlayerOptionsArray( T* p, lua_State *L )
	{
		ModsLevel m = Enum::Check<ModsLevel>( L, 1 );
		std::vector<RString> s;
		p->m_PlayerOptions.Get(m).GetMods(s);
		LuaHelpers::CreateTableFromArray<RString>( s, L );
		return 1;
	}
	static int GetPlayerOptionsString( T* p, lua_State *L )
	{
		ModsLevel m = Enum::Check<ModsLevel>( L, 1 );
		RString s = p->m_PlayerOptions.Get(m).GetString();
		LuaHelpers::Push( L, s );
		return 1;
	}
	static int GetCurrentPlayerOptions( T* p, lua_State *L )
	{
		p->m_PlayerOptions.GetCurrent().PushSelf(L);
		return 1;
	}
	DEFINE_METHOD( GetHealthState, m_HealthState );
	DEFINE_METHOD( GetSuperMeterLevel, m_fSuperMeter );

	LunaPlayerState()
	{
		ADD_METHOD( ApplyPreferredOptionsToOtherLevels );
		ADD_METHOD( GetPlayerNumber );
		ADD_METHOD( GetMultiPlayerNumber );
		ADD_METHOD( GetPlayerController );
		ADD_METHOD( SetPlayerOptions );
		ADD_METHOD( GetPlayerOptions );
		ADD_METHOD( GetPlayerOptionsArray );
		ADD_METHOD( GetPlayerOptionsString );
		ADD_METHOD( GetCurrentPlayerOptions );
		ADD_METHOD( GetSongPosition );
		ADD_METHOD( GetHealthState );
		ADD_METHOD( GetSuperMeterLevel );
	}
};

LUA_REGISTER_CLASS( PlayerState )
// lua end

/*
 * (c) 2001-2004 Chris Danford, Chris Gomez
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

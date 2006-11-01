#include "global.h"
#include "PlayerState.h"
#include "GameState.h"
#include "RageLog.h"
#include "Foreach.h"

PlayerState::PlayerState()
{
	m_PlayerNumber = PLAYER_INVALID;
	m_mp = MultiPlayer_Invalid;
	Reset();
}

void PlayerState::Reset()
{
	m_PlayerOptions.Init();

	m_fLastDrawnBeat = -100;

	m_HealthState = ALIVE;

	m_PlayerController = PC_HUMAN;
	
	m_iCpuSkill = 5;

	m_iLastPositiveSumOfAttackLevels = 0;
	m_fSecondsUntilAttacksPhasedOut = 0;
	m_bAttackBeganThisUpdate = false;
	m_bAttackEndedThisUpdate = false;
	m_ActiveAttacks.clear();
	m_ModsToApply.clear();

	m_fSuperMeter = 0;	// between 0 and NUM_ATTACK_LEVELS
	m_fSuperMeterGrowthScale = 1;

	for( int i=0; i<NUM_INVENTORY_SLOTS; i++ )
		m_Inventory[i].MakeBlank();
}

void PlayerState::Update( float fDelta )
{
	m_PlayerOptions.Update( fDelta );

	// TRICKY: GAMESTATE->Update is run before any of the Screen update's,
	// so we'll clear these flags here and let them get turned on later
	m_bAttackBeganThisUpdate = false;
	m_bAttackEndedThisUpdate = false;

	bool bRebuildPlayerOptions = false;

	/* See if any delayed attacks are starting or ending. */
	for( unsigned s=0; s<m_ActiveAttacks.size(); s++ )
	{
		Attack &attack = m_ActiveAttacks[s];
		
		// -1 is the "starts now" sentinel value.  You must add the attack
		// by calling GameState::LaunchAttack, or else the -1 won't be 
		// converted into the current music time.  
		ASSERT( attack.fStartSecond != -1 );

		bool bCurrentlyEnabled =
			attack.bGlobal ||
			( attack.fStartSecond < GAMESTATE->m_fMusicSeconds &&
			GAMESTATE->m_fMusicSeconds < attack.fStartSecond+attack.fSecsRemaining );

		if( m_ActiveAttacks[s].bOn == bCurrentlyEnabled )
			continue; /* OK */

		if( m_ActiveAttacks[s].bOn && !bCurrentlyEnabled )
			m_bAttackEndedThisUpdate = true;
		else if( !m_ActiveAttacks[s].bOn && bCurrentlyEnabled )
			m_bAttackBeganThisUpdate = true;

		bRebuildPlayerOptions = true;

		m_ActiveAttacks[s].bOn = bCurrentlyEnabled;
	}

	if( bRebuildPlayerOptions )
		RebuildPlayerOptionsFromActiveAttacks();

	if( m_fSecondsUntilAttacksPhasedOut > 0 )
		m_fSecondsUntilAttacksPhasedOut = max( 0, m_fSecondsUntilAttacksPhasedOut - fDelta );
}

/* This is called to launch an attack, or to queue an attack if a.fStartSecond
 * is set.  This is also called by GameState::Update when activating a queued attack. */
void PlayerState::LaunchAttack( const Attack& a )
{
	LOG->Trace( "Launch attack '%s' against P%d at %f", a.sModifiers.c_str(), m_mp+1, a.fStartSecond );

	Attack attack = a;

	/* If fStartSecond is -1, it means "launch as soon as possible".  For m_ActiveAttacks,
	 * mark the real time it's starting (now), so Update() can know when the attack started
	 * so it can be removed later.  For m_ModsToApply, leave the -1 in, so Player::Update
	 * knows to apply attack transforms correctly.  (yuck) */
	m_ModsToApply.push_back( attack );
	if( attack.fStartSecond == -1 )
		attack.fStartSecond = GAMESTATE->m_fMusicSeconds;
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
	FOREACH( Attack, m_ActiveAttacks, a )
		a->fSecsRemaining = 0;
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
	for( unsigned s=0; s<m_ActiveAttacks.size(); s++ )
	{
		if( !m_ActiveAttacks[s].bOn )
			continue; /* hasn't started yet */
		po.FromString( m_ActiveAttacks[s].sModifiers );
	}
	m_PlayerOptions.Assign( ModsLevel_Song, po );


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

// lua start
#include "LuaBinding.h"

class LunaPlayerState: public Luna<PlayerState>
{
public:
	static int SetPlayerOptions( T* p, lua_State *L )
	{
		ModsLevel m = Enum::Check<ModsLevel>( L, 1 );

		PlayerOptions po;
		po.FromString( SArg(2) );
		p->m_PlayerOptions.Assign( m, po );
		return 0;
	}

	LunaPlayerState()
	{
		ADD_METHOD( SetPlayerOptions );
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

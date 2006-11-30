/* PlayerState - Holds per-player game state. */

#ifndef PlayerState_H
#define PlayerState_H

#include "Attack.h"
#include "ModsGroup.h"
#include "PlayerNumber.h"
#include "PlayerOptions.h"
#include "RageTimer.h"
struct lua_State;

class PlayerState
{
public:
	PlayerState();
	void Reset();
	void Update( float fDelta );

	// TODO: Remove use of PlayerNumber.  All data about the player should live 
	// in PlayerState and callers should not use PlayerNumber to index into 
	// GameState.
	PlayerNumber		m_PlayerNumber;
	MultiPlayer		m_mp;
	

	ModsGroup<PlayerOptions>	m_PlayerOptions;

	//
	// Used in Gameplay
	//
	mutable float		m_fLastDrawnBeat;	// Set by NoteField.  Used to push note-changing modifers back so that notes doesn't pop.

	enum HealthState { HOT, ALIVE, DANGER, DEAD };
	HealthState		m_HealthState;

	// Set to the MusicSeconds of the last note successfully strummed or hammered in a hammer chain
	float			m_fLastHopoNoteMusicSeconds;	// if -1, then there is no current hammer chain

	PlayerController	m_PlayerController;
	
	//
	// Used in Battle and Rave
	//
	void LaunchAttack( const Attack& a );
	void RemoveActiveAttacks( AttackLevel al=NUM_ATTACK_LEVELS /*all*/ );
	void EndActiveAttacks();
	void RebuildPlayerOptionsFromActiveAttacks();
	int GetSumOfActiveAttackLevels() const;
	int			m_iCpuSkill;	// only used when m_PlayerController is PC_CPU
	// Attacks take a while to transition out of use.  Account for this in PlayerAI
	// by still penalizing it for 1 second after the player options are rebuilt.
	int			m_iLastPositiveSumOfAttackLevels;
	float			m_fSecondsUntilAttacksPhasedOut;// positive means PlayerAI is still affected
	bool			m_bAttackBeganThisUpdate;	// flag for other objects to watch (play sounds)
	bool			m_bAttackEndedThisUpdate;	// flag for other objects to watch (play sounds)
	
	AttackArray		m_ActiveAttacks;
	vector<Attack>		m_ModsToApply;

	//
	// Used in Rave
	//
	float			m_fSuperMeter;	// between 0 and NUM_ATTACK_LEVELS
	float			m_fSuperMeterGrowthScale;

	//
	// Used in Battle
	//
	void RemoveAllInventory();
	Attack			m_Inventory[NUM_INVENTORY_SLOTS];

	// Lua
	void PushSelf( lua_State *L );
};

#endif

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

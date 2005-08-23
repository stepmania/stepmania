/* PlayerState - Holds per-player game state. */

#ifndef PlayerState_H
#define PlayerState_H

#include "PlayerNumber.h"
#include "PlayerOptions.h"
#include <map>
#include "Attack.h"

class PlayerState
{
public:
	PlayerState()
	{
		m_PlayerNumber = PLAYER_INVALID;
		m_mp = MultiPlayer_INVALID;
		Reset();
	}
	void Reset()
	{
		m_CurrentPlayerOptions.Init();
		m_PlayerOptions.Init();
		m_StoredPlayerOptions.Init();

		m_BeatToNoteSkin.clear();
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

	void ResetNoteSkins();
	void Update( float fDelta );

	// TODO: Remove use of PlayerNumber.  All data about the player should live 
	// in PlayerState and callers should not use PlayerNumber to index into 
	// GameState.
	PlayerNumber m_PlayerNumber;
	MultiPlayer m_mp;
	

	PlayerOptions	m_CurrentPlayerOptions;    // current approaches destination
	PlayerOptions	m_PlayerOptions;			// change this, and current will move gradually toward it
	PlayerOptions	m_StoredPlayerOptions;	// user's choices on the PlayerOptions screen

	//
	// Used in Gameplay
	//
	map<float,CString> m_BeatToNoteSkin;
	mutable float	m_fLastDrawnBeat; // Set by NoteField.  Used to push NoteSkin-changing modifers back so that the NoteSkin doesn't pop.

	enum HealthState { HOT, ALIVE, DANGER, DEAD };
	HealthState	m_HealthState;

	PlayerController	m_PlayerController;
	
	//
	// Used in Battle and Rave
	//
	void RebuildPlayerOptionsFromActiveAttacks();
	int GetSumOfActiveAttackLevels() const;
	int		m_iCpuSkill;	// only used when m_PlayerController is PC_CPU
	// Attacks take a while to transition out of use.  Account for this in PlayerAI
	// by still penalizing it for 1 second after the player options are rebuilt.
	int		m_iLastPositiveSumOfAttackLevels;
	float	m_fSecondsUntilAttacksPhasedOut;	// positive means PlayerAI is still affected
	bool	m_bAttackBeganThisUpdate;	// flag for other objects to watch (play sounds)
	bool	m_bAttackEndedThisUpdate;	// flag for other objects to watch (play sounds)
	AttackArray	m_ActiveAttacks;
	vector<Attack>	m_ModsToApply;

	//
	// Used in Rave
	//
	float	m_fSuperMeter;	// between 0 and NUM_ATTACK_LEVELS
	float	m_fSuperMeterGrowthScale;

	//
	// Used in Battle
	//
	Attack	m_Inventory[NUM_INVENTORY_SLOTS];
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

#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: RaveHelper

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "RaveHelper.h"
#include "ThemeManager.h"
#include "RageUtil.h"
#include "GameState.h"

CachedThemeMetricF ATTACK_DURATION_SECONDS	("RaveHelper","AttackDurationSeconds");

const PlayerNumber	OPPOSITE_PLAYER[NUM_PLAYERS] = { PLAYER_2, PLAYER_1 };


RaveHelper::RaveHelper()
{
	ATTACK_DURATION_SECONDS.Refresh();
}

void RaveHelper::Load( PlayerNumber pn )
{
	m_PlayerNumber = pn;

	// don't load sounds if they're not going to be used
	if( GAMESTATE->m_PlayMode == PLAY_MODE_RAVE )
	{
		for( int p=0; p<NUM_PLAYERS; p++ )
		{
			m_soundLaunchAttack.Load( THEME->GetPathTo("Sounds",ssprintf("RaveHelper launch attack p%d",p+1)) );
			m_soundAttackEnding.Load( THEME->GetPathTo("Sounds",ssprintf("RaveHelper attack end p%d",p+1)) );
		}
	}
}

void RaveHelper::Update( float fDelta )
{
	if( GAMESTATE->m_PlayMode != PLAY_MODE_RAVE )
		return;

	if( GAMESTATE->m_bActiveAttackEndedThisUpdate[m_PlayerNumber] )
		m_soundAttackEnding.Play();

//	PlayerNumber pn = m_PlayerNumber;

	// TODO: Award item based on Super meter
/*	
	// check to see if they deserve a new item
	if( GAMESTATE->m_CurStageStats.iCurCombo[pn] != m_iLastSeenCombo )
	{
		int iOldCombo = m_iLastSeenCombo;
		m_iLastSeenCombo = GAMESTATE->m_CurStageStats.iCurCombo[pn];
		int iNewCombo = m_iLastSeenCombo;
		
		int iLevelOfOldCombo = (iOldCombo/COMBO_PER_ATTACK_LEVEL) - 1;
		int iLevelOfNewCombo = (iNewCombo/COMBO_PER_ATTACK_LEVEL) - 1;

		if( iLevelOfOldCombo < iLevelOfNewCombo  &&  // combo increasing
			iLevelOfNewCombo >= ATTACK_LEVEL_1 )  // attack level not negative
		{
			// they deserve a new item
			CLAMP( iLevelOfNewCombo, 0, GAMESTATE->m_MaxAttackLevel[pn] );
			AttackLevel al = (AttackLevel)iLevelOfNewCombo;
			AwardItem( al );
		}
	}
	*/
}

void RaveHelper::LaunchAttack( AttackLevel al )
{
	CString* asAttacks = GAMESTATE->m_sAttacks[m_PlayerNumber][al];	// [NUM_ATTACKS_PER_LEVEL]
	CString sAttackToGive = asAttacks[ rand()%NUM_ATTACKS_PER_LEVEL ];
  	PlayerNumber pnToAttack = OPPOSITE_PLAYER[m_PlayerNumber];

	GameState::ActiveAttack aa;
	aa.fSecsRemaining = ATTACK_DURATION_SECONDS;
	aa.sModifier = sAttackToGive;

	GAMESTATE->LaunchAttack( pnToAttack, aa );
	GAMESTATE->RebuildPlayerOptionsFromActiveAttacks( pnToAttack );

	m_soundLaunchAttack.Play();
}

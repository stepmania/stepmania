#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScoreKeeperRave

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScoreKeeperRave.h"
#include "ThemeManager.h"
#include "RageUtil.h"
#include "GameState.h"
#include "Character.h"

CachedThemeMetricF ATTACK_DURATION_SECONDS	("ScoreKeeperRave","AttackDurationSeconds");

const PlayerNumber	OPPOSITE_PLAYER[NUM_PLAYERS] = { PLAYER_2, PLAYER_1 };


ScoreKeeperRave::ScoreKeeperRave(PlayerNumber pn) : ScoreKeeper(pn)
{
	ATTACK_DURATION_SECONDS.Refresh();

	m_soundLaunchAttack.Load( THEME->GetPathToS(ssprintf("ScoreKeeperRave launch attack p%d",pn+1)) );
	m_soundAttackEnding.Load( THEME->GetPathToS(ssprintf("ScoreKeeperRave attack end p%d",pn+1)) );
}

void ScoreKeeperRave::OnNextSong( int iSongInCourseIndex, Notes* pNotes )
{
	
}

#define CROSSED( val ) (fOld < val && fNew >= val)
#define CROSSED_ATTACK_LEVEL( level ) CROSSED(1.f/NUM_ATTACK_LEVELS*(level+1))
void ScoreKeeperRave::HandleTapRowScore( TapNoteScore scoreOfLastTap, int iNumTapsInRow )
{
	AttackLevel oldAL = (AttackLevel)(int)GAMESTATE->m_fSuperMeter[m_PlayerNumber];

	float fPercentToMove;
	switch( scoreOfLastTap )
	{
	case TNS_MARVELOUS:		fPercentToMove = +0.04f;	break;
	case TNS_PERFECT:		fPercentToMove = +0.04f;	break;
	case TNS_GREAT:			fPercentToMove = +0.02f;	break;
	case TNS_GOOD:			fPercentToMove = +0.00f;	break;
	case TNS_BOO:			fPercentToMove = -0.08f;	break;
	case TNS_MISS:			fPercentToMove = -0.16f;	break;
	default:	ASSERT(0);	fPercentToMove = +0.00f;	break;
	}

	GAMESTATE->m_fSuperMeter[m_PlayerNumber] += fPercentToMove * GAMESTATE->m_fSuperMeterGrowthScale[m_PlayerNumber];
	CLAMP( GAMESTATE->m_fSuperMeter[m_PlayerNumber], 0.f, NUM_ATTACK_LEVELS );

	AttackLevel newAL = (AttackLevel)(int)GAMESTATE->m_fSuperMeter[m_PlayerNumber];

	if( newAL > oldAL )
	{
		if( newAL == NUM_ATTACK_LEVELS )	// hit upper bounds of meter
			GAMESTATE->m_fSuperMeter[m_PlayerNumber] -= 1.f;
		LaunchAttack( oldAL );
	}
}

void ScoreKeeperRave::HandleHoldScore( HoldNoteScore holdScore, TapNoteScore tapScore )
{

}

void ScoreKeeperRave::Update( float fDelta )
{
	if( GAMESTATE->m_bActiveAttackEndedThisUpdate[m_PlayerNumber] )
		m_soundAttackEnding.Play();
}

void ScoreKeeperRave::LaunchAttack( AttackLevel al )
{
	CString* asAttacks = GAMESTATE->m_pCurCharacters[m_PlayerNumber]->m_sAttacks[al];	// [NUM_ATTACKS_PER_LEVEL]
	CString sAttackToGive = asAttacks[ rand()%NUM_ATTACKS_PER_LEVEL ];
  	PlayerNumber pnToAttack = OPPOSITE_PLAYER[m_PlayerNumber];

	GameState::Attack a;
	a.level = al;
	a.fSecsRemaining = ATTACK_DURATION_SECONDS;
	a.sModifier = sAttackToGive;

	GAMESTATE->LaunchAttack( pnToAttack, a );

	m_soundLaunchAttack.Play();
}

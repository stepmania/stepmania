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
#include "ScreenManager.h"
#include "PrefsManager.h"

CachedThemeMetricF ATTACK_DURATION_SECONDS	("ScoreKeeperRave","AttackDurationSeconds");


ScoreKeeperRave::ScoreKeeperRave(PlayerNumber pn) : ScoreKeeper(pn)
{
	ATTACK_DURATION_SECONDS.Refresh();

	m_soundLaunchAttack.Load( THEME->GetPathToS(ssprintf("ScoreKeeperRave launch attack p%d",pn+1)) );
	m_soundAttackEnding.Load( THEME->GetPathToS(ssprintf("ScoreKeeperRave attack end p%d",pn+1)) );
}

void ScoreKeeperRave::OnNextSong( int iSongInCourseIndex, const Steps* pNotes, const NoteData* pNoteData )
{
	
}

void ScoreKeeperRave::HandleTapScore( TapNoteScore score )
{
	float fPercentToMove = 0;
	switch( score )
	{
	case TNS_HIT_MINE:		fPercentToMove = PREFSMAN->m_fSuperMeterHitMinePercentChange;	break;
	}

	AddSuperMeterDelta( fPercentToMove );
}

#define CROSSED( val ) (fOld < val && fNew >= val)
#define CROSSED_ATTACK_LEVEL( level ) CROSSED(1.f/NUM_ATTACK_LEVELS*(level+1))
void ScoreKeeperRave::HandleTapRowScore( TapNoteScore scoreOfLastTap, int iNumTapsInRow )
{
	float fPercentToMove;
	switch( scoreOfLastTap )
	{
	case TNS_MARVELOUS:		fPercentToMove = PREFSMAN->m_fSuperMeterMarvelousPercentChange;	break;
	case TNS_PERFECT:		fPercentToMove = PREFSMAN->m_fSuperMeterPerfectPercentChange;	break;
	case TNS_GREAT:			fPercentToMove = PREFSMAN->m_fSuperMeterGreatPercentChange;		break;
	case TNS_GOOD:			fPercentToMove = PREFSMAN->m_fSuperMeterGoodPercentChange;		break;
	case TNS_BOO:			fPercentToMove = PREFSMAN->m_fSuperMeterBooPercentChange;		break;
	case TNS_MISS:			fPercentToMove = PREFSMAN->m_fSuperMeterMissPercentChange;		break;
	default:	ASSERT(0);	fPercentToMove = +0.00f;	break;
	}
	AddSuperMeterDelta( fPercentToMove );
}

void ScoreKeeperRave::HandleHoldScore( HoldNoteScore holdScore, TapNoteScore tapScore )
{
	float fPercentToMove = 0;
	switch( tapScore )
	{
	case TNS_HIT_MINE:		fPercentToMove = PREFSMAN->m_fSuperMeterHitMinePercentChange;	break;
	}
	AddSuperMeterDelta( fPercentToMove );
}

void ScoreKeeperRave::AddSuperMeterDelta( float fUnscaledPercentChange )
{
	if( PREFSMAN->m_bMercifulDrain  &&  fUnscaledPercentChange<0 )
	{
		float fSuperPercentage = GAMESTATE->m_fSuperMeter[m_PlayerNumber] / NUM_ATTACK_LEVELS;
		fUnscaledPercentChange *= SCALE( fSuperPercentage, 0.f, 1.f, 0.5f, 1.f);
	}

	// more mercy: Grow super meter slower or faster depending on life.
	if( PREFSMAN->m_bMercifulSuperMeter )
	{
		float fLifePercentage = 0;
		switch( m_PlayerNumber )
		{
		case PLAYER_1:	fLifePercentage = GAMESTATE->m_fTugLifePercentP1;		break;
		case PLAYER_2:	fLifePercentage = 1 - GAMESTATE->m_fTugLifePercentP1;	break;
		default:	ASSERT(0);
		}
		if( fUnscaledPercentChange > 0 )
			fUnscaledPercentChange *= SCALE( fLifePercentage, 0.f, 1.f, 1.8f, 0.2f);
		else	// fUnscaledPercentChange <= 0
			fUnscaledPercentChange *= SCALE( fLifePercentage, 0.f, 1.f, 1.8f, 0.2f);
	}


	AttackLevel oldAL = (AttackLevel)(int)GAMESTATE->m_fSuperMeter[m_PlayerNumber];

	float fPercentToMove = fUnscaledPercentChange;
	GAMESTATE->m_fSuperMeter[m_PlayerNumber] += fPercentToMove * GAMESTATE->m_fSuperMeterGrowthScale[m_PlayerNumber];
	CLAMP( GAMESTATE->m_fSuperMeter[m_PlayerNumber], 0.f, NUM_ATTACK_LEVELS );

	AttackLevel newAL = (AttackLevel)(int)GAMESTATE->m_fSuperMeter[m_PlayerNumber];

	if( newAL > oldAL )
	{
		LaunchAttack( oldAL );
		if( newAL == NUM_ATTACK_LEVELS )	// hit upper bounds of meter
			GAMESTATE->m_fSuperMeter[m_PlayerNumber] -= 1.f;
	}

	// mercy
	if( fUnscaledPercentChange < 0 )
		GAMESTATE->RemoveActiveAttacksForPlayer( m_PlayerNumber );
}


void ScoreKeeperRave::Update( float fDelta )
{
	if( GAMESTATE->m_bAttackEndedThisUpdate[m_PlayerNumber] )
		m_soundAttackEnding.Play();
}

void ScoreKeeperRave::LaunchAttack( AttackLevel al )
{
	CString* asAttacks = GAMESTATE->m_pCurCharacters[m_PlayerNumber]->m_sAttacks[al];	// [NUM_ATTACKS_PER_LEVEL]
	CString sAttackToGive;

	if (GAMESTATE->m_pCurCharacters[m_PlayerNumber] != NULL)		
		sAttackToGive = asAttacks[ rand()%NUM_ATTACKS_PER_LEVEL ];
	else
	{
		/* If you add any note skins here, you need to make sure they're cached, too. */
		CString DefaultAttacks[8] = { "1.5x", "2.0x", "0.5x", "reverse", "sudden", "boost", "brake", "wave" };
		sAttackToGive = DefaultAttacks[ rand()%8 ];
	}

  	PlayerNumber pnToAttack = OPPOSITE_PLAYER[m_PlayerNumber];

	Attack a;
	a.level = al;
	a.fSecsRemaining = ATTACK_DURATION_SECONDS;
	a.sModifier = sAttackToGive;

	// remove current attack (if any)
	GAMESTATE->RemoveActiveAttacksForPlayer( pnToAttack );

	// apply new attack
	GAMESTATE->LaunchAttack( pnToAttack, a );

//	SCREENMAN->SystemMessage( ssprintf( "attacking %d with %s", pnToAttack, sAttackToGive.c_str() ) );

	m_soundLaunchAttack.Play();
}

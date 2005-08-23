#include "global.h"
#include "ScoreKeeperRave.h"
#include "ThemeManager.h"
#include "RageUtil.h"
#include "GameState.h"
#include "Character.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "ThemeMetric.h"
#include "PlayerState.h"


ThemeMetric<float> ATTACK_DURATION_SECONDS	("ScoreKeeperRave","AttackDurationSeconds");


ScoreKeeperRave::ScoreKeeperRave( PlayerState* pPlayerState, PlayerStageStats* pPlayerStageStats ) : 
	ScoreKeeper(pPlayerState,pPlayerStageStats)
{
}

void ScoreKeeperRave::OnNextSong( int iSongInCourseIndex, const Steps* pSteps, const NoteData* pNoteData )
{
	
}

void ScoreKeeperRave::HandleTapScore( TapNoteScore score )
{
	float fPercentToMove = 0;
	switch( score )
	{
	case TNS_HIT_MINE:		fPercentToMove = PREFSMAN->m_fSuperMeterPercentChangeHitMine;	break;
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
	case TNS_MARVELOUS:		fPercentToMove = PREFSMAN->m_fSuperMeterPercentChangeMarvelous;	break;
	case TNS_PERFECT:		fPercentToMove = PREFSMAN->m_fSuperMeterPercentChangePerfect;	break;
	case TNS_GREAT:			fPercentToMove = PREFSMAN->m_fSuperMeterPercentChangeGreat;		break;
	case TNS_GOOD:			fPercentToMove = PREFSMAN->m_fSuperMeterPercentChangeGood;		break;
	case TNS_BOO:			fPercentToMove = PREFSMAN->m_fSuperMeterPercentChangeBoo;		break;
	case TNS_MISS:			fPercentToMove = PREFSMAN->m_fSuperMeterPercentChangeMiss;		break;
	default:	ASSERT(0);	fPercentToMove = +0.00f;	break;
	}
	AddSuperMeterDelta( fPercentToMove );
}

void ScoreKeeperRave::HandleHoldScore( HoldNoteScore holdScore, TapNoteScore tapScore )
{
	float fPercentToMove = 0;
	switch( tapScore )
	{
	case TNS_HIT_MINE:		fPercentToMove = PREFSMAN->m_fSuperMeterPercentChangeHitMine;	break;
	}
	AddSuperMeterDelta( fPercentToMove );
}

void ScoreKeeperRave::AddSuperMeterDelta( float fUnscaledPercentChange )
{
	if( PREFSMAN->m_bMercifulDrain  &&  fUnscaledPercentChange<0 )
	{
		float fSuperPercentage = m_pPlayerState->m_fSuperMeter / NUM_ATTACK_LEVELS;
		fUnscaledPercentChange *= SCALE( fSuperPercentage, 0.f, 1.f, 0.5f, 1.f);
	}

	// more mercy: Grow super meter slower or faster depending on life.
	if( PREFSMAN->m_bMercifulSuperMeter )
	{
		float fLifePercentage = 0;
		switch( m_pPlayerState->m_PlayerNumber )
		{
		case PLAYER_1:	fLifePercentage = GAMESTATE->m_fTugLifePercentP1;		break;
		case PLAYER_2:	fLifePercentage = 1 - GAMESTATE->m_fTugLifePercentP1;	break;
		default:	ASSERT(0);
		}
		CLAMP( fLifePercentage, 0.f, 1.f );
		if( fUnscaledPercentChange > 0 )
			fUnscaledPercentChange *= SCALE( fLifePercentage, 0.f, 1.f, 1.7f, 0.3f);
		else	// fUnscaledPercentChange <= 0
			fUnscaledPercentChange /= SCALE( fLifePercentage, 0.f, 1.f, 1.7f, 0.3f);
	}


	// mercy: drop super meter faster if at a higher level
	if( fUnscaledPercentChange < 0 )
		fUnscaledPercentChange *= SCALE( m_pPlayerState->m_fSuperMeter, 0.f, 1.f, 0.01f, 1.f );

	AttackLevel oldAL = (AttackLevel)(int)m_pPlayerState->m_fSuperMeter;

	float fPercentToMove = fUnscaledPercentChange;
	m_pPlayerState->m_fSuperMeter += fPercentToMove * m_pPlayerState->m_fSuperMeterGrowthScale;
	CLAMP( m_pPlayerState->m_fSuperMeter, 0.f, NUM_ATTACK_LEVELS );

	AttackLevel newAL = (AttackLevel)(int)m_pPlayerState->m_fSuperMeter;

	if( newAL > oldAL )
	{
		LaunchAttack( oldAL );
		if( newAL == NUM_ATTACK_LEVELS )	// hit upper bounds of meter
			m_pPlayerState->m_fSuperMeter -= 1.f;
	}

	// mercy: if losing remove attacks on life drain
	if( fUnscaledPercentChange < 0 )
	{
		bool bWinning;
		switch( m_pPlayerState->m_PlayerNumber )
		{
		case PLAYER_1:	bWinning = GAMESTATE->m_fTugLifePercentP1 > 0.5f;	break;
		case PLAYER_2:	bWinning = GAMESTATE->m_fTugLifePercentP1 < 0.5f;	break;
		default:	ASSERT(0);
		}
		if( !bWinning )
			GAMESTATE->EndActiveAttacksForPlayer( m_pPlayerState->m_PlayerNumber );
	}
}


void ScoreKeeperRave::LaunchAttack( AttackLevel al )
{
	PlayerNumber pn = m_pPlayerState->m_PlayerNumber;

	CString* asAttacks = GAMESTATE->m_pCurCharacters[pn]->m_sAttacks[al];	// [NUM_ATTACKS_PER_LEVEL]
	CString sAttackToGive;

	if (GAMESTATE->m_pCurCharacters[pn] != NULL)		
		sAttackToGive = asAttacks[ rand()%NUM_ATTACKS_PER_LEVEL ];
	else
	{
		/* If you add any note skins here, you need to make sure they're cached, too. */
		CString DefaultAttacks[8] = { "1.5x", "2.0x", "0.5x", "reverse", "sudden", "boost", "brake", "wave" };
		sAttackToGive = DefaultAttacks[ rand()%8 ];
	}

  	PlayerNumber pnToAttack = OPPOSITE_PLAYER[pn];

	Attack a;
	a.level = al;
	a.fSecsRemaining = ATTACK_DURATION_SECONDS;
	a.sModifiers = sAttackToGive;

	// remove current attack (if any)
	GAMESTATE->RemoveActiveAttacksForPlayer( pnToAttack );

	// apply new attack
	GAMESTATE->LaunchAttack( (MultiPlayer)pnToAttack, a );

//	SCREENMAN->SystemMessage( ssprintf( "attacking %d with %s", pnToAttack, sAttackToGive.c_str() ) );
}

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

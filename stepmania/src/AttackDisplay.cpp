#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: AttackDisplay

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "AttackDisplay.h"
#include "ThemeManager.h"
#include "GameState.h"

#define TEXT_ON_COMMAND( p )	THEME->GetMetric("AttackDisplay",ssprintf("TextP%dOnCommand",p+1))


AttackDisplay::AttackDisplay()
{
	if( GAMESTATE->m_PlayMode != PLAY_MODE_BATTLE &&
		GAMESTATE->m_PlayMode != PLAY_MODE_RAVE )
		return;

	m_textAttack.LoadFromFont( THEME->GetPathToF("AttackDisplay") );
	m_textAttack.SetDiffuseAlpha( 0 );	// invisible
	this->AddChild( &m_textAttack );
}

void AttackDisplay::Update( float fDelta )
{
	ActorFrame::Update( fDelta );

	if( GAMESTATE->m_PlayMode != PLAY_MODE_BATTLE &&
		GAMESTATE->m_PlayMode != PLAY_MODE_RAVE )
		return;

	if( !GAMESTATE->m_bAttackBeganThisUpdate[m_PlayerNumber] )
		return;
	// don't handle this again
	GAMESTATE->m_bAttackBeganThisUpdate[m_PlayerNumber] = false;

	for( unsigned s=0; s<GAMESTATE->m_ActiveAttacks[m_PlayerNumber].size(); s++ )
	{
		if( GAMESTATE->m_ActiveAttacks[m_PlayerNumber][s].fStartSecond >= 0 )
			continue; /* hasn't started yet */

		if( GAMESTATE->m_ActiveAttacks[m_PlayerNumber][s].fSecsRemaining <= 0 )
			continue; /* ended already */

		if( GAMESTATE->m_ActiveAttacks[m_PlayerNumber][s].IsBlank() )
			continue;

		CString sText = GAMESTATE->m_ActiveAttacks[m_PlayerNumber][s].sModifier;

		m_textAttack.SetDiffuseAlpha( 1 );
		m_textAttack.SetText( sText );
		m_textAttack.Command( TEXT_ON_COMMAND(m_PlayerNumber) );

		break;
	}
}
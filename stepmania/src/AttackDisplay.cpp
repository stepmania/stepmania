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


AttackDisplay::AttackDisplay()
{
	m_textAttack.LoadFromFont( THEME->GetPathToF("AttackDisplay") );
	m_textAttack.SetDiffuseAlpha( 0 );	// invisible
	this->AddChild( &m_textAttack );
}

void AttackDisplay::Update( float fDelta )
{
	ActorFrame::Update( fDelta );

	// FIXME: Make GAMESTATE->m_ActiveAttacks a vector
	if( !GAMESTATE->m_ActiveAttacks[m_PlayerNumber][0].IsBlank() )
	{
		m_textAttack.SetDiffuseAlpha( 1 );
		m_textAttack.SetText( GAMESTATE->m_ActiveAttacks[m_PlayerNumber][0].sModifier );
	}
	else
	{
		m_textAttack.SetDiffuseAlpha( 0 );
	}
}
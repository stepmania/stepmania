#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: EnemyHealth

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "EnemyHealth.h"
#include "ThemeManager.h"
#include "GameState.h"


EnemyHealth::EnemyHealth() : 
MeterDisplay( 
	THEME->GetPathToG("EnemyHealth stream"),
	THEME->GetPathToG("EnemyHealth frame"),
	464 )
{
	m_fLastSeenHealthPercent = -1;
}

void EnemyHealth::Update( float fDelta )
{
	if( m_fLastSeenHealthPercent != GAMESTATE->m_fOpponentHealthPercent )
	{
		this->SetPercent( GAMESTATE->m_fOpponentHealthPercent );
		m_fLastSeenHealthPercent = GAMESTATE->m_fOpponentHealthPercent;
	}

	MeterDisplay::Update( fDelta );
}

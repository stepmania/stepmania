#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: CombinedLifeMeterEnemy

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "CombinedLifeMeterEnemy.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "MusicWheel.h"
#include "CombinedLifeMeterEnemy.h"
#include "RageTimer.h"
#include <math.h>
#include "ThemeManager.h"
#include "GameState.h"
#include "CombinedLifeMeterEnemy.h"
#include "ThemeManager.h"
#include "GameState.h"


const float SECONDS_TO_SHOW_FACE = 1.5f;


CombinedLifeMeterEnemy::CombinedLifeMeterEnemy()
{
	m_sprFace.Load( THEME->GetPathToG("CombinedLifeMeterEnemy face 2x3") );
	ASSERT( m_sprFace.GetNumStates() >= NUM_FACES );
	m_sprFace.StopAnimating();
	this->AddChild( &m_sprFace );

	m_meterHealth.Load( THEME->GetPathToG("CombinedLifeMeterEnemy stream"), 464 );
	m_fLastSeenHealthPercent = -1;
	this->AddChild( &m_meterHealth );

	m_sprFrame.Load( THEME->GetPathToG("CombinedLifeMeterEnemy frame") );
	this->AddChild( &m_sprFrame );
}

void CombinedLifeMeterEnemy::Update( float fDelta )
{
	CombinedLifeMeter::Update( fDelta );

	if( m_fSecondsUntilReturnToNormalFace > 0 )
	{
		m_fSecondsUntilReturnToNormalFace -= fDelta;

		if( m_fSecondsUntilReturnToNormalFace < 0 )
		{
			m_fSecondsUntilReturnToNormalFace = 0;
			this->SetState( normal );
		}
	}

	if( GAMESTATE->m_fOpponentHealthPercent == 0 )
		this->SetState( defeated );

	if( m_fLastSeenHealthPercent != GAMESTATE->m_fOpponentHealthPercent )
	{
		m_meterHealth.SetPercent( GAMESTATE->m_fOpponentHealthPercent );
		m_fLastSeenHealthPercent = GAMESTATE->m_fOpponentHealthPercent;
	}


	/*
	SetFace( Face face )

	if( GAMESTATE->m_fOpponentHealthPercent == 0 )
		return;

	this->SetState( face );
	m_fSecondsUntilReturnToNormalFace = SECONDS_TO_SHOW_FACE;
	*/
}





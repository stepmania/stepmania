#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: EnemyFace

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "EnemyFace.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "MusicWheel.h"
#include "EnemyFace.h"
#include "RageTimer.h"
#include <math.h>
#include "ThemeManager.h"
#include "GameState.h"


const float SECONDS_TO_SHOW_FACE = 1.5f;


EnemyFace::EnemyFace()
{
	Load( THEME->GetPathToG("EnemyFace 2x3") );
	ASSERT( this->GetNumStates() >= NUM_FACES );
	StopAnimating();
	SetFace( normal );
}

void EnemyFace::Update( float fDelta )
{
	if( m_fSecondsUntilReturnToNormal > 0 )
	{
		m_fSecondsUntilReturnToNormal -= fDelta;

		if( m_fSecondsUntilReturnToNormal < 0 )
		{
			m_fSecondsUntilReturnToNormal = 0;
			this->SetState( normal );
		}
	}

	if( GAMESTATE->m_fOpponentHealthPercent == 0 )
		this->SetState( defeated );

	Sprite::Update( fDelta );
}

void EnemyFace::SetFace( Face face )
{
	if( GAMESTATE->m_fOpponentHealthPercent == 0 )
		return;

	this->SetState( face );
	m_fSecondsUntilReturnToNormal = SECONDS_TO_SHOW_FACE;
}


#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScoreDisplayRave.h

 Desc: A graphic displayed in the ScoreDisplayRave during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScoreDisplayRave.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "GameState.h"
#include "ThemeManager.h"


const float LEVEL_WIDTH = 50;


ScoreDisplayRave::ScoreDisplayRave()
{
	LOG->Trace( "ScoreDisplayRave::ScoreDisplayRave()" );

	for( int i=0; i<NUM_ATTACK_LEVELS; i++ )	
	{
		m_LevelStream[i].Load( THEME->GetPathToG(ssprintf("ScoreDisplayRave stream level%d",i+1)), LEVEL_WIDTH );
		float fSpan = (NUM_ATTACK_LEVELS-1)*LEVEL_WIDTH;
		float fX = SCALE(i,0.f,NUM_ATTACK_LEVELS-1.f, -fSpan/2, fSpan/2 );
		m_LevelStream[i].SetX( fX );
		this->AddChild( &m_LevelStream[i] );
	}

	this->AddChild( &m_sprFrame );
}

void ScoreDisplayRave::Init( PlayerNumber pn )
{
	ScoreDisplay::Init( pn );

	m_sprFrame.Load( THEME->GetPathToG(ssprintf("ScoreDisplayRave frame p%d",pn+1)) );
}

void ScoreDisplayRave::Update( float fDelta )
{
	ScoreDisplay::Update( fDelta );

	for( int i=0; i<NUM_ATTACK_LEVELS; i++ )	
	{
		float fLevelPercent = GAMESTATE->m_fSuperMeter[m_PlayerNumber] - i;
		CLAMP( fLevelPercent, 0 ,1 );
		m_LevelStream[i].SetPercent( fLevelPercent );
	}
}

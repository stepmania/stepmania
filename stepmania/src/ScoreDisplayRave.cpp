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


ScoreDisplayRave::ScoreDisplayRave()
{
	LOG->Trace( "ScoreDisplayRave::ScoreDisplayRave()" );

	m_lastLevelSeen = ATTACK_LEVEL_1;

	for( int i=0; i<NUM_ATTACK_LEVELS; i++ )	
	{
		m_sprMeter[i].Load( THEME->GetPathToG(ssprintf("ScoreDisplayRave stream level%d",i+1)) ); 
		m_sprMeter[i].SetCropRight( 1.f );
		this->AddChild( &m_sprMeter[i] );
	}

	m_textLevel.LoadFromNumbers( THEME->GetPathToN("ScoreDisplayRave level") );
	m_textLevel.SetText( "1" );
	this->AddChild( &m_textLevel );

	this->AddChild( &m_sprFrame );
}

void ScoreDisplayRave::Init( PlayerNumber pn )
{
	ScoreDisplay::Init( pn );

	if( pn == PLAYER_2 )
		for( int i=0; i<NUM_ATTACK_LEVELS; i++ )
			m_sprMeter[i].SetZoomX( -1 );

	m_sprFrame.Load( THEME->GetPathToG(ssprintf("ScoreDisplayRave frame p%d",pn+1)) );
}

void ScoreDisplayRave::Update( float fDelta )
{
	ScoreDisplay::Update( fDelta );

	float fLevel = GAMESTATE->m_fSuperMeter[m_PlayerNumber];
	AttackLevel level = (AttackLevel)(int)fLevel;

	if( level != m_lastLevelSeen )
	{
		m_sprMeter[m_lastLevelSeen].SetCropRight( 1.f );
		m_lastLevelSeen = level;
		m_textLevel.SetText( ssprintf("%d",level+1) );
	}

	float fPercent = fLevel - level;
	m_sprMeter[level].SetCropRight( 1-fPercent );
}

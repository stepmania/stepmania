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
#include "ActorUtil.h"


ScoreDisplayRave::ScoreDisplayRave()
{
	LOG->Trace( "ScoreDisplayRave::ScoreDisplayRave()" );
	
	this->SetName( "ScoreDisplayRave" );

	m_lastLevelSeen = ATTACK_LEVEL_1;

	this->AddChild( &m_sprFrameBase );

	for( int i=0; i<NUM_ATTACK_LEVELS; i++ )	
	{
		m_sprMeter[i].Load( THEME->GetPathToG(ssprintf("ScoreDisplayRave stream level%d",i+1)) ); 
		m_sprMeter[i].SetCropRight( 1.f );
		this->AddChild( &m_sprMeter[i] );
	}

	this->AddChild( &m_sprFrameOverlay );

	m_textLevel.LoadFromNumbers( THEME->GetPathToN("ScoreDisplayRave level") );
	m_textLevel.SetText( "1" );
	m_textLevel.SetShadowLength( 0 );
	this->AddChild( &m_textLevel );
}

void ScoreDisplayRave::Init( PlayerNumber pn )
{
	ScoreDisplay::Init( pn );

	m_sprFrameBase.Load( THEME->GetPathToG(ssprintf("ScoreDisplayRave frame base p%d",pn+1)) );
	m_sprFrameOverlay.Load( THEME->GetPathToG(ssprintf("ScoreDisplayRave frame overlay p%d",pn+1)) );

	for( int i=0; i<NUM_ATTACK_LEVELS; i++ )	
	{
		m_sprMeter[i].SetName( ssprintf("MeterP%d",pn+1) );
		ON_COMMAND( m_sprMeter[i] );
	}
		
	m_textLevel.SetName( ssprintf("LevelP%d",pn+1) );
	ON_COMMAND( m_textLevel );
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

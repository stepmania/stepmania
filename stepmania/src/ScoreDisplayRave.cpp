#include "global.h"
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

	m_textLevel.LoadFromFont( THEME->GetPathF("ScoreDisplayRave","level") );
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

/*
 * (c) 2003 Chris Danford
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

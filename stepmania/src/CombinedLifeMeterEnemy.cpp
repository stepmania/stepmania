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


#define FACE_X	THEME->GetMetricF("CombinedLifeMeterEnemy","FaceX")
#define FACE_Y	THEME->GetMetricF("CombinedLifeMeterEnemy","FaceY")
#define HEALTH_X	THEME->GetMetricF("CombinedLifeMeterEnemy","HealthX")
#define HEALTH_Y	THEME->GetMetricF("CombinedLifeMeterEnemy","HealthY")
#define FRAME_X	THEME->GetMetricF("CombinedLifeMeterEnemy","FrameX")
#define FRAME_Y	THEME->GetMetricF("CombinedLifeMeterEnemy","FrameY")


CombinedLifeMeterEnemy::CombinedLifeMeterEnemy()
{
	m_sprFace.Load( THEME->GetPathToG("CombinedLifeMeterEnemy face 2x3") );
	ASSERT( m_sprFace.GetNumStates() >= NUM_FACES );
	m_sprFace.StopAnimating();
	m_sprFace.SetXY( FACE_X, FACE_Y );
	this->AddChild( &m_sprFace );

	m_sprHealthBackground.Load( THEME->GetPathToG("CombinedLifeMeterEnemy health background") );
	m_sprHealthBackground.SetXY( HEALTH_X, HEALTH_Y );
	this->AddChild( &m_sprHealthBackground );

	m_sprHealthStream.Load( THEME->GetPathToG("CombinedLifeMeterEnemy health stream") );
	m_sprHealthStream.SetXY( HEALTH_X, HEALTH_Y );
	m_sprHealthStream.SetTexCoordVelocity(-0.2f,0.5f);
	this->AddChild( &m_sprHealthStream );

	m_fLastSeenHealthPercent = -1;

	m_sprFrame.Load( THEME->GetPathToG("CombinedLifeMeterEnemy frame") );
	m_sprFrame.SetName( "Frame" );
	m_sprFrame.SetXY( FRAME_X, FRAME_Y );
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
			m_sprFace.SetState( normal );
		}
	}

	if( GAMESTATE->m_fOpponentHealthPercent == 0 )
	{
		m_sprFace.SetState( defeated );
	}

	if( m_fLastSeenHealthPercent != GAMESTATE->m_fOpponentHealthPercent )
	{
		m_sprHealthStream.SetGlow( RageColor(1,1,1,1) );
		m_sprHealthStream.BeginTweening( 0.5f, TWEEN_DECELERATE );
		m_sprHealthStream.SetCropRight( 1-GAMESTATE->m_fOpponentHealthPercent );
		m_sprHealthStream.SetGlow( RageColor(1,1,1,0) );
		m_fLastSeenHealthPercent = GAMESTATE->m_fOpponentHealthPercent;

		m_sprFace.SetState( damage );
		m_fSecondsUntilReturnToNormalFace = SECONDS_TO_SHOW_FACE;

		if( GAMESTATE->m_fOpponentHealthPercent == 0 )
		{
			m_sprFrame.BeginTweening( 0.5f );
			m_sprFrame.SetDiffuse( RageColor(0.5f,0.5f,0.5f,1) );
		}
	}
}





#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: HoldGhostArrow

 Desc: A graphic displayed in the HoldJudgement during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Ben Nordstrom
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "HoldGhostArrow.h"
#include "PrefsManager.h"
#include "RageException.h"
#include "RageTimer.h"


const float  HOLD_GHOST_ARROW_TWEEN_TIME = 0.5f;

HoldGhostArrow::HoldGhostArrow()
{
	m_bWasSteppedOnLastFrame = false;
	m_fHeatLevel = 0;

//	LoadFromSpriteFile( THEME->GetPathTo(GRAPHIC_HOLD_GHOST_ARROW) );
	SetDiffuse( D3DXCOLOR(1,1,1,1) );
//	SetZoom( 1.1f );
}

void HoldGhostArrow::Update( float fDeltaTime )
{
	Sprite::Update( fDeltaTime );

	if( m_bWasSteppedOnLastFrame )
		m_fHeatLevel += fDeltaTime * 4;
	else
		m_fHeatLevel -= fDeltaTime * 4;

	CLAMP( m_fHeatLevel, 0, 1 );
//	if( m_fHeatLevel > 0 )
//		printf( "m_fHeatLevel = %f\n", m_fHeatLevel );

	int iStateNum = (int)min( m_fHeatLevel * GetNumStates(), GetNumStates()-1 );
	SetState( iStateNum );

	if( m_fHeatLevel == 1 )
	{
		bool bZooomALittle = fmodf( TIMER->GetTimeSinceStart(), 1/20.0f ) > 1/40.0f;
		SetZoom( bZooomALittle ? 1.04f : 1.0f );
	}
	else
		SetZoom( 1 );
	
	SetDiffuse( D3DXCOLOR(1,1,1,m_fHeatLevel*3) );

	m_bWasSteppedOnLastFrame = false;	// reset for next frame
}

void HoldGhostArrow::DrawPrimitives()
{
	Sprite::DrawPrimitives();
}

void HoldGhostArrow::Step()
{
	m_bWasSteppedOnLastFrame = true;
}

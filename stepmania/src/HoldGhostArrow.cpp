#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: HoldGhostArrow

 Desc: A graphic displayed in the HoldJudgment during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Ben Nordstrom
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "HoldGhostArrow.h"
#include "PrefsManager.h"
#include "RageException.h"
#include "RageTimer.h"
#include <math.h>
#include "ThemeManager.h"


CachedThemeMetricF		WARM_UP_SECONDS		("HoldGhostArrow","WarmUpSeconds");


HoldGhostArrow::HoldGhostArrow()
{
	WARM_UP_SECONDS.Refresh();

	m_bWasSteppedOnLastFrame = false;
	m_fHeatLevel = 0;

//	LoadFromSpriteFile( THEME->GetPathTo(GRAPHIC_HOLD_GHOST_ARROW) );
	SetDiffuse( RageColor(1,1,1,1) );
}

void HoldGhostArrow::Update( float fDeltaTime )
{
	Sprite::Update( fDeltaTime );

	if( m_bWasSteppedOnLastFrame )
		m_fHeatLevel += fDeltaTime/(float)WARM_UP_SECONDS;
	else
		m_fHeatLevel -= fDeltaTime/(float)WARM_UP_SECONDS;

	CLAMP( m_fHeatLevel, 0, 1 );

	int iStateNum = (int)min( m_fHeatLevel * GetNumStates(), GetNumStates()-1 );
	SetState( iStateNum );

	if( m_fHeatLevel == 1 )
	{
		bool bZooomALittle = fmodf( RageTimer::GetTimeSinceStart(), 1/20.0f ) > 1/40.0f;
		SetZoom( bZooomALittle ? 1.04f : 1.0f );
	}
	else
		SetZoom( 1 );
	
	SetDiffuse( RageColor(1,1,1,m_fHeatLevel*3) );

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

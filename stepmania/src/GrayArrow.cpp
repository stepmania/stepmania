#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: GrayArrow

 Desc: A gray arrow that "receives" ColorNotes.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Ben Nordstrom
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GrayArrow.h"
#include "PrefsManager.h"
#include "GameState.h"


#define STEP_SECONDS		THEME->GetMetricF("GrayArrow","StepSeconds")
#define STEP_ZOOM			THEME->GetMetricF("GrayArrow","StepZoom")

float g_fStepSeconds, g_fStepZoom;


GrayArrow::GrayArrow()
{
	g_fStepSeconds = STEP_SECONDS;
	g_fStepZoom = STEP_ZOOM;

	StopAnimating();
}

void GrayArrow::Update( float fDeltaTime )
{
	ASSERT( Sprite::GetNumStates() > 0 );	// you forgot to call Load()

	Sprite::Update( fDeltaTime );

	float fPercentIntoBeat = fmodf(GAMESTATE->m_fSongBeat,1);
	if( fPercentIntoBeat < 0 )
		fPercentIntoBeat += 1;
	int iNewState = fPercentIntoBeat<0.25f ? 0 : 1;
	CLAMP( iNewState, 0, Sprite::GetNumStates() );
	SetState( iNewState );
}

void GrayArrow::Step()
{
	SetZoom( g_fStepZoom );
	StopTweening();
	BeginTweening( g_fStepSeconds );
	SetTweenZoom( 1 );
}

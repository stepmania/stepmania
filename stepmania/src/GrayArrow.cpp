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
#include <math.h>
#include "ThemeManager.h"

#include "RageLog.h"

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

	/* These could be metrics or configurable.  I'd prefer the flash to
	 * start on the beat, I think ... -glenn */

	/* Start flashing 10% of a beat before the beat starts. */
	const float flash_offset = -0.1f;

	/* Flash for 20% of a beat. */
	const float flash_length = 0.2f;

	float cur_beat = GAMESTATE->m_fSongBeat;

	/* Make sure that the first flash (when cur_beat is passing 0) is just as
	 * long as others. */
	cur_beat += 1.0f;
	cur_beat -= flash_offset;

	float fPercentIntoBeat = fmodf(cur_beat, 1);

	int iNewState = (fPercentIntoBeat < flash_length)? 0:1;

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

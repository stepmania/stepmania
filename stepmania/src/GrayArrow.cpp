#include "global.h"
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

CachedThemeMetricF	GR_STEP_SECONDS			("GrayArrow","StepSeconds");
CachedThemeMetricF	GR_STEP_ZOOM			("GrayArrow","StepZoom");


GrayArrow::GrayArrow()
{
	GR_STEP_SECONDS.Refresh();
	GR_STEP_ZOOM.Refresh();

	StopAnimating();
}

void GrayArrow::Update( float fDeltaTime )
{
	ASSERT( Sprite::GetNumStates() > 0 );	// you forgot to call Load()

	Sprite::Update( fDeltaTime );

	/* XXX: get rid of this once we update the note skins */
	int IdleState = (GetNumStates() == 3)? 0: 1;
	int OnState = (GetNumStates() == 3)? 1: 0;
	int OffState = (GetNumStates() == 3)? 2: 1;
	
	if( !GAMESTATE->m_bPastHereWeGo )
	{
		SetState( IdleState );
		return;
	}

	/* These could be metrics or configurable.  I'd prefer the flash to
	 * start on the beat, I think ... -glenn */

	/* Start flashing 10% of a beat before the beat starts. */
	const float flash_offset = -0.1f;

	/* Flash for 20% of a beat. */
	const float flash_length = 0.2f;

	float cur_beat = GAMESTATE->m_fSongBeat;

	/* Beats can start in very negative territory (many BMR songs, Drop Out 
	 * -Remix-). */
	cur_beat += 100.0f;

	cur_beat -= flash_offset;
	float fPercentIntoBeat = fmodf(cur_beat, 1);
	SetState( (fPercentIntoBeat<flash_length)? OnState : OffState );
}

void GrayArrow::Step()
{
	SetZoom( GR_STEP_ZOOM );
	StopTweening();
	BeginTweening( GR_STEP_SECONDS );
	SetZoom( 1 );
}

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
#include "NoteFieldPositioning.h"
#include "NoteSkinManager.h"

#include "RageLog.h"

CachedThemeMetricF	GR_STEP_SECONDS			("GrayArrow","StepSeconds");
CachedThemeMetricF	GR_STEP_ZOOM			("GrayArrow","StepZoom");


GrayArrow::GrayArrow()
{
	GR_STEP_SECONDS.Refresh();
	GR_STEP_ZOOM.Refresh();
	m_bIsPressed = false;
	StopAnimating();
}

bool GrayArrow::Load( CString NoteSkin, PlayerNumber pn, int iColNo )
{
	CString Button = g_NoteFieldMode[pn].GrayButtonNames[iColNo];
	if( Button == "" )
		Button = NoteSkinManager::ColToButtonName( iColNo );

	CString sPath = NOTESKIN->GetPathTo( NoteSkin, Button, "receptor" );
	bool ret = Sprite::Load( sPath );

	// XXX
	if( GetNumStates() != 2 &&
		GetNumStates() != 3 )
		RageException::Throw( "'%s' must have 2 or 3 frames", sPath.c_str() );

	sPath = NOTESKIN->GetPathTo( NoteSkin, Button, "KeypressBlock" );
	bool ret2 = m_sprPressBlock.Load( sPath );
	m_sprPressBlock.SetXY(0, THEME->GetMetricF("ScreenGameplay","PressBlockY"));
	return ret && ret2; // return both loaded or fail
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
	m_sprPressBlock.Update(fDeltaTime);
}

void GrayArrow::Draw()
{
	Sprite::Draw();
	if(m_bIsPressed)
	{
		m_sprPressBlock.Draw();
		m_bIsPressed = false;
	}
}

void GrayArrow::Step()
{
	SetZoom( GR_STEP_ZOOM );
	StopTweening();
	BeginTweening( GR_STEP_SECONDS );
	SetZoom( 1 );
}

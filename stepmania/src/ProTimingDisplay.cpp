#include "global.h"
#include "ProTimingDisplay.h"
#include "ThemeManager.h"

static CachedThemeMetric	MARVELOUS_COMMAND	("ProTimingDisplay","MarvelousCommand");
static CachedThemeMetric	PERFECT_COMMAND		("ProTimingDisplay","PerfectCommand");
static CachedThemeMetric	GREAT_COMMAND		("ProTimingDisplay","GreatCommand");
static CachedThemeMetric	GOOD_COMMAND		("ProTimingDisplay","GoodCommand");
static CachedThemeMetric	BOO_COMMAND			("ProTimingDisplay","BooCommand");
static CachedThemeMetric	MISS_COMMAND		("ProTimingDisplay","MissCommand");


ProTimingDisplay::ProTimingDisplay()
{
	m_Judgment.LoadFromNumbers( THEME->GetPathToF("ProTimingDisplay Judgment") );
	m_Judgment.SetName( "Judgment" );
	this->AddChild( &m_Judgment );

	MARVELOUS_COMMAND.Refresh();
	PERFECT_COMMAND.Refresh();
	GREAT_COMMAND.Refresh();
	GOOD_COMMAND.Refresh();
	BOO_COMMAND.Refresh();
	MISS_COMMAND.Refresh();
}

void ProTimingDisplay::Reset()
{
	m_Judgment.SetText("");

	m_Judgment.SetDiffuse( RageColor(1,1,1,0) );
	m_Judgment.SetXY( 0, 0 );
	m_Judgment.StopTweening();
	m_Judgment.SetEffectNone();
}

void ProTimingDisplay::SetJudgment( int ms, TapNoteScore score )
{
	Reset();

	m_Judgment.SetText( ssprintf("%i", ms) );

	const CachedThemeMetric *Commands[NUM_TAP_NOTE_SCORES] =
	{
		NULL, /* no TNS_NONE */
		&MISS_COMMAND, &BOO_COMMAND, &GOOD_COMMAND, &GREAT_COMMAND, &PERFECT_COMMAND, &MARVELOUS_COMMAND
	};
	ASSERT( score != TNS_NONE );
	ASSERT( score < NUM_TAP_NOTE_SCORES );

	m_Judgment.Command( *Commands[score] );
}

/*
-----------------------------------------------------------------------------
 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Glenn Maynard
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "global.h"
#include "ProTimingDisplay.h"
#include "ThemeManager.h"
#include "RageUtil.h"
#include "ThemeMetric.h"

static const ThemeMetric<Commands>	MARVELOUS_COMMAND	("ProTimingDisplay","MarvelousCommand");
static const ThemeMetric<Commands>	PERFECT_COMMAND		("ProTimingDisplay","PerfectCommand");
static const ThemeMetric<Commands>	GREAT_COMMAND		("ProTimingDisplay","GreatCommand");
static const ThemeMetric<Commands>	GOOD_COMMAND		("ProTimingDisplay","GoodCommand");
static const ThemeMetric<Commands>	BOO_COMMAND			("ProTimingDisplay","BooCommand");
static const ThemeMetric<Commands>	MISS_COMMAND		("ProTimingDisplay","MissCommand");
static const ThemeMetric<Commands>	HIT_MINE_COMMAND	("ProTimingDisplay","HitMineCommand");

static const ThemeMetric<Commands> *g_Commands[NUM_TAP_NOTE_SCORES] =
{
	NULL, /* no TNS_NONE */
	&HIT_MINE_COMMAND, 
	&MISS_COMMAND, 
	&BOO_COMMAND, 
	&GOOD_COMMAND, 
	&GREAT_COMMAND, 
	&PERFECT_COMMAND, 
	&MARVELOUS_COMMAND
};

ProTimingDisplay::ProTimingDisplay()
{
	m_Judgment.LoadFromFont( THEME->GetPathF("ProTimingDisplay","Judgment") );
	m_Judgment.SetName( "Judgment" );
	this->AddChild( &m_Judgment );
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

	ASSERT( score != TNS_NONE );
	ASSERT( score < NUM_TAP_NOTE_SCORES );

	m_Judgment.RunCommands( *g_Commands[score] );
}

/*
 * (c) 2001-2003 Glenn Maynard, Chris Danford
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

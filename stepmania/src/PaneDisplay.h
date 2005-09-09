/* PaneDisplay - An Actor that displays song information. */

#ifndef PANE_DISPLAY_H
#define PANE_DISPLAY_H

#include "ActorFrame.h"
#include "ActorCommands.h"
#include "PlayerNumber.h"
#include "BitmapText.h"
#include "AutoActor.h"
#include "GameConstantsAndTypes.h"
#include "ThemeMetric.h"

enum PaneTypes
{
	PANE_SONG_DIFFICULTY,
	PANE_COURSE_MACHINE_SCORES,
	NUM_PANES,
	PANE_INVALID
};

/* If the same piece of data is in multiple panes, use separate contents entries,
 * so it can be themed differently. */
enum PaneContents
{
	SONG_NUM_STEPS,
	SONG_JUMPS,
	SONG_HOLDS,
	SONG_ROLLS,
	SONG_MINES,
	SONG_HANDS,
	SONG_MACHINE_HIGH_SCORE,
	SONG_MACHINE_HIGH_NAME,
	SONG_PROFILE_HIGH_SCORE,
	COURSE_MACHINE_HIGH_SCORE,
	COURSE_MACHINE_HIGH_NAME,
	COURSE_PROFILE_HIGH_SCORE,
	COURSE_NUM_STEPS,
	COURSE_JUMPS,
	COURSE_HOLDS,
	COURSE_MINES,
	COURSE_HANDS,
	COURSE_ROLLS,
	NUM_PANE_CONTENTS
};
#define FOREACH_PaneContents( p ) FOREACH_ENUM( PaneContents, NUM_PANE_CONTENTS, p )

class PaneDisplay: public ActorFrame
{
public:
	PaneDisplay();
	virtual Actor *Copy() const;

	void Load( const CString &sClass, PlayerNumber pn );
	void SetFromGameState( SortOrder so );

	// Lua
	void PushSelf( lua_State *L );

private:
	void SetFocus( PaneTypes NewPane );
	PaneTypes GetPane() const;
	void SetContent( PaneContents c );

	SortOrder		m_SortOrder;
	AutoActor		m_sprPaneUnder;

	BitmapText		m_textContents[NUM_PANE_CONTENTS];
	AutoActor		m_Labels[NUM_PANE_CONTENTS];
	ActorFrame		m_ContentsFrame;

	PaneTypes		m_CurPane;
	PlayerNumber	m_PlayerNumber;

	ThemeMetric<CString> EMPTY_MACHINE_HIGH_SCORE_NAME;
};

#endif

/*
 * (c) 2003 Glenn Maynard
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

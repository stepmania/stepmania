#ifndef PANE_DISPLAY_H
#define PANE_DISPLAY_H

#include "ActorFrame.h"
#include "Sprite.h"
#include "PlayerNumber.h"
#include "BitmapText.h"
#include "ActorUtil.h"
#include "GameConstantsAndTypes.h"

enum PaneTypes
{
	PANE_SONG_DIFFICULTY,
	PANE_SONG_PROFILE_SCORES,
	PANE_SONG_MACHINE_SCORES,
//	PANE_SONG_STATISTICS,
	PANE_BATTLE_DIFFICULTY,
	PANE_COURSE_MACHINE_SCORES,
	PANE_COURSE_PROFILE_SCORES,
//	PANE_COURSE_DIFFICULTY,
	NUM_PANES,
	PANE_INVALID
};

enum PaneModes
{
	PANEMODE_SONG,
	PANEMODE_BATTLE,
	PANEMODE_COURSE,
	NUM_PANE_MODES
};

/* If the same piece of data is in multiple panes, use separate contents entries,
 * so it can be themed differently. */
enum PaneContents
{
	SONG_NUM_STEPS,
	SONG_JUMPS,
	SONG_HOLDS,
	SONG_MINES,
	SONG_HANDS,
	SONG_DIFFICULTY_RADAR_STREAM,
	SONG_DIFFICULTY_RADAR_CHAOS,
	SONG_DIFFICULTY_RADAR_FREEZE,
	SONG_DIFFICULTY_RADAR_AIR,
	SONG_DIFFICULTY_RADAR_VOLTAGE,
	SONG_MACHINE_HIGH_SCORE,
	SONG_MACHINE_NUM_PLAYS,
	SONG_MACHINE_RANK,
	SONG_MACHINE_HIGH_NAME,
	SONG_PROFILE_HIGH_SCORE,
	SONG_PROFILE_NUM_PLAYS,
	SONG_PROFILE_RANK,
	COURSE_MACHINE_HIGH_SCORE,
	COURSE_MACHINE_NUM_PLAYS,
	COURSE_MACHINE_RANK,
	COURSE_MACHINE_HIGH_NAME,
	COURSE_PROFILE_HIGH_SCORE,
	COURSE_PROFILE_NUM_PLAYS,
	COURSE_PROFILE_RANK,
	COURSE_NUM_STEPS,
	COURSE_JUMPS,
	COURSE_HOLDS,
	COURSE_MINES,
	COURSE_HANDS,
	NUM_PANE_CONTENTS
};

class PaneDisplay: public ActorFrame
{
public:
	PaneDisplay();

	void Load( PlayerNumber pn );
	void SetFromGameState();
	void Move( int dir );

	void Update( float fDeltaTime );

private:
	bool PaneIsValid( PaneTypes p ) const;
	PaneTypes GetNext( PaneTypes current, int dir ) const;
	void SetFocus( PaneTypes NewPane );
//	void SetMode();
	PaneModes GetMode() const;
	void SetContent( PaneContents c );

	AutoActor		m_sprPaneUnder;
	AutoActor		m_sprPaneOver;

	BitmapText		m_textContents[NUM_PANE_CONTENTS];
	AutoActor		m_Labels[NUM_PANE_CONTENTS];
	ActorFrame		m_ContentsFrame;

	PaneTypes		m_CurPane;
	PaneModes		m_CurMode;
	PlayerNumber	m_PlayerNumber;

	PaneTypes		m_PreferredPaneForMode[NUM_PANE_MODES];
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

/* ScreenEnding - Ending screen that shows stats. */

#ifndef SCREEN_ENDING_H
#define SCREEN_ENDING_H

#include "BitmapText.h"
#include "ScreenAttract.h"

enum EndingStatsLine
{
	PERCENT_COMPLETE,
	TOTAL_CALORIES,
	TOTAL_SONGS_PLAYED,
	CURRENT_COMBO,
	NUM_ENDING_STATS_LINES
};

class ScreenEnding : public ScreenAttract
{
public:
	ScreenEnding( CString sName );
	~ScreenEnding();

	void Update( float fDeltaTime );
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	void HandleScreenMessage( const ScreenMessage SM );

private:
	BitmapText m_textPlayerName[NUM_PLAYERS];
	BitmapText m_textStatsTitle[NUM_PLAYERS][NUM_ENDING_STATS_LINES];
	BitmapText m_textStatsValue[NUM_PLAYERS][NUM_ENDING_STATS_LINES];

	Sprite	m_sprRemoveMemoryCard[NUM_PLAYERS];
	bool m_bWaitingForRemoveCard[NUM_PLAYERS];
};

#endif

/*
 * (c) 2004 Chris Danford
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

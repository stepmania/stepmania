/* ScreenEnding - Ending screen that shows stats. */

#ifndef SCREEN_ENDING_H
#define SCREEN_ENDING_H

#include "BitmapText.h"
#include "ScreenAttract.h"
#include "Sprite.h"

enum EndingStatsLine
{
	CALORIES_TODAY,
	CURRENT_COMBO,
	PERCENT_COMPLETE,
	PERCENT_COMPLETE_EASY,
	PERCENT_COMPLETE_MEDIUM,
	PERCENT_COMPLETE_HARD,
	PERCENT_COMPLETE_CHALLENGE,
	NUM_ENDING_STATS_LINES
};
#define FOREACH_EndingStatsLine( l ) FOREACH_ENUM( EndingStatsLine, NUM_ENDING_STATS_LINES, l )

class ScreenEnding : public ScreenAttract
{
public:
	ScreenEnding( CString sName );
	virtual void Init();
	~ScreenEnding();

	void Update( float fDeltaTime );
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	void HandleScreenMessage( const ScreenMessage SM );

private:
	struct Line
	{
		BitmapText title;
		BitmapText value;
	} m_Lines[NUM_ENDING_STATS_LINES][NUM_PLAYERS];

	Sprite	m_sprRemoveMemoryCard[NUM_PLAYERS];
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

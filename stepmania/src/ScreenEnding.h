/*
-----------------------------------------------------------------------------
 File: ScreenEnding.h

 Desc: Ending screen that shows stats.

 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Glenn Maynard
-----------------------------------------------------------------------------
*/

#include "BitmapText.h"
#include "ScreenAttract.h"

class ScreenEnding : public ScreenAttract
{
public:
	ScreenEnding( CString sName );
	~ScreenEnding();

	void Update( float fDeltaTime );
	void HandleScreenMessage( const ScreenMessage SM );

private:
	BitmapText m_textPlayerName[NUM_PLAYERS];
#define NUM_ENDING_STATS_LINES 3	// Total calories burned, total songs played, current combo
	BitmapText m_textStatsTitle[NUM_PLAYERS][NUM_ENDING_STATS_LINES];
	BitmapText m_textStatsValue[NUM_PLAYERS][NUM_ENDING_STATS_LINES];

	Sprite	m_sprRemoveMemoryCard[NUM_PLAYERS];
	bool m_bWaitingForRemoveCard[NUM_PLAYERS];
	float m_fTimeUntilBeginFadingOut;
};



#ifndef LYRIC_DISPLAY_H
#define LYRIC_DISPLAY_H

#include "ActorFrame.h"
#include "song.h"
#include "BitmapText.h"

class LyricDisplay: public ActorFrame {
public:
	LyricDisplay();
	void Update(float fDeltaTime);
	
	/* Call when song changes: */
	void Init();

private:
	BitmapText m_textLyrics;
	unsigned m_iCurLyricNumber;
	float m_fLastSecond;
};

#endif
/*
-----------------------------------------------------------------------------
 Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
    Kevin Slaughter
	Glenn Maynard
-----------------------------------------------------------------------------
*/

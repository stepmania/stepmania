/*
-----------------------------------------------------------------------------
 File: ScreenSongOptions.h

 Desc: Select a song.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/
#ifndef SCREEN_SONG_OPTIONS_H
#define SCREEN_SONG_OPTIONS_H

#include "ScreenOptions.h"

class ScreenSongOptions : public ScreenOptions
{
public:
	ScreenSongOptions();

private:
	void ImportOptions();
	void ExportOptions();

	void GoToNextState();
	void GoToPrevState();
};

#endif

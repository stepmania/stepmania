/*
-----------------------------------------------------------------------------
 File: ScreenSongOptions.h

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/
#ifndef SCREEN_SONG_OPTIONS_H
#define SCREEN_SONG_OPTIONS_H

#include "ScreenOptionsMaster.h"

class ScreenSongOptions : public ScreenOptionsMaster
{
public:
	ScreenSongOptions( CString sName );
	static CString GetNextScreen();

private:
	void GoToNextState();
	void GoToPrevState();
};

#endif

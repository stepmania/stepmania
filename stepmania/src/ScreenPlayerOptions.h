/*
-----------------------------------------------------------------------------
 File: ScreenPlayerOptions.h

 Desc: Select a song.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/
#ifndef SCREEN_PLAYER_OPTIONS_H
#define SCREEN_PLAYER_OPTIONS_H


#include "ScreenOptions.h"


class ScreenPlayerOptions : public ScreenOptions
{
public:
	ScreenPlayerOptions();

private:
	void ImportOptions();
	void ExportOptions();

	void GoToNextState();
	void GoToPrevState();
};



#endif
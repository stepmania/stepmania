/*
-----------------------------------------------------------------------------
 File: ScreenMachineOptions

 Desc: Select a song.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/
#ifndef SCREEN_MACHINE_OPTIONS_H
#define SCREEN_MACHINE_OPTIONS_H

#include "ScreenOptions.h"

class ScreenMachineOptions : public ScreenOptions
{
public:
	ScreenMachineOptions();

private:
	void ImportOptions();
	void ExportOptions();

	void GoToNextState();
	void GoToPrevState();
};

#endif

#ifndef SCREENMACHINEOPTIONS_H
#define SCREENMACHINEOPTIONS_H
/*
-----------------------------------------------------------------------------
 File: ScreenMachineOptions

 Desc: Select a song.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

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

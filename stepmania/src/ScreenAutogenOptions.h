#ifndef ScreenAutogenOptions_H
#define ScreenAutogenOptions_H
/*
-----------------------------------------------------------------------------
 File: ScreenAutogenOptions

 Desc: Select a song.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenOptions.h"

class ScreenAutogenOptions : public ScreenOptions
{
public:
	ScreenAutogenOptions();

private:
	void ImportOptions();
	void ExportOptions();

	void GoToNextState();
	void GoToPrevState();
};

#endif

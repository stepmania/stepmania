/*
-----------------------------------------------------------------------------
 File: ScreenBackgroundOptions

 Desc: Select a song.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenOptions.h"

class ScreenBackgroundOptions : public ScreenOptions
{
public:
	ScreenBackgroundOptions();

private:
	void ImportOptions();
	void ExportOptions();

	void GoToNextState();
	void GoToPrevState();
};


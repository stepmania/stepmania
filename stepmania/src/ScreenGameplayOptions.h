/*
-----------------------------------------------------------------------------
 File: ScreenGameplayOptions

 Desc: Select a song.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenOptions.h"

class ScreenGameplayOptions : public ScreenOptions
{
public:
	ScreenGameplayOptions();

private:
	void ImportOptions();
	void ExportOptions();

	void GoToNextState();
	void GoToPrevState();
};


/*
-----------------------------------------------------------------------------
 File: ScreenSelectGame

 Desc: Switch the current game

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenOptions.h"

class ScreenSelectGame : public ScreenOptions
{
public:
	ScreenSelectGame();

private:
	void ImportOptions();
	void ExportOptions();

	void GoToNextState();
	void GoToPrevState();
};

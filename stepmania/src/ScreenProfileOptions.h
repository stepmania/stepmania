/*
-----------------------------------------------------------------------------
 File: ScreenProfileOptions

 Desc: Select a song.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenOptions.h"

class ScreenProfileOptions : public ScreenOptions
{
public:
	ScreenProfileOptions();

	virtual void MenuStart( PlayerNumber pn );

private:
	void ImportOptions();
	void ExportOptions();

	void GoToNextState();
	void GoToPrevState();
};


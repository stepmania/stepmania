/*
-----------------------------------------------------------------------------
 Class: ScreenOptionsMenu

 Desc: Select a theme and announcer.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "ScreenOptions.h"


class ScreenOptionsMenu : public ScreenOptions
{
public:
	ScreenOptionsMenu();

protected:
	void MenuStart( PlayerNumber pn );

private:
	void ImportOptions();
	void ExportOptions();

	void GoToNextState();
	void GoToPrevState();

	void MenuLeft( PlayerNumber pn ) { MenuUp(pn); }
	void MenuRight( PlayerNumber pn ) { MenuDown(pn); }
};

#ifndef SCREENRAVEOPTIONS_H
#define SCREENRAVEOPTIONS_H
/*
-----------------------------------------------------------------------------
 File: ScreenRaveOptions

 Desc: Select a song.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenOptionsMaster.h"

class ScreenRaveOptions : public ScreenOptionsMaster
{
public:
	ScreenRaveOptions( CString sName );

private:
	void GoToNextState();
	void GoToPrevState();
};

#endif


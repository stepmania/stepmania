/*
-----------------------------------------------------------------------------
 Class: ScreenUnlock

 Desc: Base class for all attraction screens.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/
#include "ScreenAttract.h"
#include "GameConstantsAndTypes.h"	// for NUM_RANKING_LINES
#include "Style.h"


class Course;


class ScreenUnlock : public ScreenAttract
{
public:
	ScreenUnlock();
protected:
	BitmapText PointsUntilNextUnlock;
};

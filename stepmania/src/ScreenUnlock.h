/*
-----------------------------------------------------------------------------
 Class: ScreenUnlock

 Desc: Base class for all attraction screens.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Andrew Wong
-----------------------------------------------------------------------------
*/
#include "ScreenAttract.h"
#include "Style.h"
#include "ActorUtil.h"
#include "Sprite.h"
#include "BitmapText.h"

#define NUM_UNLOCKS 30

class Course;

class ScreenUnlock : public ScreenAttract
{
public:
	ScreenUnlock();
protected:
	BitmapText PointsUntilNextUnlock;
	Sprite Unlocks[NUM_UNLOCKS];       // support 30 unlocks right now
};

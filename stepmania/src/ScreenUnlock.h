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
#include "Sprite.h"
#include "BitmapText.h"

class Course;

class ScreenUnlock : public ScreenAttract
{
public:
	ScreenUnlock();

protected:
	BitmapText PointsUntilNextUnlock;
	vector<Sprite*> Unlocks;
	vector<BitmapText*> item; // scrolling text
	vector<Sprite*> ItemIcons;  // icons for scrolling text
};

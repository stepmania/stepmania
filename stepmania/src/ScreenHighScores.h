/*
-----------------------------------------------------------------------------
 Class: ScreenHighScores

 Desc: Base class for all attraction screens.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenAttract.h"

#define NUM_HIGH_SCORES	5

class ScreenHighScores : public ScreenAttract
{
public:
	ScreenHighScores();

protected:
	BitmapText m_textCategory;
	Sprite m_sprBullets[NUM_HIGH_SCORES];
	BitmapText m_textNames[NUM_HIGH_SCORES];
	BitmapText m_textScores[NUM_HIGH_SCORES];
};




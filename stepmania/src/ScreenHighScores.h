/*
-----------------------------------------------------------------------------
 Class: ScreenHighScores

 Desc: Base class for all attraction screens.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenAttract.h"
#include "GameConstantsAndTypes.h"	// for NUM_HIGH_SCORE_LINES


class ScreenHighScores : public ScreenAttract
{
public:
	ScreenHighScores();

protected:
	BitmapText m_textCategory;
	Sprite m_sprBullets[NUM_HIGH_SCORE_LINES];
	BitmapText m_textNames[NUM_HIGH_SCORE_LINES];
	BitmapText m_textScores[NUM_HIGH_SCORE_LINES];
};




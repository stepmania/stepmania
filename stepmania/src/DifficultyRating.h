#ifndef DIFFICULTYRATING_H
#define DIFFICULTYRATING_H
/*
-----------------------------------------------------------------------------
 Class: DifficultyRating

 Desc: Displays a whole bunch of graphics, either left aligned or center aligned.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Andrew Livy
-----------------------------------------------------------------------------
*/

#include "ActorFrame.h"
#include "Banner.h"
#include "Sprite.h"
#include "ThemeManager.h"

class DifficultyRating : public ActorFrame
{
public:
	DifficultyRating();
	~DifficultyRating();
	void SetDifficulty(int Difficulty);
	void SetOrientation(int Orientation);
	virtual void DrawPrimitives();
private:
	int iMaxElements;
	int iOrientation;
	int iCurrentDifficulty;
	vector<Sprite*>	m_apSprites;	// stores the list of elements (left to right)
};

#endif
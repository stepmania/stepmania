/*
-----------------------------------------------------------------------------
 File: ScoreDisplay.h

 Desc: A graphic displayed in the ScoreDisplay during Dancing.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/


#ifndef _ScoreDisplay_H_
#define _ScoreDisplay_H_


#include "Sprite.h"
#include "Song.h"
#include "ActorFrame.h"
#include "BitmapText.h"


const int MAX_SCORE_DIGITS	=	9;


class ScoreDisplay : public ActorFrame
{
public:
	ScoreDisplay();
	void SetScore( float fNewScore );

protected:
	BitmapText m_textDigits[MAX_SCORE_DIGITS];
};

#endif
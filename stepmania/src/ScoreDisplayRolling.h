/*
-----------------------------------------------------------------------------
 File: ScoreDisplayRolling.h

 Desc: A graphic displayed in the ScoreDisplayRolling during Dancing.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/


#ifndef _ScoreDisplayRolling_H_
#define _ScoreDisplayRolling_H_


#include "Sprite.h"
#include "Song.h"
#include "ActorFrame.h"
#include "BitmapText.h"


const int MAX_SCORE_DIGITS	=	9;


class ScoreDisplayRolling : public ActorFrame
{
public:
	ScoreDisplayRolling();
	void SetScore( float fNewScore );

	virtual void Update( float fDeltaTime );

protected:
	BitmapText m_textDigits[MAX_SCORE_DIGITS];
	int iCurrentScoreDigits[MAX_SCORE_DIGITS];
	int iDestinationScoreDigits[MAX_SCORE_DIGITS];

	float m_fTimeUntilNextTick;
};

#endif
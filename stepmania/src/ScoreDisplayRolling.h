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


const int NUM_SCORE_DIGITS	=	9;


class ScoreDisplayRolling : public BitmapText
{
public:
	ScoreDisplayRolling();

	void SetScore( float fNewScore );
	float GetScore();
	void AddToScore( TapStepScore stepscore, int iCurCombo );

	virtual void Update( float fDeltaTime );
	virtual void Draw();

protected:
	float m_fScore;

	int m_iCurrentScoreDigits[NUM_SCORE_DIGITS];
	int m_iDestinationScoreDigits[NUM_SCORE_DIGITS];
};

#endif
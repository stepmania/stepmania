//-----------------------------------------------------------------------------
// File: Judgement.h
//
// Desc: Feedback about the last step that appears in the middle of a player's stream of arrows.
//
// Copyright (c) 2001 Chris Danford.  All rights reserved.
//-----------------------------------------------------------------------------

#ifndef _JUDGEMENT_H_
#define _JUDGEMENT_H_


#include "Sprite.h"


class Judgement
{
public:
	Judgement();
	~Judgement();

	void SetX( int iNewX );

	void Update( const FLOAT &fDeltaTime );
	void Draw();

	void Perfect();
	void Great();
	void Good();
	void Boo();
	void Miss();

private:
	void TweenFromBigToSmall();


	FLOAT m_fDisplayTimeLeft;

	Sprite m_sprJudgement;
};




#endif
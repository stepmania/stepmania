#pragma once
/*
-----------------------------------------------------------------------------
 Class: ScoreDisplayNormal

 Desc: Shows point score during gameplay and used in some menus.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScoreDisplay.h"


const int NUM_SCORE_DIGITS	=	9;


class ScoreDisplayNormal : public ScoreDisplay
{
public:
	ScoreDisplayNormal();

	virtual void Init( PlayerNumber pn );

	virtual void Update( float fDeltaTime );
	virtual void Draw();

	virtual void SetScore( float fNewScore );

protected:
	float m_fScore;			// the actual score
	float m_fTrailingScore;	// what is displayed temporarily
	float m_fScoreVelocity;	// how fast trailing approaches real score
};

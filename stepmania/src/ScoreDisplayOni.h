#ifndef SCOREDISPLAYONI_H
#define SCOREDISPLAYONI_H
/*
-----------------------------------------------------------------------------
 Class: ScoreDisplayOni

 Desc: Shows time into course.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Song.h"
#include "ScoreDisplay.h"



class ScoreDisplayOni : public ScoreDisplay
{
public:
	ScoreDisplayOni();

	virtual void Init( PlayerNumber pn );

	virtual void Update( float fDeltaTime );
	virtual void Draw();

	virtual void SetScore( float fNewScore );

protected:
};

#endif

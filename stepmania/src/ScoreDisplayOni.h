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

#include "ScoreDisplay.h"
#include "BitmapText.h"


class ScoreDisplayOni : public ScoreDisplay
{
public:
	ScoreDisplayOni();

	virtual void Init( PlayerNumber pn );

	virtual void Update( float fDelta );

protected:
	Sprite		m_sprFrame;
	BitmapText	m_text;
};

#endif

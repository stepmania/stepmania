#ifndef SCOREDISPLAY_H
#define SCOREDISPLAY_H
/*
-----------------------------------------------------------------------------
 Class: ScoreDisplay

 Desc: A graphic displayed in the ScoreDisplay during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GameConstantsAndTypes.h"
#include "ActorFrame.h"

class ScoreDisplay : public ActorFrame
{
public:
	virtual void Init( PlayerNumber pn ) { m_PlayerNumber = pn; };

	virtual void SetScore( float fNewScore ) {};

protected:
	PlayerNumber m_PlayerNumber;	// needed to look up statistics in GAMESTATE
};

#endif

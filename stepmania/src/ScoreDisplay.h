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

#include "Sprite.h"
#include "GameConstantsAndTypes.h"
#include "ActorFrame.h"
#include "BitmapText.h"

class ScoreDisplay : public BitmapText
{
public:
	virtual void Init( PlayerNumber pn ) { m_PlayerNumber = pn; };

	virtual void Update( float fDeltaTime ) = 0;
	virtual void Draw() = 0;

	virtual void SetScore( float fNewScore ) = 0;

protected:
	PlayerNumber m_PlayerNumber;	// needed to look up statistics in GAMESTATE
};

#endif

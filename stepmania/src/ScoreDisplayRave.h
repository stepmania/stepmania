#ifndef ScoreDisplayRave_H
#define ScoreDisplayRave_H
/*
-----------------------------------------------------------------------------
 Class: ScoreDisplayRave

 Desc: Shows point score during gameplay and used in some menus.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScoreDisplay.h"
#include "GameConstantsAndTypes.h"
#include "OptionIcon.h"
#include "MeterDisplay.h"
#include "Sprite.h"


class ScoreDisplayRave : public ScoreDisplay
{
public:
	ScoreDisplayRave();
	virtual void Init( PlayerNumber pn );

	virtual void Update( float fDelta );

protected:
	Sprite m_sprMeter[NUM_ATTACK_LEVELS];
	Sprite m_sprFrame;
	BitmapText	m_textLevel;

	AttackLevel	m_lastLevelSeen;
};

#endif

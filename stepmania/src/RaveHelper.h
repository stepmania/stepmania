#ifndef RaveHelper_H
#define RaveHelper_H
/*
-----------------------------------------------------------------------------
 Class: RaveHelper

 Desc: Launches attacks in PLAY_MODE_RAVE.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Actor.h"
#include "PlayerNumber.h"
#include "RageSound.h"
#include "GameConstantsAndTypes.h"


class RaveHelper : public Actor
{
public:
	RaveHelper();
	void Load( PlayerNumber pn );

	virtual void Update( float fDelta );
	virtual void DrawPrimitives() {};

protected:
	void LaunchAttack( AttackLevel al );

	PlayerNumber m_PlayerNumber;

	RageSound m_soundLaunchAttack;
	RageSound m_soundAttackEnding;
};

#endif

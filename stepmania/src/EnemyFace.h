#ifndef EnemyFace_H
#define EnemyFace_H
/*
-----------------------------------------------------------------------------
 Class: EnemyFace

 Desc: A little graphic to the left of the song's text banner in the MusicWheel.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "Sprite.h"


class EnemyFace : public Sprite
{
public:
	EnemyFace();

	virtual void Update( float fDelta );

	enum Face { normal=0, taunt, attack, damage, defeated, NUM_FACES };
	void SetFace( Face face );

protected:
	float m_fSecondsUntilReturnToNormal;
};

#endif

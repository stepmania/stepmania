#ifndef EnemyHealth_H
#define EnemyHealth_H
/*
-----------------------------------------------------------------------------
 Class: EnemyHealth

 Desc: A little graphic to the left of the song's text banner in the MusicWheel.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "MeterDisplay.h"


class EnemyHealth : public MeterDisplay
{
public:
	EnemyHealth();

	virtual void Update( float fDelta );

protected:
	float m_fLastSeenHealthPercent;
};

#endif

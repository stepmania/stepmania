#ifndef CombinedLifeMeterEnemy_H
#define CombinedLifeMeterEnemy_H
/*
-----------------------------------------------------------------------------
 Class: CombinedLifeMeterEnemy

 Desc: A little graphic to the left of the song's text banner in the MusicWheel.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "LifeMeter.h"
#include "Sprite.h"
#include "MeterDisplay.h"


class CombinedLifeMeterEnemy : public CombinedLifeMeter
{
public:
	CombinedLifeMeterEnemy();

	virtual void Update( float fDelta );

	enum Face { normal=0, taunt, attack, damage, defeated, NUM_FACES };

	virtual void ChangeLife( PlayerNumber pn, TapNoteScore score ) {};
	virtual void ChangeLife( PlayerNumber pn, HoldNoteScore score, TapNoteScore tscore ) {};
	virtual void OnDancePointsChange( PlayerNumber pn ) {};
	virtual bool IsInDanger( PlayerNumber pn ) { return false; };
	virtual bool IsHot( PlayerNumber pn ) { return false; };
	virtual bool IsFailing( PlayerNumber pn ) { return false; };
	virtual bool FailedEarlier( PlayerNumber pn ) { return false; };

protected:
	Sprite m_sprHealthStream;
	Sprite	m_sprHealthBackground;
	float m_fLastSeenHealthPercent;

	Sprite m_sprFace;
	float m_fSecondsUntilReturnToNormalFace;

	Sprite m_sprFrame;
};


#endif

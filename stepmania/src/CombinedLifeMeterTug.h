#ifndef CombinedLifeMeterTug_H
#define CombinedLifeMeterTug_H
/*
-----------------------------------------------------------------------------
 File: CombinedLifeMeterTug.h

 Desc: 

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "LifeMeter.h"
#include "Sprite.h"
#include "MeterDisplay.h"


class CombinedLifeMeterTug : public CombinedLifeMeter
{
public:
	CombinedLifeMeterTug();
	virtual void Update( float fDelta );

	virtual void ChangeLife( PlayerNumber pn, TapNoteScore score );
	virtual void ChangeLife( PlayerNumber pn, HoldNoteScore score, TapNoteScore tscore );
	virtual void OnDancePointsChange( PlayerNumber pn ) {};
	virtual bool IsInDanger( PlayerNumber pn ) { return false; };
	virtual bool IsHot( PlayerNumber pn ) { return false; };
	virtual bool IsFailing( PlayerNumber pn ) { return false; };
	virtual bool FailedEarlier( PlayerNumber pn ) { return false; };

protected:

	MeterDisplay	m_Stream[NUM_PLAYERS];
	Sprite m_sprFrame;
};

#endif

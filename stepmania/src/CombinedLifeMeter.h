#ifndef COMBINEDLIFEMETER_H
#define COMBINEDLIFEMETER_H
/*
-----------------------------------------------------------------------------
 Class: LifeMeter

 Desc: A graphic displayed in the LifeMeterBar during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "PlayerNumber.h"
#include "GameConstantsAndTypes.h"
#include "ActorFrame.h"


class CombinedLifeMeter : public ActorFrame
{
public:
	CombinedLifeMeter() {};
	virtual ~CombinedLifeMeter() {};
	
	virtual void OnSongEnded() {};
	/* Change life after receiving a tap note grade.  This *is* called for
	 * the head of hold notes. */
	virtual void ChangeLife( PlayerNumber pn, TapNoteScore score ) = 0;
	/* Change life after receiving a hold note grade.  tscore is the score
	 * received for the initial tap note. */
	virtual void ChangeLife( PlayerNumber pn, HoldNoteScore score, TapNoteScore tscore ) = 0;
	virtual void ChangeLifeMine( PlayerNumber pn ) = 0;
	virtual void OnDancePointsChange( PlayerNumber pn ) = 0;	// look in GAMESTATE and update the display
	virtual bool IsInDanger( PlayerNumber pn ) = 0;
	virtual bool IsHot( PlayerNumber pn ) = 0;
	virtual bool IsFailing( PlayerNumber pn ) = 0;
	virtual void OnTaunt() {};
};



#endif

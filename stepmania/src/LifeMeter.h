#ifndef LIFEMETER_H
#define LIFEMETER_H
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


class LifeMeter : public ActorFrame
{
public:
	LifeMeter() {};
	virtual ~LifeMeter() {};
	
	virtual void Load( PlayerNumber pn ) { m_PlayerNumber = pn; }
	virtual void OnSongEnded() {};
	/* Change life after receiving a tap note grade.  This *is* called for
	 * the head of hold notes. */
	virtual void ChangeLife( TapNoteScore score ) = 0;
	/* Change life after receiving a hold note grade.  tscore is the score
	 * received for the initial tap note. */
	virtual void ChangeLife( HoldNoteScore score, TapNoteScore tscore ) = 0;
	virtual void ChangeLifeMine() = 0;
	virtual void OnDancePointsChange() = 0;	// look in GAMESTATE and update the display
	virtual bool IsInDanger() const = 0;
	virtual bool IsHot() const = 0;
	virtual bool IsFailing() const = 0;

	virtual float GetLife() const { return 0; } // for cosmetic use only
	virtual void UpdateNonstopLifebar(int cleared, int total, int ProgressiveLifebarDifficulty) = 0;

protected:
	PlayerNumber m_PlayerNumber;
};


#endif

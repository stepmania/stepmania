#ifndef Transition_H
#define Transition_H
/*
-----------------------------------------------------------------------------
 Class: Transition

 Desc: Transition that draws BGAnimation.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Actor.h"
#include "BGAnimation.h"
#include "ScreenMessage.h"
#include "RandomSample.h"


class Transition : public Actor
{
public:
	Transition();

	void Load( CString sBGAniDir );

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	virtual void StartTransitioning( ScreenMessage send_when_done = SM_None );

	bool IsTransitioning()	{ return m_State == transitioning; };
	bool IsFinished()	{ return m_State == finished; };
	float GetLengthSeconds();

protected:

	enum State { 
		waiting,	
		transitioning, 
		finished 
	} m_State;
	float	m_fSecsIntoTransition;


	BGAnimation	m_BGAnimation;
	RandomSample	m_sound;

	ScreenMessage	m_MessageToSendWhenDone;
};


#endif

#ifndef TransitionBGAnimation_H
#define TransitionBGAnimation_H
/*
-----------------------------------------------------------------------------
 Class: TransitionBGAnimation

 Desc: TransitionBGAnimation that draws BGAnimation.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Actor.h"
#include "BGAnimation.h"
#include "ScreenMessage.h"
#include "RandomSample.h"


class TransitionBGAnimation : public Actor
{
public:
	TransitionBGAnimation();

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

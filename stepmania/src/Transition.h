/*
-----------------------------------------------------------------------------
 File: Transition.cpp

 Desc: Abstract base class for all transitions.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/


#ifndef _Transition_H_
#define _Transition_H_


#include "RageDisplay.h"
#include "Screen.h"
#include "ScreenManager.h"
#include "Actor.h"
#include "RandomSample.h"


const float DEFAULT_TRANSITION_TIME		=	0.40f;


class Transition : public Actor
{
public:
	Transition();
	virtual ~Transition();

	virtual void Update( float fDeltaTime );
	virtual void Draw()
	{
		if( m_TransitionState == opened )
			return;		// don't draw!

		Actor::Draw();
	}

	virtual void SetOpened();
	virtual void SetClosed();

	virtual void OpenWipingRight( ScreenMessage send_when_done = SM_None );
	virtual void OpenWipingLeft(  ScreenMessage send_when_done = SM_None );
	virtual void CloseWipingRight(ScreenMessage send_when_done = SM_None );
	virtual void CloseWipingLeft( ScreenMessage send_when_done = SM_None );

	bool IsOpened()		{ return m_TransitionState == opened; };
	bool IsOpening()	{ return m_TransitionState == opening_right  ||  m_TransitionState == opening_left; };
	bool IsClosed()		{ return m_TransitionState == closed; };
	bool IsClosing()	{ return m_TransitionState == closing_right  ||  m_TransitionState == closing_left; };

	void SetTransitionTime( float fNewTransitionTime ) { m_fTransitionTime = fNewTransitionTime; };
	float GetTransitionTime() { return m_fTransitionTime; };

protected:

	enum TransitionState { opened,		  closed, 
						   opening_right, opening_left, 
						   closing_right, closing_left };

	TransitionState	m_TransitionState;
	float			m_fTransitionTime;
	float			m_fPercentThroughTransition;
	float	GetPercentageOpen()
	{
		switch( m_TransitionState )
		{
		case opening_right:
		case opening_left:
			return m_fPercentThroughTransition;
		case closing_right:
		case closing_left:
			return 1 - m_fPercentThroughTransition;
		case opened:
			return 1;
		case closed:
			return 0;
		default:
			ASSERT( false );
			return 0;
		}
	};
	float	GetPercentageClosed() { return 1-GetPercentageOpen(); };


	ScreenMessage	m_MessageToSendWhenDone;
};


#endif
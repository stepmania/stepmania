/*
-----------------------------------------------------------------------------
 File: Transition.cpp

 Desc: Abstract base class for all transitions.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/


#ifndef _Transition_H_
#define _Transition_H_


#include "RageScreen.h"
#include "Window.h"
#include "WindowManager.h"
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

	virtual void OpenWipingRight( WindowMessage send_when_done = SM_None );
	virtual void OpenWipingLeft(  WindowMessage send_when_done = SM_None );
	virtual void CloseWipingRight(WindowMessage send_when_done = SM_None );
	virtual void CloseWipingLeft( WindowMessage send_when_done = SM_None );

	bool IsOpened()		{ return m_TransitionState == opened; };
	bool IsOpening()	{ return m_TransitionState == opening_right  ||  m_TransitionState == opening_left; };
	bool IsClosed()		{ return m_TransitionState == closed; };
	bool IsClosing()	{ return m_TransitionState == closing_right  ||  m_TransitionState == closing_left; };

	void SetTransitionTime( float fNewTransitionTime ) { m_fTransitionTime = fNewTransitionTime; };

	void SetColor( D3DXCOLOR new_color ) { m_Color = new_color; };

protected:

	enum TransitionState { opened,		  closed, 
						   opening_right, opening_left, 
						   closing_right, closing_left };

	TransitionState	m_TransitionState;
	float			m_fTransitionTime;
	float			m_fPercentThroughTransition;
	float GetPercentageOpen()
	{
		switch( m_TransitionState )
		{
		case opening_right:
		case opening_left:
			return m_fPercentThroughTransition;
		case closing_right:
		case closing_left:
			return 1.0f - m_fPercentThroughTransition;
		case opened:
			return 0;
		case closed:
			return 1;
		default:
			ASSERT( false );
			return 0;
		}
	};


	WindowMessage	m_MessageToSendWhenDone;

	D3DXCOLOR m_Color;
};


#endif
#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: Transition.cpp

 Desc: Abstract base class for all transitions.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "RageUtil.h"

#include "Transition.h"
#include "ThemeManager.h"


Transition::Transition()
{
	m_TransitionState = closed,
	m_fTransitionTime = DEFAULT_TRANSITION_TIME;
	m_fPercentThroughTransition = 0.0f;
	m_Color = D3DXCOLOR(0,0,0,1);
}

Transition::~Transition()
{
}

void Transition::Update( float fDeltaTime )
{
	Actor::Update( fDeltaTime );

	switch( m_TransitionState )
	{ 
	case opening_right:
	case opening_left:
	case closing_right:
	case closing_left:

		if( m_fPercentThroughTransition > 1.0f )	// the wipe is over
		{
			m_fPercentThroughTransition = 0.0;
			switch( m_TransitionState )
			{ 
			case opening_right:
			case opening_left:
				m_TransitionState = opened;
				break;
			case closing_right:
			case closing_left:
				m_TransitionState = closed;
				break;
			}

			WM->SendMessageToTopWindow( m_MessageToSendWhenDone, 0 );

		}
		m_fPercentThroughTransition += fDeltaTime/m_fTransitionTime;
		break;
	}
}

void Transition::SetClosed()
{
	m_TransitionState = closed;
}

void Transition::SetOpened()
{
	m_TransitionState = opened;
}

void Transition::OpenWipingRight( WindowMessage send_when_done )
{
	m_MessageToSendWhenDone = send_when_done;
	m_TransitionState = opening_right;
	m_fPercentThroughTransition = 0.0;
}

void Transition::OpenWipingLeft( WindowMessage send_when_done )
{
	m_MessageToSendWhenDone = send_when_done;
	m_TransitionState = opening_left;
	m_fPercentThroughTransition = 0.0;
}

void Transition::CloseWipingRight( WindowMessage send_when_done )
{
	m_MessageToSendWhenDone = send_when_done;
	m_TransitionState = closing_right;
	m_fPercentThroughTransition = 0.0;
}

void Transition::CloseWipingLeft( WindowMessage send_when_done )
{
	m_MessageToSendWhenDone = send_when_done;
	m_TransitionState = closing_left;
	m_fPercentThroughTransition = 0.0;
}


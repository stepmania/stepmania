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

#define SOUND_BACK		"Sounds\\back.mp3"
#define SOUND_NEXT		"Sounds\\next.mp3"


Transition::Transition() :
	m_TransitionState( closed ),
	m_fTransitionTime( DEFAULT_TRANSITION_TIME ),
	m_fPercentThroughTransition( 0.0f ),
	m_Color(0,0,0,1)
{
	m_bPlayCloseWipingRightSound = TRUE;
	m_bPlayCloseWipingLeftSound = TRUE;
	m_hCloseWipingRightSound  = SOUND->LoadSample( SOUND_NEXT );
	m_hCloseWipingLeftSound = SOUND->LoadSample( SOUND_BACK );	
}

Transition::~Transition()
{
	SOUND->UnloadSample( m_hCloseWipingRightSound );
	SOUND->UnloadSample( m_hCloseWipingLeftSound );
}

void Transition::Update( float fDeltaTime )
{
	switch( m_TransitionState )
	{ 
	case opening_right:
	case opening_left:
	case closing_right:
	case closing_left:

		m_fPercentThroughTransition += fDeltaTime/m_fTransitionTime;
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
	if( m_bPlayCloseWipingRightSound )
		SOUND->PlaySample( m_hCloseWipingRightSound );
}

void Transition::CloseWipingLeft( WindowMessage send_when_done )
{
	m_MessageToSendWhenDone = send_when_done;
	m_TransitionState = closing_left;
	m_fPercentThroughTransition = 0.0;
	if( m_bPlayCloseWipingLeftSound )
		SOUND->PlaySample( m_hCloseWipingLeftSound );
}


void Transition::SetCloseWipingRightSound( CString sSoundPath )
{ 
	if( sSoundPath == "" )
	{
		SOUND->UnloadSample( m_hCloseWipingRightSound );
		m_bPlayCloseWipingRightSound = FALSE;
	}
	else
	{
		SOUND->UnloadSample( m_hCloseWipingRightSound );
		m_hCloseWipingRightSound  = SOUND->LoadSample( SOUND_NEXT );
		m_bPlayCloseWipingRightSound = TRUE;
	}
}

void Transition::SetCloseWipingLeftSound( CString sSoundPath ) 
{ 
	if( sSoundPath == "" )
	{
		SOUND->UnloadSample( m_hCloseWipingRightSound );
		m_bPlayCloseWipingLeftSound = FALSE;
	}
	else
	{
		SOUND->UnloadSample( m_hCloseWipingLeftSound );
		m_hCloseWipingLeftSound  = SOUND->LoadSample( SOUND_NEXT );
		m_bPlayCloseWipingLeftSound = TRUE;
	}
};

/*
-----------------------------------------------------------------------------
 File: Transition.cpp

 Desc: Abstract base class for all transitions.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/


#ifndef _Transition_H_
#define _Transition_H_


#include "Actor.h"
#include "RageScreen.h"
#include "RageSound.h"
#include "Window.h"
#include "WindowManager.h"


#define DEFAULT_TRANSITION_TIME		1.0f


class Transition
{
public:
	Transition();
	~Transition();

	void Update( const FLOAT &fDeltaTime );
	virtual void Draw() PURE;

	virtual void SetOpened();
	virtual void SetClosed();

	virtual void OpenWipingRight( WindowMessage send_when_done );
	virtual void OpenWipingLeft(  WindowMessage send_when_done );
	virtual void CloseWipingRight(WindowMessage send_when_done );
	virtual void CloseWipingLeft( WindowMessage send_when_done );

	bool IsClosed() { return m_TransitionState == closed; };
	bool IsClosing() { return m_TransitionState == closing_right  ||  m_TransitionState == closing_left; };

	void SetTransitionTime( FLOAT fNewTransitionTime ) { m_fTransitionTime = fNewTransitionTime; };

	void SetColor( D3DXCOLOR new_color ) { m_Color = new_color; };
	void SetCloseWipingRightSound( CString sSoundPath );
	void SetCloseWipingLeftSound( CString sSoundPath );

protected:

	enum TransitionState { opened,		  closed, 
						   opening_right, opening_left, 
						   closing_right, closing_left };

	TransitionState	m_TransitionState;
	float			m_fTransitionTime;
	float			m_fPercentThroughTransition;

	WindowMessage	m_MessageToSendWhenDone;

	bool m_bPlayCloseWipingRightSound;
	bool m_bPlayCloseWipingLeftSound;
	HSAMPLE m_hCloseWipingRightSound;
	HSAMPLE m_hCloseWipingLeftSound;

	D3DXCOLOR m_Color;
};


#endif
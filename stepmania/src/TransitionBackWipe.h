#pragma once
/*
-----------------------------------------------------------------------------
 Class: TransitionBackWipe

 Desc: Black bands (horizontal window blinds) gradually close.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Transition.h"
#include "RageDisplay.h"
#include "RageSound.h"
#include "Quad.h"


class TransitionBackWipe : public Transition
{
public:
	virtual void DrawPrimitives();

	virtual void CloseWipingRight(ScreenMessage send_when_done = SM_None ) 
	{ 
		m_soundBack.Play(); 
		Transition::CloseWipingRight(send_when_done); 
	}
	virtual void CloseWipingLeft( ScreenMessage send_when_done = SM_None )
	{ 
		m_soundBack.Play(); 
		Transition::CloseWipingLeft(send_when_done); 
	}

protected:
	Quad m_quad;
	RageSoundSample m_soundBack;
};


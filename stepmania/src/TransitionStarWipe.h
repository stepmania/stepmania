/*
-----------------------------------------------------------------------------
 File: TransitionStarWipe.cpp

 Desc: Shooting start across the screen leave a black trail.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#ifndef _TransitionStarWipe_H_
#define _TransitionStarWipe_H_


#include "Transition.h"
#include "RageDisplay.h"
#include "RageSound.h"
#include "Quad.h"


class TransitionStarWipe : public Transition
{
public:
	TransitionStarWipe();
	~TransitionStarWipe();

	virtual void DrawPrimitives();

	void OpenWipingRight( ScreenMessage send_when_done );
	void OpenWipingLeft(  ScreenMessage send_when_done );
	void CloseWipingRight(ScreenMessage send_when_done );
	void CloseWipingLeft( ScreenMessage send_when_done );

protected:
	void LoadNewStarSprite( CString sFileName );

	Sprite m_sprStar;
	int m_iStarWidth;
	int m_iStarHeight;

	Quad	m_rect;
};




#endif
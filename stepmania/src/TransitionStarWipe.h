/*
-----------------------------------------------------------------------------
 File: TransitionStarWipe.cpp

 Desc: Shooting start across the screen leave a black trail.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

#ifndef _TransitionStarWipe_H_
#define _TransitionStarWipe_H_


#include "Transition.h"
#include "RageScreen.h"
#include "RageSound.h"
#include "Rectangle.h"


class TransitionStarWipe : public Transition
{
public:
	TransitionStarWipe();
	~TransitionStarWipe();

	virtual void RenderPrimitives();

	void OpenWipingRight( WindowMessage send_when_done );
	void OpenWipingLeft(  WindowMessage send_when_done );
	void CloseWipingRight(WindowMessage send_when_done );
	void CloseWipingLeft( WindowMessage send_when_done );

protected:
	void LoadNewStarSprite( CString sFileName );

	Sprite m_sprStar;
	int m_iStarWidth;
	int m_iStarHeight;

	RectangleActor	m_rect;
};




#endif
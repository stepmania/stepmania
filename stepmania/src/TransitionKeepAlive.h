/*
-----------------------------------------------------------------------------
 Class: TransitionKeepAlive

 Desc: The transition between menu screens. "Let's Move On"

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#ifndef _TransitionKeepAlive_H_
#define _TransitionKeepAlive_H_


#include "Sprite.h"
#include "Transition.h"


class TransitionKeepAlive : public Transition
{
public:
	TransitionKeepAlive();

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	virtual void OpenWipingRight( ScreenMessage send_when_done = SM_None );
	virtual void OpenWipingLeft(  ScreenMessage send_when_done = SM_None );
	virtual void CloseWipingRight(ScreenMessage send_when_done = SM_None );
	virtual void CloseWipingLeft( ScreenMessage send_when_done = SM_None );

protected:
	Sprite m_sprLogo;
	Quad m_rect;
};




#endif

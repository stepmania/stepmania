/*
-----------------------------------------------------------------------------
 Class: TransitionKeepAlive

 Desc: The transition between menu screens. "Let's Move On"

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#ifndef _TransitionKeepAlive_H_
#define _TransitionKeepAlive_H_


#include "Sprite.h"
#include "TransitionFadeWipe.h"


class TransitionKeepAlive : public Transition
{
public:
	TransitionKeepAlive();

	virtual void Update( float fDeltaTime );
	virtual void RenderPrimitives();

	virtual void OpenWipingRight( WindowMessage send_when_done = SM_None );
	virtual void OpenWipingLeft(  WindowMessage send_when_done = SM_None );
	virtual void CloseWipingRight(WindowMessage send_when_done = SM_None );
	virtual void CloseWipingLeft( WindowMessage send_when_done = SM_None );

protected:
	Sprite m_sprLogo;
	RectangleActor m_rect;
};




#endif
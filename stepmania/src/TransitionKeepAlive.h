/*
-----------------------------------------------------------------------------
 File: TransitionKeepAlive.cpp

 Desc: Fades out or in.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
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

protected:
	Sprite m_sprLogo;
};




#endif
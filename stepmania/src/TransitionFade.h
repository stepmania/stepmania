/*
-----------------------------------------------------------------------------
 File: TransitionFade.cpp

 Desc: Fades out or in.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/


#ifndef _TransitionFade_H_
#define _TransitionFade_H_


#include "Transition.h"
#include "RageScreen.h"
#include "RageSound.h"
#include "RectangleActor.h"


class TransitionFade : public Transition
{
public:
	TransitionFade();
	virtual ~TransitionFade();

	virtual void RenderPrimitives();

protected:
	RectangleActor m_rect;
};




#endif
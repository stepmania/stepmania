/*
-----------------------------------------------------------------------------
 File: TransitionFade.cpp

 Desc: Fades out or in.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/


#ifndef _TransitionFade_H_
#define _TransitionFade_H_


#include "Transition.h"
#include "RageDisplay.h"
#include "RageSound.h"
#include "Quad.h"


class TransitionFade : public Transition
{
public:
	TransitionFade();
	virtual ~TransitionFade();

	virtual void DrawPrimitives();

protected:
	Quad m_rect;
};




#endif
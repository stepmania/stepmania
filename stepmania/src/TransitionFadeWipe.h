/*
-----------------------------------------------------------------------------
 File: TransitionFadeWipe.cpp

 Desc: Fades out or in.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#ifndef TRANSITION_FADE_WIPE_H
#define TRANSITION_FADE_WIPE_H


#include "Transition.h"
#include "Quad.h"


class TransitionFadeWipe : public Transition
{
public:
	TransitionFadeWipe();
	~TransitionFadeWipe();

	virtual void DrawPrimitives();

protected:
	Quad m_rectBlack, m_rectGradient;

};




#endif

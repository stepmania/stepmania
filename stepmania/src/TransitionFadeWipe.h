/*
-----------------------------------------------------------------------------
 File: TransitionFadeWipe.cpp

 Desc: Fades out or in.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#ifndef _TransitionFadeWipe_H_
#define _TransitionFadeWipe_H_


#include "Transition.h"
#include "RageDisplay.h"
#include "RageSound.h"
#include "Sprite.h"
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
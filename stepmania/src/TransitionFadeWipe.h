/*
-----------------------------------------------------------------------------
 File: TransitionFadeWipe.cpp

 Desc: Fades out or in.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

#ifndef _TransitionFadeWipe_H_
#define _TransitionFadeWipe_H_


#include "Transition.h"
#include "RageScreen.h"
#include "RageSound.h"
#include "Sprite.h"
#include "RectangleActor.h"


class TransitionFadeWipe : public Transition
{
public:
	TransitionFadeWipe();
	~TransitionFadeWipe();

	virtual void RenderPrimitives();

protected:
	RectangleActor m_rectBlack, m_rectGradient;

};




#endif
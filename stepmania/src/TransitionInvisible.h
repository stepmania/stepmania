/*
-----------------------------------------------------------------------------
 File: TransitionInvisible.cpp

 Desc: Fades out or in.

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/


#ifndef _TransitionInvisible_H_
#define _TransitionInvisible_H_


#include "Transition.h"
#include "RageScreen.h"
#include "RageSound.h"
#include "RectangleActor.h"


class TransitionInvisible : public Transition
{
public:
	TransitionInvisible();
	virtual ~TransitionInvisible();

	virtual void RenderPrimitives();

protected:
};




#endif
/*
-----------------------------------------------------------------------------
 File: TransitionInvisible.cpp

 Desc: Fades out or in.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/


#ifndef _TransitionInvisible_H_
#define _TransitionInvisible_H_


#include "Transition.h"
#include "RageScreen.h"
#include "RageSound.h"
#include "Rectangle.h"


class TransitionInvisible : public Transition
{
public:
	TransitionInvisible();
	virtual ~TransitionInvisible();

	virtual void RenderPrimitives();

protected:
};




#endif
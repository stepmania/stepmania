/*
-----------------------------------------------------------------------------
 File: TransitionInvisible.cpp

 Desc: Fades out or in.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/


#ifndef _TransitionInvisible_H_
#define _TransitionInvisible_H_


#include "Transition.h"
#include "RageDisplay.h"
#include "RageSound.h"
#include "Quad.h"


class TransitionInvisible : public Transition
{
public:
	TransitionInvisible();
	virtual ~TransitionInvisible();

	virtual void DrawPrimitives();

protected:
};




#endif
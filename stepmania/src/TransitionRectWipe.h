//-----------------------------------------------------------------------------
// File: TransitionRectWipe.cpp
//
// Desc: "Window blinds"-type transition.
//
// Copyright (c) 2001 Chris Danford.  All rights reserved.
//-----------------------------------------------------------------------------


#ifndef _TransitionRectWipe_H_
#define _TransitionRectWipe_H_


#include "Transition.h"
#include "RageScreen.h"
#include "RageSound.h"


class TransitionRectWipe : public Transition
{
public:
	TransitionRectWipe();
	~TransitionRectWipe();

	void Draw();
};




#endif
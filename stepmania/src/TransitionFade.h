//-----------------------------------------------------------------------------
// File: TransitionFade.cpp
//
// Desc: "Window blinds"-type transition.
//
// Copyright (c) 2001 Chris Danford.  All rights reserved.
//-----------------------------------------------------------------------------


#ifndef _TransitionFade_H_
#define _TransitionFade_H_


#include "Transition.h"
#include "RageScreen.h"
#include "RageSound.h"


class TransitionFade : public Transition
{
public:
	TransitionFade();
	~TransitionFade();

	void Draw();

};




#endif
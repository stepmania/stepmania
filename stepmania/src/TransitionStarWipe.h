//-----------------------------------------------------------------------------
// File: TransitionStarWipe.cpp
//
// Desc: "Window blinds"-type transition.
//
// Copyright (c) 2001 Chris Danford.  All rights reserved.
//-----------------------------------------------------------------------------


#ifndef _TransitionStarWipe_H_
#define _TransitionStarWipe_H_


#include "Transition.h"
#include "RageScreen.h"
#include "RageSound.h"


class TransitionStarWipe : public Transition
{
public:
	TransitionStarWipe();
	~TransitionStarWipe();

	void Draw();

	void OpenWipingRight( WindowMessage send_when_done );
	void OpenWipingLeft(  WindowMessage send_when_done );
	void CloseWipingRight(WindowMessage send_when_done );
	void CloseWipingLeft( WindowMessage send_when_done );

protected:
	void LoadNewStarSprite( CString sFileName );

	Sprite m_sprStar;
	int m_iStarWidth;
	int m_iStarHeight;
};




#endif
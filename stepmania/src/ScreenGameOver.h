/*
-----------------------------------------------------------------------------
 Class: ScreenGameOver

 Desc: Shows the stage number.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "BGAnimation.h"
#include "Transition.h"


class ScreenGameOver : public Screen
{
public:
	ScreenGameOver();

	virtual void HandleScreenMessage( const ScreenMessage SM );

	virtual void MenuStart( PlayerNumber pn );

private:

	Transition	m_In;
	Transition	m_Out;
	BGAnimation				m_Background;
};



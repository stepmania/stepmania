#ifndef SCREENSTYELESPLASH_H
#define SCREENSTYLESPLASH_H

/*
-----------------------------------------------------------------------------
 Class: ScreenStyleSplash

 Desc: Screen that displays the style the player selected for a short period of time.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "Screen.h"
#include "TransitionFade.h"
#include "TransitionFadeWipe.h"
#include "RandomSample.h"
#include "BGAnimation.h"


class ScreenStyleSplash : public Screen
{
public:
	ScreenStyleSplash();

	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

protected:
	void MenuStart( PlayerNumber pn );
	void MenuBack(	PlayerNumber pn );
	BGAnimation m_Background;
	TransitionFade		m_Wipe;
	TransitionFadeWipe	m_FadeWipe;
};

#endif

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
#include "BGAnimation.h"
#include "MenuElements.h"


class ScreenStyleSplash : public Screen
{
public:
	ScreenStyleSplash();

	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );
	virtual void DrawPrimitives();

protected:
	void MenuStart( PlayerNumber pn );
	void MenuBack(	PlayerNumber pn );

	MenuElements m_Menu;
	BGAnimation m_Background;
};

#endif

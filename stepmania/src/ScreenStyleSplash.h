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


#include "ScreenWithMenuElements.h"
#include "BGAnimation.h"


class ScreenStyleSplash : public ScreenWithMenuElements
{
public:
	ScreenStyleSplash( CString sName );

	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );
	virtual void DrawPrimitives();

protected:
	void MenuStart( PlayerNumber pn );
	void MenuBack(	PlayerNumber pn );

	BGAnimation m_Background;
};

#endif

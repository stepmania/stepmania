#ifndef ScreenSelectStyle_H
#define ScreenSelectStyle_H
/*
-----------------------------------------------------------------------------
 Class: ScreenSelectStyle

 Desc: Base class for Style, Difficulty, and Mode selection screens.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "Screen.h"
#include "MenuElements.h"
class ModeSelector;

class ScreenSelectStyle : public Screen
{
public:
	ScreenSelectStyle();
	virtual ~ScreenSelectStyle();

	virtual void Update( float fDelta );
	virtual void DrawPrimitives();
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	virtual void MenuLeft( PlayerNumber pn );
	virtual void MenuRight( PlayerNumber pn );
	virtual void MenuUp( PlayerNumber pn );
	virtual void MenuDown( PlayerNumber pn );
	virtual void MenuStart( PlayerNumber pn );
	virtual void MenuBack( PlayerNumber pn );

private:

	MenuElements m_Menu;

	ModeSelector* m_pSelector;
};

#endif

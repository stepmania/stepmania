/*
-----------------------------------------------------------------------------
 Class: ScreenEditCoursesMenu

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenWithMenuElements.h"
#include "EditCoursesMenu.h"
#include "BitmapText.h"

class ScreenEditCoursesMenu : public ScreenWithMenuElements
{
public:
	ScreenEditCoursesMenu( CString sName );
	virtual ~ScreenEditCoursesMenu();

	virtual void DrawPrimitives();
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

private:

	void MenuUp( PlayerNumber pn );
	void MenuDown( PlayerNumber pn );
	void MenuLeft( PlayerNumber pn, const InputEventType type );
	void MenuRight( PlayerNumber pn, const InputEventType type );
	void MenuBack( PlayerNumber pn );
	void MenuStart( PlayerNumber pn );

	EditCoursesMenu		m_Selector;

	BitmapText		m_textExplanation;
};




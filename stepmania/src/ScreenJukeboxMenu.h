/*
-----------------------------------------------------------------------------
 Class: ScreenJukeboxMenu

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "JukeboxMenu.h"
#include "BitmapText.h"
#include "TransitionFade.h"
#include "RandomSample.h"
#include "MenuElements.h"


class ScreenJukeboxMenu : public Screen
{
public:
	ScreenJukeboxMenu();
	virtual ~ScreenJukeboxMenu();

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

	JukeboxMenu	 m_Selector;

	BitmapText		m_textExplanation;

	MenuElements m_Menu;

	TransitionFade	m_Fade;

	RandomSample	m_soundSelect;
	RandomSample	m_soundCreate;
};




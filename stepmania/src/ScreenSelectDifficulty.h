/*
-----------------------------------------------------------------------------
 File: ScreenSelectDifficulty.h

 Desc: Select the difficulty (easy, medium, hard).

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "MenuElements.h"
class ModeSelector;


class ScreenSelectDifficulty : public Screen
{
public:

	enum Page { PAGE_1, PAGE_2, NUM_PAGES };
#define MAX_CHOICES_PER_PAGE	6

	ScreenSelectDifficulty();
	virtual ~ScreenSelectDifficulty();

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	void MenuUp( PlayerNumber pn );
	void MenuDown( PlayerNumber pn );
	void MenuLeft( PlayerNumber pn );
	void MenuRight( PlayerNumber pn );
	void MenuStart( PlayerNumber pn );
	void MenuBack( PlayerNumber pn );

private:

	MenuElements	m_Menu;

	ModeSelector*	m_pSelector;
};



/*
-----------------------------------------------------------------------------
 Class: ScreenRemoveMemoryCard

 Desc: Remind the player to remove their memory card.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "MenuElements.h"


class ScreenRemoveMemoryCard : public Screen
{
public:
	ScreenRemoveMemoryCard( CString sName );
	virtual ~ScreenRemoveMemoryCard();

	virtual void DrawPrimitives();
	virtual void Update( float fDelta );
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	virtual void MenuStart( PlayerNumber pn );
	virtual void MenuBack( PlayerNumber pn );

private:
	bool AnyCardsInserted();

	MenuElements m_Menu;
};


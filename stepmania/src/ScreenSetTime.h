/*
-----------------------------------------------------------------------------
 Class: ScreenSetTime

 Desc: Show coin drop stats.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "Sprite.h"
#include "BitmapText.h"
#include "PrefsManager.h"
#include "InputMapper.h"
#include "MenuElements.h"
#include "RageInputDevice.h"


class ScreenSetTime : public Screen
{
public:
	ScreenSetTime( CString sName );

	virtual void Update( float fDelta );
	virtual void DrawPrimitives();
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	virtual void MenuUp( PlayerNumber pn );
	virtual void MenuDown( PlayerNumber pn );
	virtual void MenuLeft( PlayerNumber pn );
	virtual void MenuRight( PlayerNumber pn );
	virtual void MenuStart( PlayerNumber pn );
	virtual void MenuBack( PlayerNumber pn );

private:
	enum Selection { hour, minute, second, year, month, day, NUM_SELECTIONS } m_Selection;
	void ChangeSelection( int iDirection );
	void ChangeValue( int iDirection );

	BitmapText	m_text[NUM_SELECTIONS];
	BitmapText	m_textDayOfWeek;

	MenuElements m_Menu;
};


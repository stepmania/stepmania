/*
-----------------------------------------------------------------------------
 Class: ScreenBookkeeping

 Desc: Where the player maps device input to instrument buttons.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "Sprite.h"
#include "BitmapText.h"
#include "PrefsManager.h"
#include "GrayArrow.h"
#include "InputMapper.h"
#include "MenuElements.h"
#include "RageInputDevice.h"

const int NUM_BOOKKEEPING_COLS = 4;

class ScreenBookkeeping : public Screen
{
public:
	ScreenBookkeeping( CString sName );
	virtual ~ScreenBookkeeping();

	virtual void DrawPrimitives();
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	virtual void MenuLeft( PlayerNumber pn );
	virtual void MenuRight( PlayerNumber pn );
	virtual void MenuStart( PlayerNumber pn );
	virtual void MenuBack( PlayerNumber pn );

private:
	enum View { LAST_7_DAYS, LAST_52_WEEKS, DAY_OF_WEEK, HOUR_OF_DAY, NUM_VIEWS };
	
	void ChangeView( View newView );

	View m_View;
	BitmapText	m_textTitle;
	BitmapText	m_textCols[NUM_BOOKKEEPING_COLS];

	MenuElements m_Menu;
};


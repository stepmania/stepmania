/*
-----------------------------------------------------------------------------
 Class: ScreenSetTime

 Desc: Show coin drop stats.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenWithMenuElements.h"
#include "Sprite.h"
#include "BitmapText.h"
#include "PrefsManager.h"
#include "InputMapper.h"
#include "RageInputDevice.h"

enum SetTimeSelection
{ 
	year, 
	month, 
	day, 
	hour, 
	minute, 
	second, 
	NUM_SET_TIME_SELECTIONS 
};

class ScreenSetTime : public ScreenWithMenuElements
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
	SetTimeSelection m_Selection;
	void ChangeSelection( int iDirection );
	void ChangeValue( int iDirection );

	BitmapText	m_textTitle[NUM_SET_TIME_SELECTIONS];
	BitmapText	m_textValue[NUM_SET_TIME_SELECTIONS];
	BitmapText	m_textDayOfWeek;
};


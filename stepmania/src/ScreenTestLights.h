/*
-----------------------------------------------------------------------------
 Class: ScreenTestLights

 Desc: Where the player maps device input to instrument buttons.

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


class ScreenTestLights : public ScreenWithMenuElements
{
public:
	ScreenTestLights( CString sName );
	virtual ~ScreenTestLights();

	virtual void DrawPrimitives();
	virtual void Update( float fDelta );
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	virtual void MenuStart( PlayerNumber pn );
	virtual void MenuBack( PlayerNumber pn );

private:
	BitmapText	m_textInputs;
};


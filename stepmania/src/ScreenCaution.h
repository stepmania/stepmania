/*
-----------------------------------------------------------------------------
 Class: ScreenCaution

 Desc: Screen that displays while SelectSong is being loaded.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "Screen.h"
#include "TransitionFade.h"
#include "RandomSample.h"


class ScreenCaution : public Screen
{
public:
	ScreenCaution();

	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	Sprite m_sprCaution;

	TransitionFade	m_Wipe;

protected:
	void MenuStart( const PlayerNumber p );
};


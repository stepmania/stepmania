#ifndef SCREENHOWTOPLAY_H
#define SCREENHOWTOPLAY_H
/*
-----------------------------------------------------------------------------
 Class: ScreenHowToPlay

 Desc: A grid of options, and the selected option is drawn with a highlight rectangle.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "Screen.h"
#include "Sprite.h"
#include "RandomSample.h"
#include "TransitionInvisible.h"
#include "MenuElements.h"


class ScreenHowToPlay : public Screen
{
public:
	ScreenHowToPlay();
	virtual ~ScreenHowToPlay();

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	virtual void MenuBack( PlayerNumber pn );
	virtual void MenuStart( PlayerNumber pn );

protected:
	Sprite			m_sprHowToPlay;
	MenuElements	m_Menu;
};


#endif

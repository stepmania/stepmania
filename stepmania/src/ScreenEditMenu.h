/*
-----------------------------------------------------------------------------
 Class: ScreenEditMenu

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "SongSelector.h"
#include "BitmapText.h"
#include "TransitionFade.h"
#include "RandomSample.h"

class ScreenEditMenu : public Screen
{
public:
	ScreenEditMenu();
	virtual ~ScreenEditMenu();

	virtual void DrawPrimitives();
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

private:

	void MenuUp( const PlayerNumber p );
	void MenuDown( const PlayerNumber p );
	void MenuLeft( const PlayerNumber p, const InputEventType type );
	void MenuRight( const PlayerNumber p, const InputEventType type );
	void MenuBack( const PlayerNumber p );
	void MenuStart( const PlayerNumber p );

	SongSelector Selector;

	BitmapText		m_textExplanation;

	MenuElements m_Menu;

	TransitionFade	m_Fade;

	RandomSample	m_soundSelect;
};




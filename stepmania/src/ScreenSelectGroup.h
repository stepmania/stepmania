/*
-----------------------------------------------------------------------------
 File: ScreenSelectGroup.h

 Desc: Set the current song group by selecting from a list.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "Sprite.h"
#include "RandomSample.h"
#include "MenuElements.h"
#include "FadingBanner.h"
#include "MusicList.h"
#include "GroupList.h"


class ScreenSelectGroup : public Screen
{
public:
	ScreenSelectGroup();
	virtual ~ScreenSelectGroup();

	virtual void DrawPrimitives();
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	void AfterChange();

	void MenuLeft( PlayerNumber pn );
	void MenuRight( PlayerNumber pn );
	void MenuUp( PlayerNumber pn );
	void MenuDown( PlayerNumber pn );
	void MenuStart( PlayerNumber pn );
	void MenuBack( PlayerNumber pn );

	void TweenOffScreen();
	void TweenOnScreen();

private:

	MenuElements	m_Menu;

	Sprite			m_sprExplanation;
	Sprite			m_sprFrame;
	Banner			m_Banner;
	BitmapText		m_textNumber;
	Sprite			m_sprContents;
	
	MusicList		m_MusicList;
	GroupList		m_GroupList;

	RandomSample m_soundChange;
	RandomSample m_soundSelect;

	bool m_bChosen;
};



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
#include "Screen.h"
#include "Quad.h"
#include "MenuElements.h"
#include "FadingBanner.h"
#include "MusicList.h"


const int MAX_GROUPS = 15;

class ScreenSelectGroup : public Screen
{
public:
	ScreenSelectGroup();
	virtual ~ScreenSelectGroup();

	virtual void DrawPrimitives();
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	void BeforeChange();
	void AfterChange();

	void MenuLeft( const PlayerNumber p );
	void MenuRight( const PlayerNumber p );
	void MenuUp( const PlayerNumber p );
	void MenuDown( const PlayerNumber p );
	void MenuStart( const PlayerNumber p );
	void MenuBack( const PlayerNumber p );

	void TweenOffScreen();
	void TweenOnScreen();

private:

	MenuElements	m_Menu;

	Sprite			m_sprExplanation;
	Sprite			m_sprFrame;
	FadingBanner	m_Banner;
	BitmapText		m_textNumber;
	Sprite			m_sprButton[MAX_GROUPS];
	BitmapText		m_textLabel[MAX_GROUPS];
	Sprite			m_sprContents;
	MusicList		m_MusicList;

	RandomSample m_soundChange;
	RandomSample m_soundSelect;

	CStringArray m_asGroupNames;
	int m_iSelection;
	bool m_bChosen;
};



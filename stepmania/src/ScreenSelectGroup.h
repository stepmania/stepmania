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
#include "TransitionKeepAlive.h"
#include "RandomSample.h"
#include "Screen.h"
#include "Quad.h"
#include "MenuElements.h"
#include "GroupInfoFrame.h"


const int MAX_GROUPS = 12;
const int NUM_CONTENTS_COLUMNS = 3;

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

	void MenuLeft( PlayerNumber p );
	void MenuRight( PlayerNumber p );
	void MenuUp( PlayerNumber p );
	void MenuDown( PlayerNumber p );
	void MenuStart( PlayerNumber p );
	void MenuBack( PlayerNumber p );

	void TweenOffScreen();
	void TweenOnScreen();

private:

	MenuElements m_Menu;

	Sprite			m_sprExplanation;
	GroupInfoFrame	m_GroupInfoFrame;
	Sprite			m_sprGroupButton[MAX_GROUPS];
	BitmapText		m_textGroup[MAX_GROUPS];
	Sprite			m_sprContentsHeader;
	BitmapText		m_textContents[NUM_CONTENTS_COLUMNS];

	RandomSample m_soundChange;
	RandomSample m_soundSelect;
	RandomSample m_soundFlyOff;

	CStringArray m_arrayGroupNames;
	int m_iSelection;
	bool m_bChosen;

	TransitionKeepAlive m_Fade;
};



/*
-----------------------------------------------------------------------------
 File: ScreenSelectDifficulty.h

 Desc: Select the game mode (single, versus, double).

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
#include "RandomSample.h"
#include "ModeChoice.h"


class ScreenSelectDifficulty : public Screen
{
public:

	enum Page { PAGE_1, PAGE_2, NUM_PAGES };
#define MAX_CHOICES_PER_PAGE	6

	ScreenSelectDifficulty();
	virtual ~ScreenSelectDifficulty();

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	void MenuLeft( PlayerNumber pn );
	void MenuRight( PlayerNumber pn );
	void MenuStart( PlayerNumber pn );
	void MenuBack( PlayerNumber pn );

	void TweenOffScreen();
	void TweenOnScreen();

private:
	void ChangeWithinPage( PlayerNumber pn, int iNewChoice, bool bChangingPages );
	void ChangePage( int iNewPage );

	MenuElements m_Menu;

	ActorFrame	m_framePages;	// to hold the 2 pages

	Sprite	m_sprHeader[NUM_PAGES][MAX_CHOICES_PER_PAGE];
	Sprite	m_sprPicture[NUM_PAGES][MAX_CHOICES_PER_PAGE];
	Sprite	m_sprExplanation[NUM_PAGES];
	Sprite	m_sprMoreArrows[NUM_PAGES];

	Sprite	m_sprCursor[NUM_PLAYERS];
	Sprite	m_sprJoinMessagehadow[NUM_PLAYERS];
	Sprite	m_sprOK[NUM_PLAYERS];

	RageSound	m_soundChange;
	RageSound	m_soundSelect;
	RandomSample m_soundDifficult;

	vector<ModeChoice> m_ModeChoices[NUM_PAGES];

	int m_iCurrentPage;
	int m_iChoiceOnPage[NUM_PLAYERS];
	bool m_bChosen[NUM_PLAYERS];

	float m_fLockInputTime;
};



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


const int NUM_ITEMS_ON_PAGE_1 = 3;	// easy, medium, hard, 
const int NUM_ITEMS_ON_PAGE_2 = 2;	// Oni, Endless
const int NUM_DIFFICULTY_ITEMS = NUM_ITEMS_ON_PAGE_1 + NUM_ITEMS_ON_PAGE_2;
const int NUM_PAGES = 2;	// easy-medium-hard, Oni

class ScreenSelectDifficulty : public Screen
{
public:
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
	void ChangeTo( PlayerNumber pn, int iSelectionWas, int iSelectionIs );

	bool IsItemOnPage2( int iItemIndex );
	bool SelectedSomethingOnPage2();	// checks selection of players

	MenuElements m_Menu;

	ActorFrame	m_framePages;	// 2 pages

	Sprite	m_sprHeader[NUM_DIFFICULTY_ITEMS];
	Sprite	m_sprPicture[NUM_DIFFICULTY_ITEMS];
	Sprite	m_sprExplanation[NUM_PAGES];
	Sprite	m_sprMoreArrows[NUM_PAGES];

	Sprite	m_sprCursor[NUM_PLAYERS];
	Sprite	m_sprJoinMessagehadow[NUM_PLAYERS];
	Sprite	m_sprOK[NUM_PLAYERS];

	RageSoundSample m_soundChange;
	RageSoundSample m_soundSelect;
	RandomSample m_soundDifficult;

	int m_iSelection[NUM_PLAYERS];
	bool m_bChosen[NUM_PLAYERS];

	float m_fLockInputTime;
};



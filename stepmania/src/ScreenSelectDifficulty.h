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


const int NUM_DIFFICULTY_ITEMS = NUM_DIFFICULTY_CLASSES + 2;	// easy, medium, hard, Oni, Endless
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

	void MenuLeft( const PlayerNumber p );
	void MenuRight( const PlayerNumber p );
	void MenuStart( const PlayerNumber p );
	void MenuBack( const PlayerNumber p );

	void TweenOffScreen();
	void TweenOnScreen();

private:
	void ChangeTo( const PlayerNumber pn, int iSelectionWas, int iSelectionIs );

	bool IsItemOnPage2( int iItemIndex );
	bool SelectedSomethingOnPage2();	// checks selection of players

	MenuElements m_Menu;

	ActorFrame	m_framePages;	// 2 pages

	Sprite	m_sprHeader[NUM_DIFFICULTY_ITEMS];
	Sprite	m_sprPicture[NUM_DIFFICULTY_ITEMS];
	Sprite	m_sprExplanation[NUM_PAGES];
	Sprite	m_sprMoreArrows[NUM_PAGES];

	Sprite	m_sprCursor[NUM_PLAYERS];
	Sprite	m_sprCursorShadow[NUM_PLAYERS];
	Sprite	m_sprOK[NUM_PLAYERS];

	RageSoundSample m_soundChange;
	RageSoundSample m_soundSelect;

	bool m_bPlayedChallengeSound;

	int m_iSelection[NUM_PLAYERS];
	bool m_bChosen[NUM_PLAYERS];

	float m_fLockInputTime;
};



/*
-----------------------------------------------------------------------------
 File: ScreenSelectDifficulty.h

 Desc: Select the game mode (single, versus, double).

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "Sprite.h"
#include "TransitionKeepAlive.h"
#include "RandomSample.h"
#include "Screen.h"
#include "Quad.h"
#include "MenuElements.h"



class ScreenSelectDifficulty : public Screen
{
public:
	ScreenSelectDifficulty();
	virtual ~ScreenSelectDifficulty();

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

	MenuElements m_Menu;

	Sprite	m_sprDifficultyHeader[NUM_DIFFICULTY_CLASSES];
	Sprite	m_sprDifficultyPicture[NUM_DIFFICULTY_CLASSES];
	Sprite	m_sprArrow[NUM_PLAYERS];
	Sprite	m_sprArrowShadow[NUM_PLAYERS];
	Sprite	m_sprOK[NUM_PLAYERS];
	Sprite	m_sprExplanation;

	RandomSample m_soundChange;
	RandomSample m_soundSelect;

	int m_iSelection[NUM_PLAYERS];
	bool m_bChosen[NUM_PLAYERS];

	TransitionKeepAlive m_Fade;
};



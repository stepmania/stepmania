/*
-----------------------------------------------------------------------------
 File: ScreenSelectDifficultyEX.h

 Desc: Select the game mode - DDRExtreme style.

 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Kevin Slaughter
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


class ScreenSelectDifficultyEX : public Screen
{
public:

	enum ChoiceEx
	{
		CHOICE_EX_BEGINNER = 0,
		CHOICE_EX_EASY,
		CHOICE_EX_MEDIUM,
		CHOICE_EX_HARD,
		CHOICE_EX_NONSTOP,
		CHOICE_EX_ONI,	
		CHOICE_EX_ENDLESS,
		CHOICE_EX_BATTLE,
		NUM_CHOICES_EX
	};


	ScreenSelectDifficultyEX();
	virtual ~ScreenSelectDifficultyEX();

private:
	void ChangeTo( PlayerNumber pn, int iOldChoice, int iNewChoice );
	void ShowSelected( PlayerNumber pv );

	MenuElements m_Menu;

	ActorFrame	m_framePages;

	Sprite	m_sprHeader[NUM_PLAYERS];
	Sprite	m_sprPicture[NUM_PLAYERS];

	Sprite	m_sprCursor[NUM_PLAYERS];
	Sprite	m_sprJoinMessageShadow[NUM_PLAYERS];
	Sprite	m_sprOK[NUM_PLAYERS];

	RageSound	m_soundChange;
	RageSound	m_soundSelect;
	RandomSample m_soundDifficult;

	ChoiceEx m_Choice[NUM_PLAYERS];
	bool m_bChosen[NUM_PLAYERS];
	bool AnyPlayerIsOnCourse();
	bool PlayerIsOnCourse( PlayerNumber pl );

	float m_fLockInputTime;


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
};
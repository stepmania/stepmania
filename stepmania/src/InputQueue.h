#pragma once
/*
-----------------------------------------------------------------------------
 Class: InputQueue

 Desc: Stores a list of the most recently pressed MenuInputs for each player.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "MenuInput.h"
#include "GameConstantsAndTypes.h"

const int MAX_INPUT_QUEUE_LENGTH = 8;

class InputQueue
{
public:
	InputQueue();

	void HandleInput( const PlayerNumber p, const MenuButton b );
	bool MatchesPattern( const PlayerNumber p, const MenuButton* button_sequence, const int iNumButtons, float fMaxSecondsBack = -1 );


protected:
	struct MenuButtonAndTime
	{
		MenuButtonAndTime() {}
		MenuButtonAndTime( MenuButton b, float t ) { button = b; fTime = t; };
		MenuButton	button;
		float		fTime;
	};
	CArray<MenuButtonAndTime,MenuButtonAndTime> m_aQueue[NUM_PLAYERS];
};


extern InputQueue*	INPUTQUEUE;	// global and accessable from anywhere in our program

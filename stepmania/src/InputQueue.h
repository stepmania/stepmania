#ifndef INPUTQUEUE_H
#define INPUTQUEUE_H
/*
-----------------------------------------------------------------------------
 Class: InputQueue

 Desc: Stores a list of the most recently pressed MenuInputs for each player.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GameConstantsAndTypes.h"
#include "GameInput.h"
#include "MenuInput.h"

const unsigned MAX_INPUT_QUEUE_LENGTH = 8;

class InputQueue
{
public:
	InputQueue();

	void RememberInput( GameInput );
	bool MatchesSequence( GameController c, const GameButton* button_sequence, int iNumButtons, float fMaxSecondsBack );
	bool MatchesSequence( GameController c, const MenuButton* button_sequence, int iNumButtons, float fMaxSecondsBack );
	bool AllWerePressedRecently( GameController c, const GameButton* buttons, int iNumButtons, float fMaxSecondsBack );

protected:
	struct GameButtonAndTime
	{
		GameButtonAndTime() {}
		GameButtonAndTime( GameButton b, float t ) { button = b; fTime = t; };
		GameButton	button;
		float		fTime;
	};
	vector<GameButtonAndTime> m_aQueue[MAX_GAME_CONTROLLERS];
};


extern InputQueue*	INPUTQUEUE;	// global and accessable from anywhere in our program

#endif

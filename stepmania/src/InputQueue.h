/* InputQueue - Stores a list of the most recently pressed MenuInputs for each player. */

#ifndef INPUTQUEUE_H
#define INPUTQUEUE_H

#include "GameConstantsAndTypes.h"
#include "GameInput.h"
#include "MenuInput.h"

const unsigned MAX_INPUT_QUEUE_LENGTH = 16;

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

/*
 * (c) 2001-2003 Chris Danford
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/* InputQueue - Stores a list of the most recently pressed MenuInputs for each player. */

#ifndef INPUT_QUEUE_H
#define INPUT_QUEUE_H

#include "GameConstantsAndTypes.h"
#include "GameInput.h"

const unsigned MAX_INPUT_QUEUE_LENGTH = 16;
class InputEventPlus;
class RageTimer;

class InputQueue
{
public:
	InputQueue();

	void RememberInput( const InputEventPlus &gi );
	bool MatchesSequence( GameController c, const GameButton* button_sequence, int iNumButtons, float fMaxSecondsBack );
	bool WasPressedRecently( GameController c, const GameButton button, const RageTimer &OldestTimeAllowed, InputEventPlus *pIEP = NULL );
	bool AllWerePressedRecently( GameController c, const GameButton* buttons, int iNumButtons, float fMaxSecondsBack );

protected:
	vector<InputEventPlus> m_aQueue[NUM_GameController];
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

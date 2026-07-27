#ifndef INPUT_QUEUE_H
#define INPUT_QUEUE_H

#include "GameInput.h"
#include "InputFilter.h"

#include <vector>


class InputEventPlus;
class RageTimer;

/** @brief Stores a list of the most recently pressed MenuInputs for each player. */
class InputQueue
{
public:
	InputQueue();

	void RememberInput( const InputEventPlus &gi );
	bool WasPressedRecently( GameController c, const GameButton button, const RageTimer &OldestTimeAllowed, InputEventPlus *pIEP = nullptr );
	const std::vector<InputEventPlus> &GetQueue( GameController c ) const { return m_aQueue[c]; }
	void ClearQueue( GameController c );

protected:
	std::vector<InputEventPlus> m_aQueue[NUM_GameController];
};

struct InputQueueCode
{
public:
	bool Load( RString sButtonsNames );
	bool EnteredCode( GameController controller ) const;

	InputQueueCode(): m_aPresses() {}

private:
	struct ButtonPress
	{
		ButtonPress(): m_aButtonsToHold(), m_aButtonsToNotHold(),
			m_aButtonsToPress(),
			m_bAllowIntermediatePresses(false)
		{
			memset( m_InputTypes, 0, sizeof(m_InputTypes) );
			m_InputTypes[IET_FIRST_PRESS] = true;
		}
		std::vector<GameButton> m_aButtonsToHold;
		std::vector<GameButton> m_aButtonsToNotHold;
		std::vector<GameButton> m_aButtonsToPress;

		bool m_InputTypes[NUM_InputEventType];
		bool m_bAllowIntermediatePresses;
	};
	std::vector<ButtonPress> m_aPresses;

	float m_fMaxSecondsBack;
};

extern InputQueue*	INPUTQUEUE;	// global and accessible from anywhere in our program

#endif

/*
 * (c) 2001-2007 Chris Danford, Glenn Maynard
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

#include "global.h"
#include "InputQueue.h"
#include "GameManager.h"
#include "RageException.h"
#include "RageTimer.h"
#include "InputMapper.h"


InputQueue*	INPUTQUEUE = NULL;	// global and accessable from anywhere in our program


InputQueue::InputQueue()
{
	FOREACH_GameController ( gc )
		m_aQueue[gc].insert(m_aQueue[gc].begin(), MAX_INPUT_QUEUE_LENGTH, GameButtonAndTime() );
}

void InputQueue::RememberInput( const GameInput GameI )
{
	int c = GameI.controller;
	if( m_aQueue[c].size() >= MAX_INPUT_QUEUE_LENGTH )	// full
		m_aQueue[c].erase( m_aQueue[c].begin(), m_aQueue[c].begin() + (m_aQueue[c].size()-MAX_INPUT_QUEUE_LENGTH+1) );
	m_aQueue[c].push_back( GameButtonAndTime(GameI.button,RageTimer::GetTimeSinceStart()) );
}

bool InputQueue::MatchesSequence( GameController c, const MenuButton* button_sequence, const int iNumButtons, float fMaxSecondsBack )
{
	if( c == GAME_CONTROLLER_INVALID )
		return false;

	if( fMaxSecondsBack == -1 )
		fMaxSecondsBack = 0.4f + iNumButtons*0.15f;

	float fOldestTimeAllowed = RageTimer::GetTimeSinceStart() - fMaxSecondsBack;

	int sequence_index = iNumButtons-1;	// count down
	for( int queue_index=m_aQueue[c].size()-1; queue_index>=0; queue_index-- )	// iterate newest to oldest
	{
		GameButtonAndTime BandT = m_aQueue[c][queue_index];
		GameInput GameI( c, BandT.button );
		MenuInput MenuI;
		INPUTMAPPER->GameToMenu( GameI, MenuI );
		if( MenuI.button != button_sequence[sequence_index]  ||
			BandT.fTime < fOldestTimeAllowed )
		{
			return false;
		}
		if( sequence_index == 0 )		// we matched the whole pattern
		{
			m_aQueue[c].clear();	// empty the queue so we don't match on it again
			return true;
		}
		sequence_index--;
	}
	return false;
}

bool InputQueue::MatchesSequence( GameController c, const GameButton* button_sequence, const int iNumButtons, float fMaxSecondsBack )
{
	if( c == GAME_CONTROLLER_INVALID )
		return false;

	if( fMaxSecondsBack == -1 )
		fMaxSecondsBack = 0.4f + iNumButtons*0.15f;

	float fOldestTimeAllowed = RageTimer::GetTimeSinceStart() - fMaxSecondsBack;

	int sequence_index = iNumButtons-1;	// count down
	for( int queue_index=m_aQueue[c].size()-1; queue_index>=0; queue_index-- )	// iterate newest to oldest
	{
		GameButtonAndTime BandT = m_aQueue[c][queue_index];
		if( BandT.button != button_sequence[sequence_index]  ||
			BandT.fTime < fOldestTimeAllowed )
		{
			return false;
		}
		if( sequence_index == 0 )		// we matched the whole pattern
		{
			m_aQueue[c].clear();	// empty the queue so we don't match on it again
			return true;
		}
		sequence_index--;
	}
	return false;
}

bool InputQueue::AllWerePressedRecently( GameController c, const GameButton* buttons, int iNumButtons, float fMaxSecondsBack )
{
	float fOldestTimeAllowed = RageTimer::GetTimeSinceStart() - fMaxSecondsBack;

	for( int b=0; b<iNumButtons; b++ )
	{
		GameButton button = buttons[b];

		for( int queue_index=m_aQueue[c].size()-1; queue_index>=0; queue_index-- )	// iterate newest to oldest
		{
			GameButtonAndTime BandT = m_aQueue[c][queue_index];
			if( BandT.fTime < fOldestTimeAllowed )	// buttons are too old.  Stop searching because we're not going to find a match
				return false;

			if( BandT.button == button )
				goto found_button;
		}
		return false;	// didn't find the button
found_button:
		;	// hush VC6
	}

	m_aQueue[c].clear();	// empty the queue so we don't match on it again
	return true;
}

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

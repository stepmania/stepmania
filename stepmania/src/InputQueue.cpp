#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: InputQueue

 Desc: See Header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "InputQueue.h"
#include "IniFile.h"
#include "GameManager.h"
#include "RageException.h"
#include "RageTimer.h"
#include "InputMapper.h"


InputQueue*	INPUTQUEUE = NULL;	// global and accessable from anywhere in our program


InputQueue::InputQueue()
{
	for( int p=0; p<NUM_PLAYERS; p++ )
		m_aQueue[p].insert(m_aQueue[p].begin(), MAX_INPUT_QUEUE_LENGTH, GameButtonAndTime() );
}

void InputQueue::RememberInput( const GameInput GameI )
{
	int c = GameI.controller;
	/* XXX: this should be a deque, and just pop_back */
	if( m_aQueue[c].size() >= MAX_INPUT_QUEUE_LENGTH )	// full
		m_aQueue[c].erase( m_aQueue[c].begin(), m_aQueue[c].begin() + (m_aQueue[c].size()-MAX_INPUT_QUEUE_LENGTH+1) );
	m_aQueue[c].push_back( GameButtonAndTime(GameI.button,RageTimer::GetTimeSinceStart()) );
}

bool InputQueue::MatchesPattern( const GameController c, const MenuButton* button_sequence, const int iNumButtons, float fMaxSecondsBack )
{
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

bool InputQueue::MatchesPattern( const GameController c, const GameButton* button_sequence, const int iNumButtons, float fMaxSecondsBack )
{
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
#include "stdafx.h"
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


InputQueue*	INPUTQUEUE = NULL;	// global and accessable from anywhere in our program


InputQueue::InputQueue()
{
	for( int p=0; p<NUM_PLAYERS; p++ )
		m_aQueue[p].SetSize( MAX_INPUT_QUEUE_LENGTH );
}

void InputQueue::RememberInput( const GameInput GameI )
{
	int c = GameI.controller;
	if( m_aQueue[c].GetSize() >= MAX_INPUT_QUEUE_LENGTH )	// full
		m_aQueue[c].RemoveAt( 0, m_aQueue[c].GetSize()-MAX_INPUT_QUEUE_LENGTH+1 );
	m_aQueue[c].Add( GameButtonAndTime(GameI.button,TIMER->GetTimeSinceStart()) );
};

bool InputQueue::MatchesPattern( const GameController c, const GameButton* button_sequence, const int iNumButtons, float fMaxSecondsBack )
{
	if( fMaxSecondsBack == -1 )
		fMaxSecondsBack = 0.4f + iNumButtons*0.15f;

	float fOldestTimeAllowed = TIMER->GetTimeSinceStart() - fMaxSecondsBack;

	int sequence_index = iNumButtons-1;	// count down
	for( int queue_index=m_aQueue[c].GetSize()-1; queue_index>=0; queue_index-- )	// iterate newest to oldest
	{
		GameButtonAndTime BandT = m_aQueue[c][queue_index];
		if( BandT.button != button_sequence[sequence_index]  ||
			BandT.fTime < fOldestTimeAllowed )
		{
			return false;
		}
		if( sequence_index == 0 )		// we matched the whole pattern
		{
			m_aQueue[c].RemoveAll();	// empty the queue so we don't match on it again
			return true;
		}
		sequence_index--;
	}
	return false;
}
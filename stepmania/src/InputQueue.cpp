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

void InputQueue::HandleInput( const PlayerNumber p, const MenuButton b )
{
	if( m_aQueue[p].GetSize() >= MAX_INPUT_QUEUE_LENGTH )	// full
		m_aQueue[p].RemoveAt( 0, m_aQueue[p].GetSize()-MAX_INPUT_QUEUE_LENGTH+1 );
	m_aQueue[p].Add( MenuButtonAndTime(b,TIMER->GetTimeSinceStart()) );
};

bool InputQueue::MatchesPattern( const PlayerNumber p, const MenuButton* button_sequence, const int iNumButtons, float fMaxSecondsBack )
{
	if( fMaxSecondsBack == -1 )
		fMaxSecondsBack = 0.4f + iNumButtons*0.15f;

	float fOldestTimeAllowed = TIMER->GetTimeSinceStart() - fMaxSecondsBack;

	int sequence_index = iNumButtons-1;	// count down
	for( int queue_index=m_aQueue[p].GetSize()-1; queue_index>=0; queue_index-- )	// iterate newest to oldest
	{
		MenuButtonAndTime BandT = m_aQueue[p][queue_index];
		if( BandT.button != button_sequence[sequence_index]  ||
			BandT.fTime < fOldestTimeAllowed )
		{
			return false;
		}
		if( sequence_index == 0 )		// we matched the whole pattern
		{
			m_aQueue[p].RemoveAll();	// empty the queue so we don't match on it again
			return true;
		}
		sequence_index--;
	}
	return false;
}
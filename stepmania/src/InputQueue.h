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
	InputQueue()
	{
		for( int p=0; p<NUM_PLAYERS; p++ )
			m_aQueue[p].SetSize( MAX_INPUT_QUEUE_LENGTH );
	}

	void HandleInput( const PlayerNumber p, const MenuButton b )
	{
		if( m_aQueue[p].GetSize() >= MAX_INPUT_QUEUE_LENGTH )	// full
			m_aQueue[p].RemoveAt( 0, m_aQueue[p].GetSize()-MAX_INPUT_QUEUE_LENGTH+1 );
		m_aQueue[p].Add( MenuButtonAndTime(b,TIMER->GetTimeSinceStart()) );
	};
	bool MatchesPattern( const PlayerNumber p, const MenuButton* button_sequence, const int iNumButtons, float fMaxSecondsBack = -1 )
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

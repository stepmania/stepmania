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
			m_aMenuButtonQueue[p].SetSize( MAX_INPUT_QUEUE_LENGTH );
	}

	void HandleInput( const PlayerNumber p, const MenuButton b )
	{
		if( m_aMenuButtonQueue[p].GetSize() >= MAX_INPUT_QUEUE_LENGTH )	// full
			m_aMenuButtonQueue[p].RemoveAt( 0, m_aMenuButtonQueue[p].GetSize()-MAX_INPUT_QUEUE_LENGTH+1 );
		m_aMenuButtonQueue[p].Add( MenuButtonAndTickCount(b,GetTickCount()) );
	};
	bool MatchesPattern( const PlayerNumber p, const MenuButton* button_sequence, const int iNumButtons, float fMaxSecondsBack = -1 )
	{
		if( fMaxSecondsBack == -1 )
			fMaxSecondsBack = 0.4f + iNumButtons*0.15f;

		DWORD dwMaxTicksBack = roundf(fMaxSecondsBack*1000);
		DWORD dwOldestTickAllowed = GetTickCount() - dwMaxTicksBack;

		int sequence_index = iNumButtons-1;	// count down
		for( int queue_index=m_aMenuButtonQueue[p].GetSize()-1; queue_index>=0; queue_index-- )	// iterate newest to oldest
		{
			MenuButtonAndTickCount BandT = m_aMenuButtonQueue[p][queue_index];
			if( BandT.button != button_sequence[sequence_index]  ||
				BandT.dwTickCount < dwOldestTickAllowed )
			{
				return false;
			}
			if( sequence_index == 0 )		// we matched the whole pattern
			{
				m_aMenuButtonQueue[p].RemoveAll();	// empty the queue so we don't match on it again
				return true;
			}
			sequence_index--;
		}
		return false;
	}

protected:
	struct MenuButtonAndTickCount
	{
		MenuButtonAndTickCount() {}
		MenuButtonAndTickCount( MenuButton b, DWORD t ) { button = b; dwTickCount = t; };
		MenuButton button;
		DWORD	dwTickCount;
	};
	CArray<MenuButtonAndTickCount,MenuButtonAndTickCount> m_aMenuButtonQueue[NUM_PLAYERS];
};


extern InputQueue*	INPUTQUEUE;	// global and accessable from anywhere in our program

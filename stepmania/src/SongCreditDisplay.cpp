#include "global.h"
/*
-----------------------------------------------------------------------------
 File: SongCreditDisplay.h

 Desc: A graphic displayed in the SongCreditDisplay during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "SongCreditDisplay.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "song.h"
#include "Steps.h"

SongCreditDisplay::SongCreditDisplay()
{
	m_text.LoadFromFont( THEME->GetPathToF("SongCreditDisplay",(CString)"text") );
	
	Song* pSong = GAMESTATE->m_pCurSong;
	CString s;
	s += pSong->GetFullDisplayTitle() + "\n";
	s += pSong->GetDisplayArtist() + "\n";
	if( !pSong->m_sCredit.empty() )
		s += pSong->m_sCredit + "\n";

	// use a vector and not a set so that ordering is maintained
	vector<Steps*> vpStepsToShow;
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( GAMESTATE->IsHumanPlayer(p) )
			vpStepsToShow.push_back( GAMESTATE->m_pCurNotes[p] );
	}
	for( unsigned i=0; i<vpStepsToShow.size(); i++ )
	{
		Steps* pSteps = vpStepsToShow[i];
		s += Capitalize( DifficultyToString(pSteps->GetDifficulty()) ) + " steps by " + pSteps->GetDescription();
		s += "\n";
	}

	// erase the last newline
	s.erase( s.end()-1 );

	m_text.SetText( s );
	m_text.SetHorizAlign( Actor::align_left );
	this->AddChild( &m_text );
}
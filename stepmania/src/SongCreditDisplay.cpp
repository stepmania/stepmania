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
#include "SongManager.h"

SongCreditDisplay::SongCreditDisplay()
{
	this->LoadFromFont( THEME->GetPathToF("SongCreditDisplay",(CString)"text") );
	
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
		if( !GAMESTATE->IsHumanPlayer(p) )
			continue;	// skip
		
		Steps* pSteps = GAMESTATE->m_pCurNotes[p];
		bool bAlreadyAdded = find( vpStepsToShow.begin(), vpStepsToShow.end(), pSteps ) != vpStepsToShow.end();
		if( !bAlreadyAdded )
			vpStepsToShow.push_back( pSteps );
	}
	for( unsigned i=0; i<vpStepsToShow.size(); i++ )
	{
		Steps* pSteps = vpStepsToShow[i];
		CString sDifficulty = SONGMAN->GetDifficultyThemeName( pSteps->GetDifficulty() );
		
		// HACK: reset capitalization
		sDifficulty.MakeLower();
		sDifficulty = Capitalize( sDifficulty );
		
		s += sDifficulty + " steps: " + pSteps->GetDescription();
		s += "\n";
	}

	// erase the last newline
	s.erase( s.end()-1 );

	this->SetText( s );
}


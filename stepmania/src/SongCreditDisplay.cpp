#include "global.h"
#include "SongCreditDisplay.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "song.h"
#include "Steps.h"
#include "SongManager.h"

SongCreditDisplay::SongCreditDisplay()
{
	if( GAMESTATE->IsCourseMode() )
		return;

	this->LoadFromFont( THEME->GetPathF("SongCreditDisplay","text") );
	Song* pSong = GAMESTATE->m_pCurSong;
	ASSERT( pSong );

	CString s;
	s += pSong->GetFullDisplayTitle() + "\n";
	s += pSong->GetDisplayArtist() + "\n";
	if( !pSong->m_sCredit.empty() )
		s += pSong->m_sCredit + "\n";

	// use a vector and not a set so that ordering is maintained
	vector<Steps*> vpStepsToShow;
	FOREACH_PlayerNumber( p )
	{
		if( !GAMESTATE->IsHumanPlayer(p) )
			continue;	// skip
		
		Steps* pSteps = GAMESTATE->m_pCurSteps[p];
		bool bAlreadyAdded = find( vpStepsToShow.begin(), vpStepsToShow.end(), pSteps ) != vpStepsToShow.end();
		if( !bAlreadyAdded )
			vpStepsToShow.push_back( pSteps );
	}
	for( unsigned i=0; i<vpStepsToShow.size(); i++ )
	{
		Steps* pSteps = vpStepsToShow[i];
		CString sDifficulty = DifficultyToThemedString( pSteps->GetDifficulty() );
		
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

/*
 * (c) 2004 Chris Danford
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

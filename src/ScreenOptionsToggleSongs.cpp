#include "global.h"
#include "ScreenOptionsToggleSongs.h"
#include "OptionRowHandler.h"
#include "RageUtil.h"
#include "ScreenManager.h"
#include "Song.h"
#include "SongManager.h"
#include "UnlockManager.h"
#include "PrefsManager.h"
#include "MessageManager.h"

#include <vector>


// main page (group list)
REGISTER_SCREEN_CLASS( ScreenOptionsToggleSongs );

void ScreenOptionsToggleSongs::BeginScreen()
{
	m_asGroups.clear();

	std::vector<OptionRowHandler*> vHands;

	std::vector<RString> asAllGroups;
	SONGMAN->GetSongGroupNames(asAllGroups);
	for (RString const &sGroup : asAllGroups)
	{
		vHands.push_back( OptionRowHandlerUtil::MakeNull() );
		OptionRowDefinition &def = vHands.back()->m_Def;

		def.m_sName = sGroup;
		def.m_sExplanationName = "Select Group";
		def.m_bAllowThemeTitle = false;	// not themable
		def.m_bAllowThemeItems = false;	// already themed
		def.m_bOneChoiceForAllPlayers = true;
		def.m_vsChoices.clear();
		def.m_vsChoices.push_back( "" );

		m_asGroups.push_back( sGroup );
	}
	ScreenOptions::InitMenu( vHands );

	ScreenOptions::BeginScreen();
}

void ScreenOptionsToggleSongs::ProcessMenuStart( const InputEventPlus &input )
{
	if( IsTransitioning() )
		return;

	// switch to the subpage with the specified group
	int iRow = GetCurrentRow();
	if( m_pRows[iRow]->GetRowType() == OptionRow::RowType_Exit )
	{
		ScreenOptions::ProcessMenuStart( input );
		return;
	}

	ToggleSongs::m_sGroup = m_asGroups[iRow];
	SCREENMAN->SetNewScreen("ScreenOptionsToggleSongsSubPage");
}

void ScreenOptionsToggleSongs::ImportOptions( int row, const std::vector<PlayerNumber> &vpns )
{

}
void ScreenOptionsToggleSongs::ExportOptions( int row, const std::vector<PlayerNumber> &vpns )
{

}

// subpage (has the songs in a specific group)
REGISTER_SCREEN_CLASS( ScreenOptionsToggleSongsSubPage );
void ScreenOptionsToggleSongsSubPage::BeginScreen()
{
	m_apSongs.clear();

	std::vector<OptionRowHandler*> vHands;

	const std::vector<Song*> &apAllSongs = SONGMAN->GetSongs(ToggleSongs::m_sGroup);
	for (Song *pSong : apAllSongs)
	{
		if( UNLOCKMAN->SongIsLocked(pSong) & ~LOCKED_DISABLED )
			continue;

		vHands.push_back( OptionRowHandlerUtil::MakeNull() );
		OptionRowDefinition &def = vHands.back()->m_Def;

		def.m_sName = pSong->GetTranslitFullTitle();
		def.m_bAllowThemeTitle = false;	// not themable
		def.m_sExplanationName = "Toggle Song";
		def.m_bOneChoiceForAllPlayers = true;
		def.m_vsChoices.clear();
		def.m_vsChoices.push_back( "On" );
		def.m_vsChoices.push_back( "Off" );
		def.m_bAllowThemeItems = false;	// already themed

		m_apSongs.push_back( pSong );
	}

	InitMenu( vHands );

	ScreenOptions::BeginScreen();
}

void ScreenOptionsToggleSongsSubPage::ImportOptions( int iRow, const std::vector<PlayerNumber> &vpns )
{
	if( iRow >= (int)m_apSongs.size() )	// exit row
		return;

	OptionRow &row = *m_pRows[iRow];
	bool bEnable = m_apSongs[iRow]->GetEnabled();
	int iSelection = bEnable? 0:1;
	row.SetOneSharedSelection( iSelection );
}

void ScreenOptionsToggleSongsSubPage::ExportOptions( int iRow, const std::vector<PlayerNumber> &vpns )
{
	if( iRow >= (int)m_apSongs.size() )	// exit row
		return;

	const OptionRow &row = *m_pRows[iRow];
	int iSelection = row.GetOneSharedSelection();
	bool bEnable = (iSelection == 0);
	m_apSongs[iRow]->SetEnabled( bEnable );

	SONGMAN->SaveEnabledSongsToPref();
	PREFSMAN->SavePrefsToDisk();
}

/*
 * (c) 2007 Glenn Maynard
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

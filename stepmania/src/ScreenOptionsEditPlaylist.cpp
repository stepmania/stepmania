#include "global.h"

#include "ScreenOptionsEditPlaylist.h"
#include "ScreenMiniMenu.h"
#include "SongUtil.h"
#include "SongManager.h"
#include "OptionRowHandler.h"
#include "song.h"
#include "Workout.h"
#include "GameState.h"
#include "ScreenPrompt.h"
#include "LocalizedString.h"
#include "WorkoutManager.h"
#include "song.h"

static const MenuRowDef g_MenuSong =	MenuRowDef( -1,	"Song",	true, EditMode_Practice, false, true, 0, "Off", "On" );

REGISTER_SCREEN_CLASS( ScreenOptionsEditPlaylist );

void ScreenOptionsEditPlaylist::Init()
{
	ScreenOptions::Init();
	SongUtil::GetAllSongGenres( m_vsSongGenres );
}

void ScreenOptionsEditPlaylist::BeginScreen()
{
	vector<OptionRowHandler*> vHands;

	FOREACH_CONST( RString, m_vsSongGenres, s )
	{
		OptionRowHandler *pHand = OptionRowHandlerUtil::MakeSimple( g_MenuSong );
		vHands.push_back( pHand );
		pHand->m_Def.m_sName = (*s);
		pHand->m_Def.m_sExplanationName = "Enable Song Genre";
		pHand->m_Def.m_layoutType = LAYOUT_SHOW_ALL_IN_ROW;
		pHand->m_Def.m_bExportOnChange = true;
	}

	ScreenOptions::InitMenu( vHands );

	ScreenOptions::BeginScreen();

	this->AfterChangeRow( PLAYER_1 );
}

ScreenOptionsEditPlaylist::~ScreenOptionsEditPlaylist()
{

}

void ScreenOptionsEditPlaylist::ImportOptions( int iRow, const vector<PlayerNumber> &vpns )
{
	OptionRow &row = *m_pRows[iRow];
	if( row.GetRowType() == OptionRow::RowType_Exit )
		return;

	RString sSongGenre = m_vsSongGenres[iRow];

	bool bEnabled = false;
	FOREACH_CONST( RString, WORKOUTMAN->m_pCurWorkout->m_vsSongGenres, s )
	{
		if( *s == sSongGenre )
		{
			bEnabled = true;
			break;
		}
	}

	row.SetOneSharedSelection( bEnabled ? 1:0 );
}

void ScreenOptionsEditPlaylist::ExportOptions( int iRow, const vector<PlayerNumber> &vpns )
{
	OptionRow &row = *m_pRows[iRow];
	if( row.GetRowType() == OptionRow::RowType_Exit )
		return;

	RString sSongGenre = m_vsSongGenres[iRow];

	bool bEnabled = !!row.GetOneSharedSelection();
	vector<RString>::iterator iter = find( WORKOUTMAN->m_pCurWorkout->m_vsSongGenres.begin(), WORKOUTMAN->m_pCurWorkout->m_vsSongGenres.end(), sSongGenre );
	bool bAlreadyExists = iter != WORKOUTMAN->m_pCurWorkout->m_vsSongGenres.end();
	if( bEnabled && !bAlreadyExists )
	{
		WORKOUTMAN->m_pCurWorkout->m_vsSongGenres.push_back( sSongGenre );
		sort( WORKOUTMAN->m_pCurWorkout->m_vsSongGenres.begin(), WORKOUTMAN->m_pCurWorkout->m_vsSongGenres.end() );
	}
	else if( !bEnabled && bAlreadyExists )
	{
		WORKOUTMAN->m_pCurWorkout->m_vsSongGenres.erase( iter );
	}
}

void ScreenOptionsEditPlaylist::GoToNextScreen()
{
}

void ScreenOptionsEditPlaylist::GoToPrevScreen()
{
}

void ScreenOptionsEditPlaylist::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_ExportOptions )
	{
		//g_Workout.m_vpSongs.clear();
	}

	ScreenOptions::HandleScreenMessage( SM );
}

static RString g_sSongGenre;

void ScreenOptionsEditPlaylist::AfterChangeRow( PlayerNumber pn )
{
	ScreenOptions::AfterChangeRow( pn );

	int iRow = m_iCurrentRow[pn];
	OptionRow &row = *m_pRows[iRow];
	if( row.GetRowType() == OptionRow::RowType_Exit )
		g_sSongGenre = "";
	else
		g_sSongGenre = m_vsSongGenres[iRow];

	MESSAGEMAN->Broadcast( "EditPlaylistSongGenreChanged" );
}

void ScreenOptionsEditPlaylist::AfterChangeValueInRow( int iRow, PlayerNumber pn )
{
	ScreenOptions::AfterChangeValueInRow( iRow, pn );

	MESSAGEMAN->Broadcast( "WorkoutChanged" );
}

const int MIN_ENABLED_SONGS = 10;

static LocalizedString MUST_ENABLE_AT_LEAST("ScreenOptionsEditPlaylist","You must enable at least %d songs.");
void ScreenOptionsEditPlaylist::ProcessMenuStart( const InputEventPlus &input )
{
	if( IsTransitioning() )
		return;

	int iRow = m_iCurrentRow[GAMESTATE->m_MasterPlayerNumber];

	vector<Song*> vpSongGenreSongs;
	WORKOUTMAN->GetWorkoutSongsForGenres( WORKOUTMAN->m_pCurWorkout->m_vsSongGenres, vpSongGenreSongs );

	if( m_pRows[iRow]->GetRowType() == OptionRow::RowType_Exit  &&  vpSongGenreSongs.size() < unsigned(MIN_ENABLED_SONGS) )
	{
		ScreenPrompt::Prompt( SM_None, ssprintf(MUST_ENABLE_AT_LEAST.GetValue(),MIN_ENABLED_SONGS) );
		return;
	}

	ScreenOptions::ProcessMenuStart( input );
}

RString GetEditPlaylistSelectedSongGenreText( int iColumnIndex )
{
	vector<RString> vsSongGenres;
	vsSongGenres.push_back( g_sSongGenre );
	vector<Song*> vpSongGenreSongs;
	WORKOUTMAN->GetWorkoutSongsForGenres( vsSongGenres, vpSongGenreSongs );

	const int iSongPerCoumn = 20;
	vector<RString> vs;
	for( int i=iColumnIndex*iSongPerCoumn; i<(iColumnIndex+1)*iSongPerCoumn && i<(int)vpSongGenreSongs.size(); i++ )
	{
		Song *pSong = vpSongGenreSongs[i];
		RString s2 = pSong->GetDisplayFullTitle();
		//if( s2.size() > 16 )
		//	s2 = s2.Left(14) + "...";
		//s2.Replace( " ", "&nbsp;" );
		vs.push_back( s2 );
	}

	return join("\n", vs);
}

#include "LuaManager.h"
LuaFunction( GetEditPlaylistSelectedSongGenre, g_sSongGenre );
LuaFunction( GetEditPlaylistSelectedSongGenreText, GetEditPlaylistSelectedSongGenreText(IArg(1)) );


/*
 * (c) 2003-2004 Chris Danford
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

#include "global.h"
#include "ScreenEditCourseEntry.h"
#include "RageLog.h"
#include "GameState.h"
#include "SongManager.h"
#include "CommonMetrics.h"
#include "GameManager.h"
#include "song.h"

enum EditCourseEntryRow
{
	ROW_SONG_GROUP, 
	ROW_SONG, 
	ROW_BASE_DIFFICULTY, 
	ROW_LOW_METER,
	ROW_HIGH_METER, 
	ROW_SORT, 
	ROW_CHOOSE_INDEX, 
	ROW_SET_MODS, 
	ROW_DONE,
	NUM_EditCourseEntryRow
};
#define FOREACH_EditCourseEntryRow( i ) FOREACH_ENUM( EditCourseEntryRow, NUM_EditCourseEntryRow, i )


REGISTER_SCREEN_CLASS( ScreenEditCourseEntry );
ScreenEditCourseEntry::ScreenEditCourseEntry( CString sName ) : ScreenOptions( sName )
{
	LOG->Trace( "ScreenEditCourseEntry::ScreenEditCourseEntry()" );
}

void ScreenEditCourseEntry::Init()
{
	ScreenOptions::Init();


	// save a backup that we'll use if we revert.
	Course *pCourse = GAMESTATE->m_pCurCourse;
	m_Original = pCourse->m_vEntries[ GAMESTATE->m_iEditCourseEntryIndex ];


	vector<OptionRowDefinition> vDefs;
	vector<OptionRowHandler*> vHands;

	OptionRowDefinition def;
	def.layoutType = LAYOUT_SHOW_ONE_IN_ROW;
	
	def.name = "Song Group";
	def.choices.clear();
	vector<CString> vsSongGroups;
	SONGMAN->GetSongGroupNames( vsSongGroups );
	def.choices.push_back( "(any)" );
	FOREACH_CONST( CString, vsSongGroups, s )
		def.choices.push_back( *s );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	def.name = "Song";
	def.choices.clear();
	def.choices.push_back( "" );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	def.name = "Base Difficulty";
	def.choices.clear();
	def.choices.push_back( "(any)" );
	FOREACH_CONST( Difficulty, DIFFICULTIES_TO_SHOW.GetValue(), dc )
		def.choices.push_back( DifficultyToThemedString(*dc) );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	def.name = "Low Meter";
	def.choices.clear();
	def.choices.push_back( "(any)" );
	for( int i=MIN_METER; i<=MAX_METER; i++ )
		def.choices.push_back( ssprintf("%i",i) );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	def.name = "High Meter";
	def.choices.clear();
	def.choices.push_back( "(any)" );
	for( int i=MIN_METER; i<=MAX_METER; i++ )
		def.choices.push_back( ssprintf("%i",i) );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	def.name = "Sort";
	def.choices.clear();
	FOREACH_SongSort( i )
		def.choices.push_back( SongSortToThemedString(i) );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	def.name = "Choose";
	def.choices.clear();
	for( int i=0; i<20; i++ )
		def.choices.push_back( FormatNumberAndSuffix(i+1) );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	def.name = "Set Mods";
	def.choices.clear();
	def.choices.push_back( "Set Mods" );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	ScreenOptions::InitMenu( INPUTMODE_SHARE_CURSOR, vDefs, vHands );

	ImportAllOptions();
}

void ScreenEditCourseEntry::HandleScreenMessage( const ScreenMessage SM )
{
	ScreenOptions::HandleScreenMessage( SM );
}
	
void ScreenEditCourseEntry::AfterChangeValueInRow( PlayerNumber pn )
{
	ScreenOptions::AfterChangeValueInRow( pn );
	Course *pCourse = GAMESTATE->m_pCurCourse;
	CourseEntry &ce = pCourse->m_vEntries[ GAMESTATE->m_iEditCourseEntryIndex ];

	switch( m_iCurrentRow[pn] )
	{
	case ROW_SONG_GROUP:
		// export song group
		{
			OptionRow &row = *m_pRows[ROW_SONG_GROUP];
			int iChoice = row.GetChoiceInRowWithFocus( GAMESTATE->m_MasterPlayerNumber );
			if( iChoice == 0 )
				ce.sSongGroup = "";
			else
				ce.sSongGroup = row.GetRowDef().choices[ iChoice ];
		}
		// refresh songs
		{
			OptionRow &row = *m_pRows[ROW_SONG];
			OptionRowDefinition def = row.GetRowDef();
			def.choices.clear();
			def.choices.push_back( "(any)" );
			vector<Song*> vpSongs;
			if( ce.sSongGroup.empty() )
				SONGMAN->GetSongs( vpSongs );
			else
				SONGMAN->GetSongs( vpSongs, ce.sSongGroup );
			FOREACH_CONST( Song*, vpSongs, s )
				def.choices.push_back( (*s)->GetTranslitFullTitle() );
			vector<Song*>::const_iterator iter = find( vpSongs.begin(), vpSongs.end(), ce.pSong );
			if( iter != vpSongs.end() )
			{
				int iSongIndex = iter - vpSongs.begin();
				row.SetOneSharedSelection( iSongIndex+1 );
			}
			else
			{
				row.SetOneSharedSelection( 0 );
			}
			row.Reload( def );
		}
		// fall through
	case ROW_SONG:
		// export song
		{
			OptionRow &row = *m_pRows[ROW_SONG];
			int iChoice = row.GetChoiceInRowWithFocus( GAMESTATE->m_MasterPlayerNumber );
			if( iChoice == 0 )
				ce.pSong = NULL;
			else
				ce.pSong = SONGMAN->GetAllSongs()[iChoice-1];
		}
		// fall through
	case ROW_BASE_DIFFICULTY:
		// export difficulty
		{
			OptionRow &row = *m_pRows[ROW_BASE_DIFFICULTY];
			int iChoice = row.GetChoiceInRowWithFocus( GAMESTATE->m_MasterPlayerNumber );
			ce.baseDifficulty = DIFFICULTIES_TO_SHOW.GetValue()[iChoice];
		}
		// fall through
	case ROW_LOW_METER:
		// export low meter
		{
			OptionRow &row = *m_pRows[ROW_LOW_METER];
			int iChoice = row.GetChoiceInRowWithFocus( GAMESTATE->m_MasterPlayerNumber );
			if( iChoice == 0 )
				ce.iLowMeter = -1;
			else
				ce.iLowMeter = iChoice;
		}
		// fall through
	case ROW_HIGH_METER:
		// export high meter
		{
			OptionRow &row = *m_pRows[ROW_HIGH_METER];
			int iChoice = row.GetChoiceInRowWithFocus( GAMESTATE->m_MasterPlayerNumber );
			if( iChoice == 0 )
				ce.iHighMeter = -1;
			else
				ce.iHighMeter = iChoice;
		}
		// fall through
	case ROW_SORT:
		// export sort
		{
			OptionRow &row = *m_pRows[ROW_SORT];
			int iChoice = row.GetChoiceInRowWithFocus( GAMESTATE->m_MasterPlayerNumber );
			ce.songSort = (SongSort)iChoice;
		}
		// fall through
	case ROW_CHOOSE_INDEX:
		// export choose index
		{
			OptionRow &row = *m_pRows[ROW_CHOOSE_INDEX];
			int iChoice = row.GetChoiceInRowWithFocus( GAMESTATE->m_MasterPlayerNumber );
			ce.iChooseIndex = iChoice;
		}
		// fall through
	case ROW_SET_MODS:
		// fall through
	default:
		; // nothing left to do
	}
}

void ScreenEditCourseEntry::ImportAllOptions()
{
	// fill choices before importing
	AfterChangeValueInRow( GAMESTATE->m_MasterPlayerNumber );


	Course *pCourse = GAMESTATE->m_pCurCourse;
	CourseEntry &ce = pCourse->m_vEntries[ GAMESTATE->m_iEditCourseEntryIndex ];

	// import song group
	{
		OptionRow &row = *m_pRows[ROW_SONG_GROUP];
		FOREACH_CONST( CString, row.GetRowDef().choices, s )
		{
			if( *s == ce.sSongGroup )
			{
				int iChoice = s - row.GetRowDef().choices.begin();
				row.SetOneSharedSelection( iChoice );
				AfterChangeValueInRow( GAMESTATE->m_MasterPlayerNumber );
				break;
			}
		}
	}
	// import song
	{
		vector<Song*> vpSongs;
		SONGMAN->GetSongs( vpSongs, ce.sSongGroup );
		vector<Song*>::const_iterator iter = find( vpSongs.begin(), vpSongs.end(), ce.pSong );
		int iChoice = 0;
		if( iter != vpSongs.end() )
			iChoice = iter - vpSongs.begin() + 1;
		OptionRow &row = *m_pRows[ROW_SONG];
		row.SetOneSharedSelection( iChoice );
	}
	// import base difficulty
	{
		vector<Difficulty>::const_iterator iter = find( DIFFICULTIES_TO_SHOW.GetValue().begin(), DIFFICULTIES_TO_SHOW.GetValue().end(), ce.baseDifficulty );
		int iChoice = 0;
		if( iter != DIFFICULTIES_TO_SHOW.GetValue().end() )
			iChoice = iter - DIFFICULTIES_TO_SHOW.GetValue().begin() + 1;
		OptionRow &row = *m_pRows[ROW_BASE_DIFFICULTY];
		row.SetOneSharedSelection( iChoice );
	}
	// import low meter
	{
		int iChoice = 0;
		if( ce.iLowMeter != -1 )
			iChoice = ce.iLowMeter;
		OptionRow &row = *m_pRows[ROW_LOW_METER];
		row.SetOneSharedSelection( iChoice );
	}
	// import high meter
	{
		int iChoice = 0;
		if( ce.iHighMeter != -1 )
			iChoice = ce.iHighMeter;
		OptionRow &row = *m_pRows[ROW_HIGH_METER];
		row.SetOneSharedSelection( iChoice );
	}
	// import sort
	{
		int iChoice = ce.songSort;
		OptionRow &row = *m_pRows[ROW_SORT];
		row.SetOneSharedSelection( iChoice );
	}
	// import choose index
	{
		int iChoice = ce.iChooseIndex;
		OptionRow &row = *m_pRows[ROW_CHOOSE_INDEX];
		row.SetOneSharedSelection( iChoice );
	}
}

void ScreenEditCourseEntry::ExportOptions( int row, const vector<PlayerNumber> &vpns )
{

}

void ScreenEditCourseEntry::GoToNextScreen()
{
	switch( m_iCurrentRow[GAMESTATE->m_MasterPlayerNumber] )
	{
	case ROW_SONG_GROUP: 
	case ROW_SONG: 
	case ROW_BASE_DIFFICULTY: 
	case ROW_LOW_METER:
	case ROW_HIGH_METER: 
	case ROW_CHOOSE_INDEX: 
		break;
	case ROW_SET_MODS:
		SCREENMAN->SetNewScreen( "ScreenEditCourseMods" );
		break;
	case ROW_DONE:
		SCREENMAN->SetNewScreen( "ScreenEditCourse" );
		break;
	}
}

void ScreenEditCourseEntry::GoToPrevScreen()
{
	// revert
	Course *pCourse = GAMESTATE->m_pCurCourse;
	pCourse->m_vEntries[ GAMESTATE->m_iEditCourseEntryIndex ] = m_Original;

	SCREENMAN->SetNewScreen( "ScreenEditCourse" );
}

void ScreenEditCourseEntry::ProcessMenuStart( PlayerNumber pn, const InputEventType type )
{
	switch( m_iCurrentRow[GAMESTATE->m_MasterPlayerNumber] )
	{
	case ROW_SONG_GROUP: 
	case ROW_SONG: 
	case ROW_BASE_DIFFICULTY: 
	case ROW_LOW_METER:
	case ROW_HIGH_METER: 
	case ROW_SORT: 
	case ROW_CHOOSE_INDEX: 
		break;
	case ROW_SET_MODS:
	case ROW_DONE:
		ScreenOptions::ProcessMenuStart( pn, type );
		break;
	}
}

/*
 * (c) 2002-2004 Chris Danford
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

#include "global.h"
#include "ScreenOptionsEditCourseEntry.h"
#include "ScreenManager.h"
#include "RageLog.h"
#include "GameState.h"
#include "SongManager.h"
#include "CommonMetrics.h"
#include "GameManager.h"
#include "song.h"
#include "Steps.h"

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

static void FillSongsAndChoices( const CString &sSongGroup, vector<Song*> &vpSongsOut, vector<CString> &vsChoicesOut )
{
	vpSongsOut.clear();
	vsChoicesOut.clear();

	if( sSongGroup.empty() )
		SONGMAN->GetSongs( vpSongsOut );
	else
		SONGMAN->GetSongs( vpSongsOut, sSongGroup );
	vpSongsOut.insert( vpSongsOut.begin(), NULL );

	FOREACH_CONST( Song*, vpSongsOut, s )
	{
		if( *s == NULL )
			vsChoicesOut.push_back( "(any)" );
		else
			vsChoicesOut.push_back( (*s)->GetTranslitFullTitle() );
	}
}


REGISTER_SCREEN_CLASS( ScreenOptionsEditCourseEntry );
ScreenOptionsEditCourseEntry::ScreenOptionsEditCourseEntry( CString sName ) : ScreenOptionsEditCourseSubMenu( sName )
{
	LOG->Trace( "ScreenOptionsEditCourseEntry::ScreenOptionsEditCourseEntry()" );
}

void ScreenOptionsEditCourseEntry::Init()
{
	ScreenOptions::Init();


	// save a backup that we'll use if we revert.
	Course *pCourse = GAMESTATE->m_pCurCourse;
	const CourseEntry &ce = pCourse->m_vEntries[ GAMESTATE->m_iEditCourseEntryIndex ];
	m_Original = ce;


	vector<OptionRowDefinition> vDefs;
	vector<OptionRowHandler*> vHands;

	OptionRowDefinition def;
	def.m_layoutType = LAYOUT_SHOW_ONE_IN_ROW;
	def.m_bExportOnChange = true;
	
	def.m_sName = "Song Group";
	def.m_vsChoices.clear();
	vector<CString> vsSongGroups;
	SONGMAN->GetSongGroupNames( vsSongGroups );
	def.m_vsChoices.push_back( "(any)" );
	FOREACH_CONST( CString, vsSongGroups, s )
		def.m_vsChoices.push_back( *s );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	def.m_sName = "Song";
	def.m_vsChoices.clear();
	FillSongsAndChoices( ce.sSongGroup, m_vpDisplayedSongs, def.m_vsChoices );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	def.m_sName = "Base Difficulty";
	def.m_vsChoices.clear();
	def.m_vsChoices.push_back( "(any)" );
	FOREACH_CONST( Difficulty, DIFFICULTIES_TO_SHOW.GetValue(), dc )
		def.m_vsChoices.push_back( DifficultyToThemedString(*dc) );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	def.m_sName = "Low Meter";
	def.m_vsChoices.clear();
	def.m_vsChoices.push_back( "(any)" );
	for( int i=MIN_METER; i<=MAX_METER; i++ )
		def.m_vsChoices.push_back( ssprintf("%i",i) );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	def.m_sName = "High Meter";
	def.m_vsChoices.clear();
	def.m_vsChoices.push_back( "(any)" );
	for( int i=MIN_METER; i<=MAX_METER; i++ )
		def.m_vsChoices.push_back( ssprintf("%i",i) );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	def.m_sName = "Sort";
	def.m_vsChoices.clear();
	FOREACH_SongSort( i )
		def.m_vsChoices.push_back( SongSortToThemedString(i) );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	def.m_sName = "Choose";
	def.m_vsChoices.clear();
	for( int i=0; i<20; i++ )
		def.m_vsChoices.push_back( FormatNumberAndSuffix(i+1) );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	def.m_sName = "Set Mods";
	def.m_vsChoices.clear();
	CString s = ssprintf( "%d mod changes", ce.GetNumModChanges() );
	def.m_vsChoices.push_back( s );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	ScreenOptions::InitMenu( vDefs, vHands );
}

void ScreenOptionsEditCourseEntry::BeginScreen()
{
	ScreenOptions::BeginScreen();
}

void ScreenOptionsEditCourseEntry::HandleScreenMessage( const ScreenMessage SM )
{
	Course *pCourse = GAMESTATE->m_pCurCourse;

	if( SM == SM_GoToNextScreen )
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
			{
				Trail *pTrail = GAMESTATE->m_pCurTrail[PLAYER_1];
				TrailEntry *pTrailEntry = &pTrail->m_vEntries[GAMESTATE->m_iEditCourseEntryIndex];
				Song *pSong = pTrailEntry->pSong;
				Steps *pSteps = pTrailEntry->pSteps;

				// Set up for ScreenEdit
				const Style *pStyle = GAMEMAN->GetEditorStyleForStepsType(pSteps->m_StepsType);
				GAMESTATE->m_pCurStyle.Set( pStyle );
				GAMESTATE->m_pCurSong.Set( pSong );
				GAMESTATE->m_pCurSteps[PLAYER_1].Set( pSteps );

				SCREENMAN->SetNewScreen( "ScreenEditCourseMods" );
			}
			break;
		case ROW_DONE:
			SCREENMAN->SetNewScreen( "ScreenOptionsEditCourse" );
			break;
		}
		return;
	}
	else if( SM == SM_GoToPrevScreen )
	{
		// revert
		pCourse->m_vEntries[ GAMESTATE->m_iEditCourseEntryIndex ] = m_Original;

		SCREENMAN->SetNewScreen( "ScreenOptionsEditCourse" );
		return;
	}

	ScreenOptions::HandleScreenMessage( SM );
}
	
void ScreenOptionsEditCourseEntry::AfterChangeValueInRow( int iRow, PlayerNumber pn )
{
	ScreenOptions::AfterChangeValueInRow( iRow, pn );

	Course *pCourse = GAMESTATE->m_pCurCourse;
	GAMESTATE->m_pCurTrail[PLAYER_1].Set( NULL );
	Trail *pTrail = pCourse->GetTrailForceRegenCache( GAMESTATE->m_stEdit, GAMESTATE->m_PreferredCourseDifficulty[PLAYER_1] );
	int iEntryIndex = GAMESTATE->m_iEditCourseEntryIndex;
	ASSERT( iEntryIndex >= 0 && iEntryIndex < pCourse->m_vEntries.size() );
	CourseEntry &ce = pCourse->m_vEntries[ iEntryIndex ];

	switch( iRow )
	{
	case ROW_SONG_GROUP:
		// refresh songs
		{
			vector<PlayerNumber> vpns;
			vpns.push_back( PLAYER_1 );
			ExportOptions( ROW_SONG_GROUP, vpns );

			OptionRow &row = *m_pRows[ROW_SONG];
			OptionRowDefinition def = row.GetRowDef();
			FillSongsAndChoices( ce.sSongGroup, m_vpDisplayedSongs, def.m_vsChoices );

			row.Reload( def );

			vector<Song*>::const_iterator iter = find( m_vpDisplayedSongs.begin(), m_vpDisplayedSongs.end(), ce.pSong );
			if( iter == m_vpDisplayedSongs.end() )
			{
				this->ChangeValueInRowAbsolute( ROW_SONG, PLAYER_1, 0, false );
			}
			else
			{
				int iSongIndex = iter - m_vpDisplayedSongs.begin();
				this->ChangeValueInRowAbsolute( ROW_SONG, PLAYER_1, iSongIndex, false );
			}
		}
		break;
	}


	// cause overlay elements to refresh by changing the course
	GAMESTATE->m_pCurCourse.Set( pCourse );
	GAMESTATE->m_pCurTrail[PLAYER_1].Set( pTrail );

}

void ScreenOptionsEditCourseEntry::ImportOptions( int iRow, const vector<PlayerNumber> &vpns )
{
	Course *pCourse = GAMESTATE->m_pCurCourse;
	int iEntryIndex = GAMESTATE->m_iEditCourseEntryIndex;
	ASSERT( iEntryIndex >= 0 && iEntryIndex < pCourse->m_vEntries.size() );
	CourseEntry &ce = pCourse->m_vEntries[ iEntryIndex ];

	OptionRow &row = *m_pRows[iRow];

	switch( iRow )
	{
	case ROW_SONG_GROUP:
		FOREACH_CONST( CString, row.GetRowDef().m_vsChoices, s )
		{
			if( *s == ce.sSongGroup )
			{
				int iChoice = s - row.GetRowDef().m_vsChoices.begin();
				row.SetOneSharedSelection( iChoice );
				AfterChangeValueInRow( iRow, GAMESTATE->m_MasterPlayerNumber );
				break;
			}
		}
		break;
	case ROW_SONG:
		{
			vector<Song*>::const_iterator iter = find( m_vpDisplayedSongs.begin(), m_vpDisplayedSongs.end(), ce.pSong );
			int iChoice = 0;
			if( iter != m_vpDisplayedSongs.end() )
				iChoice = iter - m_vpDisplayedSongs.begin();
			OptionRow &row = *m_pRows[ROW_SONG];
			row.SetOneSharedSelection( iChoice );
		}
		break;
	case ROW_BASE_DIFFICULTY:
		{
			vector<Difficulty>::const_iterator iter = find( DIFFICULTIES_TO_SHOW.GetValue().begin(), DIFFICULTIES_TO_SHOW.GetValue().end(), ce.baseDifficulty );
			int iChoice = 0;
			if( iter != DIFFICULTIES_TO_SHOW.GetValue().end() )
				iChoice = iter - DIFFICULTIES_TO_SHOW.GetValue().begin() + 1;
			OptionRow &row = *m_pRows[ROW_BASE_DIFFICULTY];
			row.SetOneSharedSelection( iChoice );
		}
		break;
	case ROW_LOW_METER:
		{
			int iChoice = 0;
			if( ce.iLowMeter != -1 )
				iChoice = ce.iLowMeter;
			OptionRow &row = *m_pRows[ROW_LOW_METER];
			row.SetOneSharedSelection( iChoice );
		}
		break;
	case ROW_HIGH_METER:
		{
			int iChoice = 0;
			if( ce.iHighMeter != -1 )
				iChoice = ce.iHighMeter;
			OptionRow &row = *m_pRows[ROW_HIGH_METER];
			row.SetOneSharedSelection( iChoice );
		}
		break;
	case ROW_SORT:
		{
			int iChoice = ce.songSort;
			OptionRow &row = *m_pRows[ROW_SORT];
			row.SetOneSharedSelection( iChoice );
		}
		break;
	case ROW_CHOOSE_INDEX:
		{
			int iChoice = ce.iChooseIndex;
			OptionRow &row = *m_pRows[ROW_CHOOSE_INDEX];
			row.SetOneSharedSelection( iChoice );
		}
	}
}

void ScreenOptionsEditCourseEntry::ExportOptions( int iRow, const vector<PlayerNumber> &vpns )
{
	Course *pCourse = GAMESTATE->m_pCurCourse;
	int iEntryIndex = GAMESTATE->m_iEditCourseEntryIndex;
	ASSERT( iEntryIndex >= 0 && iEntryIndex < pCourse->m_vEntries.size() );
	CourseEntry &ce = pCourse->m_vEntries[ iEntryIndex ];

	switch( iRow )
	{
	case ROW_SONG_GROUP:
		{
			OptionRow &row = *m_pRows[ROW_SONG_GROUP];
			int iChoice = row.GetChoiceInRowWithFocus( GAMESTATE->m_MasterPlayerNumber );
			if( iChoice == 0 )
				ce.sSongGroup = "";
			else
				ce.sSongGroup = row.GetRowDef().m_vsChoices[ iChoice ];
		}
		break;
	case ROW_SONG:
		{
			OptionRow &row = *m_pRows[ROW_SONG];
			int iChoice = row.GetChoiceInRowWithFocus( GAMESTATE->m_MasterPlayerNumber );
			ce.pSong = m_vpDisplayedSongs[iChoice];
		}
		break;
	case ROW_BASE_DIFFICULTY:
		{
			OptionRow &row = *m_pRows[ROW_BASE_DIFFICULTY];
			int iChoice = row.GetChoiceInRowWithFocus( GAMESTATE->m_MasterPlayerNumber );
			if( iChoice == 0 )
			{
				ce.baseDifficulty = DIFFICULTY_INVALID;
			}
			else
			{
				Difficulty d = DIFFICULTIES_TO_SHOW.GetValue()[iChoice-1];
				ce.baseDifficulty = d;
			}
		}
		break;
	case ROW_LOW_METER:
		{
			OptionRow &row = *m_pRows[ROW_LOW_METER];
			int iChoice = row.GetChoiceInRowWithFocus( GAMESTATE->m_MasterPlayerNumber );
			if( iChoice == 0 )
				ce.iLowMeter = -1;
			else
				ce.iLowMeter = iChoice;
		}
		break;
	case ROW_HIGH_METER:
		{
			OptionRow &row = *m_pRows[ROW_HIGH_METER];
			int iChoice = row.GetChoiceInRowWithFocus( GAMESTATE->m_MasterPlayerNumber );
			if( iChoice == 0 )
				ce.iHighMeter = -1;
			else
				ce.iHighMeter = iChoice;
		}
		break;
	case ROW_SORT:
		{
			OptionRow &row = *m_pRows[ROW_SORT];
			int iChoice = row.GetChoiceInRowWithFocus( GAMESTATE->m_MasterPlayerNumber );
			ce.songSort = (SongSort)iChoice;
		}
		break;
	case ROW_CHOOSE_INDEX:
		{
			OptionRow &row = *m_pRows[ROW_CHOOSE_INDEX];
			int iChoice = row.GetChoiceInRowWithFocus( GAMESTATE->m_MasterPlayerNumber );
			ce.iChooseIndex = iChoice;
		}
		break;
	}
}

void ScreenOptionsEditCourseEntry::ProcessMenuStart( PlayerNumber pn, const InputEventType type )
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
		ScreenOptions::BeginFadingOut();
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

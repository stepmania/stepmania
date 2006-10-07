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
#include "OptionRowHandler.h"
#include "PlayerState.h"
#include "SongUtil.h"

enum EditCourseEntryRow
{
	ROW_SONG_GROUP,
	ROW_SONG,
	ROW_BASE_DIFFICULTY,
	ROW_LOW_METER,
	ROW_HIGH_METER,
	ROW_SORT,
	ROW_CHOOSE_INDEX,
	ROW_SECRET,
	ROW_SET_MODS,
	ROW_DONE,
	NUM_EditCourseEntryRow
};
#define FOREACH_EditCourseEntryRow( i ) FOREACH_ENUM2( EditCourseEntryRow, i )

AutoScreenMessage( SM_BackFromCoursePlayerOptions );

static void FillSongsAndChoices( const RString &sSongGroup, vector<Song*> &vpSongsOut, vector<RString> &vsChoicesOut )
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


class OptionRowHandlerSongChoices: public OptionRowHandler
{
public:
	// corresponds with m_vsChoices:
	vector<Song*> m_vpDisplayedSongs;
	RString m_sSongGroup;

	OptionRowHandlerSongChoices() { Init(); }
	
	virtual void LoadInternal( const Commands &cmds )
	{
		FillSongsAndChoices( m_sSongGroup, m_vpDisplayedSongs, m_Def.m_vsChoices );
	}

	virtual ReloadChanged Reload()
	{
		FillSongsAndChoices( m_sSongGroup, m_vpDisplayedSongs, m_Def.m_vsChoices );
		return RELOAD_CHANGED_ALL;
	}

	virtual void ImportOption( const vector<PlayerNumber> &vpns, vector<bool> vbSelectedOut[NUM_PLAYERS] ) const
	{
		vector<Song*>::const_iterator iter = find( m_vpDisplayedSongs.begin(), m_vpDisplayedSongs.end(), GAMESTATE->m_pCurSong.Get() );
		int iChoice = 0;
		if( iter != m_vpDisplayedSongs.end() )
			iChoice = iter - m_vpDisplayedSongs.begin();
		FOREACH_PlayerNumber(pn)
			OptionRowHandlerUtil::SelectExactlyOne( iChoice, vbSelectedOut[pn] );
	}
	virtual int ExportOption( const vector<PlayerNumber> &vpns, const vector<bool> vbSelected[NUM_PLAYERS] ) const
	{
		int iChoice = OptionRowHandlerUtil::GetOneSelection( vbSelected[GAMESTATE->m_MasterPlayerNumber] );
		GAMESTATE->m_pCurSong.Set( m_vpDisplayedSongs[iChoice] );
		return 0;
	}
};

REGISTER_SCREEN_CLASS( ScreenOptionsEditCourseEntry );

void ScreenOptionsEditCourseEntry::Init()
{
	ScreenOptionsEditCourseSubMenu::Init();

	vector<OptionRowHandler*> vHands;
	OptionRowHandler *pHand = OptionRowHandlerUtil::MakeNull();
	pHand->m_Def.m_sName = "Song Group";
	pHand->m_Def.m_layoutType = LAYOUT_SHOW_ONE_IN_ROW;
	pHand->m_Def.m_bExportOnChange = true;
	pHand->m_Def.m_bAllowThemeItems = false;
	pHand->m_Def.m_bOneChoiceForAllPlayers = true;
	vector<RString> vsSongGroups;
	SONGMAN->GetSongGroupNames( vsSongGroups );
	pHand->m_Def.m_vsChoices.push_back( "(any)" );
	FOREACH_CONST( RString, vsSongGroups, group )
		pHand->m_Def.m_vsChoices.push_back( *group );
	vHands.push_back( pHand );

	{
		m_pSongHandler = new OptionRowHandlerSongChoices;
	//	m_pSongHandler->m_sSongGroup = ce.sSongGroup;
		m_pSongHandler->Load( Commands() );
		m_pSongHandler->m_Def.m_sName = "Song";
		m_pSongHandler->m_Def.m_layoutType = LAYOUT_SHOW_ONE_IN_ROW;
		m_pSongHandler->m_Def.m_bExportOnChange = true;
		m_pSongHandler->m_Def.m_bAllowThemeItems = false;
		m_pSongHandler->m_Def.m_bOneChoiceForAllPlayers = true;
		vHands.push_back( m_pSongHandler );
	}

	pHand = OptionRowHandlerUtil::MakeNull();
	pHand->m_Def.m_sName = "Base Difficulty";
	pHand->m_Def.m_layoutType = LAYOUT_SHOW_ONE_IN_ROW;
	pHand->m_Def.m_bExportOnChange = true;
	pHand->m_Def.m_bAllowThemeItems = false;
	pHand->m_Def.m_bOneChoiceForAllPlayers = true;
	pHand->m_Def.m_vsChoices.push_back( "(any)" );
	FOREACH_CONST( Difficulty, CommonMetrics::DIFFICULTIES_TO_SHOW.GetValue(), dc )
		pHand->m_Def.m_vsChoices.push_back( DifficultyToLocalizedString(*dc) );
	vHands.push_back( pHand );

	pHand = OptionRowHandlerUtil::MakeNull();
	pHand->m_Def.m_sName = "Low Meter";
	pHand->m_Def.m_layoutType = LAYOUT_SHOW_ONE_IN_ROW;
	pHand->m_Def.m_bExportOnChange = true;
	pHand->m_Def.m_bAllowThemeItems = false;
	pHand->m_Def.m_bOneChoiceForAllPlayers = true;
	pHand->m_Def.m_vsChoices.push_back( "(any)" );
	for( int i=MIN_METER; i<=MAX_METER; ++i )
		pHand->m_Def.m_vsChoices.push_back( ssprintf("%i",i) );
	vHands.push_back( pHand );

	pHand = OptionRowHandlerUtil::MakeNull();
	pHand->m_Def.m_sName = "High Meter";
	pHand->m_Def.m_layoutType = LAYOUT_SHOW_ONE_IN_ROW;
	pHand->m_Def.m_bExportOnChange = true;
	pHand->m_Def.m_bAllowThemeItems = false;
	pHand->m_Def.m_bOneChoiceForAllPlayers = true;
	pHand->m_Def.m_vsChoices.push_back( "(any)" );
	for( int i=MIN_METER; i<=MAX_METER; i++ )
		pHand->m_Def.m_vsChoices.push_back( ssprintf("%i",i) );
	vHands.push_back( pHand );

	pHand = OptionRowHandlerUtil::MakeNull();
	pHand->m_Def.m_sName = "Sort";
	pHand->m_Def.m_layoutType = LAYOUT_SHOW_ONE_IN_ROW;
	pHand->m_Def.m_bExportOnChange = true;
	pHand->m_Def.m_bAllowThemeItems = false;
	pHand->m_Def.m_bOneChoiceForAllPlayers = true;
	FOREACH_SongSort( i )
		pHand->m_Def.m_vsChoices.push_back( SongSortToLocalizedString(i) );
	vHands.push_back( pHand );

	pHand = OptionRowHandlerUtil::MakeNull();
	pHand->m_Def.m_sName = "Choose";
	pHand->m_Def.m_layoutType = LAYOUT_SHOW_ONE_IN_ROW;
	pHand->m_Def.m_bExportOnChange = true;
	pHand->m_Def.m_bAllowThemeItems = false;
	pHand->m_Def.m_bOneChoiceForAllPlayers = true;
	for( int i=0; i<20; i++ )
		pHand->m_Def.m_vsChoices.push_back( FormatNumberAndSuffix(i+1) );
	vHands.push_back( pHand );
	
	pHand = OptionRowHandlerUtil::MakeNull();
	pHand->m_Def.m_sName = "Secret";
	pHand->m_Def.m_layoutType = LAYOUT_SHOW_ONE_IN_ROW;
	pHand->m_Def.m_bExportOnChange = true;
	pHand->m_Def.m_bOneChoiceForAllPlayers = true;
	pHand->m_Def.m_vsChoices.push_back( "No" );
	pHand->m_Def.m_vsChoices.push_back( "Yes" );
	vHands.push_back( pHand );
	
	SHOW_MODS_ROW.Load( m_sName, "ShowModsRow" );

	m_pModChangesHandler = NULL;
	if( SHOW_MODS_ROW )
	{
		m_pModChangesHandler = OptionRowHandlerUtil::MakeNull();
		m_pModChangesHandler->m_Def.m_sName = "Set Mods";
		m_pModChangesHandler->m_Def.m_layoutType = LAYOUT_SHOW_ONE_IN_ROW;
		m_pModChangesHandler->m_Def.m_bExportOnChange = true;
		m_pModChangesHandler->m_Def.m_bAllowThemeItems = false;
		m_pModChangesHandler->m_Def.m_bOneChoiceForAllPlayers = true;
		m_pModChangesHandler->m_Def.m_vsChoices.push_back( "" );
		vHands.push_back( m_pModChangesHandler );
	}
	
	m_pLongSong = NULL;
	
	ScreenOptions::InitMenu( vHands );
}

void ScreenOptionsEditCourseEntry::BeginScreen()
{
	// save a backup that we'll use if we revert.
	const Course *pCourse = GAMESTATE->m_pCurCourse;
	const CourseEntry &ce = pCourse->m_vEntries[ GAMESTATE->m_iEditCourseEntryIndex ];
	m_Original = ce;

	m_pSongHandler->m_sSongGroup = ce.songCriteria.m_sGroupName;

	if( SHOW_MODS_ROW )
		m_pModChangesHandler->m_Def.m_vsChoices[0] = ssprintf( "%d mod changes", ce.GetNumModChanges() );
	
	ScreenOptions::BeginScreen();
}

static LocalizedString NO_SONGS( "ScreenOptionsEditCourseEntry", "No songs loaded." );
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
			return;
		case ROW_SET_MODS:
			if( SHOW_MODS_ROW )
			{
				/* We can't rely on the trail here since it depends on what steps are available.
				 * Use the course entry directly instead. */
				CourseEntry& ce = pCourse->m_vEntries[GAMESTATE->m_iEditCourseEntryIndex];
				Song *pSong = ce.pSong;
				Steps *pSteps;
				StepsType st = GAMESTATE->m_stEdit;
				CourseDifficulty cd = ( ce.stepsCriteria.m_difficulty == DIFFICULTY_Invalid ?
							GAMESTATE->m_cdEdit : ce.stepsCriteria.m_difficulty );

				if( pSong == NULL )
					pSong = m_pLongSong;
				if( pSong == NULL )
				{
					// Find the longest non-tutorial song.
					vector<Song *> vSongs;
					
					SONGMAN->GetSongs( vSongs );
					
					float fLen = -1.f;
					
					FOREACH( Song*, vSongs, i )
					{
						if( !(*i)->IsTutorial() && (*i)->m_fMusicLengthSeconds > fLen )
						{
							pSong = *i;
							fLen = pSong->m_fMusicLengthSeconds;
						}
					}
					if( pSong == NULL )
					{
						SCREENMAN->PlayInvalidSound();
						SCREENMAN->SystemMessage( NO_SONGS );
						return;
					}
					m_pLongSong = pSong;
				}				
				
				// Try to find steps first using st and cd, then st, then cd, then any.
				pSteps = SongUtil::GetStepsByDifficulty( pSong, st, cd, false );
				if( !pSteps )
					pSteps = SongUtil::GetStepsByDifficulty( pSong, st, DIFFICULTY_Invalid, false );
				if( !pSteps )
					pSteps = SongUtil::GetStepsByDifficulty( pSong, StepsType_Invalid, cd, false );
				if( !pSteps )
					pSteps = SongUtil::GetStepsByDifficulty( pSong, StepsType_Invalid, DIFFICULTY_Invalid, false );
				ASSERT( pSteps );
				
				// Set up for ScreenEdit
				const Style *pStyle = GAMEMAN->GetEditorStyleForStepsType(pSteps->m_StepsType);
				GAMESTATE->SetCurrentStyle( pStyle );
				GAMESTATE->m_pCurSong.Set( pSong );
				GAMESTATE->m_pCurSteps[PLAYER_1].Set( pSteps );
				break;
			}
			// fall through
		case ROW_DONE:
			m_Original = pCourse->m_vEntries[ GAMESTATE->m_iEditCourseEntryIndex ];
			SCREENMAN->SetNewScreen( "ScreenOptionsEditCourse" );
			HandleScreenMessage( SM_GoToPrevScreen );
			return;
		}
	}
	else if( SM == SM_GoToPrevScreen )
	{
		// revert
		pCourse->m_vEntries[ GAMESTATE->m_iEditCourseEntryIndex ] = m_Original;
		
		// cause overlay elements to refresh by changing the course
		Trail *pTrail = pCourse->GetTrailForceRegenCache( GAMESTATE->m_stEdit, GAMESTATE->m_cdEdit );
		
		GAMESTATE->m_pCurCourse.Set( pCourse );
		GAMESTATE->m_pCurTrail[PLAYER_1].Set( pTrail );
		
	}
	else if( SM == SM_BackFromCoursePlayerOptions )
	{
		const PlayerOptions &po = GAMESTATE->m_pPlayerState[GAMESTATE->m_MasterPlayerNumber]->m_PlayerOptions.GetPreferred();
		CourseEntry &ce = pCourse->m_vEntries[GAMESTATE->m_iEditCourseEntryIndex];
		
		ce.sModifiers = po.GetString();
	}

	ScreenOptions::HandleScreenMessage( SM );
}
	
void ScreenOptionsEditCourseEntry::AfterChangeValueInRow( int iRow, PlayerNumber pn )
{
	ScreenOptions::AfterChangeValueInRow( iRow, pn );
	Course *pCourse = GAMESTATE->m_pCurCourse;
	
	GAMESTATE->m_pCurTrail[PLAYER_1].Set( NULL );
	Trail *pTrail = pCourse->GetTrailForceRegenCache( GAMESTATE->m_stEdit, GAMESTATE->m_cdEdit );
	int iEntryIndex = GAMESTATE->m_iEditCourseEntryIndex;
	ASSERT( iEntryIndex >= 0 && iEntryIndex < (int) pCourse->m_vEntries.size() );
	CourseEntry &ce = pCourse->m_vEntries[ iEntryIndex ];

	switch( iRow )
	{
	case ROW_SONG_GROUP:
	// refresh songs
	{
		vector<PlayerNumber> vpns;
		vpns.push_back( pn );
		ExportOptions( ROW_SONG_GROUP, vpns );

		m_pSongHandler->m_sSongGroup = ce.songCriteria.m_sGroupName;

		OptionRow &row = *m_pRows[ROW_SONG];
		row.Reload();
		ImportOptions( ROW_SONG, vpns );
		row.AfterImportOptions( pn );
		break;
	}
	}


	// cause overlay elements to refresh by changing the course
	GAMESTATE->m_pCurCourse.Set( pCourse );
	GAMESTATE->m_pCurTrail[PLAYER_1].Set( pTrail );

}

void ScreenOptionsEditCourseEntry::ImportOptions( int iRow, const vector<PlayerNumber> &vpns )
{
	Course *pCourse = GAMESTATE->m_pCurCourse;
	int iEntryIndex = GAMESTATE->m_iEditCourseEntryIndex;
	ASSERT( iEntryIndex >= 0 && iEntryIndex < (int) pCourse->m_vEntries.size() );
	CourseEntry &ce = pCourse->m_vEntries[ iEntryIndex ];

	OptionRow &row = *m_pRows[iRow];

	switch( iRow )
	{
	case ROW_SONG_GROUP:
		FOREACH_CONST( RString, row.GetRowDef().m_vsChoices, s )
		{
			if( *s == ce.songCriteria.m_sGroupName )
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
		GAMESTATE->m_pCurSong.Set( ce.pSong );

		// XXX: copy and pasted from ScreenOptionsMaster
		FOREACH_CONST( PlayerNumber, vpns, pn )
			ASSERT( GAMESTATE->IsHumanPlayer(*pn) );
		OptionRow &row = *m_pRows[iRow];
		row.ImportOptions( vpns );
		break;
	}
	case ROW_BASE_DIFFICULTY:
	{
		vector<Difficulty>::const_iterator iter = find( CommonMetrics::DIFFICULTIES_TO_SHOW.GetValue().begin(), CommonMetrics::DIFFICULTIES_TO_SHOW.GetValue().end(), ce.stepsCriteria.m_difficulty );
		int iChoice = 0;
		if( iter != CommonMetrics::DIFFICULTIES_TO_SHOW.GetValue().end() )
			iChoice = iter - CommonMetrics::DIFFICULTIES_TO_SHOW.GetValue().begin() + 1;
		OptionRow &row = *m_pRows[ROW_BASE_DIFFICULTY];
		row.SetOneSharedSelection( iChoice );
		break;
	}
	case ROW_LOW_METER:
	{
		int iChoice = 0;
		if( ce.stepsCriteria.m_iLowMeter != -1 )
			iChoice = ce.stepsCriteria.m_iLowMeter;
		OptionRow &row = *m_pRows[ROW_LOW_METER];
		row.SetOneSharedSelection( iChoice );
		break;
	}
	case ROW_HIGH_METER:
	{
		int iChoice = 0;
		if( ce.stepsCriteria.m_iHighMeter != -1 )
			iChoice = ce.stepsCriteria.m_iHighMeter;
		OptionRow &row = *m_pRows[ROW_HIGH_METER];
		row.SetOneSharedSelection( iChoice );
		break;
	}
	case ROW_SORT:
	{
		int iChoice = ce.songSort;
		OptionRow &row = *m_pRows[ROW_SORT];
		row.SetOneSharedSelection( iChoice );
		break;
	}
	case ROW_CHOOSE_INDEX:
	{
		int iChoice = ce.iChooseIndex;
		OptionRow &row = *m_pRows[ROW_CHOOSE_INDEX];
		row.SetOneSharedSelection( iChoice );
		break;
	}
	case ROW_SECRET:
		row.SetOneSharedSelection( ce.bSecret ? 1 : 0 );
		break;
	}
}

void ScreenOptionsEditCourseEntry::ExportOptions( int iRow, const vector<PlayerNumber> &vpns )
{
	Course *pCourse = GAMESTATE->m_pCurCourse;
	int iEntryIndex = GAMESTATE->m_iEditCourseEntryIndex;
	ASSERT( iEntryIndex >= 0 && iEntryIndex < (int) pCourse->m_vEntries.size() );
	CourseEntry &ce = pCourse->m_vEntries[ iEntryIndex ];
	OptionRow &row = *m_pRows[iRow];
	int iChoice = row.GetOneSharedSelection( true );

	switch( iRow )
	{
	case ROW_SONG_GROUP:
	{
		if( iChoice == 0 )
			ce.songCriteria.m_sGroupName = "";
		else
			ce.songCriteria.m_sGroupName = row.GetRowDef().m_vsChoices[ iChoice ];
		break;
	}
	case ROW_SONG:
	{
		// XXX: copy and pasted from ScreenOptionsMaster
		bool bRowHasFocus[NUM_PLAYERS];
		ZERO( bRowHasFocus );
		FOREACH_CONST( PlayerNumber, vpns, p )
		{
			int iCurRow = m_iCurrentRow[*p];
			bRowHasFocus[*p] = iCurRow == iRow;
		}
		row.ExportOptions( vpns, bRowHasFocus );
		
		ce.pSong = GAMESTATE->m_pCurSong;
		break;
	}
	case ROW_BASE_DIFFICULTY:
	{
		if( iChoice == 0 )
		{
			ce.stepsCriteria.m_difficulty = DIFFICULTY_Invalid;
		}
		else
		{
			Difficulty d = CommonMetrics::DIFFICULTIES_TO_SHOW.GetValue()[iChoice-1];
			ce.stepsCriteria.m_difficulty = d;
		}
		break;
	}
	case ROW_LOW_METER:
	{
		if( iChoice == 0 )
			ce.stepsCriteria.m_iLowMeter = -1;
		else
			ce.stepsCriteria.m_iLowMeter = iChoice;
		break;
	}
	case ROW_HIGH_METER:
	{
		if( iChoice == 0 )
			ce.stepsCriteria.m_iHighMeter = -1;
		else
			ce.stepsCriteria.m_iHighMeter = iChoice;
		break;
	}
	case ROW_SORT:
	{
		ce.songSort = (SongSort)iChoice;
		break;
	}
	case ROW_CHOOSE_INDEX:
	{
		ce.iChooseIndex = iChoice;
		break;
	}
	case ROW_SECRET:
		ce.bSecret = !!iChoice;
		break;
	}
	Trail *pTrail = pCourse->GetTrailForceRegenCache( GAMESTATE->m_stEdit, GAMESTATE->m_cdEdit );
	
	GAMESTATE->m_pCurCourse.Set( pCourse );
	GAMESTATE->m_pCurTrail[PLAYER_1].Set( pTrail );	
}

void ScreenOptionsEditCourseEntry::ProcessMenuStart( const InputEventPlus &input )
{
	PlayerNumber pn = GAMESTATE->m_MasterPlayerNumber;
	
	switch( m_iCurrentRow[pn] )
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
		SCREENMAN->PlayStartSound();
		ScreenOptions::BeginFadingOut();
		break;
	}
}


/*
 * (c) 2002-2006 Chris Danford, Steve Checkoway
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

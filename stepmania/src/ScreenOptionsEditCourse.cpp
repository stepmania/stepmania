#include "global.h"
#include "ScreenOptionsEditCourse.h"
#include "ScreenManager.h"
#include "RageLog.h"
#include "GameState.h"
#include "SongManager.h"
#include "CommonMetrics.h"
#include "GameManager.h"
#include "song.h"
#include "ScreenMiniMenu.h"
#include "ScreenPrompt.h"
#include "CourseUtil.h"
#include "LocalizedString.h"
#include "OptionRowHandler.h"

enum EditCourseRow
{
	EditCourseRow_Type,
	EditCourseRow_Meter,
	NUM_EditCourseRow
};

AutoScreenMessage( SM_BackFromContextMenu )

enum CourseEntryAction
{
	CourseEntryAction_Edit,
	CourseEntryAction_InsertEntry,
	CourseEntryAction_Delete,
	NUM_CourseEntryAction
};
static const char *CourseEntryActionNames[] = {
	"Edit",
	"Insert Entry",
	"Delete",
};
XToString( CourseEntryAction, NUM_CourseEntryAction );
#define FOREACH_CourseEntryAction( i ) FOREACH_ENUM( CourseEntryAction, i )

static MenuDef g_TempMenu(
	"ScreenMiniMenuContext"
);

REGISTER_SCREEN_CLASS( ScreenOptionsEditCourse );

void ScreenOptionsEditCourse::Init()
{
	ScreenOptionsEditCourseSubMenu::Init();
	SubscribeToMessage( Message_EditCourseDifficultyChanged );
}

static LocalizedString ENTRY( "OptionTitles", "Entry %d" );

void ScreenOptionsEditCourse::BeginScreen()
{
	// save a backup that we'll use if we revert.
	ASSERT( GAMESTATE->m_pCurCourse );
	Course *pCourse = GAMESTATE->m_pCurCourse;
	m_Original = *pCourse;
	SONGMAN->GetSongs( m_vpDisplayedSongs );

	vector<OptionRowHandler*> vHands;

	OptionRowHandler *pHand = OptionRowHandlerUtil::MakeNull();
	pHand->m_Def.m_sName = "Type";
	pHand->m_Def.m_layoutType = LAYOUT_SHOW_ONE_IN_ROW;
	pHand->m_Def.m_bExportOnChange = true;
	pHand->m_Def.m_bAllowThemeItems = false;
	pHand->m_Def.m_bOneChoiceForAllPlayers = true;
	FOREACH_CourseType( i )
		pHand->m_Def.m_vsChoices.push_back( CourseTypeToLocalizedString(i) );
	vHands.push_back( pHand );

	pHand = OptionRowHandlerUtil::MakeNull();
	pHand->m_Def.m_sName = "Meter";
	pHand->m_Def.m_layoutType = LAYOUT_SHOW_ONE_IN_ROW;
	pHand->m_Def.m_bExportOnChange = true;
	pHand->m_Def.m_bOneChoiceForAllPlayers = true;
	pHand->m_Def.m_vsChoices.push_back( "Auto" );
	for( int i=MIN_METER; i<=MAX_METER; i++ )
		pHand->m_Def.m_vsChoices.push_back( ssprintf("%d",i) );
	vHands.push_back( pHand );

	FOREACH_CONST( CourseEntry, pCourse->m_vEntries, ce )
	{
		pHand = OptionRowHandlerUtil::MakeNull();
		int iEntryIndex = ce - pCourse->m_vEntries.begin();
		pHand->m_Def.m_layoutType = LAYOUT_SHOW_ONE_IN_ROW;
		pHand->m_Def.m_bExportOnChange = true;
		pHand->m_Def.m_sName = ssprintf( ENTRY.GetValue(), iEntryIndex+1 );
		pHand->m_Def.m_bAllowThemeItems = false;
		pHand->m_Def.m_bAllowThemeTitle = false;
		pHand->m_Def.m_bOneChoiceForAllPlayers = true;
		pHand->m_Def.m_vsChoices.push_back( "RANDOM" ); // XXX Localize?
		FOREACH_CONST( Song*, m_vpDisplayedSongs, s )
			pHand->m_Def.m_vsChoices.push_back( (*s)->GetTranslitFullTitle() );
		vHands.push_back( pHand );
	}

	pHand = OptionRowHandlerUtil::MakeNull();
	pHand->m_Def.m_sName = "";
	pHand->m_Def.m_layoutType = LAYOUT_SHOW_ONE_IN_ROW;
	pHand->m_Def.m_bExportOnChange = false;
	pHand->m_Def.m_bOneChoiceForAllPlayers = true;
	pHand->m_Def.m_vsChoices.push_back( "Insert Entry" );
	vHands.push_back( pHand );

	ScreenOptions::InitMenu( vHands );

	ScreenOptions::BeginScreen();

	if( GAMESTATE->m_iEditCourseEntryIndex > -1 )
		this->MoveRowAbsolute( GAMESTATE->m_MasterPlayerNumber, NUM_EditCourseRow + GAMESTATE->m_iEditCourseEntryIndex );
	AfterChangeRow( GAMESTATE->m_MasterPlayerNumber );
}

static LocalizedString MAXIMUM_COURSE_ENTRIES	("ScreenOptionsEditCourse", "The maximum number of entries per course is %d.  This course already has %d entries.");
static bool AreEntriesFull()
{
	Course *pCourse = GAMESTATE->m_pCurCourse;

	if( pCourse->m_vEntries.size() >= size_t(MAX_ENTRIES_PER_COURSE) )
	{
		RString sError = ssprintf(MAXIMUM_COURSE_ENTRIES.GetValue(), MAX_ENTRIES_PER_COURSE, pCourse->m_vEntries.size() );
		ScreenPrompt::Prompt( SM_None, sError );
		return true;
	}
	return false;
}

static LocalizedString CANNOT_DELETE_LAST_ENTRY ("ScreenOptionsEditCourse", "You cannot delete the last entry in a course.");
void ScreenOptionsEditCourse::HandleScreenMessage( const ScreenMessage SM )
{
	Course *pCourse = GAMESTATE->m_pCurCourse;

	if( SM == SM_GoToNextScreen )
	{
		int iCurRow = m_iCurrentRow[GAMESTATE->m_MasterPlayerNumber];
		if( iCurRow == (int)m_pRows.size() - 1 )
		{
			m_Original = *pCourse;
			WriteCourse();
			this->HandleScreenMessage( SM_GoToPrevScreen );
			return;	// don't call base
		}
	}
	else if( SM == SM_GoToPrevScreen )
	{
		*pCourse = m_Original;
		GAMESTATE->m_pCurCourse.Set( pCourse );
		GAMESTATE->m_iEditCourseEntryIndex.Set( -1 );
	}
	else if( SM == SM_BackFromContextMenu )
	{
		if( !ScreenMiniMenu::s_bCancelled )
		{
			switch( ScreenMiniMenu::s_iLastRowCode )
			{
			case CourseEntryAction_Edit:
				ScreenOptions::BeginFadingOut();
				break;
			case CourseEntryAction_InsertEntry:
			{
				if( AreEntriesFull() )
					return;
				CourseEntry ce;
				CourseUtil::MakeDefaultEditCourseEntry( ce );
				pCourse->m_vEntries.insert( pCourse->m_vEntries.begin() + GetCourseEntryIndexWithFocus(), ce );
				SCREENMAN->SetNewScreen( this->m_sName ); // reload
				break;
			}
			case CourseEntryAction_Delete:
			{
				if( pCourse->m_vEntries.size() == 1 )
				{
					ScreenPrompt::Prompt( SM_None, CANNOT_DELETE_LAST_ENTRY );
					return;
				}
				pCourse->m_vEntries.erase( pCourse->m_vEntries.begin() + GetCourseEntryIndexWithFocus() );
				GAMESTATE->m_iEditCourseEntryIndex.Set( GAMESTATE->m_iEditCourseEntryIndex );
				SCREENMAN->SetNewScreen( this->m_sName ); // reload
				break;
			}
			}
		}
	}

	ScreenOptions::HandleScreenMessage( SM );
}
	
void ScreenOptionsEditCourse::AfterChangeRow( PlayerNumber pn )
{
	ScreenOptions::AfterChangeRow( pn );

	int iCourseEntry = GetCourseEntryIndexWithFocus();
	GAMESTATE->m_iEditCourseEntryIndex.Set( iCourseEntry );
}

void ScreenOptionsEditCourse::AfterChangeValueInRow( int iRow, PlayerNumber pn )
{
	ScreenOptions::AfterChangeValueInRow( iRow, pn );

	Course *pCourse = GAMESTATE->m_pCurCourse;
	
	// Regenerate Trails so that the new values propagate
	GAMESTATE->m_pCurTrail[PLAYER_1].Set( NULL );
	Trail *pTrail = pCourse->GetTrailForceRegenCache( GAMESTATE->m_stEdit, GAMESTATE->m_cdEdit );

	// cause overlay elements to refresh by changing the course
	GAMESTATE->m_pCurCourse.Set( pCourse );
	GAMESTATE->m_pCurTrail[PLAYER_1].Set( pTrail );
}

void ScreenOptionsEditCourse::ImportOptions( int iRow, const vector<PlayerNumber> &vpns )
{
	OptionRow &row = *m_pRows[iRow];
	
	if( row.GetRowType() == OptionRow::RowType_Exit )
		return;
	Course *pCourse = GAMESTATE->m_pCurCourse;

	if( iRow == (int)pCourse->m_vEntries.size() + NUM_EditCourseRow )
		return; // "Insert Entry"
	
	switch( iRow )
	{
	case EditCourseRow_Type:
		row.SetOneSharedSelection( pCourse->GetCourseType() );
		return;
	case EditCourseRow_Meter:
	{
		int iMeter = pCourse->m_iCustomMeter[GAMESTATE->m_cdEdit];
		if( iMeter == -1 )
			row.SetOneSharedSelection( 0 );
		else
			row.SetOneSharedSelection( iMeter + 1 - MIN_METER );
		return;
	}
	}

	const CourseEntry& ce = pCourse->m_vEntries[iRow - NUM_EditCourseRow];
	
	if( !ce.IsFixedSong() )
	{
		row.SetOneSharedSelection( 0 );
	}
	else
	{
		vector<Song *>::const_iterator iter = find( m_vpDisplayedSongs.begin(), m_vpDisplayedSongs.end(), ce.pSong );
		
		if( iter == m_vpDisplayedSongs.end() ) // This song isn't being displayed, set to "RANDOM"
			row.SetOneSharedSelection( 0 );
		else
			row.SetOneSharedSelection( iter - m_vpDisplayedSongs.begin() + 1 );
	}
}

void ScreenOptionsEditCourse::ExportOptions( int iRow, const vector<PlayerNumber> &vpns )
{
	OptionRow &row = *m_pRows[iRow];
	
	if( row.GetRowType() == OptionRow::RowType_Exit )
		return;
	Course *pCourse = GAMESTATE->m_pCurCourse;
	if( iRow == (int)pCourse->m_vEntries.size() + NUM_EditCourseRow )
		return; // "Insert Entry"

	int iSel = row.GetOneSharedSelection();

	switch( iRow )
	{
	case EditCourseRow_Type:
	{
		CourseType ct = (CourseType)iSel;
		pCourse->SetCourseType( ct );
		return;
	}
	case EditCourseRow_Meter:
	{
		if( iSel == 0 )	// "auto"
			pCourse->m_iCustomMeter[GAMESTATE->m_cdEdit] = -1;
		else
			pCourse->m_iCustomMeter[GAMESTATE->m_cdEdit] = iSel - 1 + MIN_METER;
		return;
	}
	}
	
	CourseEntry& ce = pCourse->m_vEntries[iRow - NUM_EditCourseRow];

	if( iSel == 0 )
		ce.pSong = NULL;
	else
		ce.pSong = m_vpDisplayedSongs[iSel - 1];
}

void ScreenOptionsEditCourse::ProcessMenuStart( const InputEventPlus &input )
{
	if( IsTransitioning() )
		return;

	int iCurRow = m_iCurrentRow[GAMESTATE->m_MasterPlayerNumber];
	Course *pCourse = GAMESTATE->m_pCurCourse;

	if( iCurRow < NUM_EditCourseRow )
		return;
	
	if( iCurRow == (int)m_pRows.size()-2 )	// "create entry"
	{
		if( AreEntriesFull() )
			return;
		CourseEntry ce;
		CourseUtil::MakeDefaultEditCourseEntry( ce );
		pCourse->m_vEntries.push_back( ce );
		GAMESTATE->m_iEditCourseEntryIndex.Set( pCourse->m_vEntries.size()-1 );
		SCREENMAN->SetNewScreen( this->m_sName ); // reload
	}
	else if( iCurRow == (int)m_pRows.size()-1 )	// "done"
	{
		SCREENMAN->PlayStartSound();
		this->BeginFadingOut();
	}
	else	// a course entry
	{
		g_TempMenu.rows.clear();
		FOREACH_CourseEntryAction( i )
		{
			MenuRowDef mrd( i, CourseEntryActionToString(i), true, EditMode_Home, true, true, 0, "" );
			g_TempMenu.rows.push_back( mrd );
		}

		int iWidth, iX, iY;
		this->GetWidthXY( GAMESTATE->m_MasterPlayerNumber, iCurRow, 0, iWidth, iX, iY );
		ScreenMiniMenu::MiniMenu( &g_TempMenu, SM_BackFromContextMenu, SM_BackFromContextMenu, (float)iX, (float)iY );
	}

}

void ScreenOptionsEditCourse::HandleMessage( const RString& sMessage )
{
	if( sMessage == "EditCourseDifficultyChanged" )
	{
		const vector<PlayerNumber> vpns( 1, GAMESTATE->m_MasterPlayerNumber );
		OptionRow &row = *m_pRows[EditCourseRow_Meter];
		
		ImportOptions( EditCourseRow_Meter, vpns );
		row.AfterImportOptions( GAMESTATE->m_MasterPlayerNumber );
	}
	ScreenOptionsEditCourseSubMenu::HandleMessage( sMessage );
}

int ScreenOptionsEditCourse::GetCourseEntryIndexWithFocus() const
{
	int iCurRow = m_iCurrentRow[GAMESTATE->m_MasterPlayerNumber];
	if( iCurRow < NUM_EditCourseRow )	// not a CourseEntry
		return -1;
	else if( iCurRow == (int)m_pRows.size() - 1 )	// "done"
		return -1;
	else
		return iCurRow - NUM_EditCourseRow;
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

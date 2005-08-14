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
static const CString CourseEntryActionNames[] = {
	"Edit",
	"Insert Entry",
	"Delete",
};
XToString( CourseEntryAction, NUM_CourseEntryAction );
#define FOREACH_CourseEntryAction( i ) FOREACH_ENUM( CourseEntryAction, NUM_CourseEntryAction, i )

static MenuDef g_TempMenu(
	"ScreenMiniMenuContext"
);

REGISTER_SCREEN_CLASS( ScreenOptionsEditCourse );
ScreenOptionsEditCourse::ScreenOptionsEditCourse( CString sName ) : ScreenOptionsEditCourseSubMenu( sName )
{
	LOG->Trace( "ScreenOptionsEditCourse::ScreenOptionsEditCourse()" );
}

void ScreenOptionsEditCourse::Init()
{
	ScreenOptions::Init();


	// save a backup that we'll use if we revert.
	ASSERT( GAMESTATE->m_pCurCourse );
	Course *pCourse = GAMESTATE->m_pCurCourse;
	m_Original = *pCourse;


	vector<OptionRowDefinition> vDefs;
	vector<OptionRowHandler*> vHands;

	OptionRowDefinition def;
	def.m_layoutType = LAYOUT_SHOW_ONE_IN_ROW;
	def.m_bExportOnChange = true;


	def.m_sName = "Type";
	def.m_vsChoices.clear();
	FOREACH_CourseType( i )
		def.m_vsChoices.push_back( CourseTypeToThemedString(i) );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	def.m_sName = "Meter";
	def.m_vsChoices.clear();
	def.m_vsChoices.push_back( "Auto" );
	for( int i=MIN_METER; i<=MAX_METER; i++ )
		def.m_vsChoices.push_back( ssprintf("%d",i) );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	FOREACH_CONST( CourseEntry, pCourse->m_vEntries, ce )
	{
		int iEntryIndex = ce - pCourse->m_vEntries.begin();
		def.m_sName = ssprintf( "Entry %d", iEntryIndex+1 );
		def.m_vsChoices.clear();
		def.m_vsChoices.push_back( ce->GetTextDescription() );
		vDefs.push_back( def );
		vHands.push_back( NULL );
	}

	def.m_sName = "";
	def.m_vsChoices.clear();
	def.m_vsChoices.push_back( "Insert Entry" );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	ScreenOptions::InitMenu( vDefs, vHands );
}

void ScreenOptionsEditCourse::BeginScreen()
{
	ScreenOptions::BeginScreen();

	if( GAMESTATE->m_iEditCourseEntryIndex > -1 )
		this->MoveRowAbsolute( PLAYER_1, NUM_EditCourseRow + GAMESTATE->m_iEditCourseEntryIndex, false );
	AfterChangeRow( GAMESTATE->m_MasterPlayerNumber );
}

void ScreenOptionsEditCourse::HandleScreenMessage( const ScreenMessage SM )
{
	Course *pCourse = GAMESTATE->m_pCurCourse;

	if( SM == SM_GoToNextScreen )
	{
		int iCurRow = m_iCurrentRow[GAMESTATE->m_MasterPlayerNumber];
		if( iCurRow == (int)m_pRows.size() - 1 )
		{
			this->HandleScreenMessage( SM_GoToPrevScreen );
			return;	// don't call base
		}
	}
	else if( SM == SM_GoToPrevScreen )
	{
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
					if( pCourse->m_vEntries.size() >= MAX_ENTRIES_PER_COURSE )
					{
						CString sError = "The maximum number of entries per course is %d.  This course already has %d entries.";
						ScreenPrompt::Prompt( SM_None, sError );
						return;
					}
					CourseEntry ce;
					CourseUtil::MakeDefaultEditCourseEntry( ce );
					pCourse->m_vEntries.insert( pCourse->m_vEntries.begin() + GetCourseEntryIndexWithFocus(), ce );
					SCREENMAN->SetNewScreen( this->m_sName ); // reload
				}
				break;
			case CourseEntryAction_Delete:
				{
					if( pCourse->m_vEntries.size() == 1 )
					{
						CString sError = "You cannot delete the last entry in a course.";
						ScreenPrompt::Prompt( SM_None, sError );
						return;
					}
					pCourse->m_vEntries.erase( pCourse->m_vEntries.begin() + GetCourseEntryIndexWithFocus() );
					GAMESTATE->m_iEditCourseEntryIndex.Set( GAMESTATE->m_iEditCourseEntryIndex );
					SCREENMAN->SetNewScreen( this->m_sName ); // reload
				}
				break;
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
	
	// Regemerate Trails so that the new values propagate
	GAMESTATE->m_pCurTrail[PLAYER_1].Set( NULL );
	Trail *pTrail = pCourse->GetTrailForceRegenCache( GAMESTATE->m_stEdit, GAMESTATE->m_PreferredCourseDifficulty[PLAYER_1] );

	// cause overlay elements to refresh by changing the course
	GAMESTATE->m_pCurCourse.Set( pCourse );
	GAMESTATE->m_pCurTrail[PLAYER_1].Set( pTrail );
}

void ScreenOptionsEditCourse::ImportOptions( int iRow, const vector<PlayerNumber> &vpns )
{
	OptionRow &row = *m_pRows[iRow];
	Course *pCourse = GAMESTATE->m_pCurCourse;

	switch( iRow )
	{
	case EditCourseRow_Type:
		row.SetOneSharedSelection( pCourse->GetCourseType() );
		break;
	case EditCourseRow_Meter:
		{
			int iMeter = pCourse->m_iCustomMeter[DIFFICULTY_MEDIUM];
			if( iMeter == -1 )
				row.SetOneSharedSelection( 0 );
			else
				row.SetOneSharedSelection( iMeter + 1 - MIN_METER );
		}
		break;
	}
}

void ScreenOptionsEditCourse::ExportOptions( int iRow, const vector<PlayerNumber> &vpns )
{
	OptionRow &row = *m_pRows[iRow];
	Course *pCourse = GAMESTATE->m_pCurCourse;

	switch( iRow )
	{
	case EditCourseRow_Type:
		{
			CourseType ct = (CourseType)row.GetOneSharedSelection();
			pCourse->SetCourseType( ct );
		}
		break;
	case EditCourseRow_Meter:
		{
			int iSel = row.GetOneSharedSelection();
			if( iSel == 0 )	// "auto"
				pCourse->m_iCustomMeter[DIFFICULTY_MEDIUM] = -1;
			else
				pCourse->m_iCustomMeter[DIFFICULTY_MEDIUM] = iSel - 1 + MIN_METER;
		}
		break;
	}
}

void ScreenOptionsEditCourse::ProcessMenuStart( PlayerNumber pn, const InputEventType type )
{
	int iCurRow = m_iCurrentRow[PLAYER_1];
	Course *pCourse = GAMESTATE->m_pCurCourse;

	if( iCurRow < NUM_EditCourseRow )
	{
		// ignore
	}
	else if( iCurRow == (int)m_pRows.size()-2 )	// "create entry"
	{
		if( pCourse->m_vEntries.size() >= MAX_ENTRIES_PER_COURSE )
		{
			CString sError = "The maximum number of entries per course is %d.  This course already has %d entries.";
			ScreenPrompt::Prompt( SM_None, sError );
			return;
		}
		CourseEntry ce;
		CourseUtil::MakeDefaultEditCourseEntry( ce );
		pCourse->m_vEntries.push_back( ce );
		GAMESTATE->m_iEditCourseEntryIndex.Set( pCourse->m_vEntries.size()-1 );
		SCREENMAN->SetNewScreen( this->m_sName ); // reload
	}
	else if( iCurRow == (int)m_pRows.size()-1 )	// "done"
	{
		this->BeginFadingOut();
	}
	else	// a course entry
	{
		g_TempMenu.rows.clear();
		FOREACH_CourseEntryAction( i )
		{
			MenuRowDef mrd( i, CourseEntryActionToString(i), true, EDIT_MODE_HOME, 0, "" );
			g_TempMenu.rows.push_back( mrd );
		}

		int iWidth, iX, iY;
		this->GetWidthXY( PLAYER_1, iCurRow, 0, iWidth, iX, iY );
		ScreenMiniMenu::MiniMenu( &g_TempMenu, SM_BackFromContextMenu, SM_BackFromContextMenu, (float)iX, (float)iY );
	}

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

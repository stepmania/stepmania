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

enum EditCourseRow
{
	ROW_REPEAT,
	ROW_RANDOMIZE,
	ROW_LIVES,
	ROW_METER,
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
ScreenOptionsEditCourse::ScreenOptionsEditCourse( CString sName ) : ScreenOptions( sName )
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
	
	def.m_sName = "Repeat";
	def.m_vsChoices.clear();
	def.m_vsChoices.push_back( "NO" );
	def.m_vsChoices.push_back( "YES" );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	def.m_sName = "Randomize";
	def.m_vsChoices.clear();
	def.m_vsChoices.push_back( "NO" );
	def.m_vsChoices.push_back( "YES" );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	def.m_sName = "Lives";
	def.m_vsChoices.clear();
	def.m_vsChoices.push_back( "Use Bar Life" );
	for( int i=1; i<=10; i++ )
		def.m_vsChoices.push_back( ssprintf("%d",i) );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	def.m_sName = "Meter";
	def.m_vsChoices.clear();
	for( int i=MIN_METER; i<=MAX_METER; i++ )
		def.m_vsChoices.push_back( ssprintf("%d",i) );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	FOREACH_CONST( CourseEntry, pCourse->m_vEntries, ce )
	{
		int iEntryIndex = ce - pCourse->m_vEntries.begin();
		CourseEntry &ce = pCourse->m_vEntries[iEntryIndex];
		def.m_sName = ssprintf( "Entry %d", iEntryIndex+1 );
		def.m_vsChoices.clear();
		def.m_vsChoices.push_back( ce.GetTextDescription() );
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

	this->MoveRowAbsolute( PLAYER_1, NUM_EditCourseRow + GAMESTATE->m_iEditCourseEntryIndex, false );
	AfterChangeRow( GAMESTATE->m_MasterPlayerNumber );
}

void ScreenOptionsEditCourse::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_GoToNextScreen )
	{
		int iCurRow = m_iCurrentRow[GAMESTATE->m_MasterPlayerNumber];
		if( iCurRow == m_pRows.size() - 1 )
		{
			this->HandleScreenMessage( SM_GoToPrevScreen );
			return;	// don't call base
		}
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
					Course *pCourse = GAMESTATE->m_pCurCourse;
					pCourse->m_vEntries.insert( pCourse->m_vEntries.begin() + GetCourseEntryIndexWithFocus() );
					SCREENMAN->SetNewScreen( this->m_sName ); // reload
				}
				break;
			case CourseEntryAction_Delete:
				{
					Course *pCourse = GAMESTATE->m_pCurCourse;
					pCourse->m_vEntries.erase( pCourse->m_vEntries.begin() + GetCourseEntryIndexWithFocus() );
					GAMESTATE->m_iEditCourseEntryIndex.Set( GAMESTATE->m_iEditCourseEntryIndex-1 );
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

void ScreenOptionsEditCourse::ImportOptions( int iRow, const vector<PlayerNumber> &vpns )
{
	switch( iRow )
	{
	case ROW_REPEAT:
		//row.SetChoiceInRowWithFocusShared( iEntryIndex );
		break;
	case ROW_RANDOMIZE:
		//row.SetChoiceInRowWithFocusShared( iEntryIndex );
		break;
	case ROW_LIVES:
		//row.SetChoiceInRowWithFocusShared( iEntryIndex );
		break;
	case ROW_METER:
		break;
	}
}

void ScreenOptionsEditCourse::ExportOptions( int iRow, const vector<PlayerNumber> &vpns )
{

}

void ScreenOptionsEditCourse::ProcessMenuStart( PlayerNumber pn, const InputEventType type )
{
	int iCurRow = m_iCurrentRow[PLAYER_1];
	OptionRow &row = *m_pRows[iCurRow];

	if( iCurRow < NUM_EditCourseRow )
	{
		// ignore
	}
	else if( iCurRow == m_pRows.size()-2 )	// "create entry"
	{
		ASSERT( 0 );
	}
	else if( iCurRow == m_pRows.size()-1 )	// "done"
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
		ScreenMiniMenu::MiniMenu( &g_TempMenu, SM_BackFromContextMenu, SM_BackFromContextMenu, iX, iY );
	}

}

int ScreenOptionsEditCourse::GetCourseEntryIndexWithFocus() const
{
	int iCurRow = m_iCurrentRow[GAMESTATE->m_MasterPlayerNumber];
	if( iCurRow < NUM_EditCourseRow )	// not a CourseEntry
		return -1;
	else if( iCurRow == m_pRows.size() - 1 )	// "done"
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

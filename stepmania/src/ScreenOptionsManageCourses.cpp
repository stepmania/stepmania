#include "global.h"
#include "ScreenOptionsManageCourses.h"
#include "ScreenManager.h"
#include "RageLog.h"
#include "GameState.h"
#include "SongManager.h"
#include "CommonMetrics.h"
#include "ScreenTextEntry.h"
#include "ScreenPrompt.h"
#include "ScreenMiniMenu.h"

AutoScreenMessage( SM_BackFromEnterName )
AutoScreenMessage( SM_BackFromDeleteConfirm )
AutoScreenMessage( SM_BackFromContextMenu )

enum CourseAction
{
	CourseAction_Edit,
	CourseAction_Rename,
	CourseAction_Delete,
	NUM_CourseAction
};
static const CString CourseActionNames[] = {
	"Edit",
	"Rename",
	"Delete",
};
XToString( CourseAction, NUM_CourseAction );
#define FOREACH_CourseAction( i ) FOREACH_ENUM( CourseAction, NUM_CourseAction, i )

static MenuDef g_TempMenu(
	"ScreenMiniMenuContext"
);

REGISTER_SCREEN_CLASS( ScreenOptionsManageCourses );
ScreenOptionsManageCourses::ScreenOptionsManageCourses( CString sName ) : ScreenOptions( sName )
{
	LOG->Trace( "ScreenOptionsManageCourses::ScreenOptionsManageCourses()" );
}

void ScreenOptionsManageCourses::Init()
{
	ScreenOptions::Init();

	vector<OptionRowDefinition> vDefs;
	vector<OptionRowHandler*> vHands;

	OptionRowDefinition def;
	def.m_layoutType = LAYOUT_SHOW_ONE_IN_ROW;
	
	def.m_sName = "";
	def.m_vsChoices.clear();
	def.m_vsChoices.push_back( "Create New" );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	SONGMAN->GetAllCourses( m_vpCourses, false );
	FOREACH_CONST( Course*, m_vpCourses, c )
	{
		def.m_sName = CourseTypeToThemedString( (*c)->GetCourseType() );
		def.m_vsChoices.clear();
		def.m_vsChoices.push_back( (*c)->GetDisplayFullTitle() );
		vDefs.push_back( def );
		vHands.push_back( NULL );
	}

	ScreenOptions::InitMenu( vDefs, vHands );
}

void ScreenOptionsManageCourses::BeginScreen()
{
	ScreenOptions::BeginScreen();
	
	AfterChangeRow( PLAYER_1 );
}

void ScreenOptionsManageCourses::HandleScreenMessage( const ScreenMessage SM )
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
	else if( SM == SM_BackFromEnterName )
	{
		if( !ScreenTextEntry::s_bCancelledLast )
		{
			ASSERT( ScreenTextEntry::s_sLastAnswer != "" );	// validate should have assured this
		
			CString sNewName = ScreenTextEntry::s_sLastAnswer;
			ASSERT( GAMESTATE->m_sLastSelectedProfileID.empty() );

			// create
			Course *pCourse = new Course;
			pCourse->SetLoadedFromProfile( PROFILE_SLOT_MACHINE );
			SONGMAN->AddCourse( pCourse );
			GAMESTATE->m_pCurCourse.Set( pCourse );
			this->HandleScreenMessage( SM_GoToNextScreen );
		}
	}
	else if( SM == SM_BackFromDeleteConfirm )
	{
		if( ScreenPrompt::s_LastAnswer == ANSWER_YES )
		{
			SONGMAN->DeleteCourse( GetCourseWithFocus() );
			SCREENMAN->SetNewScreen( this->m_sName ); // reload
		}
	}
	else if( SM == SM_BackFromContextMenu )
	{
		if( !ScreenMiniMenu::s_bCancelled )
		{
			switch( ScreenMiniMenu::s_iLastRowCode )
			{
			case CourseAction_Edit:
				GAMESTATE->m_pCurCourse.Set( GetCourseWithFocus() );
				ScreenOptions::BeginFadingOut();
				break;
			case CourseAction_Rename:
				{
					CString sCurrentProfileName = "New Course";
					ScreenTextEntry::TextEntry( 
						SM_BackFromEnterName, 
						"Enter a name for a new course.", 
						sCurrentProfileName, 
						MAX_EDIT_COURSE_TITLE_LENGTH, 
						SongManager::ValidateEditCourseName );
				}
				break;
			case CourseAction_Delete:
				{
					CString sTitle = GetCourseWithFocus()->GetDisplayFullTitle();
					CString sMessage = ssprintf( "Are you sure you want to delete the course '%s'?", sTitle.c_str() );
					ScreenPrompt::Prompt( SM_BackFromDeleteConfirm, sMessage, PROMPT_YES_NO );
				}
				break;
			}
		}
	}

	ScreenOptions::HandleScreenMessage( SM );
}
	
void ScreenOptionsManageCourses::AfterChangeRow( PlayerNumber pn )
{
	Course *pCourse = GetCourseWithFocus();
	Trail *pTrail = pCourse ? pCourse->GetTrail( STEPS_TYPE_DANCE_SINGLE ) : NULL;
	
	GAMESTATE->m_pCurCourse.Set( pCourse );
	GAMESTATE->m_pCurTrail[PLAYER_1].Set( pTrail );

	ScreenOptions::AfterChangeRow( pn );
}

void ScreenOptionsManageCourses::ProcessMenuStart( PlayerNumber pn, const InputEventType type )
{
	int iCurRow = m_iCurrentRow[GAMESTATE->m_MasterPlayerNumber];
	OptionRow &row = *m_pRows[iCurRow];

	if( iCurRow == 0 )	// "create new"
	{
		CString sCurrentProfileName = "New Course";
		ScreenTextEntry::TextEntry( 
			SM_BackFromEnterName, 
			"Enter a name for a new course.", 
			sCurrentProfileName, 
			MAX_EDIT_COURSE_TITLE_LENGTH, 
			SongManager::ValidateEditCourseName );
	}
	else if( iCurRow == m_pRows.size()-1 )	// "done"
	{
		this->BeginFadingOut();
	}
	else	// a course
	{
		g_TempMenu.rows.clear();
		FOREACH_CourseAction( i )
		{
			MenuRowDef mrd( i, CourseActionToString(i), true, EDIT_MODE_HOME, 0, "" );
			g_TempMenu.rows.push_back( mrd );
		}

		int iWidth, iX, iY;
		this->GetWidthXY( PLAYER_1, iCurRow, 0, iWidth, iX, iY );
		ScreenMiniMenu::MiniMenu( &g_TempMenu, SM_BackFromContextMenu, SM_BackFromContextMenu, iX, iY );
	}
}

void ScreenOptionsManageCourses::ImportOptions( int iRow, const vector<PlayerNumber> &vpns )
{

}

void ScreenOptionsManageCourses::ExportOptions( int iRow, const vector<PlayerNumber> &vpns )
{

}

Course *ScreenOptionsManageCourses::GetCourseWithFocus() const
{
	int iCurRow = m_iCurrentRow[GAMESTATE->m_MasterPlayerNumber];
	if( iCurRow == 0 )
		return NULL;
	else if( iCurRow == m_pRows.size()-1 )	// "done"
		return NULL;
	
	// a course
	int iCourseIndex = iCurRow - 1;
	return m_vpCourses[iCourseIndex];
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

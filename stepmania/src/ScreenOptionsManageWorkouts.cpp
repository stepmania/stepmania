#include "global.h"
#include "ScreenOptionsManageWorkouts.h"
#include "ScreenManager.h"
#include "ScreenMiniMenu.h"
#include "OptionRowHandler.h"
#include "Course.h"
#include "SongManager.h"
#include "LocalizedString.h"
#include "GameState.h"
#include "ScreenTextEntry.h"
#include "ScreenPrompt.h"
#include "StatsManager.h"
#include "PrefsManager.h"
#include "WorkoutManager.h"
#include "ProfileManager.h"
#include "GameManager.h"
#include "CourseUtil.h"

AutoScreenMessage( SM_BackFromRename )
AutoScreenMessage( SM_BackFromDelete )
AutoScreenMessage( SM_BackFromContextMenu )


enum ManageWorkoutsAction
{
	ManageWorkoutsAction_Choose,
	ManageWorkoutsAction_Rename,
	ManageWorkoutsAction_Delete,
	NUM_ManageWorkoutsAction
};
static const char *ManageWorkoutsActionNames[] = {
	"Choose",
	"Rename",
	"Delete",
};
XToString( ManageWorkoutsAction );
#define FOREACH_ManageWorkoutsAction( i ) FOREACH_ENUM( ManageWorkoutsAction, i )

static MenuDef g_TempMenu(
	"ScreenMiniMenuContext"
);



REGISTER_SCREEN_CLASS( ScreenOptionsManageWorkouts );

void ScreenOptionsManageWorkouts::Init()
{
	if( PREFSMAN->m_iArcadeOptionsNavigation )
		SetNavigation( NAV_THREE_KEY_MENU );

	ScreenOptions::Init();

	CREATE_NEW_SCREEN.Load( m_sName, "CreateNewScreen" );
}

void ScreenOptionsManageWorkouts::BeginScreen()
{
	STATSMAN->Reset();


	vector<const Style*> vpStyles;
	GameManager::GetStylesForGame( GAMESTATE->m_pCurGame, vpStyles );
	const Style *pStyle = vpStyles[0];
	GAMESTATE->SetCurrentStyle( pStyle );


	// Remember the current course.  All Course pointers will be invalidated when we load the machine profile below.
	CourseID cidLast;
	cidLast.FromCourse( GAMESTATE->m_pCurCourse );


	vector<OptionRowHandler*> vHands;

	int iIndex = 0;
	
	{
		vHands.push_back( OptionRowHandlerUtil::MakeNull() );
		OptionRowDefinition &def = vHands.back()->m_Def;
		def.m_layoutType = LAYOUT_SHOW_ONE_IN_ROW;
		def.m_bOneChoiceForAllPlayers = true;
		def.m_sName = "Create New Workout";
		def.m_sExplanationName = "Create New Workout";
		def.m_vsChoices.clear();
		def.m_vsChoices.push_back( "" );
		iIndex++;
	}

	FlushDirCache();
	PROFILEMAN->LoadMachineProfileEdits();
	EditCourseUtil::GetAllEditCourses( m_vpCourses );

	FOREACH_CONST( Course*, m_vpCourses, p )
	{
		vHands.push_back( OptionRowHandlerUtil::MakeNull() );
		OptionRowDefinition &def = vHands.back()->m_Def;
		
		def.m_sName = (*p)->GetDisplayFullTitle();
		def.m_bAllowThemeTitle = false;	// not themable
		def.m_sExplanationName = "Select Workout";
		def.m_vsChoices.clear();
		def.m_vsChoices.push_back( "" );
		def.m_bAllowThemeItems = false;	// already themed
		iIndex++;
	}

	ScreenOptions::InitMenu( vHands );

	ScreenOptions::BeginScreen();
	
	// select the last chosen course
	GAMESTATE->m_pCurCourse.Set( cidLast.ToCourse() );
	if( GAMESTATE->m_pCurCourse )
	{		
		EditCourseUtil::UpdateAndSetTrail();
		vector<Course*>::const_iterator iter = find( m_vpCourses.begin(), m_vpCourses.end(), GAMESTATE->m_pCurCourse );
		if( iter != m_vpCourses.end() )
		{
			int iIndex = iter - m_vpCourses.begin();
			this->MoveRowAbsolute( PLAYER_1, 1 + iIndex );
		}
	}

	AfterChangeRow( PLAYER_1 );
}

static LocalizedString ERROR_RENAMING		("ScreenOptionsManageWorkouts", "Error renaming file.");
static LocalizedString ERROR_DELETING_FILE	("ScreenOptionsManageWorkouts", "Error deleting the file '%s'.");
static LocalizedString THIS_WILL_BE_LOST	("ScreenOptionsManageWorkouts", "This file will be lost permanently.");
static LocalizedString CONTINUE_WITH_DELETE	("ScreenOptionsManageWorkouts", "Continue with delete?");
static LocalizedString ENTER_NAME		("ScreenOptionsManageWorkouts", "Enter a name.");
void ScreenOptionsManageWorkouts::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_GoToNextScreen )
	{
		int iCurRow = m_iCurrentRow[GAMESTATE->m_MasterPlayerNumber];

		if( iCurRow == 0 )	// "create new"
		{
			/* Allocate the Course now, but don't save the file until the user explicitly chooses Save */
			Course *pCourse = new Course;
			EditCourseUtil::LoadDefaults( *pCourse );
			pCourse->m_LoadedFromProfile = ProfileSlot_Machine;
			SONGMAN->AddCourse( pCourse );
			GAMESTATE->m_pCurCourse.Set( pCourse );
			EditCourseUtil::s_bNewCourseNeedsName = true;
			EditCourseUtil::UpdateAndSetTrail();

			SCREENMAN->SetNewScreen( CREATE_NEW_SCREEN );
			return;	// don't call base
		}
		else if( m_pRows[iCurRow]->GetRowType() == OptionRow::RowType_Exit )
		{
			this->HandleScreenMessage( SM_GoToPrevScreen );
			return;	// don't call base
		}
		else	// a Workout
		{
			// do base behavior
		}
	}
	else if( SM == SM_BackFromRename )
	{
		if( !ScreenTextEntry::s_bCancelledLast )
		{
			ASSERT( ScreenTextEntry::s_sLastAnswer != "" );	// validate should have assured this

			if( !EditCourseUtil::RenameAndSave(GAMESTATE->m_pCurCourse, ScreenTextEntry::s_sLastAnswer) )
			{
				ScreenPrompt::Prompt( SM_None, ERROR_RENAMING );
				return;
			}

			SCREENMAN->SetNewScreen( this->m_sName ); // reload
		}
	}
	else if( SM == SM_BackFromDelete )
	{
		if( ScreenPrompt::s_LastAnswer == ANSWER_YES )
		{
			Course *pCourse = GetCourseWithFocus();
			if( !EditCourseUtil::RemoveAndDeleteFile( pCourse ) )
			{
				ScreenPrompt::Prompt( SM_None, ssprintf(ERROR_DELETING_FILE.GetValue(),pCourse->m_sPath.c_str()) );
				return;
			}

			GAMESTATE->m_pCurCourse.Set( NULL );
			GAMESTATE->m_pCurTrail[PLAYER_1].Set( NULL );
			SCREENMAN->SetNewScreen( this->m_sName ); // reload
		}
	}
	else if( SM == SM_BackFromContextMenu )
	{
		if( !ScreenMiniMenu::s_bCancelled )
		{
			switch( ScreenMiniMenu::s_iLastRowCode )
			{
			case ManageWorkoutsAction_Choose:
				{
					GAMESTATE->m_pCurCourse.Set( GetCourseWithFocus() );
					EditCourseUtil::UpdateAndSetTrail();
					EditCourseUtil::s_bNewCourseNeedsName = false;
					ScreenOptions::BeginFadingOut();
				}
				break;
			case ManageWorkoutsAction_Rename:
				{
					ScreenTextEntry::TextEntry( 
						SM_BackFromRename, 
						ENTER_NAME, 
						GAMESTATE->m_pCurCourse->GetDisplayFullTitle(), 
						EditCourseUtil::MAX_NAME_LENGTH, 
						EditCourseUtil::ValidateEditCourseName );
				}
				break;
			case ManageWorkoutsAction_Delete:
				{
					ScreenPrompt::Prompt( SM_BackFromDelete, THIS_WILL_BE_LOST.GetValue()+"\n\n"+CONTINUE_WITH_DELETE.GetValue(), PROMPT_YES_NO, ANSWER_NO );
				}
				break;
			}
		}
	}
	else if( SM == SM_LoseFocus )
	{
		this->PlayCommand( "ScreenLoseFocus" );
	}
	else if( SM == SM_GainFocus )
	{
		this->PlayCommand( "ScreenGainFocus" );
	}

	ScreenOptions::HandleScreenMessage( SM );
}
	
void ScreenOptionsManageWorkouts::AfterChangeRow( PlayerNumber pn )
{
	ScreenOptions::AfterChangeRow( pn );
}

static LocalizedString YOU_HAVE_MAX( "ScreenOptionsManageWorkouts", "You have %d, the maximum number allowed." );
static LocalizedString YOU_MUST_DELETE( "ScreenOptionsManageWorkouts", "You must delete an existing before creating a new." );
void ScreenOptionsManageWorkouts::ProcessMenuStart( const InputEventPlus &input )
{
	if( IsTransitioning() )
		return;

	int iCurRow = m_iCurrentRow[GAMESTATE->m_MasterPlayerNumber];

	if( iCurRow == 0 )	// "create new"
	{
		vector<Course*> vpCourses;
		EditCourseUtil::GetAllEditCourses( vpCourses );
		if( vpCourses.size() >= (size_t)EditCourseUtil::MAX_PER_PROFILE )
		{
			RString s = ssprintf( YOU_HAVE_MAX.GetValue()+"\n\n"+YOU_MUST_DELETE.GetValue(), EditCourseUtil::MAX_PER_PROFILE );
			ScreenPrompt::Prompt( SM_None, s );
			return;
		}
		SCREENMAN->PlayStartSound();
		this->BeginFadingOut();
	}
	else if( m_pRows[iCurRow]->GetRowType() == OptionRow::RowType_Exit )
	{
		SCREENMAN->PlayStartSound();
		this->BeginFadingOut();
	}
	else	// a Steps
	{
		g_TempMenu.rows.clear();
		FOREACH_ManageWorkoutsAction( i )
		{
			MenuRowDef mrd( i, ManageWorkoutsActionToString(i), true, EditMode_Home, true, true, 0, "" );
			g_TempMenu.rows.push_back( mrd );
		}

		int iWidth, iX, iY;
		this->GetWidthXY( PLAYER_1, iCurRow, 0, iWidth, iX, iY );
		ScreenMiniMenu::MiniMenu( &g_TempMenu, SM_BackFromContextMenu, SM_BackFromContextMenu, (float)iX, (float)iY );
	}
}

void ScreenOptionsManageWorkouts::ImportOptions( int iRow, const vector<PlayerNumber> &vpns )
{

}

void ScreenOptionsManageWorkouts::ExportOptions( int iRow, const vector<PlayerNumber> &vpns )
{

}

Course *ScreenOptionsManageWorkouts::GetCourseWithFocus() const
{
	int iCurRow = m_iCurrentRow[GAMESTATE->m_MasterPlayerNumber];
	if( iCurRow == 0 )
		return NULL;
	else if( m_pRows[iCurRow]->GetRowType() == OptionRow::RowType_Exit )
		return NULL;
	
	// a Steps
	int iStepsIndex = iCurRow - 1;
	return m_vpCourses[iStepsIndex];
}

/*
 * (c) 2002-2005 Chris Danford
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

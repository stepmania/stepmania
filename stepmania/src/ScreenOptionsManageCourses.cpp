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
#include "GameManager.h"
#include "Difficulty.h"
#include "CourseUtil.h"
#include "LocalizedString.h"
#include "OptionRowHandler.h"
#include "ProfileManager.h"
#include "Profile.h"
#include "CourseWriterCRS.h"
#include "RageFileManager.h"

static void RefreshTrail()
{
	Course *pCourse = GAMESTATE->m_pCurCourse;
	if( pCourse == NULL )
	{
		GAMESTATE->m_pCurTrail[PLAYER_1].Set( NULL );
		return;
	}
	GAMESTATE->m_pCurCourse.Set( pCourse );
	Trail *pTrail = pCourse->GetTrail( GAMESTATE->m_stEdit, GAMESTATE->m_cdEdit );
	GAMESTATE->m_pCurTrail[PLAYER_1].Set( pTrail );
}

struct StepsTypeAndDifficulty
{
	StepsType st;
	Difficulty cd;

	StepsTypeAndDifficulty( const StepsType &s, const Difficulty &d ) : st( s ), cd( d ) { }
	bool operator==( const StepsTypeAndDifficulty &stad ) const { return st == stad.st && cd == stad.cd; }
};
static void SetNextCombination()
{
	vector<StepsTypeAndDifficulty> v;
	{
		FOREACH_CONST( StepsType, CommonMetrics::STEPS_TYPES_TO_SHOW.GetValue(), st )
		{
			FOREACH_CONST( CourseDifficulty, CommonMetrics::COURSE_DIFFICULTIES_TO_SHOW.GetValue(), cd )
				v.push_back( StepsTypeAndDifficulty(*st, *cd) );
		}
	}

	StepsTypeAndDifficulty curVal( GAMESTATE->m_stEdit, GAMESTATE->m_cdEdit );
	vector<StepsTypeAndDifficulty>::const_iterator iter = find( v.begin(), v.end(), curVal );
	if( iter == v.end() || ++iter == v.end() )
		iter = v.begin();

	curVal = *iter;

	GAMESTATE->m_stEdit.Set( curVal.st );
	GAMESTATE->m_cdEdit.Set( curVal.cd );
	// XXX Testing.
	SCREENMAN->SystemMessage( ssprintf("%s, %s", GAMEMAN->StepsTypeToString(curVal.st).c_str(),
					   CourseDifficultyToString(curVal.cd).c_str()) );

	RefreshTrail();
}

void ScreenOptionsEditCourseSubMenu::Init()
{
	m_soundDifficultyChanged.Load( THEME->GetPathS("ScreenEditCourseSubmenu", "difficulty changed") );
	ScreenOptions::Init();
}

void ScreenOptionsEditCourseSubMenu::MenuSelect( const InputEventPlus &input )
{
	if( input.type != IET_FIRST_PRESS )
		return;
	SetNextCombination();
	m_soundDifficultyChanged.Play();

}

static LocalizedString FAILED_TO_WRITE_COURSE( "ScreenOptionsEditCourseSubMenu", "Failed to write course." );
void ScreenOptionsEditCourseSubMenu::WriteCourse()
{
	Course *pCourse = GAMESTATE->m_pCurCourse;
		
	if( pCourse->m_sPath.empty() )
	{
		// Write the course to the profile directory
		const RString& dir = PROFILEMAN->GetProfileDir( pCourse->GetLoadedFromProfileSlot() );
		
		ASSERT( !dir.empty() );
		pCourse->m_sPath = dir + EDIT_COURSES_SUBDIR + pCourse->m_sMainTitle + ".crs";
	}
	if( !CourseWriterCRS::Write(*pCourse, pCourse->m_sPath, false) )
	{
		ScreenPrompt::Prompt( SM_None, FAILED_TO_WRITE_COURSE );
		return;
	}
	
	if( !pCourse->IsAnEdit() )
		CourseWriterCRS::Write( *pCourse, pCourse->GetCacheFilePath(), true );
}


AutoScreenMessage( SM_BackFromEnterNameForNew )
AutoScreenMessage( SM_BackFromRename )
AutoScreenMessage( SM_BackFromContextMenu )

enum CourseAction
{
	CourseAction_Edit,
	CourseAction_Rename,
	CourseAction_Delete,
	NUM_CourseAction
};
static const char *CourseActionNames[] = {
	"Edit",
	"Rename",
	"Delete",
};
XToString( CourseAction, NUM_CourseAction );
#define FOREACH_CourseAction( i ) FOREACH_ENUM2( CourseAction, i )

static MenuDef g_TempMenu(
	"ScreenMiniMenuContext"
);


static LocalizedString EDIT_NAME_CONFLICTS	( "ScreenOptionsManageCourses", "The name you chose conflicts with another edit. Please use a different name." );
static bool ValidateEditCourseName( const RString &sAnswer, RString &sErrorOut )
{
	if( sAnswer.empty() )
		return false;

	// Course name must be unique
	vector<Course*> v;
	SONGMAN->GetAllCourses( v, false );
	FOREACH_CONST( Course*, v, c )
	{
		if( GAMESTATE->m_pCurCourse.Get() == *c )
			continue;	// don't compare name against ourself

		if( (*c)->GetDisplayFullTitle() == sAnswer )
		{
			sErrorOut = EDIT_NAME_CONFLICTS;
			return false;
		}
	}

	return true;
}


REGISTER_SCREEN_CLASS( ScreenOptionsManageCourses );

void ScreenOptionsManageCourses::Init()
{
	ScreenOptionsEditCourseSubMenu::Init();

	EDIT_MODE.Load( m_sName,"EditMode" );
	CREATE_NEW_SCREEN.Load( m_sName, "CreateNewScreen" );
	GAMESTATE->m_MasterPlayerNumber = GAMESTATE->GetFirstHumanPlayer();
}

void ScreenOptionsManageCourses::BeginScreen()
{
	if( GAMESTATE->m_stEdit == StepsType_Invalid  ||
	    GAMESTATE->m_cdEdit == DIFFICULTY_Invalid )
	{
		SetNextCombination();
	}
	
	vector<OptionRowHandler*> vHands;
	
	int iIndex = 0;
	
	{
		vHands.push_back( OptionRowHandlerUtil::MakeNull() );
		OptionRowDefinition &def = vHands.back()->m_Def;
		def.m_layoutType = LAYOUT_SHOW_ONE_IN_ROW;
		def.m_bOneChoiceForAllPlayers = true;
		def.m_sName = "Create New Course";
		def.m_sExplanationName = "Create New Course";
		def.m_vsChoices.clear();
		def.m_vsChoices.push_back( "" );
		iIndex++;
	}	

	m_vpCourses.clear();
	SONGMAN->GetAllCourses( m_vpCourses, false );
	
	switch( EDIT_MODE.GetValue() )
	{
		default:
			RageException::Throw( "ScreenOptionsManageCourses: Invalid edit mode: %s.",
					      EditModeToString(EDIT_MODE.GetValue()).c_str() );
		case EditMode_Practice:
		case EditMode_Home:
			// Strip out non-edits.
			// VC6 is missing mem_fun for const members.  Lame.  Work around by not using mem_fun. */
			for( int i=m_vpCourses.size()-1; i>=0; i-- )
			{
				if( m_vpCourses[i]->IsAnEdit() )
					m_vpCourses.erase( m_vpCourses.begin()+i );
			}
			break;
		case EditMode_Full:
			break;
	}
	
	FOREACH_CONST( Course*, m_vpCourses, c )
	{
		vHands.push_back( OptionRowHandlerUtil::MakeNull() );
		OptionRowDefinition &def = vHands.back()->m_Def;
		
		switch( EDIT_MODE.GetValue() )
		{
			default:
				ASSERT(0);
			case EditMode_Practice:
			case EditMode_Home:
				def.m_sName = CourseTypeToLocalizedString( (*c)->GetCourseType() );
				break;
			case EditMode_Full:
				if( (*c)->IsAnEdit() )
					def.m_sName = "Edit";
				else
					def.m_sName = SONGMAN->ShortenGroupName( (*c)->m_sGroupName );
				break;
		}
		
		def.m_sName = ssprintf( "%3d  %s", iIndex, def.m_sName.c_str() );
		def.m_layoutType = LAYOUT_SHOW_ONE_IN_ROW;
		def.m_bAllowThemeItems = false;
		def.m_bAllowThemeTitle = false;
		def.m_bOneChoiceForAllPlayers = true;
		def.m_vsChoices.push_back( (*c)->GetDisplayFullTitle() );
		iIndex++;
	}
	
	ScreenOptions::InitMenu( vHands );
	
	ScreenOptions::BeginScreen();
	
	// select the last chosen course
	if( GAMESTATE->m_pCurCourse )
	{
		vector<Course*>::const_iterator iter = find( m_vpCourses.begin(), m_vpCourses.end(), GAMESTATE->m_pCurCourse );
		if( iter != m_vpCourses.end() )
		{
			int iIndex = iter - m_vpCourses.begin();
			this->MoveRowAbsolute( GAMESTATE->m_MasterPlayerNumber, 1 + iIndex );
		}
	}

	AfterChangeRow( GAMESTATE->m_MasterPlayerNumber );
}

static LocalizedString COURSE_WILL_BE_LOST	( "ScreenOptionsManageCourses", "This course will be lost permanently." );
static LocalizedString CONTINUE_WITH_DELETE	( "ScreenOptionsManageCourses", "Continue with delete?" );
static LocalizedString ENTER_COURSE_NAME	( "ScreenOptionsManageCourses", "Enter a name for the course." );
void ScreenOptionsManageCourses::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_Success )
	{
		LOG->Trace( "Delete succeeded; deleting course." );
		Course *pCourse = GetCourseWithFocus();
		
		GAMESTATE->m_pCurCourse.Set( NULL ); // Maybe I should just go to the next course
		RefreshTrail();
		SONGMAN->DeleteCourse( pCourse );
		FILEMAN->Remove( pCourse->m_sPath );
		if( !pCourse->IsAnEdit() )
			FILEMAN->Remove( pCourse->GetCacheFilePath() );
		delete pCourse;
		SCREENMAN->SetNewScreen( this->m_sName ); // reload
	}
	else if( SM == SM_Failure )
	{
		LOG->Trace( "Delete failed; not deleting course." );
	}
	else if( SM == SM_GoToNextScreen )
	{
		int iCurRow = m_iCurrentRow[GAMESTATE->m_MasterPlayerNumber];
		if( iCurRow == (int)m_pRows.size() - 1 )
		{
			this->HandleScreenMessage( SM_GoToPrevScreen );
			return;	// don't call base
		}
	}
	else if( SM == SM_BackFromEnterNameForNew )
	{
		if( !ScreenTextEntry::s_bCancelledLast )
		{
			ASSERT( ScreenTextEntry::s_sLastAnswer != "" );	// validate should have assured this
		
			// create
			Course *pCourse = new Course;
			ProfileSlot slot;
			
			pCourse->m_sMainTitle = ScreenTextEntry::s_sLastAnswer;
			
			switch( EDIT_MODE.GetValue() )
			{
				case EditMode_Practice:
				case EditMode_Home:
					slot = ProfileSlot_Machine;
					break;
				case EditMode_Full:
					if( PROFILEMAN->IsPersistentProfile(GAMESTATE->m_MasterPlayerNumber) )
						slot = (ProfileSlot)GAMESTATE->m_MasterPlayerNumber; // XXX I don't like this
					else
						slot = ProfileSlot_Machine;
					break;
				default:
					FAIL_M( "Invalid EditMode." );
			}
			
			pCourse->SetLoadedFromProfile( slot );
			CourseEntry ce;
			CourseUtil::MakeDefaultEditCourseEntry( ce );
			pCourse->m_vEntries.push_back( ce );
			SONGMAN->AddCourse( pCourse );
			GAMESTATE->m_pCurCourse.Set( pCourse );
			WriteCourse();

			RefreshTrail();
			this->HandleScreenMessage( SM_GoToNextScreen );
		}
	}
	else if( SM == SM_BackFromRename )
	{
		if( !ScreenTextEntry::s_bCancelledLast )
		{
			Course *pCourse = GAMESTATE->m_pCurCourse;
			const RString& sNewName = ScreenTextEntry::s_sLastAnswer;
			RString sDir, sName, sExt;
			
			ASSERT( ScreenTextEntry::s_sLastAnswer != "" );	// validate should have assured this
			ASSERT( !pCourse->m_sPath.empty() );
			FILEMAN->Remove( pCourse->m_sPath );
			if( !pCourse->IsAnEdit() )
				FILEMAN->Remove( pCourse->GetCacheFilePath() );			
			
			splitpath( pCourse->m_sPath, sDir, sName, sExt );
			pCourse->m_sPath = sDir + sNewName + sExt;
			pCourse->m_sMainTitle = sNewName;
			WriteCourse();
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
			{
				Course *pCourse = GetCourseWithFocus();
				Trail *pTrail = pCourse->GetTrail( GAMESTATE->m_stEdit, GAMESTATE->m_cdEdit );
				GAMESTATE->m_pCurCourse.Set( pCourse );
				GAMESTATE->m_pCurTrail[PLAYER_1].Set( pTrail );

				ScreenOptions::BeginFadingOut();
				break;
			}
			case CourseAction_Rename:
				ScreenTextEntry::TextEntry( SM_BackFromRename, 
							    ENTER_COURSE_NAME, 
							    GAMESTATE->m_pCurCourse->GetDisplayFullTitle(), 
							    MAX_EDIT_COURSE_TITLE_LENGTH, 
							    ValidateEditCourseName );
			break;
			case CourseAction_Delete:
				ScreenPrompt::Prompt( SM_None, COURSE_WILL_BE_LOST.GetValue()+"\n\n"+CONTINUE_WITH_DELETE.GetValue(), PROMPT_YES_NO, ANSWER_NO );
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
	
void ScreenOptionsManageCourses::AfterChangeRow( PlayerNumber pn )
{
	Course *pCourse = GetCourseWithFocus();
	Trail *pTrail = pCourse ? pCourse->GetTrail( GAMESTATE->m_stEdit, GAMESTATE->m_cdEdit ) : NULL;
	
	GAMESTATE->m_pCurCourse.Set( pCourse );
	GAMESTATE->m_pCurTrail[PLAYER_1].Set( pTrail );

	ScreenOptions::AfterChangeRow( pn );
}

static LocalizedString YOU_HAVE_MAXIMUM_EDITS_ALLOWED( "ScreenOptionsManageCourses", "You have %d course edits, the maximum number allowed." );
static LocalizedString YOU_MUST_DELETE( "ScreenOptionsManageCourses", "You must delete an existing course edit before creating a new course edit." );
void ScreenOptionsManageCourses::ProcessMenuStart( const InputEventPlus &input )
{
	if( IsTransitioning() )
		return;

	int iCurRow = m_iCurrentRow[GAMESTATE->m_MasterPlayerNumber];

	if( iCurRow == 0 )	// "create new"
	{
		if( !CREATE_NEW_SCREEN.GetValue().empty() )
		{
			SCREENMAN->SetNewScreen( CREATE_NEW_SCREEN );
		}
		else
		{
			if( SONGMAN->GetNumEditCourses(ProfileSlot_Machine) >= MAX_EDIT_COURSES_PER_PROFILE )
			{
				RString s = ssprintf( 
						      YOU_HAVE_MAXIMUM_EDITS_ALLOWED.GetValue() + "\n\n" + YOU_MUST_DELETE.GetValue(),
						      MAX_EDIT_COURSES_PER_PROFILE );
				ScreenPrompt::Prompt( SM_None, s );
				return;
			}
			
			RString sDefaultName;
			RString sThrowAway;
			for( int i=1; i<=9999; i++ )
			{
				sDefaultName = ssprintf( "NewCourse%04d", i );
				if( ValidateEditCourseName(sDefaultName,sThrowAway) )
					break;
			}
			ScreenTextEntry::TextEntry( SM_BackFromEnterNameForNew, 
						    ENTER_COURSE_NAME, 
						    sDefaultName, 
						    MAX_EDIT_COURSE_TITLE_LENGTH, 
						    ValidateEditCourseName );
		}
	}
	else if( iCurRow == (int)m_pRows.size()-1 )	// "done"
	{
		SCREENMAN->PlayStartSound();
		this->BeginFadingOut();
	}
	else	// a course
	{
		g_TempMenu.rows.clear();
		FOREACH_CourseAction( i )
		{
			MenuRowDef mrd( i, CourseActionToString(i), true, EditMode_Home, true, true, 0, "" );
			g_TempMenu.rows.push_back( mrd );
		}

		int iWidth, iX, iY;
		this->GetWidthXY( GAMESTATE->m_MasterPlayerNumber, iCurRow, 0, iWidth, iX, iY );
		ScreenMiniMenu::MiniMenu( &g_TempMenu, SM_BackFromContextMenu, SM_BackFromContextMenu, (float)iX, (float)iY );
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
	else if( iCurRow == (int)m_pRows.size()-1 )	// "done"
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

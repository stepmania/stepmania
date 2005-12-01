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

static void RefreshTrail()
{
	Course *pCourse = GAMESTATE->m_pCurCourse;
	if( pCourse == NULL )
		return;
	GAMESTATE->m_pCurCourse.Set( pCourse );
	Trail *pTrail = pCourse->GetTrail( GAMESTATE->m_stEdit, GAMESTATE->m_PreferredCourseDifficulty[PLAYER_1] );
	GAMESTATE->m_pCurTrail[PLAYER_1].Set( pTrail );
}

struct StepsTypeAndDifficulty
{
	StepsType st;
	Difficulty cd;

	bool operator==( const StepsTypeAndDifficulty &stad ) const { return st == stad.st && cd == stad.cd; }
};
static void SetNextCombination()
{
	vector<StepsTypeAndDifficulty> v;
	{
		FOREACH_CONST( StepsType, CommonMetrics::STEPS_TYPES_TO_SHOW.GetValue(), st )
		{
			FOREACH_CONST( CourseDifficulty, CommonMetrics::COURSE_DIFFICULTIES_TO_SHOW.GetValue(), cd )
			{
				StepsTypeAndDifficulty stad = { *st, *cd };
				v.push_back( stad );
			}
		}
	}

	StepsTypeAndDifficulty curVal = { GAMESTATE->m_stEdit, GAMESTATE->m_PreferredCourseDifficulty[PLAYER_1] };

	int iIndex = -1;
	vector<StepsTypeAndDifficulty>::const_iterator iter = find( v.begin(), v.end(), curVal );
	if( iter != v.end() )
	{
		iIndex = iter - v.begin();
	}

	iIndex++;
	wrap( iIndex, v.size() );
	curVal = v[iIndex];

	GAMESTATE->m_stEdit.Set( curVal.st );
	GAMESTATE->m_PreferredCourseDifficulty[PLAYER_1].Set( curVal.cd );

	RefreshTrail();
}

ScreenOptionsEditCourseSubMenu::ScreenOptionsEditCourseSubMenu( CString sName ) : ScreenOptions( sName )
{

}

void ScreenOptionsEditCourseSubMenu::MenuSelect( const InputEventPlus &input )
{
	if( input.type == IET_FIRST_PRESS )
	{
		SetNextCombination();

		SCREENMAN->PlayInvalidSound();

		return;
	}

	ScreenOptions::MenuSelect( input );
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


static bool ValidateEditCourseName( const CString &sAnswer, CString &sErrorOut )
{
	if( sAnswer.empty() )
		return false;

	// Course name must be unique
	vector<Course*> v;
	SONGMAN->GetAllCourses( v, false );
	FOREACH_CONST( Course*, v, c )
	{
		if( GAMESTATE->m_pCurCourse.Get() == *c )
			continue;	// don't comepare name against ourself

		if( (*c)->GetDisplayFullTitle() == sAnswer )
			return false;
	}

	return true;
}


REGISTER_SCREEN_CLASS( ScreenOptionsManageCourses );
ScreenOptionsManageCourses::ScreenOptionsManageCourses( CString sName ) : ScreenOptionsEditCourseSubMenu( sName )
{
	LOG->Trace( "ScreenOptionsManageCourses::ScreenOptionsManageCourses()" );
}

void ScreenOptionsManageCourses::Init()
{
	ScreenOptions::Init();

	EDIT_MODE.Load(m_sName,"EditMode");
}

void ScreenOptionsManageCourses::BeginScreen()
{
	if( GAMESTATE->m_stEdit == STEPS_TYPE_INVALID  ||
		GAMESTATE->m_PreferredCourseDifficulty[PLAYER_1] == DIFFICULTY_INVALID )
	{
		SetNextCombination();
	}


	vector<OptionRowDefinition> vDefs;
	vector<OptionRowHandler*> vHands;

	OptionRowDefinition def;
	def.m_layoutType = LAYOUT_SHOW_ONE_IN_ROW;

	int iIndex = 0;
	
	{
		def.m_sName = "";
		def.m_vsChoices.clear();
		def.m_vsChoices.push_back( "Create New" );
		vDefs.push_back( def );
		vHands.push_back( NULL );
		iIndex++;
	}

	SONGMAN->GetAllCourses( m_vpCourses, false );

	switch( EDIT_MODE.GetValue() )
	{
	default:
		ASSERT(0);
	case EDIT_MODE_PRACTICE:
	case EDIT_MODE_HOME:
		// strip out non-edits
		for( int i=m_vpCourses.size()-1; i>=0; i-- )
		{
			if( !m_vpCourses[i]->IsAnEdit() )
				m_vpCourses.erase( m_vpCourses.begin()+i );
		}
		break;
	case EDIT_MODE_FULL:
		break;
	}

	FOREACH_CONST( Course*, m_vpCourses, c )
	{
		switch( EDIT_MODE.GetValue() )
		{
		default:
			ASSERT(0);
		case EDIT_MODE_PRACTICE:
		case EDIT_MODE_HOME:
			def.m_sName = CourseTypeToThemedString( (*c)->GetCourseType() );
			break;
		case EDIT_MODE_FULL:
			if( (*c)->IsAnEdit() )
				def.m_sName = "Edit";
			else
				def.m_sName = (*c)->m_sGroupName;
			break;
		}

		def.m_sName = ssprintf("%d",iIndex) + " " + def.m_sName;

		def.m_vsChoices.clear();
		def.m_vsChoices.push_back( (*c)->GetDisplayFullTitle() );
		vDefs.push_back( def );
		vHands.push_back( NULL );
		iIndex++;
	}

	ScreenOptions::InitMenu( vDefs, vHands );

	ScreenOptions::BeginScreen();
	
	// select the last chosen course
	if( GAMESTATE->m_pCurCourse )
	{
		vector<Course*>::const_iterator iter = find( m_vpCourses.begin(), m_vpCourses.end(), GAMESTATE->m_pCurCourse );
		if( iter != m_vpCourses.end() )
		{
			int iIndex = iter - m_vpCourses.begin();
			this->MoveRowAbsolute( PLAYER_1, 1 + iIndex, false );
		}
	}

	AfterChangeRow( PLAYER_1 );
}

void ScreenOptionsManageCourses::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_Success )
	{
		LOG->Trace( "Delete successful; deleting course from memory" );

		SONGMAN->DeleteCourse( GetCourseWithFocus() );
		SCREENMAN->SetNewScreen( this->m_sName ); // reload
	}
	else if( SM == SM_Failure )
	{
		LOG->Trace( "Delete failed; not deleting course" );
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
		
			CString sNewName = ScreenTextEntry::s_sLastAnswer;

			// create
			Course *pCourse = new Course;
			pCourse->m_sMainTitle = ScreenTextEntry::s_sLastAnswer;
			pCourse->SetLoadedFromProfile( ProfileSlot_Machine );
			CourseEntry ce;
			CourseUtil::MakeDefaultEditCourseEntry( ce );
			pCourse->m_vEntries.push_back( ce );
			SONGMAN->AddCourse( pCourse );
			GAMESTATE->m_pCurCourse.Set( pCourse );

			RefreshTrail();

			this->HandleScreenMessage( SM_GoToNextScreen );
		}
	}
	else if( SM == SM_BackFromRename )
	{
		if( !ScreenTextEntry::s_bCancelledLast )
		{
			ASSERT( ScreenTextEntry::s_sLastAnswer != "" );	// validate should have assured this
		
			GAMESTATE->m_pCurCourse->m_sMainTitle = ScreenTextEntry::s_sLastAnswer;

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
					Trail *pTrail = pCourse->GetTrail( STEPS_TYPE_DANCE_SINGLE );
					GAMESTATE->m_pCurCourse.Set( pCourse );
					GAMESTATE->m_pCurTrail[PLAYER_1].Set( pTrail );

					ScreenOptions::BeginFadingOut();
				}
				break;
			case CourseAction_Rename:
				{
					ScreenTextEntry::TextEntry( 
						SM_BackFromRename, 
						"Enter a name for the course.", 
						GAMESTATE->m_pCurCourse->GetDisplayFullTitle(), 
						MAX_EDIT_COURSE_TITLE_LENGTH, 
						ValidateEditCourseName );
				}
				break;
			case CourseAction_Delete:
				{
					ScreenPrompt::Prompt( SM_None, "This course will be lost permanently.\n\nContinue with delete?", PROMPT_YES_NO, ANSWER_NO );
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
	
void ScreenOptionsManageCourses::AfterChangeRow( PlayerNumber pn )
{
	Course *pCourse = GetCourseWithFocus();
	Trail *pTrail = pCourse ? pCourse->GetTrail( STEPS_TYPE_DANCE_SINGLE ) : NULL;
	
	GAMESTATE->m_pCurCourse.Set( pCourse );
	GAMESTATE->m_pCurTrail[PLAYER_1].Set( pTrail );

	ScreenOptions::AfterChangeRow( pn );
}

void ScreenOptionsManageCourses::ProcessMenuStart( const InputEventPlus &input )
{
	int iCurRow = m_iCurrentRow[GAMESTATE->m_MasterPlayerNumber];

	if( iCurRow == 0 )	// "create new"
	{
		if( SONGMAN->GetNumEditCourses(ProfileSlot_Machine) >= MAX_EDIT_COURSES_PER_PROFILE )
		{
			CString s = ssprintf( 
				"You have %d course edits, the maximum number allowed.\n\n"
				"You must delete an existing course edit before creating a new course edit.",
				MAX_EDIT_COURSES_PER_PROFILE );
			ScreenPrompt::Prompt( SM_None, s );
			return;
		}

		CString sDefaultName;
		CString sThrowAway;
		for( int i=1; i<=9999; i++ )
		{
			sDefaultName = ssprintf( "NewCourse%04d", i );
			if( ValidateEditCourseName(sDefaultName,sThrowAway) )
				break;
		}
		ScreenTextEntry::TextEntry( 
			SM_BackFromEnterNameForNew, 
			"Enter a name for the new course.", 
			sDefaultName, 
			MAX_EDIT_COURSE_TITLE_LENGTH, 
			ValidateEditCourseName );
	}
	else if( iCurRow == (int)m_pRows.size()-1 )	// "done"
	{
		this->BeginFadingOut();
	}
	else	// a course
	{
		g_TempMenu.rows.clear();
		FOREACH_CourseAction( i )
		{
			MenuRowDef mrd( i, CourseActionToString(i), true, EDIT_MODE_HOME, true, 0, "" );
			g_TempMenu.rows.push_back( mrd );
		}

		int iWidth, iX, iY;
		this->GetWidthXY( PLAYER_1, iCurRow, 0, iWidth, iX, iY );
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

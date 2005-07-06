#include "global.h"
#include "ScreenCourseManager.h"
#include "RageLog.h"
#include "GameState.h"
#include "SongManager.h"
#include "CommonMetrics.h"

enum CourseManagerRow
{
	ROW_COURSE_GROUP,
	ROW_COURSE,
	ROW_ACTION,
	ROW_DONE,
};

enum CourseManagerAction
{
	ACTION_EDIT,
	ACTION_DELETE,
	ACTION_COPY_TO_NEW,
	ACTION_CREATE_NEW,
	NUM_CourseManagerAction
};
static const CString CourseManagerActionNames[] = {
	"Edit",
	"Delete",
	"Create New",
	"Copy to New",
};
XToString( CourseManagerAction, NUM_CourseManagerAction );

static void GetPossibleActions( vector<CourseManagerAction> &vActionsOut )
{	
	if( GAMESTATE->m_pCurCourse != NULL )
	{
		vActionsOut.push_back( ACTION_EDIT );
		vActionsOut.push_back( ACTION_DELETE );
		vActionsOut.push_back( ACTION_COPY_TO_NEW );
	}
	else
	{
		vActionsOut.push_back( ACTION_CREATE_NEW );
	}
}


REGISTER_SCREEN_CLASS( ScreenCourseManager );
ScreenCourseManager::ScreenCourseManager( CString sName ) : ScreenOptions( sName )
{
	LOG->Trace( "ScreenCourseManager::ScreenCourseManager()" );
}

void ScreenCourseManager::Init()
{
	ScreenOptions::Init();

	vector<OptionRowDefinition> vDefs;
	vector<OptionRowHandler*> vHands;

	OptionRowDefinition def;
	def.m_layoutType = LAYOUT_SHOW_ONE_IN_ROW;
	
	def.m_sName = "Group";
	def.m_vsChoices.clear();
	SONGMAN->GetCourseGroupNames( def.m_vsChoices );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	def.m_sName = "Course Group";
	def.m_vsChoices.clear();
	def.m_vsChoices.push_back( "" );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	def.m_sName = "Action";
	def.m_vsChoices.clear();
	def.m_vsChoices.push_back( "" );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	ScreenOptions::InitMenu( INPUTMODE_SHARE_CURSOR, vDefs, vHands );


	AfterChangeValueInRow( GAMESTATE->m_MasterPlayerNumber );
}

void ScreenCourseManager::HandleScreenMessage( const ScreenMessage SM )
{
	ScreenOptions::HandleScreenMessage( SM );
}
	
void ScreenCourseManager::AfterChangeValueInRow( PlayerNumber pn )
{
	ScreenOptions::AfterChangeValueInRow( pn );

	switch( m_iCurrentRow[pn] )
	{
	default:
		ASSERT(0);
	case ROW_COURSE_GROUP:
		// export current course group
		{
			OptionRow &row = *m_pRows[ROW_COURSE_GROUP];
			int iChoice = row.GetChoiceInRowWithFocus(pn);
			CString sCourseGroup = row.GetRowDef().m_vsChoices[iChoice];
			GAMESTATE->m_sPreferredCourseGroup.Set( sCourseGroup );
		}
		// Refresh courses
		{
			OptionRow &row = *m_pRows[ROW_COURSE];
			vector<Course*> vpCourses;
			SONGMAN->GetCoursesInGroup( vpCourses, GAMESTATE->m_sPreferredCourseGroup.Get(), false );
			OptionRowDefinition def = row.GetRowDef();
			def.m_vsChoices.clear();
			FOREACH_CONST( Course*, vpCourses, c )
				def.m_vsChoices.push_back( (*c)->GetTranslitFullTitle() );
			def.m_vsChoices.push_back( NULL );	// new course
			row.Reload( def );
		}
		// fall through
	case ROW_COURSE:
		// export current course
		{
			OptionRow &row = *m_pRows[ROW_COURSE];
			int iChoice = row.GetChoiceInRowWithFocus(pn);
			vector<Course*> vpCourses;
			SONGMAN->GetCoursesInGroup( vpCourses, GAMESTATE->m_sPreferredCourseGroup.Get(), false );
			Course *pCourse = vpCourses[iChoice];
			GAMESTATE->m_pCurCourse.Set( pCourse );
		}
		// refresh actions
		{
			OptionRow &row = *m_pRows[ROW_ACTION];
			OptionRowDefinition def = row.GetRowDef();
			def.m_vsChoices.clear();
			vector<CourseManagerAction> vActions;
			GetPossibleActions( vActions );
			FOREACH_CONST( CourseManagerAction, vActions, a )
				def.m_vsChoices.push_back( CourseManagerActionToString(*a) );
			row.Reload( def );
		}
		// fall through
	case ROW_ACTION:
		// fall through
	case ROW_DONE:
		break;
	}
}

void ScreenCourseManager::ProcessMenuStart( PlayerNumber pn, const InputEventType type )
{
	switch( m_iCurrentRow[pn] )
	{
	default:
		ASSERT(0);
	case ROW_COURSE_GROUP:
		SCREENMAN->PlayInvalidSound();
		break;
	case ROW_COURSE:
		SCREENMAN->PlayInvalidSound();
		break;
	case ROW_ACTION:
		ScreenOptions::BeginFadingOut();
		break;
	case ROW_DONE:
		ScreenOptions::BeginFadingOut();
		break;
	}
}

void ScreenCourseManager::ImportOptions( int row, const vector<PlayerNumber> &vpns )
{

}

void ScreenCourseManager::ExportOptions( int row, const vector<PlayerNumber> &vpns )
{

}

void ScreenCourseManager::GoToNextScreen()
{
	switch( m_iCurrentRow[GAMESTATE->m_MasterPlayerNumber] )
	{
	default:
		ASSERT(0);
	case ROW_COURSE_GROUP:
		SCREENMAN->PlayInvalidSound();
		break;
	case ROW_COURSE:
		SCREENMAN->PlayInvalidSound();
		break;
	case ROW_ACTION:
		{
			OptionRow &row = *m_pRows[ROW_ACTION];
			int iChoice = row.GetChoiceInRowWithFocus( GAMESTATE->m_MasterPlayerNumber );

			vector<CourseManagerAction> vActions;
			GetPossibleActions( vActions );
			CourseManagerAction action = vActions[iChoice];

			switch( action )
			{
			default:
				ASSERT(0);
			case ACTION_EDIT:
				GAMESTATE->m_iEditCourseEntryIndex.Set( 0 );
				SCREENMAN->SetNewScreen( "ScreenEditCourse" );
				break;
			case ACTION_DELETE:
				SCREENMAN->PlayInvalidSound();
				break;
			case ACTION_COPY_TO_NEW:
				SCREENMAN->PlayInvalidSound();
				break;
			case ACTION_CREATE_NEW:
				SCREENMAN->PlayInvalidSound();
				break;
			}
		}
		break;
	case ROW_DONE:
		SCREENMAN->SetNewScreen( FIRST_ATTRACT_SCREEN );
		break;
	}
}

void ScreenCourseManager::GoToPrevScreen()
{
	SCREENMAN->SetNewScreen( FIRST_ATTRACT_SCREEN );
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

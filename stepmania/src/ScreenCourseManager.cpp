#include "global.h"
#include "ScreenCourseManager.h"
#include "RageLog.h"
#include "GameState.h"
#include "SongManager.h"

enum CourseManagerRow
{
	ROW_GROUP,
	ROW_COURSE,
	ROW_ACTION
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

static void GetPossibleActions( Course *pCourse, vector<CourseManagerAction> &vActionsOut )
{	
	if( pCourse )
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
	
	def.name = "Group";
	def.choices.clear();
	SONGMAN->GetCourseGroupNames( def.choices );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	def.name = "Course";
	def.choices.clear();
	def.choices.push_back( "" );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	def.name = "Action";
	def.choices.clear();
	def.choices.push_back( "" );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	ScreenOptions::InitMenu( INPUTMODE_SHARE_CURSOR, vDefs, vHands );


	OnChange( GAMESTATE->m_MasterPlayerNumber );
}

void ScreenCourseManager::HandleScreenMessage( const ScreenMessage SM )
{
	ScreenOptions::HandleScreenMessage( SM );
}
	
void ScreenCourseManager::OnChange( PlayerNumber pn )
{
	ScreenOptions::OnChange( pn );

	switch( m_iCurrentRow[pn] )
	{
	case ROW_GROUP:
		// export current course group
		{
			OptionRow &row = *m_pRows[ROW_GROUP];
			int iChoice = row.GetChoiceInRowWithFocus(pn);
			CString sCourseGroup = row.GetRowDef().choices[iChoice];
			GAMESTATE->m_sPreferredCourseGroup.Set( sCourseGroup );
		}
		// Refresh courses
		{
			OptionRow &row = *m_pRows[ROW_COURSE];
			vector<Course*> vpCourses;
			SONGMAN->GetCoursesInGroup( vpCourses, GAMESTATE->m_sPreferredCourseGroup.Get(), false );
			OptionRowDefinition def = row.GetRowDef();
			def.choices.clear();
			FOREACH_CONST( Course*, vpCourses, c )
				def.choices.push_back( (*c)->GetTranslitFullTitle() );
			def.choices.push_back( NULL );	// new course
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
			def.choices.clear();
			vector<CourseManagerAction> vActions;
			GetPossibleActions( GAMESTATE->m_pCurCourse, vActions );
			FOREACH_CONST( CourseManagerAction, vActions, a )
				def.choices.push_back( CourseManagerActionToString(*a) );
			row.Reload( def );
		}
		// fall through
	case ROW_ACTION:
		// fall through
	default:
		; // nothing left to do
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

}

void ScreenCourseManager::GoToPrevScreen()
{

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

#include "global.h"
#include "ScreenEditCourse.h"
#include "RageLog.h"
#include "GameState.h"
#include "SongManager.h"
#include "CommonMetrics.h"
#include "GameManager.h"
#include "song.h"

enum EditCourseRow
{
	ROW_TITLE,
	ROW_REPEAT,
	ROW_RANDOMIZE,
	ROW_LIVES,
	ROW_TYPE,
	ROW_TYPE_METER,
	ROW_EDIT_ENTRY,
	ROW_INSERT_ENTRY,
	ROW_DELETE_ENTRY,
	ROW_DONE,
};

struct StepsTypeAndDifficulty
{
	StepsType st;
	CourseDifficulty cd;
};

static void GetStepsTypeAndDifficulty( vector<StepsTypeAndDifficulty> &vOut, vector<CString> &vsOut )
{
	Course *pCourse = GAMESTATE->m_pCurCourse;

	FOREACH_CONST( StepsType, STEPS_TYPES_TO_SHOW.GetValue(), st )
	{
		CString s1 = GAMEMAN->StepsTypeToString(*st);
		FOREACH_CONST( CourseDifficulty, COURSE_DIFFICULTIES_TO_SHOW.GetValue(), cd )
		{
			if( pCourse->GetTrail( *st, *cd ) == NULL )
				continue;

			CString s2 = CourseDifficultyToThemedString(*cd);
			StepsTypeAndDifficulty stad = { *st, *cd };
			vOut.push_back( stad );
			vsOut.push_back( s1+" "+s2 );
		}
	}
}

REGISTER_SCREEN_CLASS( ScreenEditCourse );
ScreenEditCourse::ScreenEditCourse( CString sName ) : ScreenOptions( sName )
{
	LOG->Trace( "ScreenEditCourse::ScreenEditCourse()" );
}

void ScreenEditCourse::Init()
{
	ScreenOptions::Init();


	// save a backup that we'll use if we revert.
	Course *pCourse = GAMESTATE->m_pCurCourse;
	m_Original = *pCourse;


	vector<OptionRowDefinition> vDefs;
	vector<OptionRowHandler*> vHands;

	OptionRowDefinition def;
	def.layoutType = LAYOUT_SHOW_ONE_IN_ROW;
	
	def.name = "Title";
	def.choices.clear();
	def.choices.push_back( pCourse->GetTranslitFullTitle() );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	def.name = "Repeat";
	def.choices.clear();
	def.choices.push_back( "NO" );
	def.choices.push_back( "YES" );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	def.name = "Randomize";
	def.choices.clear();
	def.choices.push_back( "NO" );
	def.choices.push_back( "YES" );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	def.name = "Lives";
	def.choices.clear();
	def.choices.push_back( "Use Bar Life" );
	for( int i=1; i<=10; i++ )
		def.choices.push_back( ssprintf("%d",i) );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	def.name = "Type";
	def.choices.clear();
	FOREACH_CONST( StepsType, STEPS_TYPES_TO_SHOW.GetValue(), st )
		def.choices.push_back( GAMEMAN->StepsTypeToString(*st) );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	def.name = "Type Meter";
	def.choices.clear();
	for( int i=MIN_METER; i<=MAX_METER; i++ )
		def.choices.push_back( ssprintf("%d",i) );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	def.name = "Edit Entry";
	def.choices.clear();
	for( unsigned i=0; i<pCourse->m_vEntries.size(); i++ )
		def.choices.push_back( ssprintf("%u of %u",i+1,pCourse->m_vEntries.size()) );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	def.name = "Insert Entry";
	def.choices.clear();
	for( unsigned i=0; i<=pCourse->m_vEntries.size(); i++ )
	{
		CString s;
		if( i == pCourse->m_vEntries.size() )
			s = ssprintf("After %u",i);
		else
			s = ssprintf("Before %u",i+1);
		def.choices.push_back( s );
	}
	vDefs.push_back( def );
	vHands.push_back( NULL );

	def.name = "Delete Entry";
	def.choices.clear();
	for( unsigned i=0; i<pCourse->m_vEntries.size(); i++ )
		def.choices.push_back( ssprintf("%u of %u",i+1,pCourse->m_vEntries.size()) );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	ScreenOptions::InitMenu( INPUTMODE_SHARE_CURSOR, vDefs, vHands );

	AfterChangeValueInRow( GAMESTATE->m_MasterPlayerNumber );
}

void ScreenEditCourse::HandleScreenMessage( const ScreenMessage SM )
{
	ScreenOptions::HandleScreenMessage( SM );
}
	
void ScreenEditCourse::AfterChangeValueInRow( PlayerNumber pn )
{
	ScreenOptions::AfterChangeValueInRow( pn );

	Course *pCourse = GAMESTATE->m_pCurCourse;
	StepsType st = STEPS_TYPE_INVALID;
	CourseDifficulty cd = DIFFICULTY_INVALID;
	int iMeter = -1;

	switch( m_iCurrentRow[pn] )
	{
	default:
		ASSERT(0);
	case ROW_TITLE:
		// fall through
	case ROW_REPEAT:
		// fall through
	case ROW_RANDOMIZE:
		// fall through
	case ROW_LIVES:
		// Refresh type
		{
			OptionRow &row = *m_pRows[ROW_TYPE];
			OptionRowDefinition def = row.GetRowDef();
			def.choices.clear();
			vector<StepsTypeAndDifficulty> vThrowAway;
			GetStepsTypeAndDifficulty( vThrowAway, def.choices );
			row.Reload( def );
		}
		// fall through
	case ROW_TYPE:
		// export StepsType and CouresDifficulty
		{
			OptionRow &row = *m_pRows[ROW_TYPE];
			int iChoice = row.GetChoiceInRowWithFocus( GAMESTATE->m_MasterPlayerNumber );
			vector<StepsTypeAndDifficulty> v;
			vector<CString> vsThrowAway;
			GetStepsTypeAndDifficulty( v, vsThrowAway );
			st = v[iChoice].st;
			cd = v[iChoice].cd;
			GAMESTATE->m_pCurTrail[GAMESTATE->m_MasterPlayerNumber].Set( pCourse->GetTrail( st, cd ) );
		}
		// refresh meter
		{
			OptionRow &row = *m_pRows[ROW_TYPE_METER];
			OptionRowDefinition def = row.GetRowDef();
			Trail *pTrail = GAMESTATE->m_pCurTrail[GAMESTATE->m_MasterPlayerNumber];
			ASSERT( pTrail );
			row.SetOneSharedSelection( pTrail->GetMeter()-MIN_METER );
			row.Reload( def );
		}
		// fall through
	case ROW_TYPE_METER:
		// export meter
		{
			OptionRow &row = *m_pRows[ROW_TYPE_METER];
			int iChoice = row.GetChoiceInRowWithFocus( GAMESTATE->m_MasterPlayerNumber );
			iMeter = 1+iChoice;
		}
		// fall through
	case ROW_EDIT_ENTRY:
		// export entry number
		{
			EditCourseRow ecr = m_iCurrentRow[GAMESTATE->m_MasterPlayerNumber] == ROW_EDIT_ENTRY ? ROW_EDIT_ENTRY : ROW_INSERT_ENTRY;
			OptionRow &row = *m_pRows[ecr];
			int iChoice = row.GetChoiceInRowWithFocus( GAMESTATE->m_MasterPlayerNumber );
			GAMESTATE->m_iEditCourseEntryIndex.Set( iChoice );
		}		
		// fall through
	case ROW_INSERT_ENTRY:
		// fall through
	case ROW_DELETE_ENTRY:
		// fall through
	case ROW_DONE:
		break;
	}
}

void ScreenEditCourse::ImportOptions( int iRow, const vector<PlayerNumber> &vpns )
{
	switch( iRow )
	{
	case ROW_EDIT_ENTRY:
	case ROW_INSERT_ENTRY:
	case ROW_DELETE_ENTRY:
		OptionRow &row = *m_pRows[iRow];
		row.SetChoiceInRowWithFocusShared( GAMESTATE->m_iEditCourseEntryIndex );
		break;
	}
}

void ScreenEditCourse::ExportOptions( int row, const vector<PlayerNumber> &vpns )
{

}

void ScreenEditCourse::GoToNextScreen()
{
	switch( m_iCurrentRow[GAMESTATE->m_MasterPlayerNumber] )
	{
	default:
	case ROW_TITLE:
	case ROW_REPEAT:
	case ROW_RANDOMIZE:
	case ROW_LIVES:
	case ROW_TYPE:
	case ROW_TYPE_METER:
		ASSERT(0);
	case ROW_INSERT_ENTRY:
	case ROW_DELETE_ENTRY:
		SCREENMAN->SetNewScreen( "ScreenEditCourse" );
		break;
	case ROW_EDIT_ENTRY:
		SCREENMAN->SetNewScreen( "ScreenEditCourseEntry" );
		break;
	case ROW_DONE:
		SCREENMAN->SetNewScreen( "ScreenCourseManager" );
		break;
	}
}

void ScreenEditCourse::GoToPrevScreen()
{
	// revert
	Course *pCourse = GAMESTATE->m_pCurCourse;
	*pCourse = m_Original;

	SCREENMAN->SetNewScreen( "ScreenCourseManager" );
}

void ScreenEditCourse::ProcessMenuStart( PlayerNumber pn, const InputEventType type )
{
	Course *pCourse = GAMESTATE->m_pCurCourse;

	switch( m_iCurrentRow[GAMESTATE->m_MasterPlayerNumber] )
	{
	default:
		ASSERT(0);
	case ROW_TITLE:
	case ROW_REPEAT:
	case ROW_RANDOMIZE:
	case ROW_LIVES:
	case ROW_TYPE:
	case ROW_TYPE_METER:
		SCREENMAN->PlayInvalidSound();
		break;
	case ROW_INSERT_ENTRY:
		{
			OptionRow &row = *m_pRows[ROW_INSERT_ENTRY];
			int iChoice = row.GetChoiceInRowWithFocus( GAMESTATE->m_MasterPlayerNumber );
			pCourse->m_vEntries.insert( pCourse->m_vEntries.begin()+iChoice, CourseEntry() );
			ScreenOptions::BeginFadingOut();
		}
		break;
	case ROW_DELETE_ENTRY:
		{
			OptionRow &row = *m_pRows[ROW_DELETE_ENTRY];
			int iChoice = row.GetChoiceInRowWithFocus( GAMESTATE->m_MasterPlayerNumber );
			pCourse->m_vEntries.erase( pCourse->m_vEntries.begin()+iChoice );
			ScreenOptions::BeginFadingOut();
		}
		break;
	case ROW_EDIT_ENTRY:
	case ROW_DONE:
		ScreenOptions::BeginFadingOut();
		break;
	}
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

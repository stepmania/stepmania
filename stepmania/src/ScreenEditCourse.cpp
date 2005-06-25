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
	ROW_STEPS_TYPE,
	ROW_DIFFICULTY,
	ROW_METER,
	ROW_SONG_NUMBER,
	ROW_SONG,
	ROW_STEPS,
	ROW_SET_MODS,
};

REGISTER_SCREEN_CLASS( ScreenEditCourse );
ScreenEditCourse::ScreenEditCourse( CString sName ) : ScreenOptions( sName )
{
	LOG->Trace( "ScreenEditCourse::ScreenEditCourse()" );
}

void ScreenEditCourse::Init()
{
	ScreenOptions::Init();

	vector<OptionRowDefinition> vDefs;
	vector<OptionRowHandler*> vHands;

	OptionRowDefinition def;
	def.layoutType = LAYOUT_SHOW_ONE_IN_ROW;
	
	Course *pCourse = GAMESTATE->m_pCurCourse;

	def.name = "Title";
	def.choices.clear();
	def.choices.push_back( pCourse->GetTranslitFullTitle() );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	def.name = "Steps Type";
	def.choices.clear();
	FOREACH_CONST( StepsType, STEPS_TYPES_TO_SHOW.GetValue(), st )
		def.choices.push_back( GAMEMAN->StepsTypeToString(*st) );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	def.name = "Difficulty";
	def.choices.clear();
	def.choices.push_back( "" );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	def.name = "Meter";
	def.choices.clear();
	for( int i=MIN_METER; i<=MAX_METER; i++ )
		def.choices.push_back( ssprintf("%d",i) );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	def.name = "Song Number";
	def.choices.clear();
	def.choices.push_back( "" );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	def.name = "Song";
	def.choices.clear();
	def.choices.push_back( "" );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	def.name = "Steps";
	def.choices.clear();
	def.choices.push_back( "" );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	def.name = "Set Mods";
	def.choices.clear();
	def.choices.push_back( "Set Mods" );
	vDefs.push_back( def );
	vHands.push_back( NULL );

	ScreenOptions::InitMenu( INPUTMODE_SHARE_CURSOR, vDefs, vHands );

	OnChange( GAMESTATE->m_MasterPlayerNumber );
}

void ScreenEditCourse::HandleScreenMessage( const ScreenMessage SM )
{
	ScreenOptions::HandleScreenMessage( SM );
}
	
void ScreenEditCourse::OnChange( PlayerNumber pn )
{
	ScreenOptions::OnChange( pn );
	Course *pCourse = GAMESTATE->m_pCurCourse;
	StepsType st = STEPS_TYPE_INVALID;
	CourseDifficulty cd = DIFFICULTY_INVALID;
	Trail *pTrail = NULL;
	int iMeter = -1;
	int iSongNumber = -1;
	Song *pSong = NULL;
	Steps *pSteps = NULL;

	switch( m_iCurrentRow[pn] )
	{
	case ROW_STEPS_TYPE:
		// export StepsType
		{
			OptionRow &row = *m_pRows[ROW_STEPS_TYPE];
			int iChoice = row.GetChoiceInRowWithFocus( GAMESTATE->m_MasterPlayerNumber );
			st = STEPS_TYPES_TO_SHOW.GetValue()[iChoice];
		}
		// Refresh difficulties
		{
			OptionRow &row = *m_pRows[ROW_DIFFICULTY];
			OptionRowDefinition def = row.GetRowDef();
			def.choices.clear();
			FOREACH_CONST( CourseDifficulty, COURSE_DIFFICULTIES_TO_SHOW.GetValue(), cd )
				def.choices.push_back( CourseDifficultyToThemedString(*cd) );
			row.Reload( def );
		}
		// fall through
	case ROW_DIFFICULTY:
		// export CouresDifficulty
		{
			OptionRow &row = *m_pRows[ROW_DIFFICULTY];
			int iChoice = row.GetChoiceInRowWithFocus( GAMESTATE->m_MasterPlayerNumber );
			cd = COURSE_DIFFICULTIES_TO_SHOW.GetValue()[iChoice];
			pTrail = pCourse->GetTrail( st, cd );
		}
		// refresh meter
		{
			OptionRow &row = *m_pRows[ROW_METER];
			OptionRowDefinition def = row.GetRowDef();
			row.SetOneSharedSelection( pTrail->GetMeter()-1 );
			row.Reload( def );
		}
		// fall through
	case ROW_METER:
		// export meter
		{
			OptionRow &row = *m_pRows[ROW_METER];
			int iChoice = row.GetChoiceInRowWithFocus( GAMESTATE->m_MasterPlayerNumber );
			iMeter = 1+iChoice;
		}
		// refresh song number
		{
			OptionRow &row = *m_pRows[ROW_SONG_NUMBER];
			OptionRowDefinition def = row.GetRowDef();
			row.SetOneSharedSelection( 0 );
			row.Reload( def );
		}
		// fall through
	case ROW_SONG_NUMBER:
		// export song number
		{
			OptionRow &row = *m_pRows[ROW_SONG_NUMBER];
			int iChoice = row.GetChoiceInRowWithFocus( GAMESTATE->m_MasterPlayerNumber );
			iSongNumber = 1+iChoice;
		}		
		// refresh song
		{
			OptionRow &row = *m_pRows[ROW_SONG];
			OptionRowDefinition def = row.GetRowDef();
			row.SetOneSharedSelection( pTrail->GetMeter()-1 );
			row.Reload( def );
		}
		// fall through
	case ROW_SONG:
		// export song
		{
			OptionRow &row = *m_pRows[ROW_SONG];
			int iChoice = row.GetChoiceInRowWithFocus( GAMESTATE->m_MasterPlayerNumber );
			pSong = SONGMAN->GetAllSongs()[iChoice];
		}
		// fall through
	case ROW_STEPS:
		// export steps
		{
			OptionRow &row = *m_pRows[ROW_SONG];
			int iChoice = row.GetChoiceInRowWithFocus( GAMESTATE->m_MasterPlayerNumber );
			pSteps = pSong->GetStepsByStepsType(st)[iChoice];
		}
		// fall through
	case ROW_SET_MODS:
		// fall through
	default:
		; // nothing left to do
	}
}

void ScreenEditCourse::ImportOptions( int row, const vector<PlayerNumber> &vpns )
{

}

void ScreenEditCourse::ExportOptions( int row, const vector<PlayerNumber> &vpns )
{

}

void ScreenEditCourse::GoToNextScreen()
{
	SCREENMAN->SetNewScreen( "ScreenEditCourseMenu" );
}

void ScreenEditCourse::GoToPrevScreen()
{

}

void ScreenEditCourse::BeginFadingOut()
{
	switch( m_iCurrentRow[GAMESTATE->m_MasterPlayerNumber] )
	{
	case ROW_TITLE:
		break;
	case ROW_STEPS_TYPE:
		break;
	case ROW_DIFFICULTY:
		break;
	case ROW_METER:
		break;
	case ROW_SONG_NUMBER:
		break;
	case ROW_SONG:
		break;
	case ROW_STEPS:
		break;
	case ROW_SET_MODS:
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

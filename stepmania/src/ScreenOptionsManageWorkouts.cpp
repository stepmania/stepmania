#include "global.h"
#include "ScreenOptionsManageWorkouts.h"
#include "ScreenManager.h"
#include "ScreenMiniMenu.h"
#include "OptionRowHandler.h"
#include "WorkoutManager.h"
#include "Workout.h"
#include "LocalizedString.h"
#include "GameState.h"
#include "ScreenTextEntry.h"
#include "ScreenPrompt.h"
#include "StatsManager.h"

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
XToString2( ManageWorkoutsAction );
#define FOREACH_ManageWorkoutsAction( i ) FOREACH_ENUM( ManageWorkoutsAction, i )

static MenuDef g_TempMenu(
	"ScreenMiniMenuContext"
);



REGISTER_SCREEN_CLASS( ScreenOptionsManageWorkouts );

void ScreenOptionsManageWorkouts::Init()
{
	ScreenOptions::Init();

	CREATE_NEW_SCREEN.Load( m_sName, "CreateNewScreen" );
}

void ScreenOptionsManageWorkouts::BeginScreen()
{
	STATSMAN->Reset();

	WORKOUTMAN->LoadAllFromDisk();

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

	m_vpWorkouts = WORKOUTMAN->m_vpAllWorkouts;

	FOREACH_CONST( Workout*, m_vpWorkouts, p )
	{
		vHands.push_back( OptionRowHandlerUtil::MakeNull() );
		OptionRowDefinition &def = vHands.back()->m_Def;
		
		def.m_sName = (*p)->m_sName;
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
	if( WORKOUTMAN->m_pCurWorkout )
	{
		vector<Workout*>::const_iterator iter = find( m_vpWorkouts.begin(), m_vpWorkouts.end(), WORKOUTMAN->m_pCurWorkout );
		if( iter != m_vpWorkouts.end() )
		{
			int iIndex = iter - m_vpWorkouts.begin();
			this->MoveRowAbsolute( PLAYER_1, 1 + iIndex );
		}
	}

	AfterChangeRow( PLAYER_1 );
}

static LocalizedString ERROR_RENAMING_WORKOUT		("ScreenOptionsManageWorkouts", "Error renaming workout file.");
static LocalizedString ERROR_DELETING_WORKOUT_FILE	("ScreenOptionsManageWorkouts", "Error deleting the workout file '%s'.");
static LocalizedString THIS_WORKOUT_WILL_BE_LOST	("ScreenOptionsManageWorkouts", "This workout will be lost permanently.");
static LocalizedString CONTINUE_WITH_DELETE		("ScreenOptionsManageWorkouts", "Continue with delete?");
static LocalizedString ENTER_NAME_FOR_WORKOUT		("ScreenOptionsManageWorkouts", "Enter a name for this workout.");
void ScreenOptionsManageWorkouts::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_GoToNextScreen )
	{
		int iCurRow = m_iCurrentRow[GAMESTATE->m_MasterPlayerNumber];

		if( iCurRow == 0 )	// "create new"
		{
			WORKOUTMAN->m_pCurWorkout = new Workout;
			WORKOUTMAN->LoadDefaults( *WORKOUTMAN->m_pCurWorkout );
			WORKOUTMAN->m_vpAllWorkouts.push_back( WORKOUTMAN->m_pCurWorkout );

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

			Workout *pWorkout = WORKOUTMAN->m_pCurWorkout;
			if( !WORKOUTMAN->RenameAndSave(pWorkout, ScreenTextEntry::s_sLastAnswer) )
			{
				ScreenPrompt::Prompt( SM_None, ERROR_RENAMING_WORKOUT );
				return;
			}

			SCREENMAN->SetNewScreen( this->m_sName ); // reload
		}
	}
	else if( SM == SM_BackFromDelete )
	{
		if( ScreenPrompt::s_LastAnswer == ANSWER_YES )
		{
			Workout *pWorkout = GetWorkoutWithFocus();
			if( !WORKOUTMAN->RemoveAndDeleteFile( pWorkout ) )
			{
				ScreenPrompt::Prompt( SM_None, ssprintf(ERROR_DELETING_WORKOUT_FILE.GetValue(),pWorkout->m_sFile.c_str()) );
				return;
			}

			WORKOUTMAN->m_pCurWorkout = NULL;
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
					Workout *p = GetWorkoutWithFocus();
					WORKOUTMAN->m_pCurWorkout = p;

					ScreenOptions::BeginFadingOut();
				}
				break;
			case ManageWorkoutsAction_Rename:
				{
					ScreenTextEntry::TextEntry( 
						SM_BackFromRename, 
						ENTER_NAME_FOR_WORKOUT, 
						WORKOUTMAN->m_pCurWorkout->m_sName, 
						MAX_WORKOUT_NAME_LENGTH, 
						WorkoutManager::ValidateWorkoutName );
				}
				break;
			case ManageWorkoutsAction_Delete:
				{
					ScreenPrompt::Prompt( SM_BackFromDelete, THIS_WORKOUT_WILL_BE_LOST.GetValue()+"\n\n"+CONTINUE_WITH_DELETE.GetValue(), PROMPT_YES_NO, ANSWER_NO );
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
	WORKOUTMAN->m_pCurWorkout = GetWorkoutWithFocus();

	ScreenOptions::AfterChangeRow( pn );
}

static LocalizedString YOU_HAVE_MAX_WORKOUTS( "ScreenOptionsManageWorkouts", "You have %d workouts, the maximum number allowed." );
static LocalizedString YOU_MUST_DELETE( "ScreenOptionsManageWorkouts", "You must delete an existing workout before creating a new workout." );
void ScreenOptionsManageWorkouts::ProcessMenuStart( const InputEventPlus &input )
{
	if( IsTransitioning() )
		return;

	int iCurRow = m_iCurrentRow[GAMESTATE->m_MasterPlayerNumber];

	if( iCurRow == 0 )	// "create new"
	{
		if( WORKOUTMAN->m_vpAllWorkouts.size() >= size_t(MAX_WORKOUTS_PER_PROFILE) )
		{
			RString s = ssprintf( YOU_HAVE_MAX_WORKOUTS.GetValue()+"\n\n"+YOU_MUST_DELETE.GetValue(), MAX_WORKOUTS_PER_PROFILE );
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

Workout *ScreenOptionsManageWorkouts::GetWorkoutWithFocus() const
{
	int iCurRow = m_iCurrentRow[GAMESTATE->m_MasterPlayerNumber];
	if( iCurRow == 0 )
		return NULL;
	else if( m_pRows[iCurRow]->GetRowType() == OptionRow::RowType_Exit )
		return NULL;
	
	// a Steps
	int iStepsIndex = iCurRow - 1;
	return m_vpWorkouts[iStepsIndex];
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

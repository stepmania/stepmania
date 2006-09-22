#include "global.h"

#include "ScreenOptionsReviewWorkout.h"
#include "ScreenManager.h"
#include "RageUtil.h"
#include "GameState.h"
#include "OptionRowHandler.h"
#include "ProfileManager.h"
#include "ScreenMiniMenu.h"
#include "Workout.h"
#include "LocalizedString.h"
#include "SongManager.h"
#include "SongUtil.h"
#include "ScreenTextEntry.h"
#include "WorkoutManager.h"
#include "GameManager.h"
#include "Profile.h"
#include "ScreenPrompt.h"
#include "PlayerState.h"

enum ReviewWorkoutRow
{
	ReviewWorkoutRow_Play,
	ReviewWorkoutRow_EditWorkout,
	ReviewWorkoutRow_EditPlaylist,
	ReviewWorkoutRow_Save,
	NUM_ReviewWorkoutRow
};

static const MenuRowDef g_MenuRows[] = 
{
	MenuRowDef( -1,	"Play",			true, EditMode_Practice, true, false, 0, NULL ),
	MenuRowDef( -1,	"Edit Workout",		true, EditMode_Practice, true, false, 0, NULL ),
	MenuRowDef( -1,	"Edit Playlist",	true, EditMode_Practice, true, false, 0, NULL ),
	MenuRowDef( -1,	"Save",			true, EditMode_Practice, true, false, 0, NULL ),
};

REGISTER_SCREEN_CLASS( ScreenOptionsReviewWorkout );

AutoScreenMessage( SM_BackFromEnterName )

void ScreenOptionsReviewWorkout::Init()
{
	ScreenOptions::Init();

	m_soundSave.Load( THEME->GetPathS(m_sName,"Save") );
	PLAY_SCREEN.Load(m_sName,"PlayScreen");
	EDIT_WORKOUT_SCREEN.Load(m_sName,"EditWorkoutScreen");
	EDIT_PLAYLIST_SCREEN.Load(m_sName,"EditPlaylistScreen");
}

void ScreenOptionsReviewWorkout::BeginScreen()
{
	vector<OptionRowHandler*> vHands;
	FOREACH_ENUM2( ReviewWorkoutRow, rowIndex )
	{
		const MenuRowDef &mr = g_MenuRows[rowIndex];
		OptionRowHandler *pHand = OptionRowHandlerUtil::MakeSimple( mr );
		vHands.push_back( pHand );
	}

	ScreenOptions::InitMenu( vHands );

	ScreenOptions::BeginScreen();
}

ScreenOptionsReviewWorkout::~ScreenOptionsReviewWorkout()
{

}

void ScreenOptionsReviewWorkout::ImportOptions( int iRow, const vector<PlayerNumber> &vpns )
{
	//OptionRow &row = *m_pRows[iRow];
}

void ScreenOptionsReviewWorkout::ExportOptions( int iRow, const vector<PlayerNumber> &vpns )
{
	OptionRow &row = *m_pRows[iRow];
	int iIndex = row.GetOneSharedSelection( true );
	RString sValue;
	if( iIndex >= 0 )
		sValue = row.GetRowDef().m_vsChoices[ iIndex ];
}

static LocalizedString ERROR_SAVING_WORKOUT	( "ScreenOptionsReviewWorkout", "Error saving workout." );
static LocalizedString WORKOUT_SAVED		( "ScreenOptionsReviewWorkout", "Workout saved successfully." );
void ScreenOptionsReviewWorkout::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_GoToNextScreen )
	{
		int iRow = m_iCurrentRow[GAMESTATE->m_MasterPlayerNumber];
		switch( iRow )
		{
		case ReviewWorkoutRow_Play:
			{
				Workout *pWorkout = WORKOUTMAN->m_pCurWorkout;
				pWorkout->GenerateCourse( *WORKOUTMAN->m_pTempCourse );
				GAMESTATE->m_pCurCourse.Set( WORKOUTMAN->m_pTempCourse );
				GAMESTATE->m_pCurTrail[PLAYER_1].Set( GAMESTATE->m_pCurCourse->GetTrail(STEPS_TYPE_DANCE_SINGLE) );
				switch( pWorkout->m_WorkoutStepsType )
				{
				DEFAULT_FAIL(pWorkout->m_WorkoutStepsType);
				case WorkoutStepsType_NormalSteps:
					PO_GROUP_ASSIGN_N( GAMESTATE->m_pPlayerState[PLAYER_1]->m_PlayerOptions, ModsLevel_Stage, m_bTransforms, PlayerOptions::TRANSFORM_LITTLE, false );
					PO_GROUP_ASSIGN_N( GAMESTATE->m_pPlayerState[PLAYER_1]->m_PlayerOptions, ModsLevel_Stage, m_bTransforms, PlayerOptions::TRANSFORM_NOHANDS, false );
					break;
				case WorkoutStepsType_WorkoutSteps:
					PO_GROUP_ASSIGN_N( GAMESTATE->m_pPlayerState[PLAYER_1]->m_PlayerOptions, ModsLevel_Stage, m_bTransforms, PlayerOptions::TRANSFORM_LITTLE, true );
					PO_GROUP_ASSIGN_N( GAMESTATE->m_pPlayerState[PLAYER_1]->m_PlayerOptions, ModsLevel_Stage, m_bTransforms, PlayerOptions::TRANSFORM_NOHANDS, true );
					break;
				}

				GAMESTATE->m_PlayMode.Set( PLAY_MODE_ENDLESS );
				GAMESTATE->m_bSideIsJoined[0] = true;
				GAMESTATE->m_pCurStyle.Set( GAMEMAN->GameAndStringToStyle(GAMESTATE->m_pCurGame,"single") );

				PROFILEMAN->GetProfile(ProfileSlot_Player1)->m_GoalType = GoalType_Time;
				PROFILEMAN->GetProfile(ProfileSlot_Player1)->m_iGoalSeconds = pWorkout->m_iMinutes * 60;
				SCREENMAN->SetNewScreen( PLAY_SCREEN );
			}
			return;	// handled
		case ReviewWorkoutRow_EditWorkout:
			SCREENMAN->SetNewScreen( EDIT_WORKOUT_SCREEN );
			return;	// handled
		case ReviewWorkoutRow_EditPlaylist:
			SCREENMAN->SetNewScreen( EDIT_PLAYLIST_SCREEN );
			return;	// handled
		}
	}
	else if( SM == SM_BackFromEnterName )
	{
		if( !ScreenTextEntry::s_bCancelledLast )
		{
			ASSERT( ScreenTextEntry::s_sLastAnswer != "" );	// validate should have assured this
			if( WORKOUTMAN->RenameAndSave( WORKOUTMAN->m_pCurWorkout, ScreenTextEntry::s_sLastAnswer ) )
			{
				m_soundSave.Play();
				SCREENMAN->SystemMessage( WORKOUT_SAVED );
				MESSAGEMAN->Broadcast( "WorkoutChanged" );
			}
		}
	}

	ScreenOptions::HandleScreenMessage( SM );
}

void ScreenOptionsReviewWorkout::AfterChangeValueInRow( int iRow, PlayerNumber pn )
{
	ScreenOptions::AfterChangeValueInRow( iRow, pn );
}


static LocalizedString ENTER_WORKOUT_NAME	( "ScreenOptionsReviewWorkout", "Enter a name for the workout." );
void ScreenOptionsReviewWorkout::ProcessMenuStart( const InputEventPlus &input )
{
	if( IsTransitioning() )
		return;

	int iRow = m_iCurrentRow[GAMESTATE->m_MasterPlayerNumber];
	switch( iRow )
	{
	case ReviewWorkoutRow_Play:
	case ReviewWorkoutRow_EditWorkout:
	case ReviewWorkoutRow_EditPlaylist:
		SCREENMAN->PlayStartSound();
		this->BeginFadingOut();
		return;	// handled
	case ReviewWorkoutRow_Save:
		if( WORKOUTMAN->m_pCurWorkout->m_sName.empty() )
		{
			ScreenTextEntry::TextEntry( 
				SM_BackFromEnterName, 
				ENTER_WORKOUT_NAME, 
				WORKOUTMAN->m_pCurWorkout->m_sName, 
				MAX_WORKOUT_NAME_LENGTH, 
				WorkoutManager::ValidateWorkoutName );
		}
		else
		{
			if( WORKOUTMAN->Save( WORKOUTMAN->m_pCurWorkout ) )
			{
				m_soundSave.Play();
				SCREENMAN->SystemMessage( WORKOUT_SAVED );
				MESSAGEMAN->Broadcast( "WorkoutChanged" );
			}
			else
			{
				SCREENMAN->PlayInvalidSound();
				SCREENMAN->SystemMessage( ERROR_SAVING_WORKOUT );
			}
		}
		return;	// handled
	}

	ScreenOptions::ProcessMenuStart( input );
}


/*
 * (c) 2003-2004 Chris Danford
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

#include "global.h"

#include "ScreenOptionsEditWorkout.h"
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
#include "WorkoutManager.h"

enum WorkoutDetailsRow
{
	WorkoutDetailsRow_WorkoutProgram,
	WorkoutDetailsRow_Minutes,
	WorkoutDetailsRow_Meter,
	NUM_WorkoutDetailsRow
};

const MenuRowDef g_MenuRows[] = 
{
	MenuRowDef( -1,	"Workout Program",	true, EditMode_Practice, true, false, 0, NULL ),
	MenuRowDef( -1,	"Workout Minutes",	true, EditMode_Practice, true, false, 0, NULL ),
	MenuRowDef( -1,	"Workout Difficulty",	true, EditMode_Practice, true, false, 0, NULL ),
};

REGISTER_SCREEN_CLASS( ScreenOptionsEditWorkout );

void ScreenOptionsEditWorkout::Init()
{
	ScreenOptions::Init();
}

static RString MakeMinutesString( int iMeter )
{
	return ssprintf( "%d", iMeter );
}
static RString MakeMeterString( int iMeter )
{
	return ssprintf( "%d", iMeter );
}
void ScreenOptionsEditWorkout::BeginScreen()
{
	vector<OptionRowHandler*> vHands;
	FOREACH_ENUM( WorkoutDetailsRow, rowIndex )
	{
		const MenuRowDef &mr = g_MenuRows[rowIndex];
		OptionRowHandler *pHand = OptionRowHandlerUtil::MakeSimple( mr );
	
		pHand->m_Def.m_layoutType = LAYOUT_SHOW_ONE_IN_ROW;
		pHand->m_Def.m_vsChoices.clear();
	
		switch( rowIndex )
		{
		DEFAULT_FAIL(rowIndex);
		case WorkoutDetailsRow_WorkoutProgram:
			FOREACH_ENUM( WorkoutProgram, i )
				pHand->m_Def.m_vsChoices.push_back( WorkoutProgramToLocalizedString(i) );
			break;
		case WorkoutDetailsRow_Minutes:
			for( int i=MIN_WORKOUT_MINUTES; i<=20; i+=2 )
				pHand->m_Def.m_vsChoices.push_back( MakeMinutesString(i) );
			for( int i=20; i<=MAX_WORKOUT_MINUTES; i+=5 )
				pHand->m_Def.m_vsChoices.push_back( MakeMinutesString(i) );
			break;
		case WorkoutDetailsRow_Meter:
			for( int i=MIN_METER; i<=MAX_METER; i++ )
				pHand->m_Def.m_vsChoices.push_back( MakeMeterString(i) );
			break;
		}

		pHand->m_Def.m_bExportOnChange = true;
		vHands.push_back( pHand );
	}

	ScreenOptions::InitMenu( vHands );

	ScreenOptions::BeginScreen();
}

ScreenOptionsEditWorkout::~ScreenOptionsEditWorkout()
{

}

void ScreenOptionsEditWorkout::ImportOptions( int iRow, const vector<PlayerNumber> &vpns )
{
	OptionRow &row = *m_pRows[iRow];

	/*
	FIXME
	switch( iRow )
	{
	case WorkoutDetailsRow_WorkoutProgram:
		row.SetOneSharedSelectionIfPresent( WorkoutProgramToLocalizedString(WORKOUTMAN->GetCurrentWorkout()->m_WorkoutProgram) );
		break;
	case WorkoutDetailsRow_Minutes:
		row.SetOneSharedSelectionIfPresent( MakeMinutesString(WORKOUTMAN->GetCurrentWorkout()->m_iMinutes) );
		break;
	case WorkoutDetailsRow_Meter:
		row.SetOneSharedSelectionIfPresent( MakeMeterString(WORKOUTMAN->GetCurrentWorkout()->m_iAverageMeter) );
		break;
	}
	*/
}

void ScreenOptionsEditWorkout::ExportOptions( int iRow, const vector<PlayerNumber> &vpns )
{
	/*
	FIXME
	FOREACH_ENUM( WorkoutDetailsRow, i )
	{
		iRow = i;
		OptionRow &row = *m_pRows[iRow];
		int iIndex = row.GetOneSharedSelection( true );
		RString sValue;
		if( iIndex >= 0 )
			sValue = row.GetRowDef().m_vsChoices[ iIndex ];

		switch( iRow )
		{
		DEFAULT_FAIL(iRow);
		case WorkoutDetailsRow_WorkoutProgram:
			WORKOUTMAN->GetCurrentWorkout()->m_WorkoutProgram = (WorkoutProgram)iIndex;
			break;
		case WorkoutDetailsRow_Minutes:
			sscanf( sValue, "%d", &WORKOUTMAN->GetCurrentWorkout()->m_iMinutes );
			break;
		case WorkoutDetailsRow_Meter:
			WORKOUTMAN->GetCurrentWorkout()->m_iAverageMeter = iIndex + MIN_METER;
			break;
		}
	}

	MESSAGEMAN->Broadcast( "WorkoutChanged" );
	WORKOUTMAN->UpdateAndSetTrail();
	*/
}

void ScreenOptionsEditWorkout::GoToNextScreen()
{
}

void ScreenOptionsEditWorkout::GoToPrevScreen()
{
}

void ScreenOptionsEditWorkout::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_GoToNextScreen )
	{
	}
	else if( SM == SM_GoToPrevScreen )
	{
	}

	ScreenOptions::HandleScreenMessage( SM );
}

void ScreenOptionsEditWorkout::AfterChangeValueInRow( int iRow, PlayerNumber pn )
{
	ScreenOptions::AfterChangeValueInRow( iRow, pn );
}

void ScreenOptionsEditWorkout::ProcessMenuStart( const InputEventPlus &input )
{
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

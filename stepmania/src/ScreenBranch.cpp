#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenBranch

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenBranch.h"
#include "RageLog.h"
#include "GameConstantsAndTypes.h"
#include "PlayerNumber.h"
#include "ScreenManager.h"
#include "ThemeManager.h"
#include "Grade.h"
#include "GameState.h"
#include "StageStats.h"

#define CHOICES						THEME->GetMetric (m_sName,"Choices")
#define CONDITION(choice)			THEME->GetMetric (m_sName,"Condition"+choice)
#define NEXT_SCREEN(choice)			THEME->GetMetric (m_sName,"NextScreen"+choice)

bool EvaluateCondition( CString sCondition )
{
	// For now, empty expresions evaluate to true.  Is there a better way to handle this?
	if( sCondition.empty() )
		return true;
	
	// TODO: Make this a general expression evaluator
	CStringArray as;
	split( sCondition, ",", as, false );

	if( as.size()==3 && as[0].CompareNoCase("topgrade")==0 )
	{
		Grade top_grade = GRADE_E;
		vector<Song*> vSongs;
		StageStats stats;
		GAMESTATE->GetFinalEvalStatsAndSongs( stats, vSongs );
		for( int p=0; p<NUM_PLAYERS; p++ )
			if( GAMESTATE->IsHumanPlayer(p) )
				top_grade = max( top_grade, stats.GetGrade((PlayerNumber)p) );

		CString &sOp = as[1];

		Grade grade = StringToGrade(as[2]);

		if( sOp == ">=" )
			return top_grade >= grade;
	}

	RageException::Throw( "Condition '%s' is invalid", sCondition.c_str() );
}

ScreenBranch::ScreenBranch( CString sClassName ) : Screen( sClassName )
{
	LOG->Trace( "ScreenBranch::ScreenBranch()" );
	
	CStringArray as;
	split( CHOICES, ",", as, true );
	for( unsigned i=0; i<as.size(); i++ )
	{
		CString sChoice = Capitalize( as[i] );
		CString sCondition = CONDITION(sChoice);

		if( EvaluateCondition(sCondition) )
		{
			m_sChoice = sChoice;
			HandleScreenMessage( SM_GoToNextScreen );
			return;
		}
	}

	RageException::Throw( "On screen '%s' no conditions are true", sClassName.c_str() );
}

void ScreenBranch::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_GoToNextScreen:
		{
			CString sNextScreen = NEXT_SCREEN(m_sChoice);
			LOG->Trace( "Branching to '%s'", sNextScreen.c_str() );
			SCREENMAN->SetNewScreen( sNextScreen );
		}
		break;
	}
}

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
#include "ScreenManager.h"
#include "ThemeManager.h"
#include "LuaHelpers.h"
#include "ModeChoice.h"

#define CHOICES						THEME->GetMetric (m_sName,"Choices")
#define CONDITION(choice)			THEME->GetMetric (m_sName,"Condition"+choice)
#define NEXT_SCREEN(choice)			THEME->GetMetric (m_sName,"NextScreen"+choice)

ScreenBranch::ScreenBranch( CString sClassName ) : Screen( sClassName )
{
	LOG->Trace( "ScreenBranch::ScreenBranch()" );
	
	CStringArray as;
	split( CHOICES, ",", as, true );
	for( unsigned i=0; i<as.size(); i++ )
	{
		CString sChoice = Capitalize( as[i] );
		CString sCondition = CONDITION(sChoice);

		if( Lua::RunExpression(sCondition) )
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

			ModeChoice mc;
			mc.Load( 0, sNextScreen );
			if( mc.m_sScreen == "" )
				RageException::Throw("Metric %s::%s must set \"screen\"",
					m_sName.c_str(), ("NextScreen"+m_sChoice).c_str() );
			mc.ApplyToAllPlayers();
		}
		break;
	}
}

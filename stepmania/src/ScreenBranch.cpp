#include "global.h"
#include "ScreenBranch.h"
#include "RageLog.h"
#include "ScreenManager.h"
#include "ThemeManager.h"
#include "LuaHelpers.h"
#include "ModeChoice.h"
#include "RageUtil.h"

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

/*
 * (c) 2003-2004 Chris Danford, Glenn Maynard
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

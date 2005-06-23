#include "global.h"
#include "ScreenEditCoursesMenu.h"
#include "SongManager.h"
#include "ScreenManager.h"
#include "GameConstantsAndTypes.h"
#include "RageUtil.h"
#include "GameManager.h"
#include "RageLog.h"
#include "GameState.h"
#include "GameSoundManager.h"
#include "ThemeManager.h"
#include "Steps.h"


#define NEXT_SCREEN					THEME->GetMetric (m_sName,"NextScreen")
#define EXPLANATION_TEXT			THEME->GetMetric (m_sName,"ExplanationText")
#define HELP_TEXT					THEME->GetMetric (m_sName,"HelpText")

AutoScreenMessage( SM_RefreshSelector )

REGISTER_SCREEN_CLASS( ScreenEditCoursesMenu );
ScreenEditCoursesMenu::ScreenEditCoursesMenu( CString sName ) : ScreenWithMenuElements( sName )
{
	LOG->Trace( "ScreenEditCoursesMenu::ScreenEditCoursesMenu()" );

	/* Enable all players. */
	FOREACH_PlayerNumber( pn )
		GAMESTATE->m_bSideIsJoined[pn] = true;

}

void ScreenEditCoursesMenu::Init()
{
	ScreenWithMenuElements::Init();

	m_Selector.SetXY( 0, 0 );
//	m_Selector.AllowNewNotes();
	this->AddChild( &m_Selector );
	
	m_textExplanation.SetName( "Explanation" );
	m_textExplanation.LoadFromFont( THEME->GetPathF("Common","normal") );
	SET_XY_AND_ON_COMMAND( m_textExplanation );
	m_textExplanation.SetText( EXPLANATION_TEXT );
	this->AddChild( &m_textExplanation );

	this->SortByDrawOrder();
}

void ScreenEditCoursesMenu::HandleScreenMessage( const ScreenMessage SM )
{
	m_Selector.HandleScreenMessage( SM );

	switch( SM )
	{
	case SM_GoToPrevScreen:
	case SM_GoToNextScreen:
		SCREENMAN->SetNewScreen( NEXT_SCREEN );
		break;
	}
}
	
void ScreenEditCoursesMenu::MenuUp( PlayerNumber pn )
{
	m_Selector.Up();
}

void ScreenEditCoursesMenu::MenuDown( PlayerNumber pn )
{
	m_Selector.Down();
}

void ScreenEditCoursesMenu::MenuLeft( PlayerNumber pn, const InputEventType type )
{
	m_Selector.Left();
}

void ScreenEditCoursesMenu::MenuRight( PlayerNumber pn, const InputEventType type )
{
	m_Selector.Right();
}

void ScreenEditCoursesMenu::MenuStart( PlayerNumber pn )
{
	if( IsTransitioning() )
		return;

	m_Selector.Start();
}

void ScreenEditCoursesMenu::MenuBack( PlayerNumber pn )
{	
	Cancel( SM_GoToPrevScreen );

	SOUND->StopMusic();
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

#include "global.h"
#include "ScreenJukeboxMenu.h"
#include "SongManager.h"
#include "ScreenManager.h"
#include "GameConstantsAndTypes.h"
#include "RageUtil.h"
#include "GameManager.h"
#include "RageLog.h"
#include "GameState.h"
#include "GameSoundManager.h"
#include "ThemeManager.h"
#include "ScreenJukebox.h"


//
// Defines specific to ScreenJukeboxMenu
//
#define EXPLANATION_TEXT			THEME->GetMetric("ScreenJukeboxMenu","ExplanationText")


ScreenJukeboxMenu::ScreenJukeboxMenu( CString sClassName ) : ScreenWithMenuElements( sClassName )
{
	LOG->Trace( "ScreenJukeboxMenu::ScreenJukeboxMenu()" );

	GAMESTATE->m_pCurStyle = NULL;

	FOREACH_PlayerNumber( pn )
		GAMESTATE->m_bSideIsJoined[pn] = true;

	m_Selector.SetXY( 0, 0 );
//	m_Selector.AllowNewNotes();
	this->AddChild( &m_Selector );


	m_textExplanation.SetName( "Explanation" );
	m_textExplanation.LoadFromFont( THEME->GetPathToF("Common normal") );
	m_textExplanation.SetText( EXPLANATION_TEXT );
	SET_XY_AND_ON_COMMAND( m_textExplanation );
	this->AddChild( &m_textExplanation );

	this->SortByDrawOrder();

	SOUND->PlayMusic( THEME->GetPathToS("ScreenJukeboxMenu music") );
}


ScreenJukeboxMenu::~ScreenJukeboxMenu()
{
	LOG->Trace( "ScreenJukeboxMenu::~ScreenJukeboxMenu()" );
}

void ScreenJukeboxMenu::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->Trace( "ScreenJukeboxMenu::Input()" );

	if( IsTransitioning() )
		return;

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenJukeboxMenu::MenuUp( PlayerNumber pn )
{
	m_Selector.Up();
}

void ScreenJukeboxMenu::MenuDown( PlayerNumber pn )
{
	m_Selector.Down();
}

void ScreenJukeboxMenu::MenuLeft( PlayerNumber pn, const InputEventType type )
{
	m_Selector.Left();
}

void ScreenJukeboxMenu::MenuRight( PlayerNumber pn, const InputEventType type )
{
	m_Selector.Right();
}

void ScreenJukeboxMenu::MenuStart( PlayerNumber pn )
{
	if( IsTransitioning() )
		return;

	const Style *style	= m_Selector.GetSelectedStyle();
	CString sGroup			= m_Selector.GetSelectedGroup();
	Difficulty dc			= m_Selector.GetSelectedDifficulty();
	bool bModifiers			= m_Selector.GetSelectedModifiers();

	GAMESTATE->m_pCurStyle = style;
	GAMESTATE->m_sPreferredGroup = (sGroup=="ALL MUSIC") ? GROUP_ALL_MUSIC : sGroup;
	FOREACH_PlayerNumber( p )
		GAMESTATE->m_PreferredDifficulty[p] = dc;
	GAMESTATE->m_bJukeboxUsesModifiers = bModifiers;

	if( !ScreenJukebox::SetSong(false) )
	{
		/* No songs are available for the selected style, group, and difficulty. */

		SCREENMAN->PlayInvalidSound();
		SCREENMAN->SystemMessage( "No songs available with these settings" );
		return;
	}


	SOUND->StopMusic();
	SCREENMAN->PlayStartSound();
	StartTransitioning( SM_GoToNextScreen );
}

void ScreenJukeboxMenu::MenuBack( PlayerNumber pn )
{	
	StartTransitioning( SM_GoToPrevScreen );

	SOUND->StopMusic();
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

#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenEditMenu

 Desc: The main title screen and menu.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenEditMenu.h"
#include "SongManager.h"
#include "ScreenManager.h"
#include "GameConstantsAndTypes.h"
#include "RageUtil.h"
#include "PrefsManager.h"
#include "GameManager.h"
#include "RageLog.h"
#include "GameState.h"
#include "RageMusic.h"


//
// Defines specific to ScreenEditMenu
//
const float EXPLANATION_X		=	CENTER_X;
const float EXPLANATION_Y		=	SCREEN_BOTTOM - 70;
const CString EXPLANATION_TEXT	= 
	"In this mode, you can edit existing notes patterns,\n"
	"create note patterns, or synchronize notes with the music.";

const ScreenMessage SM_GoToPrevScreen		=	ScreenMessage(SM_User+1);
const ScreenMessage SM_GoToNextScreen		=	ScreenMessage(SM_User+2);


ScreenEditMenu::ScreenEditMenu()
{
	LOG->Trace( "ScreenEditMenu::ScreenEditMenu()" );

	Selector.SetXY( 0, 0 );
//	Selector.AllowNewNotes();
	this->AddSubActor( &Selector );

	m_Menu.Load( 
		THEME->GetPathTo("Graphics","edit menu background"), 
		THEME->GetPathTo("Graphics","edit menu top edge"),
		ssprintf("%c %c change line    %c %c change value    START to continue", char(3), char(4), char(1), char(2) ),
		false, 99 
		);
	this->AddSubActor( &m_Menu );


	m_textExplanation.LoadFromFont( THEME->GetPathTo("Fonts","normal") );
	m_textExplanation.SetXY( EXPLANATION_X, EXPLANATION_Y );
	m_textExplanation.SetText( EXPLANATION_TEXT );
	m_textExplanation.SetZoom( 0.7f );
	this->AddSubActor( &m_textExplanation );

	m_Fade.SetOpened();
	this->AddSubActor( &m_Fade);

	MUSIC->LoadAndPlayIfNotAlready( THEME->GetPathTo("Sounds","edit menu music") );

	m_soundSelect.Load( THEME->GetPathTo("Sounds","menu start") );

	m_Menu.TweenOnScreenFromBlack( SM_None );
}


ScreenEditMenu::~ScreenEditMenu()
{
	LOG->Trace( "ScreenEditMenu::~ScreenEditMenu()" );
}

void ScreenEditMenu::DrawPrimitives()
{
	m_Menu.DrawBottomLayer();
	Screen::DrawPrimitives();
	m_Menu.DrawTopLayer();
}

void ScreenEditMenu::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->Trace( "ScreenEditMenu::Input()" );

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenEditMenu::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_GoToPrevScreen:
		SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
		break;
	case SM_GoToNextScreen:
		// set the current style based on the notes type

		// Dro Kulix:
		// A centralized solution for this switching mess...
		// (See GameConstantsAndTypes.h)
		//
		// Chris:
		//	Find the first Style that will play the selected notes type.
		//  Set the current Style, then let ScreenEdit infer the desired
		//  NotesType from that Style.
		NotesType nt = Selector.GetSelectedNotesType();
		Style style = GAMEMAN->GetEditStyleThatPlaysNotesType( nt );
		GAMESTATE->m_CurStyle = style;
		GAMESTATE->m_CurGame = GAMEMAN->GetStyleDefForStyle(style)->m_Game;

		SCREENMAN->SetNewScreen( "ScreenEdit" );
		break;
	}
}
	
void ScreenEditMenu::MenuUp( PlayerNumber p )
{
	Selector.Up();
}

void ScreenEditMenu::MenuDown( PlayerNumber p )
{
	Selector.Down();
}

void ScreenEditMenu::MenuLeft( PlayerNumber p, const InputEventType type )
{
	Selector.Left();
}

void ScreenEditMenu::MenuRight( PlayerNumber p, const InputEventType type )
{
	Selector.Right();
}

void ScreenEditMenu::MenuStart( PlayerNumber p )
{
	MUSIC->Stop();

	GAMESTATE->m_pCurSong = Selector.GetSelectedSong();

	// find the first style that matches this notes type
	GameDef* pGameDef = GAMESTATE->GetCurrentGameDef();
	GAMESTATE->m_CurStyle = GAMEMAN->GetEditStyleThatPlaysNotesType( Selector.GetSelectedNotesType() );
	GAMESTATE->m_pCurNotes[PLAYER_1] = Selector.GetSelectedNotes();

	m_soundSelect.PlayRandom();

	m_Menu.TweenOffScreenToBlack( SM_None, false  );

	m_Fade.CloseWipingRight( SM_GoToNextScreen );
}

void ScreenEditMenu::MenuBack( PlayerNumber p )
{	
	m_Menu.TweenOffScreenToBlack( SM_None, true );

	MUSIC->Stop();

	m_Fade.CloseWipingLeft( SM_GoToPrevScreen );
}

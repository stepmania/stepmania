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
#include "ScreenTitleMenu.h"
#include "ScreenEdit.h"
#include "GameConstantsAndTypes.h"
#include "RageUtil.h"
#include "PrefsManager.h"
#include "GameManager.h"
#include "ScreenPrompt.h"
#include "RageLog.h"
#include "GameState.h"


//
// Defines specific to ScreenEditMenu
//

const float EXPLANATION_X		=	CENTER_X;
const float EXPLANATION_Y		=	SCREEN_BOTTOM - 70;
const CString EXPLANATION_TEXT	= 
	"In this mode, you can edit existing notes patterns,\n"
	"create note patterns, or synchronize notes with the music.";

const ScreenMessage SM_GoToPrevState		=	ScreenMessage(SM_User+1);
const ScreenMessage SM_GoToNextState		=	ScreenMessage(SM_User+2);


ScreenEditMenu::ScreenEditMenu()
{
	LOG->Trace( "ScreenEditMenu::ScreenEditMenu()" );

	Selector.SetXY( 0, 0 );
//	Selector.AllowNewNotes();
	this->AddSubActor( &Selector );

	m_textExplanation.LoadFromFont( THEME->GetPathTo("Fonts","normal") );
	m_textExplanation.SetXY( EXPLANATION_X, EXPLANATION_Y );
	m_textExplanation.SetText( EXPLANATION_TEXT );
	m_textExplanation.SetZoom( 0.7f );
	this->AddSubActor( &m_textExplanation );

	m_Fade.SetOpened();
	this->AddSubActor( &m_Fade);

	MUSIC->Load( THEME->GetPathTo("Sounds","edit menu music") );
	MUSIC->Play( true );
	m_soundSelect.Load( THEME->GetPathTo("Sounds","menu start") );
}


ScreenEditMenu::~ScreenEditMenu()
{
	LOG->Trace( "ScreenEditMenu::~ScreenEditMenu()" );
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
	case SM_GoToPrevState:
		SCREENMAN->SetNewScreen( new ScreenTitleMenu );
		break;
	case SM_GoToNextState:
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

		SCREENMAN->SetNewScreen( new ScreenEdit );
		break;
	}
}
	
void ScreenEditMenu::MenuUp( const PlayerNumber p )
{
	Selector.Up();
}

void ScreenEditMenu::MenuDown( const PlayerNumber p )
{
	Selector.Down();
}

void ScreenEditMenu::MenuLeft( const PlayerNumber p )
{
	Selector.Left();
}

void ScreenEditMenu::MenuRight( const PlayerNumber p )
{
	Selector.Right();
}

void ScreenEditMenu::MenuStart( const PlayerNumber p )
{
	Selector.TweenOffScreenToBlack( SM_None, false );

	MUSIC->Stop();

	GAMESTATE->m_pCurSong = Selector.GetSelectedSong();

	// find the first style that matches this notes type
	GameDef* pGameDef = GAMESTATE->GetCurrentGameDef();
	GAMESTATE->m_CurStyle = GAMEMAN->GetEditStyleThatPlaysNotesType( Selector.GetSelectedNotesType() );
	GAMESTATE->m_pCurNotes[PLAYER_1] = Selector.GetSelectedNotes();

	m_soundSelect.PlayRandom();

	m_Fade.CloseWipingRight( SM_GoToNextState );
}

void ScreenEditMenu::MenuBack( const PlayerNumber p )
{	
	Selector.TweenOffScreenToBlack( SM_None, true );

	MUSIC->Stop();

	m_Fade.CloseWipingLeft( SM_GoToPrevState );
}

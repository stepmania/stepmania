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
#define EXPLANATION_X				THEME->GetMetricF("ScreenEditMenu","ExplanationX")
#define EXPLANATION_Y				THEME->GetMetricF("ScreenEditMenu","ExplanationY")
#define EXPLANATION_TEXT			THEME->GetMetric("ScreenEditMenu","ExplanationText")
#define HELP_TEXT					THEME->GetMetric("ScreenEditMenu","HelpText")

const ScreenMessage SM_GoToPrevScreen		=	ScreenMessage(SM_User+1);
const ScreenMessage SM_GoToNextScreen		=	ScreenMessage(SM_User+2);


ScreenEditMenu::ScreenEditMenu()
{
	LOG->Trace( "ScreenEditMenu::ScreenEditMenu()" );

	GAMESTATE->m_CurStyle = STYLE_NONE;

	Selector.SetXY( 0, 0 );
//	Selector.AllowNewNotes();
	this->AddChild( &Selector );

	m_Menu.Load( 
		THEME->GetPathTo("BGAnimations","edit menu"), 
		THEME->GetPathTo("Graphics","edit menu top edge"),
		HELP_TEXT, false, false, 99 
		);
	this->AddChild( &m_Menu );


	m_textExplanation.LoadFromFont( THEME->GetPathTo("Fonts","normal") );
	m_textExplanation.SetXY( EXPLANATION_X, EXPLANATION_Y );
	m_textExplanation.SetText( EXPLANATION_TEXT );
	m_textExplanation.SetZoom( 0.7f );
	this->AddChild( &m_textExplanation );

	m_Fade.SetOpened();
	this->AddChild( &m_Fade);

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
		Style style = Selector.GetSelectedStyle();
		GAMESTATE->m_CurStyle = style;
		GAMESTATE->m_CurGame = GAMEMAN->GetStyleDefForStyle(style)->m_Game;

		SCREENMAN->SetNewScreen( "ScreenEdit" );
		break;
	}
}
	
void ScreenEditMenu::MenuUp( PlayerNumber pn )
{
	Selector.Up();
}

void ScreenEditMenu::MenuDown( PlayerNumber pn )
{
	Selector.Down();
}

void ScreenEditMenu::MenuLeft( PlayerNumber pn, const InputEventType type )
{
	Selector.Left();
}

void ScreenEditMenu::MenuRight( PlayerNumber pn, const InputEventType type )
{
	Selector.Right();
}

void ScreenEditMenu::MenuStart( PlayerNumber pn )
{
	GAMESTATE->m_pCurSong = Selector.GetSelectedSong();

	if( !GAMESTATE->m_pCurSong->HasMusic() )
	{
		SCREENMAN->SystemMessage( "This song is missing a music file and cannot be edited" );
		SOUND->PlayOnceStreamed( THEME->GetPathTo("Sounds","menu invalid") );
		return;
	}

	MUSIC->Stop();


	// get the style
	GAMESTATE->m_CurStyle = Selector.GetSelectedStyle();
	GAMESTATE->m_pCurNotes[PLAYER_1] = Selector.GetSelectedNotes();

	m_soundSelect.PlayRandom();

	m_Menu.TweenOffScreenToBlack( SM_GoToNextScreen, false  );

	m_Fade.CloseWipingRight( SM_None );
}

void ScreenEditMenu::MenuBack( PlayerNumber pn )
{	
	m_Menu.TweenOffScreenToBlack( SM_None, true );

	MUSIC->Stop();

	m_Fade.CloseWipingLeft( SM_GoToPrevScreen );
}

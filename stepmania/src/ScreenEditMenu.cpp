#include "global.h"
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
#include "RageSoundManager.h"
#include "ThemeManager.h"
#include "Notes.h"


//
// Defines specific to ScreenEditMenu
//
#define EXPLANATION_X				THEME->GetMetricF("ScreenEditMenu","ExplanationX")
#define EXPLANATION_Y				THEME->GetMetricF("ScreenEditMenu","ExplanationY")
#define EXPLANATION_TEXT			THEME->GetMetric("ScreenEditMenu","ExplanationText")
#define HELP_TEXT					THEME->GetMetric("ScreenEditMenu","HelpText")

const ScreenMessage SM_RefreshSelector	=	(ScreenMessage)(SM_User+1);

ScreenEditMenu::ScreenEditMenu() : Screen("ScreenEditMenu")
{
	LOG->Trace( "ScreenEditMenu::ScreenEditMenu()" );

	GAMESTATE->m_CurStyle = STYLE_INVALID;

	m_Selector.SetXY( 0, 0 );
//	m_Selector.AllowNewNotes();
	this->AddChild( &m_Selector );

	m_Menu.Load( "ScreenEditMenu", false );	// disable timer
	this->AddChild( &m_Menu );


	m_textExplanation.LoadFromFont( THEME->GetPathTo("Fonts","Common normal") );
	m_textExplanation.SetXY( EXPLANATION_X, EXPLANATION_Y );
	m_textExplanation.SetText( EXPLANATION_TEXT );
	m_textExplanation.SetZoom( 0.7f );
	this->AddChild( &m_textExplanation );

	SOUNDMAN->PlayMusic( THEME->GetPathTo("Sounds","ScreenEditMenu music") );
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
	case SM_RefreshSelector:
		m_Selector.RefreshNotes();
		break;
	case SM_GoToPrevScreen:
		SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
		break;
	case SM_GoToNextScreen:
		SCREENMAN->SetNewScreen( "ScreenEdit" );
		break;
	}
}
	
void ScreenEditMenu::MenuUp( PlayerNumber pn )
{
	m_Selector.Up();
}

void ScreenEditMenu::MenuDown( PlayerNumber pn )
{
	m_Selector.Down();
}

void ScreenEditMenu::MenuLeft( PlayerNumber pn, const InputEventType type )
{
	m_Selector.Left();
}

void ScreenEditMenu::MenuRight( PlayerNumber pn, const InputEventType type )
{
	m_Selector.Right();
}


// helpers for MenuStart() below
void DeleteCurNotes()
{
	Song* pSong = GAMESTATE->m_pCurSong;
	Notes* pNotesToDelete = GAMESTATE->m_pCurNotes[PLAYER_1];
	pSong->RemoveNotes( pNotesToDelete );
	pSong->Save();
}

void ScreenEditMenu::MenuStart( PlayerNumber pn )
{
	if( m_Menu.IsTransitioning() )
		return;

	Song* pSong					= m_Selector.GetSelectedSong();
	NotesType nt				= m_Selector.GetSelectedNotesType();
	Difficulty dc				= m_Selector.GetSelectedDifficulty();
	Notes* pNotes				= m_Selector.GetSelectedNotes();
//	NotesType soureNT			= m_Selector.GetSelectedSourceNotesType();
//	Difficulty sourceDiff		= m_Selector.GetSelectedSourceDifficulty();
	Notes* pSourceNotes			= m_Selector.GetSelectedSourceNotes();
	EditMenu::Action action		= m_Selector.GetSelectedAction();

	GAMESTATE->m_pCurSong = pSong;
	GAMESTATE->m_CurStyle = GAMEMAN->GetEditorStyleForNotesType( nt );
	GAMESTATE->m_pCurNotes[PLAYER_1] = pNotes;

	//
	// handle error cases
	//

	if( !pSong->HasMusic() )
	{
		SCREENMAN->Prompt( SM_None, "This song is missing a music file\nand cannot be edited" );
		return;
	}

	switch( action )
	{
	case EditMenu::ACTION_EDIT:
		// Prepare prepare for ScreenEdit
		ASSERT( pNotes );
		SOUNDMAN->StopMusic();
		SOUNDMAN->PlayOnce( THEME->GetPathTo("Sounds","Common start") );
		m_Menu.StartTransitioning( SM_GoToNextScreen );
		break;
	case EditMenu::ACTION_DELETE:
		ASSERT( pNotes );
		SCREENMAN->Prompt( SM_RefreshSelector, "These notes will be lost permanently.\n\nContinue with delete?", true, false, DeleteCurNotes );
		m_Selector.RefreshNotes();
		return;
	case EditMenu::ACTION_COPY:
		ASSERT( !pNotes );
		ASSERT( pSourceNotes );
		{
			// Yuck.  Doing the memory allocation doesn't seem right since
			// Song allocates all of the other Notes.
			Notes* pNewNotes = new Notes;
			pNewNotes->CopyFrom( pSourceNotes, nt );
			pNewNotes->SetDifficulty( dc );
			pSong->AddNotes( pNewNotes );
		
			SCREENMAN->SystemMessage( "Notes created from copy." );
			SOUNDMAN->PlayOnce( THEME->GetPathTo("Sounds","ScreenEditMenu create") );
			m_Selector.RefreshNotes();
			pSong->Save();
		}
		return;
	case EditMenu::ACTION_AUTOGEN:
		ASSERT( !pNotes );
		ASSERT( pSourceNotes );
		{
			// Yuck.  Doing the memory allocation doesn't seem right since
			// Song allocates all of the other Notes.
			Notes* pNewNotes = new Notes;
			pNewNotes->AutogenFrom( pSourceNotes, nt );
			pNewNotes->DeAutogen();
			pNewNotes->SetDifficulty( dc );	// override difficulty with the user's choice
			pSong->AddNotes( pNewNotes );
		
			SCREENMAN->SystemMessage( "Notes created from AutoGen." );
			SOUNDMAN->PlayOnce( THEME->GetPathTo("Sounds","ScreenEditMenu create") );
			m_Selector.RefreshNotes();
			pSong->Save();
		}
		return;
	case EditMenu::ACTION_BLANK:
		ASSERT( !pNotes );
		{
			// Yuck.  Doing the memory allocation doesn't seem right since
			// Song allocates all of the other Notes.
			Notes* pNewNotes = new Notes;
			pNewNotes->CreateBlank( nt );
			pNewNotes->SetDifficulty( dc );
			pNewNotes->SetMeter( 1 );
			pSong->AddNotes( pNewNotes );
		
			SCREENMAN->SystemMessage( "Blank Notes created." );
			SOUNDMAN->PlayOnce( THEME->GetPathTo("Sounds","ScreenEditMenu create") );
			m_Selector.RefreshNotes();
			pSong->Save();
		}
		return;
	default:
		ASSERT(0);
	}
}

void ScreenEditMenu::MenuBack( PlayerNumber pn )
{	
	m_Menu.Back( SM_GoToPrevScreen );

	SOUNDMAN->StopMusic();
}

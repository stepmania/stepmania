#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenEditCoursesMenu

 Desc: The main title screen and menu.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenEditCoursesMenu.h"
#include "SongManager.h"
#include "ScreenManager.h"
#include "GameConstantsAndTypes.h"
#include "RageUtil.h"
#include "PrefsManager.h"
#include "GameManager.h"
#include "RageLog.h"
#include "GameState.h"
#include "RageSounds.h"
#include "ThemeManager.h"
#include "Steps.h"


//
// Defines specific to ScreenEditCoursesMenu
//
#define EXPLANATION_X				THEME->GetMetricF("ScreenEditCoursesMenu","ExplanationX")
#define EXPLANATION_Y				THEME->GetMetricF("ScreenEditCoursesMenu","ExplanationY")
#define EXPLANATION_TEXT			THEME->GetMetric("ScreenEditCoursesMenu","ExplanationText")
#define HELP_TEXT					THEME->GetMetric("ScreenEditCoursesMenu","HelpText")

const ScreenMessage SM_RefreshSelector	=	(ScreenMessage)(SM_User+1);

ScreenEditCoursesMenu::ScreenEditCoursesMenu( CString sName ) : ScreenWithMenuElements( sName )
{
	LOG->Trace( "ScreenEditCoursesMenu::ScreenEditCoursesMenu()" );

	/* Enable all players. */
	for( int pn=0; pn<NUM_PLAYERS; pn++ )
		GAMESTATE->m_bSideIsJoined[pn] = true;

	GAMESTATE->m_CurStyle = STYLE_INVALID;

	m_Selector.SetXY( 0, 0 );
//	m_Selector.AllowNewNotes();
	this->AddChild( &m_Selector );

	
	m_textExplanation.LoadFromFont( THEME->GetPathToF("Common normal") );
	m_textExplanation.SetXY( EXPLANATION_X, EXPLANATION_Y );
	m_textExplanation.SetText( EXPLANATION_TEXT );
	m_textExplanation.SetZoom( 0.7f );
	this->AddChild( &m_textExplanation );

	SOUND->PlayMusic( THEME->GetPathToS("ScreenEditCoursesMenu music") );
}


ScreenEditCoursesMenu::~ScreenEditCoursesMenu()
{
	LOG->Trace( "ScreenEditCoursesMenu::~ScreenEditCoursesMenu()" );
}

void ScreenEditCoursesMenu::DrawPrimitives()
{
	Screen::DrawPrimitives();
}

void ScreenEditCoursesMenu::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->Trace( "ScreenEditCoursesMenu::Input()" );

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenEditCoursesMenu::HandleScreenMessage( const ScreenMessage SM )
{
	m_Selector.HandleScreenMessage( SM );

	switch( SM )
	{
	case SM_GoToPrevScreen:
	case SM_GoToNextScreen:
		SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
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
	Back( SM_GoToPrevScreen );

	SOUND->StopMusic();
}

#include "global.h"
#include "ScreenEditMenu.h"
#include "SongManager.h"
#include "ScreenManager.h"
#include "GameConstantsAndTypes.h"
#include "RageUtil.h"
#include "PrefsManager.h"
#include "GameManager.h"
#include "RageLog.h"
#include "GameState.h"
#include "GameSoundManager.h"
#include "ThemeManager.h"
#include "Steps.h"
#include "song.h"


#define EXPLANATION_TEXT			THEME->GetMetric(m_sName,"ExplanationText")
#define HELP_TEXT					THEME->GetMetric(m_sName,"HelpText")

const ScreenMessage SM_RefreshSelector	=	(ScreenMessage)(SM_User+1);

ScreenEditMenu::ScreenEditMenu( CString sName ) : ScreenWithMenuElements( sName )
{
	LOG->Trace( "ScreenEditMenu::ScreenEditMenu()" );

	/* Enable all players. */
	FOREACH_PlayerNumber( pn )
		GAMESTATE->m_bSideIsJoined[pn] = true;

	m_Selector.SetXY( 0, 0 );
//	m_Selector.AllowNewNotes();
	this->AddChild( &m_Selector );


	m_textExplanation.SetName( "Explanation" );
	m_textExplanation.LoadFromFont( THEME->GetPathToF("Common normal") );
	SET_XY_AND_ON_COMMAND( m_textExplanation );
	m_textExplanation.SetText( EXPLANATION_TEXT );
	this->AddChild( &m_textExplanation );

	this->SortByDrawOrder();

	SOUND->PlayMusic( THEME->GetPathToS("ScreenEditMenu music") );
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
void DeleteCurNotes( void* pThrowAway )
{
	Song* pSong = GAMESTATE->m_pCurSong;
	Steps* pStepsToDelete = GAMESTATE->m_pCurSteps[PLAYER_1];
	pSong->RemoveSteps( pStepsToDelete );
	pSong->Save();
}


void ScreenEditMenu::MenuStart( PlayerNumber pn )
{
	if( IsTransitioning() )
		return;

	Song* pSong					= m_Selector.GetSelectedSong();
	StepsType st				= m_Selector.GetSelectedStepsType();
	Difficulty dc				= m_Selector.GetSelectedDifficulty();
	Steps* pSteps				= m_Selector.GetSelectedNotes();
//	StepsType soureNT			= m_Selector.GetSelectedSourceStepsType();
//	Difficulty sourceDiff		= m_Selector.GetSelectedSourceDifficulty();
	Steps* pSourceNotes			= m_Selector.GetSelectedSourceNotes();
	EditMenu::Action action		= m_Selector.GetSelectedAction();

	GAMESTATE->m_pCurSong = pSong;
	GAMESTATE->m_pCurStyle = GAMEMAN->GetEditorStyleForStepsType( st );
	GAMESTATE->m_pCurSteps[PLAYER_1] = pSteps;

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
		ASSERT( pSteps );
		SOUND->StopMusic();
		SCREENMAN->PlayStartSound();
		StartTransitioning( SM_GoToNextScreen );
		break;
	case EditMenu::ACTION_DELETE:
		ASSERT( pSteps );
		SCREENMAN->Prompt( SM_RefreshSelector, "These notes will be lost permanently.\n\nContinue with delete?", true, false, DeleteCurNotes );
		m_Selector.RefreshNotes();
		return;
	case EditMenu::ACTION_COPY:
		ASSERT( !pSteps );
		ASSERT( pSourceNotes );
		{
			// Yuck.  Doing the memory allocation doesn't seem right since
			// Song allocates all of the other Steps.
			Steps* pNewNotes = new Steps;
			pNewNotes->CopyFrom( pSourceNotes, st );
			pNewNotes->SetDifficulty( dc );
			pSong->AddSteps( pNewNotes );
		
			SCREENMAN->SystemMessage( "Steps created from copy." );
			SOUND->PlayOnce( THEME->GetPathToS("ScreenEditMenu create") );
			m_Selector.RefreshNotes();
			pSong->Save();
		}
		return;
	case EditMenu::ACTION_AUTOGEN:
		ASSERT( !pSteps );
		ASSERT( pSourceNotes );
		{
			// Yuck.  Doing the memory allocation doesn't seem right since
			// Song allocates all of the other Steps.
			Steps* pNewNotes = new Steps;
			pNewNotes->AutogenFrom( pSourceNotes, st );
			pNewNotes->DeAutogen();
			pNewNotes->SetDifficulty( dc );	// override difficulty with the user's choice
			pSong->AddSteps( pNewNotes );
		
			SCREENMAN->SystemMessage( "Steps created from AutoGen." );
			SOUND->PlayOnce( THEME->GetPathToS("ScreenEditMenu create") );
			m_Selector.RefreshNotes();
			pSong->Save();
		}
		return;
	case EditMenu::ACTION_BLANK:
		ASSERT( !pSteps );
		{
			// Yuck.  Doing the memory allocation doesn't seem right since
			// Song allocates all of the other Steps.
			Steps* pNewNotes = new Steps;
			pNewNotes->CreateBlank( st );
			pNewNotes->SetDifficulty( dc );
			pNewNotes->SetMeter( 1 );
			pSong->AddSteps( pNewNotes );
		
			SCREENMAN->SystemMessage( "Blank Steps created." );
			SOUND->PlayOnce( THEME->GetPathToS("ScreenEditMenu create") );
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
	Back( SM_GoToPrevScreen );

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

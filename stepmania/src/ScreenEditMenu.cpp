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
#include "CommonMetrics.h"
#include "ScreenTextEntry.h"

#define PREV_SCREEN				THEME->GetMetric(m_sName,"PrevScreen")
#define EXPLANATION_TEXT( row )	THEME->GetMetric(m_sName,"Explanation"+EditMenuRowToString(row))

const ScreenMessage SM_RefreshSelector	=	(ScreenMessage)(SM_User+1);
const ScreenMessage SM_BackFromEditDescription	=	(ScreenMessage)(SM_User+2);

REGISTER_SCREEN_CLASS( ScreenEditMenu );
ScreenEditMenu::ScreenEditMenu( CString sName ) : ScreenWithMenuElements( sName )
{
	LOG->Trace( "ScreenEditMenu::ScreenEditMenu()" );

	/* Enable all players. */
	FOREACH_PlayerNumber( pn )
		GAMESTATE->m_bSideIsJoined[pn] = true;
}

void ScreenEditMenu::Init()
{
	ScreenWithMenuElements::Init();

	m_Selector.SetXY( 0, 0 );
	this->AddChild( &m_Selector );


	m_textExplanation.SetName( "Explanation" );
	m_textExplanation.LoadFromFont( THEME->GetPathF(m_sName,"explanation") );
	SET_XY( m_textExplanation );
	RefreshExplanationText();
	this->AddChild( &m_textExplanation );

	m_textNumStepsLoadedFromProfile.SetName( "NumStepsLoadedFromProfile" );
	m_textNumStepsLoadedFromProfile.LoadFromFont( THEME->GetPathF(m_sName,"NumStepsLoadedFromProfile") );
	SET_XY_AND_ON_COMMAND( m_textNumStepsLoadedFromProfile );
	RefreshNumStepsLoadedFromProfile();
	this->AddChild( &m_textNumStepsLoadedFromProfile );

	this->SortByDrawOrder();

	SOUND->PlayMusic( THEME->GetPathS(m_sName,"music") );
}

// helpers for MenuStart() below
void DeleteCurSteps( void* pThrowAway )
{
	Song* pSong = GAMESTATE->m_pCurSong;
	Steps* pStepsToDelete = GAMESTATE->m_pCurSteps[PLAYER_1];
	pSong->RemoveSteps( pStepsToDelete );
	if( !HOME_EDIT_MODE )
	{
		pSong->Save();
		SCREENMAN->ZeroNextUpdate();
	}
	SCREENMAN->SendMessageToTopScreen( SM_RefreshSelector );
}



void ScreenEditMenu::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_RefreshSelector:
		m_Selector.RefreshAll();
		RefreshNumStepsLoadedFromProfile();
		break;
	case SM_GoToPrevScreen:
		SCREENMAN->SetNewScreen( PREV_SCREEN );
		break;
	case SM_GoToNextScreen:
		SCREENMAN->SetNewScreen( "ScreenEdit" );
		break;
	case SM_Success:
		LOG->Trace( "Delete successful; deleting steps from memory" );
		DeleteCurSteps( NULL );
		break;
	case SM_Failure:
		LOG->Trace( "Delete failed; not deleting steps" );
		break;
	case SM_BackFromEditDescription:
		if( !ScreenTextEntry::s_bCancelledLast )
		{
			SOUND->StopMusic();
			StartTransitioning( SM_GoToNextScreen );
		}
		break;
	}
}
	
void ScreenEditMenu::MenuUp( PlayerNumber pn )
{
	if( m_Selector.CanGoUp() )
	{
		m_Selector.Up();
		RefreshExplanationText();
	}
}

void ScreenEditMenu::MenuDown( PlayerNumber pn )
{
	if( m_Selector.CanGoDown() )
	{
		m_Selector.Down();
		RefreshExplanationText();
	}
}

void ScreenEditMenu::MenuLeft( PlayerNumber pn, const InputEventType type )
{
	if( m_Selector.CanGoLeft() )
	{
		m_Selector.Left();
	}
}

void ScreenEditMenu::MenuRight( PlayerNumber pn, const InputEventType type )
{
	if( m_Selector.CanGoRight() )
	{
		m_Selector.Right();
	}
}

static CString GetCopyDescription( const Steps *pSourceSteps )
{
	CString s;
	if( pSourceSteps->GetDifficulty() == DIFFICULTY_EDIT )
		s = pSourceSteps->GetDescription();
	else
		s = DifficultyToThemedString( pSourceSteps->GetDifficulty() );
	return s;
}

static void SetCurrentStepsDescription( CString s )
{
	GAMESTATE->m_pCurSteps[0]->SetDescription( s );
}
	
static void DeleteCurrentSteps()
{
	GAMESTATE->m_pCurSong->RemoveSteps( GAMESTATE->m_pCurSteps[0] );
	GAMESTATE->m_pCurSteps[0].Set( NULL );
}
	
void ScreenEditMenu::MenuStart( PlayerNumber pn )
{
	if( IsTransitioning() )
		return;

	if( m_Selector.CanGoDown() )
	{
		m_Selector.Down();
		RefreshExplanationText();
		return;
	}

	Song* pSong				= m_Selector.GetSelectedSong();
	StepsType st			= m_Selector.GetSelectedStepsType();
	Difficulty dc			= m_Selector.GetSelectedDifficulty();
	Steps* pSteps			= m_Selector.GetSelectedSteps();
//	StepsType soureNT		= m_Selector.GetSelectedSourceStepsType();
//	Difficulty sourceDiff	= m_Selector.GetSelectedSourceDifficulty();
	Steps* pSourceSteps		= m_Selector.GetSelectedSourceSteps();
	EditMenuAction action	= m_Selector.GetSelectedAction();

	GAMESTATE->m_pCurSong.Set( pSong );
	GAMESTATE->m_pCurStyle = GAMEMAN->GetEditorStyleForStepsType( st );
	GAMESTATE->m_pCurSteps[PLAYER_1].Set( pSteps );

	//
	// handle error cases
	//

	if( !pSong->HasMusic() )
	{
		SCREENMAN->Prompt( SM_None, "This song is missing a music file and cannot be edited" );
		return;
	}

	//
	// Do work
	//
	switch( action )
	{
	case EDIT_MENU_ACTION_EDIT:
		break;
	case EDIT_MENU_ACTION_DELETE:
		ASSERT( pSteps );
		if( HOME_EDIT_MODE )
		{
			SCREENMAN->AddNewScreenToTop( "ScreenEditMenuDeleteSteps", SM_None );
		}
		else
		{
			SCREENMAN->Prompt( SM_RefreshSelector, "These steps will be lost permanently.\n\nContinue with delete?", PROMPT_YES_NO, ANSWER_NO, DeleteCurSteps );
		}
		break;
	case EDIT_MENU_ACTION_COPY:
	case EDIT_MENU_ACTION_AUTOGEN:
	case EDIT_MENU_ACTION_BLANK:
		ASSERT( !pSteps );
		{
			// Yuck.  Doing the memory allocation doesn't seem right since
			// Song allocates all of the other Steps.
			pSteps = new Steps;
			CString sEditName;
			switch( action )
			{
				case EDIT_MENU_ACTION_COPY:
					ASSERT( pSourceSteps );
					pSteps->CopyFrom( pSourceSteps, st );
					sEditName = GetCopyDescription(pSourceSteps);
					break;
				case EDIT_MENU_ACTION_AUTOGEN:
					ASSERT( pSourceSteps );
					pSteps->AutogenFrom( pSourceSteps, st );
					pSteps->DeAutogen();
					sEditName = GetCopyDescription(pSourceSteps);
					break;
				case EDIT_MENU_ACTION_BLANK:
					pSteps->CreateBlank( st );
					pSteps->SetMeter( 1 );
					sEditName = "Blank";
					break;
				default:
					ASSERT(0);
			}
			pSteps->SetDifficulty( dc );	// override difficulty with the user's choice
			pSong->MakeUniqueEditDescription( st, sEditName ); 
			pSteps->SetDescription( sEditName );
			pSong->AddSteps( pSteps );
				
			SCREENMAN->PlayStartSound();

			GAMESTATE->m_pCurSong.Set( pSong );
			GAMESTATE->m_pCurSteps[0].Set( pSteps );
		}
		break;
	default:
		ASSERT(0);
	}

	
	//
	// Go to the next screen.
	//
	switch( action )
	{
	case EDIT_MENU_ACTION_EDIT:
	case EDIT_MENU_ACTION_COPY:
	case EDIT_MENU_ACTION_AUTOGEN:
	case EDIT_MENU_ACTION_BLANK:
		{
			// Prepare prepare for ScreenEdit
			ASSERT( pSteps );
			bool bPromptToNameSteps = (action != EDIT_MENU_ACTION_EDIT && dc == DIFFICULTY_EDIT);
			if( bPromptToNameSteps )
			{
				SCREENMAN->TextEntry( SM_BackFromEditDescription, "Name this new edit.", GAMESTATE->m_pCurSteps[0]->GetDescription(), SetCurrentStepsDescription, DeleteCurrentSteps );
			}
			else
			{
				SOUND->StopMusic();
				SCREENMAN->PlayStartSound();
				StartTransitioning( SM_GoToNextScreen );
			}
		}
		break;
	case EDIT_MENU_ACTION_DELETE:
		break;
	default:
		ASSERT(0);
	}
}

void ScreenEditMenu::MenuBack( PlayerNumber pn )
{	
	Cancel( SM_GoToPrevScreen );

	SOUND->StopMusic();
}

void ScreenEditMenu::RefreshExplanationText()
{
	m_textExplanation.SetText( EXPLANATION_TEXT(m_Selector.GetSelectedRow()) );
	m_textExplanation.StopTweening();
	ON_COMMAND( m_textExplanation );
}

void ScreenEditMenu::RefreshNumStepsLoadedFromProfile()
{
	CString s = ssprintf( "edits used: %d", SONGMAN->GetNumStepsLoadedFromProfile() );
	m_textNumStepsLoadedFromProfile.SetText( s );
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

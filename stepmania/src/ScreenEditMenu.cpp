#include "global.h"
#include "ScreenEditMenu.h"
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
#include "song.h"
#include "CommonMetrics.h"
#include "ScreenTextEntry.h"
#include "ScreenPrompt.h"
#include "LocalizedString.h"

#define EXPLANATION_TEXT( row )	THEME->GetString(m_sName,"Explanation"+EditMenuRowToString(row))
#define EDIT_MENU_TYPE			THEME->GetMetric(m_sName,"EditMenuType")

AutoScreenMessage( SM_RefreshSelector )
AutoScreenMessage( SM_BackFromEditDescription )

REGISTER_SCREEN_CLASS_NEW( ScreenEditMenu );

void ScreenEditMenu::Init()
{
	/* Enable all players. */
	FOREACH_PlayerNumber( pn )
		GAMESTATE->m_bSideIsJoined[pn] = true;

	ScreenWithMenuElements::Init();

	m_Selector.SetName( "EditMenu" );
	m_Selector.Load( EDIT_MENU_TYPE );
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
}


void ScreenEditMenu::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_RefreshSelector )
	{
		m_Selector.RefreshAll();
		RefreshNumStepsLoadedFromProfile();
	}
	else if( SM == SM_Success )
	{
		LOG->Trace( "Delete successful; deleting steps from memory" );

		Song* pSong = GAMESTATE->m_pCurSong;
		Steps* pStepsToDelete = GAMESTATE->m_pCurSteps[PLAYER_1];
		bool bSaveSong = !pStepsToDelete->WasLoadedFromProfile();
		pSong->DeleteSteps( pStepsToDelete );

		/* Only save to the main .SM file if the steps we're deleting were loaded
		 * from it. */
		if( bSaveSong )
		{
			pSong->Save();
			SCREENMAN->ZeroNextUpdate();
		}
		SCREENMAN->SendMessageToTopScreen( SM_RefreshSelector );
	}
	else if( SM == SM_Failure )
	{
		LOG->Trace( "Delete failed; not deleting steps" );
	}
	else if( SM == SM_BackFromEditDescription )
	{
		if( !ScreenTextEntry::s_bCancelledLast )
		{
			SOUND->StopMusic();
			StartTransitioningScreen( SM_GoToNextScreen );
		}
	}

	ScreenWithMenuElements::HandleScreenMessage( SM );
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

void ScreenEditMenu::MenuLeft( const InputEventPlus &input )
{
	if( m_Selector.CanGoLeft() )
	{
		m_Selector.Left();
	}
}

void ScreenEditMenu::MenuRight( const InputEventPlus &input )
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
		s = DifficultyToLocalizedString( pSourceSteps->GetDifficulty() );
	return s;
}

static LocalizedString YOU_MUST_SUPPLY_NAME	( "ScreenEditMenu", "You must supply a name for your new edit." );
static LocalizedString EDIT_NAME_CONFLICTS	( "ScreenEditMenu", "The name you chose conflicts with another edit. Please use a different name." );
static bool ValidateCurrentStepsDescription( const CString &s, CString &sErrorOut )
{
	ASSERT( GAMESTATE->m_pCurSteps[0]->GetDifficulty() == DIFFICULTY_EDIT );

	if( s.empty() )
	{
		sErrorOut = YOU_MUST_SUPPLY_NAME;
		return false;
	}

	if( !GAMESTATE->m_pCurSong->IsEditDescriptionUnique(GAMESTATE->m_pCurSteps[0]->m_StepsType, s, GAMESTATE->m_pCurSteps[0]) )
	{
		sErrorOut = EDIT_NAME_CONFLICTS;
		return false;
	}

	return true;
}
	
static void SetCurrentStepsDescription( const CString &s )
{
	GAMESTATE->m_pCurSteps[0]->SetDescription( s );
}
	
static void DeleteCurrentSteps()
{
	GAMESTATE->m_pCurSong->DeleteSteps( GAMESTATE->m_pCurSteps[0] );
	GAMESTATE->m_pCurSteps[0].Set( NULL );
}
	
static LocalizedString MISSING_MUSIC_FILE		( "ScreenEditMenu", "This song is missing a music file and cannot be edited." );
static LocalizedString STEPS_WILL_BE_LOST		( "ScreenEditMenu", "These steps will be lost permanently." );
static LocalizedString CONTINUE_WITH_DELETE		( "ScreenEditMenu", "Continue with delete?" );
static LocalizedString ENTER_EDIT_DESCRIPTION	( "ScreenEditMenu", "Enter a description for this edit.");
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
	GAMESTATE->m_pCurStyle.Set( GAMEMAN->GetEditorStyleForStepsType(st) );
	GAMESTATE->m_pCurSteps[PLAYER_1].Set( pSteps );

	//
	// handle error cases
	//

	if( !pSong->HasMusic() )
	{
		ScreenPrompt::Prompt( SM_None, MISSING_MUSIC_FILE );
		return;
	}

	//
	// Do work
	//
	switch( action )
	{
	case EditMenuAction_Edit:
	case EditMenuAction_Practice:
		break;
	case EditMenuAction_Delete:
		ASSERT( pSteps );
		ScreenPrompt::Prompt( SM_None, STEPS_WILL_BE_LOST.GetValue() + "\n\n" + CONTINUE_WITH_DELETE.GetValue(), PROMPT_YES_NO, ANSWER_NO );
		break;
	case EditMenuAction_Create:
		ASSERT( !pSteps );
		{
			// Yuck.  Doing the memory allocation doesn't seem right since
			// Song allocates all of the other Steps.
			pSteps = new Steps;
			
			switch( m_Selector.EDIT_MODE )
			{
			default:
				ASSERT(0);
			case EditMode_Full:
				break;
			case EditMode_Home:
				pSteps->SetLoadedFromProfile( ProfileSlot_Machine );
				break;
			case EditMode_Practice:
				ASSERT(0);
				break;
			}

			CString sEditName;
			if( pSourceSteps )
			{
				pSteps->CopyFrom( pSourceSteps, st, pSong->m_fMusicLengthSeconds );
				sEditName = GetCopyDescription(pSourceSteps);
			}
			else
			{
				pSteps->CreateBlank( st );
				pSteps->SetMeter( 1 );
				sEditName = "Blank";
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
	case EditMenuAction_Edit:
	case EditMenuAction_Create:
	case EditMenuAction_Practice:
		{
			// Prepare prepare for ScreenEdit
			ASSERT( pSteps );
			bool bPromptToNameSteps = (action == EditMenuAction_Create && dc == DIFFICULTY_EDIT);
			if( bPromptToNameSteps )
			{
				ScreenTextEntry::TextEntry( 
					SM_BackFromEditDescription, 
					ENTER_EDIT_DESCRIPTION, 
					GAMESTATE->m_pCurSteps[0]->GetDescription(), 
					MAX_EDIT_STEPS_DESCRIPTION_LENGTH,
					ValidateCurrentStepsDescription,
					SetCurrentStepsDescription, 
					DeleteCurrentSteps );
			}
			else
			{
				SOUND->StopMusic();
				SCREENMAN->PlayStartSound();
				StartTransitioningScreen( SM_GoToNextScreen );
			}
		}
		break;
	case EditMenuAction_Delete:
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

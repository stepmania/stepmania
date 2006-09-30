#include "global.h"
#include "ScreenOptionsManageEditSteps.h"
#include "ScreenManager.h"
#include "RageLog.h"
#include "GameState.h"
#include "SongManager.h"
#include "ScreenTextEntry.h"
#include "ScreenPrompt.h"
#include "GameManager.h"
#include "Steps.h"
#include "ScreenMiniMenu.h"
#include "RageUtil.h"
#include "RageFileManager.h"
#include "LocalizedString.h"
#include "OptionRowHandler.h"
#include "SongUtil.h"
#include "song.h"
#include "ProfileManager.h"
#include "Profile.h"
#include "SpecialFiles.h"
#include "NotesWriterSM.h"

AutoScreenMessage( SM_BackFromRename )
AutoScreenMessage( SM_BackFromDelete )
AutoScreenMessage( SM_BackFromContextMenu )

enum StepsEditAction
{
	StepsEditAction_Edit,
	StepsEditAction_Rename,
	StepsEditAction_Delete,
	NUM_StepsEditAction
};
static const char *StepsEditActionNames[] = {
	"Edit",
	"Rename",
	"Delete",
};
XToString( StepsEditAction, NUM_StepsEditAction );
#define FOREACH_StepsEditAction( i ) FOREACH_ENUM2( StepsEditAction, i )

static MenuDef g_TempMenu(
	"ScreenMiniMenuContext"
);



REGISTER_SCREEN_CLASS( ScreenOptionsManageEditSteps );

void ScreenOptionsManageEditSteps::Init()
{
	ScreenOptions::Init();

	CREATE_NEW_SCREEN.Load( m_sName, "CreateNewScreen" );
}

void ScreenOptionsManageEditSteps::BeginScreen()
{
	// Reload so that we're consistent with the disk in case the user has been dinking around with their edits.
	SONGMAN->FreeAllLoadedFromProfile( ProfileSlot_Machine );
	SONGMAN->LoadAllFromProfileDir( PROFILEMAN->GetProfileDir(ProfileSlot_Machine), ProfileSlot_Machine );
	GAMESTATE->m_pCurSong.Set( NULL );
	GAMESTATE->m_pCurSteps[PLAYER_1].Set( NULL );

	vector<OptionRowHandler*> vHands;

	int iIndex = 0;
	
	{
		vHands.push_back( OptionRowHandlerUtil::MakeNull() );
		OptionRowDefinition &def = vHands.back()->m_Def;
		def.m_layoutType = LAYOUT_SHOW_ONE_IN_ROW;
		def.m_bOneChoiceForAllPlayers = true;
		def.m_sName = "Create New Edit Steps";
		def.m_sExplanationName = "Create New Edit Steps";
		def.m_vsChoices.clear();
		def.m_vsChoices.push_back( "" );
		iIndex++;
	}

	SONGMAN->GetStepsLoadedFromProfile( m_vpSteps, ProfileSlot_Machine );

	FOREACH_CONST( Steps*, m_vpSteps, s )
	{
		vHands.push_back( OptionRowHandlerUtil::MakeNull() );
		OptionRowDefinition &def = vHands.back()->m_Def;
		
		Song *pSong = SONGMAN->GetSongFromSteps( *s );

		def.m_sName = pSong->GetTranslitFullTitle() + " - " + (*s)->GetDescription();
		def.m_bAllowThemeTitle = false;	// not themable
		def.m_sExplanationName = "Select Edit Steps";
		def.m_vsChoices.clear();
		RString sType = GAMEMAN->StepsTypeToLocalizedString( (*s)->m_StepsType );
		def.m_vsChoices.push_back( sType );
		def.m_bAllowThemeItems = false;	// already themed
		iIndex++;
	}

	ScreenOptions::InitMenu( vHands );

	ScreenOptions::BeginScreen();
	
	// select the last chosen course
	if( GAMESTATE->m_pCurSteps[PLAYER_1] )
	{
		vector<Steps*>::const_iterator iter = find( m_vpSteps.begin(), m_vpSteps.end(), GAMESTATE->m_pCurSteps[PLAYER_1] );
		if( iter != m_vpSteps.end() )
		{
			int iIndex = iter - m_vpSteps.begin();
			this->MoveRowAbsolute( PLAYER_1, 1 + iIndex );
		}
	}

	AfterChangeRow( PLAYER_1 );
}

static LocalizedString THESE_STEPS_WILL_BE_LOST	("ScreenOptionsManageEditSteps", "These steps will be lost permanently.");
static LocalizedString CONTINUE_WITH_DELETE		("ScreenOptionsManageEditSteps", "Continue with delete?");
static LocalizedString ENTER_NAME_FOR_STEPS		("ScreenOptionsManageEditSteps", "Enter a name for these steps.");
void ScreenOptionsManageEditSteps::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_GoToNextScreen )
	{
		int iCurRow = m_iCurrentRow[GAMESTATE->m_MasterPlayerNumber];

		if( iCurRow == 0 )	// "create new"
		{
			SCREENMAN->SetNewScreen( CREATE_NEW_SCREEN );
			return;	// don't call base
		}
		else if( m_pRows[iCurRow]->GetRowType() == OptionRow::RowType_Exit )
		{
			this->HandleScreenMessage( SM_GoToPrevScreen );
			return;	// don't call base
		}
		else	// a Steps
		{
			Steps *pSteps = GAMESTATE->m_pCurSteps[PLAYER_1];
			ASSERT( pSteps );
			const Style *pStyle = GAMEMAN->GetEditorStyleForStepsType( pSteps->m_StepsType );
			GAMESTATE->SetCurrentStyle( pStyle );
			// do base behavior
		}
	}
	else if( SM == SM_BackFromRename )
	{
		if( !ScreenTextEntry::s_bCancelledLast )
		{
			ASSERT( ScreenTextEntry::s_sLastAnswer != "" );	// validate should have assured this

			Steps *pSteps = GAMESTATE->m_pCurSteps[PLAYER_1];
			Song *pSong = SONGMAN->GetSongFromSteps( pSteps );

			RString sOldDescription = pSteps->GetDescription();
			pSteps->SetDescription( ScreenTextEntry::s_sLastAnswer );

			RString sError;
			NotesWriterSM sm;
			if( !sm.WriteEditFileToMachine(pSong,pSteps,sError) )
			{
				ScreenPrompt::Prompt( SM_None, sError );
				return;
			}

			SCREENMAN->SetNewScreen( this->m_sName ); // reload
		}
	}
	else if( SM == SM_BackFromDelete )
	{
		if( ScreenPrompt::s_LastAnswer == ANSWER_YES )
		{
			LOG->Trace( "Delete successful; deleting Steps from memory" );

			Steps *pSteps = GetStepsWithFocus();
			FILEMAN->Remove( pSteps->GetFilename() );
			SONGMAN->DeleteSteps( pSteps );
			GAMESTATE->m_pCurSteps[PLAYER_1].Set( NULL );
			SCREENMAN->SetNewScreen( this->m_sName ); // reload
		}
	}
	else if( SM == SM_BackFromContextMenu )
	{
		if( !ScreenMiniMenu::s_bCancelled )
		{
			switch( ScreenMiniMenu::s_iLastRowCode )
			{
			case StepsEditAction_Edit:
				{
					Steps *pSteps = GetStepsWithFocus();
					Song *pSong = SONGMAN->GetSongFromSteps( pSteps );
					GAMESTATE->m_pCurSong.Set( pSong );
					GAMESTATE->m_pCurSteps[PLAYER_1].Set( pSteps );

					ScreenOptions::BeginFadingOut();
				}
				break;
			case StepsEditAction_Rename:
				{
					ScreenTextEntry::TextEntry( 
						SM_BackFromRename, 
						ENTER_NAME_FOR_STEPS, 
						GAMESTATE->m_pCurSteps[PLAYER_1]->GetDescription(), 
						MAX_EDIT_STEPS_DESCRIPTION_LENGTH, 
						SongUtil::ValidateCurrentEditStepsDescription );
				}
				break;
			case StepsEditAction_Delete:
				{
					ScreenPrompt::Prompt( SM_BackFromDelete, THESE_STEPS_WILL_BE_LOST.GetValue()+"\n\n"+CONTINUE_WITH_DELETE.GetValue(), PROMPT_YES_NO, ANSWER_NO );
				}
				break;
			}
		}
	}
	else if( SM == SM_LoseFocus )
	{
		this->PlayCommand( "ScreenLoseFocus" );
	}
	else if( SM == SM_GainFocus )
	{
		this->PlayCommand( "ScreenGainFocus" );
	}

	ScreenOptions::HandleScreenMessage( SM );
}
	
void ScreenOptionsManageEditSteps::AfterChangeRow( PlayerNumber pn )
{
	Steps *pSteps = GetStepsWithFocus();
	Song *pSong = pSteps ? SONGMAN->GetSongFromSteps( pSteps ) : NULL;
	
	GAMESTATE->m_pCurSong.Set( pSong );
	GAMESTATE->m_pCurSteps[PLAYER_1].Set( pSteps );

	ScreenOptions::AfterChangeRow( pn );
}

static LocalizedString YOU_HAVE_MAX_STEP_EDITS( "ScreenOptionsManageEditSteps", "You have %d step edits, the maximum number allowed." );
static LocalizedString YOU_MUST_DELETE( "ScreenOptionsManageEditSteps", "You must delete an existing steps edit before creating a new steps edit." );
void ScreenOptionsManageEditSteps::ProcessMenuStart( const InputEventPlus &input )
{
	if( IsTransitioning() )
		return;

	int iCurRow = m_iCurrentRow[GAMESTATE->m_MasterPlayerNumber];

	if( iCurRow == 0 )	// "create new"
	{
		vector<Steps*> v;
		SONGMAN->GetStepsLoadedFromProfile( v, ProfileSlot_Machine );
		if( v.size() >= size_t(MAX_EDIT_STEPS_PER_PROFILE) )
		{
			RString s = ssprintf( YOU_HAVE_MAX_STEP_EDITS.GetValue()+"\n\n"+YOU_MUST_DELETE.GetValue(), MAX_EDIT_STEPS_PER_PROFILE );
			ScreenPrompt::Prompt( SM_None, s );
			return;
		}
		SCREENMAN->PlayStartSound();
		this->BeginFadingOut();
	}
	else if( m_pRows[iCurRow]->GetRowType() == OptionRow::RowType_Exit )
	{
		SCREENMAN->PlayStartSound();
		this->BeginFadingOut();
	}
	else	// a Steps
	{
		g_TempMenu.rows.clear();
		FOREACH_StepsEditAction( i )
		{
			MenuRowDef mrd( i, StepsEditActionToString(i), true, EditMode_Home, true, true, 0, "" );
			g_TempMenu.rows.push_back( mrd );
		}

		int iWidth, iX, iY;
		this->GetWidthXY( PLAYER_1, iCurRow, 0, iWidth, iX, iY );
		ScreenMiniMenu::MiniMenu( &g_TempMenu, SM_BackFromContextMenu, SM_BackFromContextMenu, (float)iX, (float)iY );
	}
}

void ScreenOptionsManageEditSteps::ImportOptions( int iRow, const vector<PlayerNumber> &vpns )
{

}

void ScreenOptionsManageEditSteps::ExportOptions( int iRow, const vector<PlayerNumber> &vpns )
{

}

Steps *ScreenOptionsManageEditSteps::GetStepsWithFocus() const
{
	int iCurRow = m_iCurrentRow[GAMESTATE->m_MasterPlayerNumber];
	if( iCurRow == 0 )
		return NULL;
	else if( m_pRows[iCurRow]->GetRowType() == OptionRow::RowType_Exit )
		return NULL;
	
	// a Steps
	int iStepsIndex = iCurRow - 1;
	return m_vpSteps[iStepsIndex];
}

/*
 * (c) 2002-2005 Chris Danford
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

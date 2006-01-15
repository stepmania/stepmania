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
#define FOREACH_StepsEditAction( i ) FOREACH_ENUM( StepsEditAction, NUM_StepsEditAction, i )

static MenuDef g_TempMenu(
	"ScreenMiniMenuContext"
);


static bool ValidateEditStepsDescription( const CString &sAnswer, CString &sErrorOut )
{
	if( sAnswer.empty() )
	{
		sErrorOut = "Description cannot be blank";
		return false;
	}

	// Steps name must be unique
	vector<Steps*> v;
	SONGMAN->GetStepsLoadedFromProfile( v, ProfileSlot_Machine );
	FOREACH_CONST( Steps*, v, s )
	{
		if( GAMESTATE->m_pCurSteps[PLAYER_1].Get() == *s )
			continue;	// don't comepare name against ourself

		if( (*s)->GetDescription() == sAnswer )
		{
			sErrorOut = "There is already another edit steps with this description.  Each description must be unique.";
			return false;
		}
	}

	return true;
}


REGISTER_SCREEN_CLASS( ScreenOptionsManageEditSteps );

void ScreenOptionsManageEditSteps::Init()
{
	ScreenOptions::Init();
}

void ScreenOptionsManageEditSteps::BeginScreen()
{
	vector<OptionRowDefinition> vDefs;
	vector<OptionRowHandler*> vHands;

	OptionRowDefinition def;
	def.m_layoutType = LAYOUT_SHOW_ONE_IN_ROW;
	def.m_bOneChoiceForAllPlayers = true;

	int iIndex = 0;
	
	{
		def.m_sName = "Create New Edit Steps";
		def.m_sExplanationName = "Create New Edit Steps";
		def.m_vsChoices.clear();
		def.m_vsChoices.push_back( "" );
		vDefs.push_back( def );
		vHands.push_back( NULL );
		iIndex++;
	}

	SONGMAN->GetStepsLoadedFromProfile( m_vpSteps, ProfileSlot_Machine );

	FOREACH_CONST( Steps*, m_vpSteps, s )
	{
		def.m_sName = (*s)->GetDescription();
		def.m_bAllowThemeTitle = false;	// not themable
		def.m_sExplanationName = "Edit Steps";
		def.m_vsChoices.clear();
		CString sType = GAMEMAN->StepsTypeToLocalizedString( (*s)->m_StepsType );
		def.m_vsChoices.push_back( sType );
		def.m_bAllowThemeItems = false;	// already themed
		vDefs.push_back( def );
		vHands.push_back( NULL );
		iIndex++;
	}

	ScreenOptions::InitMenu( vDefs, vHands );

	ScreenOptions::BeginScreen();
	
	// select the last chosen course
	if( GAMESTATE->m_pCurSteps[PLAYER_1] )
	{
		vector<Steps*>::const_iterator iter = find( m_vpSteps.begin(), m_vpSteps.end(), GAMESTATE->m_pCurSteps[PLAYER_1] );
		if( iter != m_vpSteps.end() )
		{
			int iIndex = iter - m_vpSteps.begin();
			this->MoveRowAbsolute( PLAYER_1, 1 + iIndex, false );
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
		int iCurRow = m_iCurrentRow[PLAYER_1];

		if( iCurRow == 0 )	// "create new"
		{
			// do base behavior
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
			GAMESTATE->m_pCurStyle.Set( pStyle );
			SCREENMAN->SetNewScreen( "ScreenEdit" );
			return;	// don't call base
		}
	}
	else if( SM == SM_BackFromRename )
	{
		if( !ScreenTextEntry::s_bCancelledLast )
		{
			ASSERT( ScreenTextEntry::s_sLastAnswer != "" );	// validate should have assured this
		
			GAMESTATE->m_pCurSteps[PLAYER_1]->SetDescription( ScreenTextEntry::s_sLastAnswer );

			SCREENMAN->SetNewScreen( this->m_sName ); // reload
		}
	}
	else if( SM == SM_BackFromDelete )
	{
		if( !ScreenTextEntry::s_bCancelledLast )
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
						ValidateEditStepsDescription );
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
	int iCurRow = m_iCurrentRow[GAMESTATE->m_MasterPlayerNumber];

	if( iCurRow == 0 )	// "create new"
	{
		vector<Steps*> v;
		SONGMAN->GetStepsLoadedFromProfile( v, ProfileSlot_Machine );
		if( v.size() >= size_t(MAX_EDIT_STEPS_PER_PROFILE) )
		{
			CString s = ssprintf( YOU_HAVE_MAX_STEP_EDITS.GetValue()+"\n\n"+YOU_MUST_DELETE.GetValue(), MAX_EDIT_STEPS_PER_PROFILE );
			ScreenPrompt::Prompt( SM_None, s );
			return;
		}
		this->BeginFadingOut();
	}
	else if( m_pRows[iCurRow]->GetRowType() == OptionRow::RowType_Exit )
	{
		this->BeginFadingOut();
	}
	else	// a Steps
	{
		g_TempMenu.rows.clear();
		FOREACH_StepsEditAction( i )
		{
			MenuRowDef mrd( i, StepsEditActionToString(i), true, EditMode_Home, true, 0, "" );
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

#include "global.h"
#include "ScreenOptionsManageEditSteps.h"
#include "ScreenManager.h"
#include "RageLog.h"
#include "GameState.h"
#include "SongManager.h"
//#include "CommonMetrics.h"
#include "ScreenTextEntry.h"
#include "ScreenPrompt.h"
//#include "ScreenMiniMenu.h"
#include "GameManager.h"
#include "Steps.h"
#include "ScreenMiniMenu.h"
#include "RageUtil.h"

AutoScreenMessage( SM_BackFromRename )
AutoScreenMessage( SM_BackFromContextMenu )

enum StepsEditAction
{
	StepsEditAction_Edit,
	StepsEditAction_Rename,
	StepsEditAction_Delete,
	NUM_StepsEditAction
};
static const CString StepsEditActionNames[] = {
	"Edit",
	"Rename",
	"Delete",
};
XToString( StepsEditAction, NUM_StepsEditAction );
#define FOREACH_StepsEditAction( i ) FOREACH_ENUM( StepsEditAction, NUM_StepsEditAction, i )

static MenuDef g_TempMenu(
	"ScreenMiniMenuContext"
);


static bool ValidateEditStepsName( const CString &sAnswer, CString &sErrorOut )
{
	if( sAnswer.empty() )
		return false;

	// Steps name must be unique
	vector<Steps*> v;
	SONGMAN->GetStepsLoadedFromProfile( v, ProfileSlot_Machine );
	FOREACH_CONST( Steps*, v, s )
	{
		if( GAMESTATE->m_pCurSteps[PLAYER_1].Get() == *s )
			continue;	// don't comepare name against ourself

		if( (*s)->GetDescription() == sAnswer )
			return false;
	}

	return true;
}


REGISTER_SCREEN_CLASS( ScreenOptionsManageEditSteps );
ScreenOptionsManageEditSteps::ScreenOptionsManageEditSteps( CString sName ) : ScreenOptions( sName )
{
	LOG->Trace( "ScreenOptionsManageEditSteps::ScreenOptionsManageEditSteps()" );
}

void ScreenOptionsManageEditSteps::Init()
{
	ScreenOptions::Init();

	EDIT_MODE.Load(m_sName,"EditMode");
}

void ScreenOptionsManageEditSteps::BeginScreen()
{
	vector<OptionRowDefinition> vDefs;
	vector<OptionRowHandler*> vHands;

	OptionRowDefinition def;
	def.m_layoutType = LAYOUT_SHOW_ONE_IN_ROW;

	int iIndex = 0;
	
	{
		def.m_sName = "";
		def.m_bAllowThemeTitle = false;
		def.m_vsChoices.clear();
		def.m_vsChoices.push_back( "Create New" );
		vDefs.push_back( def );
		vHands.push_back( NULL );
		iIndex++;
	}

	SONGMAN->GetStepsLoadedFromProfile( m_vpSteps, ProfileSlot_Machine );

	FOREACH_CONST( Steps*, m_vpSteps, s )
	{
		def.m_sName = GAMEMAN->StepsTypeToThemedString( (*s)->m_StepsType );
		def.m_bAllowThemeTitle = false;
		def.m_vsChoices.clear();
		def.m_vsChoices.push_back( (*s)->GetDescription() );
		def.m_bAllowThemeItems = false;
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

void ScreenOptionsManageEditSteps::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_Success )
	{
		LOG->Trace( "Delete successful; deleting Steps from memory" );

		SONGMAN->DeleteSteps( GetStepsWithFocus() );
		SCREENMAN->SetNewScreen( this->m_sName ); // reload
	}
	else if( SM == SM_Failure )
	{
		LOG->Trace( "Delete failed; not deleting Steps" );
	}
	else if( SM == SM_GoToNextScreen )
	{
		int iCurRow = m_iCurrentRow[PLAYER_1];

		if( iCurRow == 0 )	// "create new"
		{
			SCREENMAN->SetNewScreen( "ScreenEditMenuNew" );
			return;	// don't call base
		}
		else if( iCurRow == (int)m_pRows.size()-1 )	// "done"
		{
			this->HandleScreenMessage( SM_GoToPrevScreen );
			return;	// don't call base
		}
		else	// a Steps
		{
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
						"Enter a name for the Steps.", 
						GAMESTATE->m_pCurSteps[PLAYER_1]->GetDescription(), 
						MAX_EDIT_STEPS_DESCRIPTION_LENGTH, 
						ValidateEditStepsName );
				}
				break;
			case StepsEditAction_Delete:
				{
					ScreenPrompt::Prompt( SM_None, "This Steps will be lost permanently.\n\nContinue with delete?", PROMPT_YES_NO, ANSWER_NO );
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

void ScreenOptionsManageEditSteps::ProcessMenuStart( const InputEventPlus &input )
{
	int iCurRow = m_iCurrentRow[GAMESTATE->m_MasterPlayerNumber];

	if( iCurRow == 0 )	// "create new"
	{
		vector<Steps*> v;
		SONGMAN->GetStepsLoadedFromProfile( v, ProfileSlot_Machine );
		if( v.size() >= MAX_EDIT_STEPS_PER_PROFILE )
		{
			CString s = ssprintf( 
				"You have %d Steps edits, the maximum number allowed.\n\n"
				"You must delete an existing Steps edit before creating a new Steps edit.",
				MAX_EDIT_STEPS_PER_PROFILE );
			ScreenPrompt::Prompt( SM_None, s );
			return;
		}
		this->BeginFadingOut();
	}
	else if( iCurRow == (int)m_pRows.size()-1 )	// "done"
	{
		this->BeginFadingOut();
	}
	else	// a Steps
	{
		g_TempMenu.rows.clear();
		FOREACH_StepsEditAction( i )
		{
			MenuRowDef mrd( i, StepsEditActionToString(i), true, EDIT_MODE_HOME, 0, "" );
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
	else if( iCurRow == (int)m_pRows.size()-1 )	// "done"
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

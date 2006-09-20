#include "global.h"
#include "ScreenOptionsManageProfiles.h"
#include "ScreenManager.h"
#include "RageLog.h"
#include "GameState.h"
#include "CommonMetrics.h"
#include "ScreenTextEntry.h"
#include "ScreenPrompt.h"
#include "ScreenMiniMenu.h"
#include "ProfileManager.h"
#include "Profile.h"
#include "OptionRowHandler.h"
#include "LocalizedString.h"

static LocalizedString NEW_PROFILE_DEFAULT_NAME(	"ScreenOptionsManageProfiles", "NewProfileDefaultName" );

#define SHOW_CREATE_NEW (!PROFILEMAN->FixedProfiles())

AutoScreenMessage( SM_BackFromEnterNameForNew )
AutoScreenMessage( SM_BackFromRename )
AutoScreenMessage( SM_BackFromDeleteConfirm )
AutoScreenMessage( SM_BackFromClearConfirm )
AutoScreenMessage( SM_BackFromContextMenu )

enum ProfileAction
{
	ProfileAction_SetDefaultP1,
	ProfileAction_SetDefaultP2,
	ProfileAction_Edit,
	ProfileAction_Rename,
	ProfileAction_Delete,
	ProfileAction_Clear,
	NUM_ProfileAction
};
static const char *ProfileActionNames[] = {
	"SetDefaultP1",
	"SetDefaultP2",
	"Edit",
	"Rename",
	"Delete",
	"Clear",
};
XToString( ProfileAction, NUM_ProfileAction );
XToLocalizedString( ProfileAction );
#define FOREACH_ProfileAction( i ) FOREACH_ENUM( ProfileAction, NUM_ProfileAction, i )

static MenuDef g_TempMenu(
	"ScreenMiniMenuContext"
);

static LocalizedString PROFILE_NAME_BLANK	( "ScreenEditMenu", "Profile name cannot be blank." );
static LocalizedString PROFILE_NAME_CONFLICTS	( "ScreenEditMenu", "The name you chose conflicts with another profile. Please use a different name." );
static bool ValidateLocalProfileName( const RString &sAnswer, RString &sErrorOut )
{
	if( sAnswer == "" )
	{
		sErrorOut = PROFILE_NAME_BLANK;
		return false;
	}

	Profile *pProfile = PROFILEMAN->GetLocalProfile( GAMESTATE->m_sEditLocalProfileID );
	if( pProfile != NULL && sAnswer == pProfile->m_sDisplayName )
		return true; // unchanged

	vector<RString> vsProfileNames;
	PROFILEMAN->GetLocalProfileDisplayNames( vsProfileNames );
	bool bAlreadyAProfileWithThisName = find( vsProfileNames.begin(), vsProfileNames.end(), sAnswer ) != vsProfileNames.end();
	if( bAlreadyAProfileWithThisName )
	{
		sErrorOut = PROFILE_NAME_CONFLICTS;
		return false;
	}

	return true;
}

REGISTER_SCREEN_CLASS( ScreenOptionsManageProfiles );

void ScreenOptionsManageProfiles::Init()
{
	ScreenOptions::Init();

	SetNavigation( NAV_THREE_KEY_MENU );
	SetInputMode( INPUTMODE_SHARE_CURSOR );
}

void ScreenOptionsManageProfiles::BeginScreen()
{
	// FIXME
	// int iIndex = 0;
	vector<OptionRowHandler*> OptionRowHandlers;

	if( SHOW_CREATE_NEW )
	{
		OptionRowHandler *pHand = OptionRowHandlerUtil::Make( ParseCommands(ssprintf("gamecommand;screen,%s;name,dummy",m_sName.c_str())) );
		OptionRowDefinition &def = pHand->m_Def;
		def.m_layoutType = LAYOUT_SHOW_ALL_IN_ROW;
		def.m_bAllowThemeTitle = true;
		def.m_bAllowThemeItems = false;
		def.m_sName = "Create New Profile";
		def.m_sExplanationName = "Create New Profile";
		OptionRowHandlers.push_back( pHand );

		// FIXME
		// gc.Load( iIndex++,  );
	}

	PROFILEMAN->GetLocalProfileIDs( m_vsLocalProfileID );

	FOREACH_CONST( RString, m_vsLocalProfileID, s )
	{
		Profile *pProfile = PROFILEMAN->GetLocalProfile( *s );
		ASSERT( pProfile );

		RString sCommand = ssprintf( "gamecommand;screen,ScreenOptionsEditProfile;profileid,%s;name,dummy", s->c_str() );
		OptionRowHandler *pHand = OptionRowHandlerUtil::Make( ParseCommands(sCommand) );
		OptionRowDefinition &def = pHand->m_Def;
		def.m_layoutType = LAYOUT_SHOW_ALL_IN_ROW;
		def.m_bAllowThemeTitle = false;
		def.m_bAllowThemeItems = false;
		def.m_sName = pProfile->m_sDisplayName;
		def.m_sExplanationName = "Select Profile";

		PlayerNumber pn = PLAYER_INVALID;
		FOREACH_PlayerNumber( p )
			if( *s == ProfileManager::m_sDefaultLocalProfileID[p].Get() )
				pn = p;
		if( pn != PLAYER_INVALID )
			def.m_vsChoices.push_back( PlayerNumberToLocalizedString(pn) );
		OptionRowHandlers.push_back( pHand );

		// FIXME
		// gc.Load( iIndex++,  );
	}

	ScreenOptions::InitMenu( OptionRowHandlers );

	// Save sEditLocalProfileID before calling ScreenOptions::BeginScreen, because it will get clobbered.
	RString sEditLocalProfileID = GAMESTATE->m_sEditLocalProfileID;

	ScreenOptions::BeginScreen();
	
	// select the last chosen profile
	if( !sEditLocalProfileID.empty() )
	{
		vector<RString>::const_iterator iter = find( m_vsLocalProfileID.begin(), m_vsLocalProfileID.end(), sEditLocalProfileID );
		if( iter != m_vsLocalProfileID.end() )
		{
			int iIndex = iter - m_vsLocalProfileID.begin();
			this->MoveRowAbsolute( PLAYER_1, 1 + iIndex );
		}
	}
	else if( !m_vsLocalProfileID.empty() )
	{
		// select the first item below "create new"
		this->MoveRowAbsolute( PLAYER_1, 1 );
	}

	AfterChangeRow( PLAYER_1 );
}

static LocalizedString CONFIRM_DELETE_PROFILE	( "ScreenOptionsManageProfiles", "Are you sure you want to delete the profile '%s'?" );
static LocalizedString CONFIRM_CLEAR_PROFILE	( "ScreenOptionsManageProfiles", "Are you sure you want to clear all data in the profile '%s'?" );
static LocalizedString ENTER_PROFILE_NAME	( "ScreenOptionsManageProfiles", "Enter a name for the profile." );
void ScreenOptionsManageProfiles::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_GoToNextScreen )
	{
		int iCurRow = m_iCurrentRow[GAMESTATE->m_MasterPlayerNumber];
		OptionRow &row = *m_pRows[iCurRow];
		if( row.GetRowType() == OptionRow::RowType_Exit )
		{
			this->HandleScreenMessage( SM_GoToPrevScreen );
			return;	// don't call base
		}
	}
	else if( SM == SM_BackFromEnterNameForNew )
	{
		if( !ScreenTextEntry::s_bCancelledLast )
		{
			ASSERT( ScreenTextEntry::s_sLastAnswer != "" );	// validate should have assured this
		
			RString sNewName = ScreenTextEntry::s_sLastAnswer;
			ASSERT( GAMESTATE->m_sEditLocalProfileID.Get().empty() );

			int iNumProfiles = PROFILEMAN->GetNumLocalProfiles();

			// create
			RString sProfileID;
			PROFILEMAN->CreateLocalProfile( ScreenTextEntry::s_sLastAnswer, sProfileID );	// TODO: Check return value
			GAMESTATE->m_sEditLocalProfileID.Set( sProfileID );

			if( iNumProfiles < NUM_PLAYERS )
			{
				int iFirstUnused = -1;
				FOREACH_CONST( Preference<RString>*, PROFILEMAN->m_sDefaultLocalProfileID.m_v, i )
				{
					RString sLocalProfileID = (*i)->Get();
					if( sLocalProfileID.empty() )
					{
						iFirstUnused = i - PROFILEMAN->m_sDefaultLocalProfileID.m_v.begin();
						break;
					}
				}
				if( iFirstUnused != -1 )
				{
					PROFILEMAN->m_sDefaultLocalProfileID.m_v[iFirstUnused]->Set( sProfileID );
				}
			}

			SCREENMAN->SetNewScreen( this->m_sName ); // reload
		}
	}
	else if( SM == SM_BackFromRename )
	{
		if( !ScreenTextEntry::s_bCancelledLast )
		{
			ASSERT( ScreenTextEntry::s_sLastAnswer != "" );	// validate should have assured this
		
			RString sNewName = ScreenTextEntry::s_sLastAnswer;
			PROFILEMAN->RenameLocalProfile( GAMESTATE->m_sEditLocalProfileID, sNewName );

			SCREENMAN->SetNewScreen( this->m_sName ); // reload
		}
	}
	else if( SM == SM_BackFromDeleteConfirm )
	{
		if( ScreenPrompt::s_LastAnswer == ANSWER_YES )
		{
			// Select the profile nearest to the one that was just deleted.
			int iIndex = -1;
			vector<RString>::const_iterator iter = find( m_vsLocalProfileID.begin(), m_vsLocalProfileID.end(), GAMESTATE->m_sEditLocalProfileID.Get() );
			if( iter != m_vsLocalProfileID.end() )
				iIndex = iter - m_vsLocalProfileID.begin();
			CLAMP( iIndex, 0, m_vsLocalProfileID.size()-1 );
			GAMESTATE->m_sEditLocalProfileID.Set( m_vsLocalProfileID[iIndex] );

			PROFILEMAN->DeleteLocalProfile( GetLocalProfileIDWithFocus() );
			
			SCREENMAN->SetNewScreen( this->m_sName ); // reload
		}
	}
	else if( SM == SM_BackFromClearConfirm )
	{
		if( ScreenPrompt::s_LastAnswer == ANSWER_YES )
		{
			GAMESTATE->GetEditLocalProfile()->InitAll();

			SCREENMAN->SetNewScreen( this->m_sName ); // reload
		}
	}
	else if( SM == SM_BackFromContextMenu )
	{
		if( !ScreenMiniMenu::s_bCancelled )
		{
			Profile *pProfile = PROFILEMAN->GetLocalProfile( GAMESTATE->m_sEditLocalProfileID );
			ASSERT( pProfile );

			switch( ScreenMiniMenu::s_iLastRowCode )
			{
			default:
				ASSERT(0);
			case ProfileAction_SetDefaultP1:
			case ProfileAction_SetDefaultP2:
				{
					FOREACH_PlayerNumber( p )
						if( ProfileManager::m_sDefaultLocalProfileID[p].Get() == GetLocalProfileIDWithFocus() )
							ProfileManager::m_sDefaultLocalProfileID[p].Set("");

					PlayerNumber pn = (PlayerNumber)(ScreenMiniMenu::s_iLastRowCode - ProfileAction_SetDefaultP1);
					ProfileManager::m_sDefaultLocalProfileID[pn].Set( GetLocalProfileIDWithFocus() );
		
					SCREENMAN->SetNewScreen( this->m_sName ); // reload
				}
				break;
			case ProfileAction_Edit:
				{
					GAMESTATE->m_sEditLocalProfileID.Set( GetLocalProfileIDWithFocus() );

					ScreenOptions::BeginFadingOut();
				}
				break;
			case ProfileAction_Rename:
				{
					ScreenTextEntry::TextEntry( 
						SM_BackFromRename, 
						ENTER_PROFILE_NAME, 
						pProfile->m_sDisplayName, 
						PROFILE_MAX_DISPLAY_NAME_LENGTH, 
						ValidateLocalProfileName );
				}
				break;
			case ProfileAction_Delete:
				{
					RString sTitle = pProfile->m_sDisplayName;
					RString sMessage = ssprintf( CONFIRM_DELETE_PROFILE.GetValue(), sTitle.c_str() );
					ScreenPrompt::Prompt( SM_BackFromDeleteConfirm, sMessage, PROMPT_YES_NO );
				}
				break;
			case ProfileAction_Clear:
				{
					RString sTitle = pProfile->m_sDisplayName;
					RString sMessage = ssprintf( CONFIRM_CLEAR_PROFILE.GetValue(), sTitle.c_str() );
					ScreenPrompt::Prompt( SM_BackFromClearConfirm, sMessage, PROMPT_YES_NO );
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
	
void ScreenOptionsManageProfiles::AfterChangeRow( PlayerNumber pn )
{
	GAMESTATE->m_sEditLocalProfileID.Set( GetLocalProfileIDWithFocus() );

	ScreenOptions::AfterChangeRow( pn );
}

void ScreenOptionsManageProfiles::ProcessMenuStart( const InputEventPlus &input )
{
	if( IsTransitioning() )
		return;

	int iCurRow = m_iCurrentRow[GAMESTATE->m_MasterPlayerNumber];
	OptionRow &row = *m_pRows[iCurRow];
	
	if( SHOW_CREATE_NEW && iCurRow == 0 )	// "create new"
	{
		vector<RString> vsUsedNames;
		PROFILEMAN->GetLocalProfileDisplayNames( vsUsedNames );

		RString sPotentialName;
		for( int i=1; i<1000; i++ )
		{
			sPotentialName = ssprintf( "%s%04d", NEW_PROFILE_DEFAULT_NAME.GetValue().c_str(), i );
			bool bNameIsUsed = find( vsUsedNames.begin(), vsUsedNames.end(), sPotentialName ) != vsUsedNames.end();
			if( !bNameIsUsed )
				break;
		}
		ScreenTextEntry::TextEntry( 
			SM_BackFromEnterNameForNew, 
			ENTER_PROFILE_NAME, 
			sPotentialName, 
			PROFILE_MAX_DISPLAY_NAME_LENGTH, 
			ValidateLocalProfileName );
	}
	else if( row.GetRowType() == OptionRow::RowType_Exit )
	{
		SCREENMAN->PlayStartSound();
		this->BeginFadingOut();
	}
	else	// a profile
	{
		g_TempMenu.rows.clear();

#define ADD_ACTION( i )	\
		g_TempMenu.rows.push_back( MenuRowDef( i, ProfileActionToLocalizedString(i), true, EditMode_Home, false, false, 0, "" ) );

		ADD_ACTION( ProfileAction_SetDefaultP1 );
		ADD_ACTION( ProfileAction_SetDefaultP2 );
		if( PROFILEMAN->FixedProfiles() )
		{
			ADD_ACTION( ProfileAction_Rename );
			ADD_ACTION( ProfileAction_Clear );
		}
		else
		{
			//ADD_ACTION( ProfileAction_Edit );
			ADD_ACTION( ProfileAction_Rename );
			ADD_ACTION( ProfileAction_Delete );
		}

		int iWidth, iX, iY;
		this->GetWidthXY( PLAYER_1, iCurRow, 0, iWidth, iX, iY );

		ScreenMiniMenu::MiniMenu( &g_TempMenu, SM_BackFromContextMenu, SM_BackFromContextMenu, (float)iX, (float)iY );
	}
}

void ScreenOptionsManageProfiles::ImportOptions( int iRow, const vector<PlayerNumber> &vpns )
{

}

void ScreenOptionsManageProfiles::ExportOptions( int iRow, const vector<PlayerNumber> &vpns )
{

}

int ScreenOptionsManageProfiles::GetLocalProfileIndexWithFocus() const
{
	int iCurRow = m_iCurrentRow[GAMESTATE->m_MasterPlayerNumber];
	OptionRow &row = *m_pRows[iCurRow];

	if( SHOW_CREATE_NEW && iCurRow == 0 )	// "create new"
		return -1;
	else if( row.GetRowType() == OptionRow::RowType_Exit )
		return -1;
	
	// a profile
	int iIndex = iCurRow + (SHOW_CREATE_NEW ? -1 : 0);
	return iIndex;
}

RString ScreenOptionsManageProfiles::GetLocalProfileIDWithFocus() const
{
	int iIndex = GetLocalProfileIndexWithFocus();
	if( iIndex == -1 )
		return RString();
	return m_vsLocalProfileID[iIndex];
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

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
#include "PrefsManager.h"

static ThemeMetric<CString> NEW_PROFILE_DEFAULT_NAME(	"ScreenOptionsManageProfiles", "NewProfileDefaultName" );

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
static const CString ProfileActionNames[] = {
	"SetDefaultP1",
	"SetDefaultP2",
	"Edit",
	"Rename",
	"Delete",
	"Clear",
};
XToString( ProfileAction, NUM_ProfileAction );
XToThemedString( ProfileAction, NUM_ProfileAction );
#define FOREACH_ProfileAction( i ) FOREACH_ENUM( ProfileAction, NUM_ProfileAction, i )

static MenuDef g_TempMenu(
	"ScreenMiniMenuContext"
);

static bool ValidateLocalProfileName( const CString &sAnswer, CString &sErrorOut )
{
	if( sAnswer == "" )
	{
		sErrorOut = "Profile name cannot be blank.";
		return false;
	}

	Profile *pProfile = PROFILEMAN->GetLocalProfile( GAMESTATE->m_sEditLocalProfileID );
	if( pProfile != NULL && sAnswer == pProfile->m_sDisplayName )
		return true; // unchanged

	vector<CString> vsProfileNames;
	PROFILEMAN->GetLocalProfileDisplayNames( vsProfileNames );
	bool bAlreadyAProfileWithThisName = find( vsProfileNames.begin(), vsProfileNames.end(), sAnswer ) != vsProfileNames.end();
	if( bAlreadyAProfileWithThisName )
	{
		sErrorOut = "There is already another profile with this name.  Please choose a different name.";
		return false;
	}

	return true;
}


class OptionRowHandlerSimple : public OptionRowHandler
{
public:
	GameCommand m_gc;

	virtual void Load( OptionRowDefinition &defOut, CString sParam )
	{

	}
	virtual void ImportOption( const OptionRowDefinition &row, const vector<PlayerNumber> &vpns, vector<bool> vbSelectedOut[NUM_PLAYERS] ) const
	{

	}
	virtual int ExportOption( const OptionRowDefinition &def, const vector<PlayerNumber> &vpns, const vector<bool> vbSelected[NUM_PLAYERS] ) const
	{
		if( vbSelected[PLAYER_1][0] )
			m_gc.ApplyToAllPlayers();
		return 0;
	}
	virtual void GetIconTextAndGameCommand( const OptionRowDefinition &def, int iFirstSelection, CString &sIconTextOut, GameCommand &gcOut ) const
	{
		sIconTextOut = "";
		gcOut = m_gc;
	}
};


REGISTER_SCREEN_CLASS( ScreenOptionsManageProfiles );
ScreenOptionsManageProfiles::ScreenOptionsManageProfiles( CString sName ) : ScreenOptions( sName )
{
	LOG->Trace( "ScreenOptionsManageProfiles::ScreenOptionsManageProfiles()" );
}

ScreenOptionsManageProfiles::~ScreenOptionsManageProfiles()
{
	FOREACH( OptionRow*, m_pRows, r )
		(*r)->DetachHandler();
	FOREACH( OptionRowHandler*, m_OptionRowHandlers, h )
		SAFE_DELETE( *h );
	m_OptionRowHandlers.clear();
	SAFE_DELETE( m_pContextMenu );
}

void ScreenOptionsManageProfiles::Init()
{
	ScreenOptions::Init();

	SetNavigation( NAV_THREE_KEY_MENU );
	SetInputMode( INPUTMODE_SHARE_CURSOR );

	m_pContextMenu = new ScreenMiniMenu( g_TempMenu.sClassName );
	m_pContextMenu->Init();
	m_pContextMenu->LoadMenu( &g_TempMenu );
}

void ScreenOptionsManageProfiles::BeginScreen()
{
	vector<OptionRowDefinition> vDefs;
	for( unsigned i = 0; i < m_OptionRowHandlers.size(); ++i )
		delete m_OptionRowHandlers[i];
	m_OptionRowHandlers.clear();

	OptionRowDefinition def;
	def.m_layoutType = LAYOUT_SHOW_ALL_IN_ROW;
	def.m_bAllowThemeTitles = false;
	def.m_bAllowThemeItems = false;

	int iIndex = 0;

	if( SHOW_CREATE_NEW )
	{
		def.m_vsChoices.clear();
		def.m_vsChoices.push_back( "Create New" );
		vDefs.push_back( def );
		
		OptionRowHandlerSimple *pHand = new OptionRowHandlerSimple;
		m_OptionRowHandlers.push_back( pHand );
		GameCommand &gc = pHand->m_gc;
		CString sCommand = "screen,ScreenOptionsEditProfile";
		gc.Load( iIndex++, ParseCommands(sCommand) );
	}

	PROFILEMAN->GetLocalProfileIDs( m_vsLocalProfileID );

	FOREACH_CONST( CString, m_vsLocalProfileID, s )
	{
		Profile *pProfile = PROFILEMAN->GetLocalProfile( *s );
		ASSERT( pProfile );

		def.m_sName = ssprintf( "%d", iIndex );
		def.m_vsChoices.clear();
		def.m_vsChoices.push_back( pProfile->m_sDisplayName );
		vDefs.push_back( def );

		OptionRowHandlerSimple *pHand = new OptionRowHandlerSimple;
		m_OptionRowHandlers.push_back( pHand );
		GameCommand &gc = pHand->m_gc;
		CString sCommand = "screen,ScreenOptionsEditProfile;profileid,"+*s;
		gc.Load( iIndex++, ParseCommands(sCommand) );
	}

	ScreenOptions::InitMenu( vDefs, m_OptionRowHandlers );

	ScreenOptions::BeginScreen();
	
	// select the last chosen profile
	if( !GAMESTATE->m_sEditLocalProfileID.Get().empty() )
	{
		vector<CString>::const_iterator iter = find( m_vsLocalProfileID.begin(), m_vsLocalProfileID.end(), GAMESTATE->m_sEditLocalProfileID.Get() );
		if( iter != m_vsLocalProfileID.end() )
		{
			int iIndex = iter - m_vsLocalProfileID.begin();
			this->MoveRowAbsolute( PLAYER_1, 1 + iIndex, false );
		}
	}
	else if( !m_vsLocalProfileID.empty() )
	{
		// select the first item below "create new"
		this->MoveRowAbsolute( PLAYER_1, 1, false );
	}

	AfterChangeRow( PLAYER_1 );
}

void ScreenOptionsManageProfiles::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_GoToNextScreen )
	{
		int iCurRow = m_iCurrentRow[GAMESTATE->m_MasterPlayerNumber];
		OptionRow &row = *m_pRows[iCurRow];
		if( row.GetRowType() == OptionRow::ROW_EXIT )
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
		
			CString sNewName = ScreenTextEntry::s_sLastAnswer;
			ASSERT( GAMESTATE->m_sEditLocalProfileID.Get().empty() );

			// create
			CString sProfileID;
			PROFILEMAN->CreateLocalProfile( ScreenTextEntry::s_sLastAnswer, sProfileID );
			GAMESTATE->m_sEditLocalProfileID.Set( sProfileID );

			this->HandleScreenMessage( SM_GoToNextScreen );
		}
	}
	else if( SM == SM_BackFromRename )
	{
		if( !ScreenTextEntry::s_bCancelledLast )
		{
			ASSERT( ScreenTextEntry::s_sLastAnswer != "" );	// validate should have assured this
		
			CString sNewName = ScreenTextEntry::s_sLastAnswer;
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
			vector<CString>::const_iterator iter = find( m_vsLocalProfileID.begin(), m_vsLocalProfileID.end(), GAMESTATE->m_sEditLocalProfileID.Get() );
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
				{
					PREFSMAN->m_sDefaultLocalProfileIDP1.Set( GetLocalProfileIDWithFocus() );
					if( PREFSMAN->m_sDefaultLocalProfileIDP1.Get() == PREFSMAN->m_sDefaultLocalProfileIDP2.Get() )
					{
						int iIndex = GetLocalProfileIndexWithFocus();
						iIndex++;
						wrap( iIndex, m_vsLocalProfileID.size() );
						PREFSMAN->m_sDefaultLocalProfileIDP2.Set( m_vsLocalProfileID[iIndex] );
					}
				}
				break;
			case ProfileAction_SetDefaultP2:
				{
					PREFSMAN->m_sDefaultLocalProfileIDP2.Set( GetLocalProfileIDWithFocus() );
					if( PREFSMAN->m_sDefaultLocalProfileIDP1.Get() == PREFSMAN->m_sDefaultLocalProfileIDP2.Get() )
					{
						int iIndex = GetLocalProfileIndexWithFocus();
						iIndex++;
						wrap( iIndex, m_vsLocalProfileID.size() );
						PREFSMAN->m_sDefaultLocalProfileIDP1.Set( m_vsLocalProfileID[iIndex] );
					}
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
						"Enter a name for the profile.", 
						pProfile->m_sDisplayName, 
						PROFILE_MAX_DISPLAY_NAME_LENGTH, 
						ValidateLocalProfileName );
				}
				break;
			case ProfileAction_Delete:
				{
					CString sTitle = pProfile->m_sDisplayName;
					CString sMessage = ssprintf( "Are you sure you want to delete the profile '%s'?", sTitle.c_str() );
					ScreenPrompt::Prompt( SM_BackFromDeleteConfirm, sMessage, PROMPT_YES_NO );
				}
				break;
			case ProfileAction_Clear:
				{
					CString sTitle = pProfile->m_sDisplayName;
					CString sMessage = ssprintf( "Are you sure you want to clear all data in the profile '%s'?", sTitle.c_str() );
					ScreenPrompt::Prompt( SM_BackFromClearConfirm, sMessage, PROMPT_YES_NO );
				}
				break;
			}
		}
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
	int iCurRow = m_iCurrentRow[GAMESTATE->m_MasterPlayerNumber];
	OptionRow &row = *m_pRows[iCurRow];
	
	if( SHOW_CREATE_NEW && iCurRow == 0 )	// "create new"
	{
		vector<CString> vsUsedNames;
		PROFILEMAN->GetLocalProfileDisplayNames( vsUsedNames );

		CString sPotentialName;
		for( int i=1; i<1000; i++ )
		{
			sPotentialName = ssprintf( "%s%04d", NEW_PROFILE_DEFAULT_NAME.GetValue().c_str(), i );
			bool bNameIsUsed = find( vsUsedNames.begin(), vsUsedNames.end(), sPotentialName ) != vsUsedNames.end();
			if( !bNameIsUsed )
				break;
		}
		ScreenTextEntry::TextEntry( 
			SM_BackFromEnterNameForNew, 
			"Enter a name for the new profile.", 
			sPotentialName, 
			PROFILE_MAX_DISPLAY_NAME_LENGTH, 
			ValidateLocalProfileName );
	}
	else if( row.GetRowType() == OptionRow::ROW_EXIT )
	{
		this->BeginFadingOut();
	}
	else	// a profile
	{
		g_TempMenu.rows.clear();

#define ADD_ACTION( i )	\
		g_TempMenu.rows.push_back( MenuRowDef( i, ProfileActionToThemedString(i), true, EDIT_MODE_HOME, 0, "" ) );

		ADD_ACTION( ProfileAction_SetDefaultP1 );
		ADD_ACTION( ProfileAction_SetDefaultP2 );
		if( PROFILEMAN->FixedProfiles() )
		{
			ADD_ACTION( ProfileAction_Rename );
			ADD_ACTION( ProfileAction_Clear );
		}
		else
		{
			ADD_ACTION( ProfileAction_Edit );
			ADD_ACTION( ProfileAction_Rename );
			ADD_ACTION( ProfileAction_Delete );
		}

		int iWidth, iX, iY;
		this->GetWidthXY( PLAYER_1, iCurRow, 0, iWidth, iX, iY );

		m_pContextMenu->LoadMenu( &g_TempMenu );
		m_pContextMenu->SetOKMessage( SM_BackFromContextMenu );
		m_pContextMenu->SetCancelMessage( SM_BackFromContextMenu );
		m_pContextMenu->SetXY( (float)iX, (float)iY );
		SCREENMAN->PushScreen( m_pContextMenu );
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
	else if( row.GetRowType() == OptionRow::ROW_EXIT )
		return -1;
	
	// a profile
	int iIndex = iCurRow + (SHOW_CREATE_NEW ? -1 : 0);
	return iIndex;
}

CString ScreenOptionsManageProfiles::GetLocalProfileIDWithFocus() const
{
	int iIndex = GetLocalProfileIndexWithFocus();
	if( iIndex == -1 )
		return NULL;
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

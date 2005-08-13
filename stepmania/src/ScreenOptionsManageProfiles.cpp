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

static ThemeMetric<CString> NEW_PROFILE_DEFAULT_NAME( "ScreenOptionsManageProfiles", "NewProfileDefaultName" );

AutoScreenMessage( SM_BackFromEnterNameForNew )
AutoScreenMessage( SM_BackFromRename )
AutoScreenMessage( SM_BackFromDeleteConfirm )
AutoScreenMessage( SM_BackFromContextMenu )

enum ProfileAction
{
	ProfileAction_Edit,
	ProfileAction_Rename,
	ProfileAction_Delete,
	NUM_ProfileAction
};
static const CString ProfileActionNames[] = {
	"Edit",
	"Rename",
	"Delete",
};
XToString( ProfileAction, NUM_ProfileAction );
#define FOREACH_ProfileAction( i ) FOREACH_ENUM( ProfileAction, NUM_ProfileAction, i )

static MenuDef g_TempMenu(
	"ScreenMiniMenuContext"
);

static bool ValidateLocalProfileName( const CString &sAnswer, CString &sErrorOut )
{
	CString sCurrentProfileOldName = PROFILEMAN->GetLocalProfile( GAMESTATE->m_sEditLocalProfileID ).m_sDisplayName;
	vector<CString> vsProfileNames;
	PROFILEMAN->GetLocalProfileDisplayNames( vsProfileNames );
	bool bAlreadyAProfileWithThisName = find( vsProfileNames.begin(), vsProfileNames.end(), sAnswer ) != vsProfileNames.end();
	
	if( sAnswer == "" )
	{
		sErrorOut = "Profile name cannot be blank.";
		return false;
	}
	else if( sAnswer == sCurrentProfileOldName )
	{
		return true;
	}
	else if( bAlreadyAProfileWithThisName )
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
}

void ScreenOptionsManageProfiles::Init()
{
	ScreenOptions::Init();

	SetInputMode( INPUTMODE_SHARE_CURSOR );
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
		Profile &profile = PROFILEMAN->GetLocalProfile( *s );

		def.m_sName = ssprintf( "%d", iIndex );
		def.m_vsChoices.clear();
		def.m_vsChoices.push_back( profile.m_sDisplayName );
		vDefs.push_back( def );

		OptionRowHandlerSimple *pHand = new OptionRowHandlerSimple;
		m_OptionRowHandlers.push_back( pHand );
		GameCommand &gc = pHand->m_gc;
		CString sCommand = "screen,ScreenOptionsEditProfile";
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
		if( iCurRow == (int)m_pRows.size() - 1 )
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
		
			Profile &profile = PROFILEMAN->GetLocalProfile( GAMESTATE->m_sEditLocalProfileID );
			profile.m_sDisplayName = ScreenTextEntry::s_sLastAnswer;

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
	else if( SM == SM_BackFromContextMenu )
	{
		if( !ScreenMiniMenu::s_bCancelled )
		{
			switch( ScreenMiniMenu::s_iLastRowCode )
			{
			case ProfileAction_Edit:
				{
					GAMESTATE->m_sEditLocalProfileID.Set( GetLocalProfileIDWithFocus() );

					ScreenOptions::BeginFadingOut();
				}
				break;
			case ProfileAction_Rename:
				{
					Profile &profile = PROFILEMAN->GetLocalProfile( GAMESTATE->m_sEditLocalProfileID );
					ScreenTextEntry::TextEntry( 
						SM_BackFromRename, 
						"Enter a name for the profile.", 
						profile.m_sDisplayName, 
						PROFILE_MAX_DISPLAY_NAME_LENGTH, 
						ValidateLocalProfileName );
				}
				break;
			case ProfileAction_Delete:
				{
					Profile &profile = PROFILEMAN->GetLocalProfile( GetLocalProfileIDWithFocus() );
					CString sTitle = profile.m_sDisplayName;
					CString sMessage = ssprintf( "Are you sure you want to delete the course '%s'?", sTitle.c_str() );
					ScreenPrompt::Prompt( SM_BackFromDeleteConfirm, sMessage, PROMPT_YES_NO );
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

void ScreenOptionsManageProfiles::ProcessMenuStart( PlayerNumber pn, const InputEventType type )
{
	int iCurRow = m_iCurrentRow[GAMESTATE->m_MasterPlayerNumber];

	if( iCurRow == 0 )	// "create new"
	{
		if( PROFILEMAN->GetNumLocalProfiles() >= MAX_NUM_LOCAL_PROFILES )
		{
			CString s = ssprintf( 
				"You have %d profiles, the maximum number allowed.\n\n"
				"You must delete an existing profile before creating a new profile.",
				MAX_NUM_LOCAL_PROFILES );
			ScreenPrompt::Prompt( SM_None, s );
			return;
		}

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
			"Enter a name for a new course.", 
			sPotentialName, 
			PROFILE_MAX_DISPLAY_NAME_LENGTH, 
			ValidateLocalProfileName );
	}
	else if( iCurRow == (int)m_pRows.size()-1 )	// "done"
	{
		this->BeginFadingOut();
	}
	else	// a course
	{
		g_TempMenu.rows.clear();
		FOREACH_ProfileAction( i )
		{
			MenuRowDef mrd( i, ProfileActionToString(i), true, EDIT_MODE_HOME, 0, "" );
			g_TempMenu.rows.push_back( mrd );
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

CString ScreenOptionsManageProfiles::GetLocalProfileIDWithFocus() const
{
	int iCurRow = m_iCurrentRow[GAMESTATE->m_MasterPlayerNumber];
	if( iCurRow == 0 )
		return NULL;
	else if( iCurRow == (int)m_pRows.size()-1 )	// "done"
		return NULL;
	
	// a profile
	int iIndex = iCurRow - 1;
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

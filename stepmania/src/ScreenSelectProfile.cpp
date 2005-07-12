#include "global.h"

#include "ScreenSelectProfile.h"
#include "ScreenMiniMenu.h"
#include "ProfileManager.h"
#include "ScreenTextEntry.h"
#include "ScreenPrompt.h"
#include "RageUtil.h"
#include "GameState.h"
#include "Profile.h"

AutoScreenMessage( SM_BackFromEnterName )
AutoScreenMessage( SM_BackFromProfileContextMenu )
AutoScreenMessage( SM_BackFromDelete )

const int PROFILE_MAX_NAME_LENGTH = 64;

static CString GetLastSelectedProfileName()
{
	Profile &pro = PROFILEMAN->GetLocalProfileEditableData( GAMESTATE->m_sLastSelectedProfileID );
	return pro.m_sDisplayName;
}

static bool ValidateProfileName( const CString &sAnswer, CString &sErrorOut )
{
	const CString &sCurrentProfileOldName = GetLastSelectedProfileName();

	vector<CString> vsUsedNames;
	PROFILEMAN->GetLocalProfileDisplayNames( vsUsedNames );
	bool bAlreadyAProfileWithThisName = find( vsUsedNames.begin(), vsUsedNames.end(), sAnswer ) != vsUsedNames.end();
	
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

enum ContextMenuAnswer
{
	A_EDIT, 
	A_RENAME, 
	A_DELETE, 
	A_CANCEL, 
};


REGISTER_SCREEN_CLASS( ScreenSelectProfile );
ScreenSelectProfile::ScreenSelectProfile( CString sName ) : 
	ScreenSelectMaster( sName )
{

}

void ScreenSelectProfile::Init()
{
	// Fill m_aGameCommands overriding whatever is in metrics
	GameCommand gc;
	m_aGameCommands.clear();

	gc.m_sName = "create";
	m_aGameCommands.push_back( gc );

	vector<CString> vProfileIDs;
	PROFILEMAN->GetLocalProfileIDs( vProfileIDs );
	FOREACH_CONST( CString, vProfileIDs, s )
	{
		gc.m_sName = "profile";
		gc.m_sProfileID = *s;
		m_aGameCommands.push_back( gc );
	}

	gc.m_sName = "exit";
	gc.m_sProfileID = "";
	m_aGameCommands.push_back( gc );


	ScreenSelectMaster::Init();
}

ScreenSelectProfile::~ScreenSelectProfile()
{

}

void ScreenSelectProfile::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_BackFromEnterName )
	{
		if( !ScreenTextEntry::s_bCancelledLast )
		{
			ASSERT( ScreenTextEntry::s_sLastAnswer != "" );	// validate should have assured this
		
			CString sNewName = ScreenTextEntry::s_sLastAnswer;
			if( GAMESTATE->m_sLastSelectedProfileID.empty() )
			{
				// create
				bool bResult = PROFILEMAN->CreateLocalProfile( sNewName );
				if( bResult )
					SCREENMAN->SetNewScreen( m_sName );	// reload
				else
					ScreenPrompt::Prompt( SM_None, ssprintf("Error creating profile '%s'.", sNewName.c_str()) );
			}
			else
			{
				// rename
				bool bResult = PROFILEMAN->RenameLocalProfile( GAMESTATE->m_sLastSelectedProfileID, sNewName );
				if( bResult )
					SCREENMAN->SetNewScreen( m_sName );	// reload
				else
					ScreenPrompt::Prompt( SM_None, ssprintf("Error renaming profile '%s'.", sNewName.c_str()) );
			}
		}
	}
	else if( SM == SM_BackFromProfileContextMenu )
	{
		if( !ScreenMiniMenu::s_bCancelled )
		{
			switch( ScreenMiniMenu::s_iLastRowCode )
			{
			default:
				ASSERT(0);
			case A_EDIT:
				SCREENMAN->SetNewScreen( "ScreenOptionsEditProfile" );
				break;
			case A_RENAME: 
				{
					ScreenTextEntry::TextEntry( SM_BackFromEnterName, "Enter a name for a new profile.", GetLastSelectedProfileName(), PROFILE_MAX_NAME_LENGTH, ValidateProfileName );
				}
				break;
			case A_DELETE: 
				{
					CString sMessage = ssprintf( "Are you sure you want to delete the profile '%s'?", GetLastSelectedProfileName().c_str() );
					ScreenPrompt::Prompt( SM_BackFromDelete, sMessage, PROMPT_YES_NO );
				}
				break;
			case A_CANCEL:
				SCREENMAN->PlayInvalidSound();
				break;
			}
		}
	}
	else if( SM == SM_BackFromDelete )
	{
		if( ScreenPrompt::s_LastAnswer == ANSWER_YES )
		{
			PROFILEMAN->DeleteLocalProfile( GAMESTATE->m_sLastSelectedProfileID );
			SCREENMAN->SetNewScreen( m_sName );	// reload
		}
	}

	ScreenSelectMaster::HandleScreenMessage( SM );
}

/*
void ScreenSelectProfile::ProcessMenuStart( PlayerNumber pn, const InputEventType type )
{
	int iRow = GetCurrentRow();;
	OptionRow &row = *m_pRows[iRow];

	if( iRow == 0 )	// "create new"
	{
		s_sLastSelectedProfileID = "";
		ScreenTextEntry::TextEntry( SM_BackFromEnterName, "Enter a name for a new profile.", PROFILEMAN->GetNewProfileDefaultName(), PROFILE_MAX_NAME_LENGTH, ValidateProfileName );
	}
	else if( row.GetRowType() == OptionRow::ROW_EXIT )
	{
		s_sLastSelectedProfileID = "";
		ScreenOptions::ProcessMenuStart( pn, type );
	}
	else
	{
		int iProfileIndex = iRow - 1;
		vector<CString> vsProfileIDs;
		PROFILEMAN->GetLocalProfileIDs( vsProfileIDs );
		s_sLastSelectedProfileID = vsProfileIDs[ iProfileIndex ];
		int iThrowAway, iX, iY;
		GetWidthXY( PLAYER_1, iRow, 0, iThrowAway, iX, iY );
		ScreenMiniMenu::MiniMenu( &g_ProfileContextMenu, SM_BackFromProfileContextMenu, SM_BackFromProfileContextMenu, (float)iX, (float)iY );
	}
}
*/

/*
 * (c) 2003-2004 Chris Danford
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

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
AutoScreenMessage( SM_BackFromDelete )


REGISTER_SCREEN_CLASS( ScreenSelectProfile );
ScreenSelectProfile::ScreenSelectProfile( CString sName ) : 
	ScreenSelectMaster( sName ),
	NEXT_SCREEN( sName, "NextScreen" )
{

}

void ScreenSelectProfile::Init()
{
	// Fill m_aGameCommands overriding whatever is in metrics
	GameCommand gc;
	int iIndex = 0;
	m_aGameCommands.clear();

	gc.Load( iIndex++, ParseCommands("name,Create") );
	m_aGameCommands.push_back( gc );

	vector<CString> vsProfileID;
	PROFILEMAN->GetLocalProfileIDs( vsProfileID );
	FOREACH_CONST( CString, vsProfileID, s )
	{
		CString sCommand = ssprintf( "name,Profile;profileid,%s;screen,%s", s->c_str(), NEXT_SCREEN.GetValue().c_str() );
		gc.Load( iIndex++, ParseCommands(sCommand) );
		m_aGameCommands.push_back( gc );
	}

	gc.Load( iIndex++, ParseCommands("name,Exit") );
	m_aGameCommands.push_back( gc );


	ScreenSelectMaster::Init();
}

ScreenSelectProfile::~ScreenSelectProfile()
{

}

bool ScreenSelectProfile::ProcessMenuStart( PlayerNumber pn )
{
	int iChoice = m_iChoice[GetSharedPlayer()];
	
	if( iChoice == 0 )	// "create"
	{
		vector<CString> vProfileIDs;
		PROFILEMAN->GetLocalProfileIDs( vProfileIDs );
		if( (int) vProfileIDs.size() >= MAX_NUM_LOCAL_PROFILES )
		{
			CString sError = ssprintf( "You may only create up to %d profiles.  You must delete an existing profile before creating a new one.", MAX_NUM_LOCAL_PROFILES );
			ScreenPrompt::Prompt( SM_None, sError );
		}
		else
		{
			ScreenTextEntry::TextEntry( SM_BackFromEnterName, "Enter a name for a new profile.", PROFILEMAN->GetNewLocalProfileDefaultName(), PROFILE_MAX_DISPLAY_NAME_LENGTH, ProfileManager::ValidateLocalProfileName );
		}
		return false;
	}

	return true;
}

void ScreenSelectProfile::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_BackFromEnterName )
	{
		if( !ScreenTextEntry::s_bCancelledLast )
		{
			ASSERT( ScreenTextEntry::s_sLastAnswer != "" );	// validate should have assured this
		
			CString sNewName = ScreenTextEntry::s_sLastAnswer;
			ASSERT( GAMESTATE->m_sLastSelectedProfileID.empty() )

			// create
			bool bResult = PROFILEMAN->CreateLocalProfile( sNewName, GAMESTATE->m_sLastSelectedProfileID );
			if( bResult )
				SCREENMAN->SetNewScreen( NEXT_SCREEN );
			else
				ScreenPrompt::Prompt( SM_None, ssprintf("Error creating profile '%s'.", sNewName.c_str()) );
		}
	}
	else if( SM == SM_GoToNextScreen )
	{
		int iSelection = GetSelectionIndex( PLAYER_1 );
		
		vector<CString> vsProfileID;
		PROFILEMAN->GetLocalProfileIDs( vsProfileID );
		if( iSelection == 0 || iSelection == (int) vsProfileID.size()+1 )
		{
			GAMESTATE->m_sLastSelectedProfileID = "";
		}
		else
		{
			GAMESTATE->m_sLastSelectedProfileID = m_aGameCommands[iSelection].m_sProfileID;
			ASSERT( !GAMESTATE->m_sLastSelectedProfileID.empty() );
		}
	}


	ScreenSelectMaster::HandleScreenMessage( SM );
}

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

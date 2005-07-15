#include "global.h"

#include "ScreenOptionsEditProfile.h"
#include "ScreenMiniMenu.h"
#include "ProfileManager.h"
#include "ScreenTextEntry.h"
#include "ScreenPrompt.h"
#include "RageUtil.h"
#include "GameState.h"
#include "Profile.h"
#include "Character.h"

AutoScreenMessage( SM_BackFromEnterName )
AutoScreenMessage( SM_BackFromDelete )

enum EditProfileRow
{
	ROW_NAME,
	ROW_CHARACTER,
	ROW_DELETE
};

REGISTER_SCREEN_CLASS( ScreenOptionsEditProfile );
ScreenOptionsEditProfile::ScreenOptionsEditProfile( CString sName ) : 
	ScreenOptions( sName )
{

}

void ScreenOptionsEditProfile::Init()
{
	ScreenOptions::Init();


	vector<OptionRowDefinition> vDefs;
	vector<OptionRowHandler*> vHands;

	OptionRowDefinition def;
	def.m_layoutType = LAYOUT_SHOW_ONE_IN_ROW;
	def.m_bOneChoiceForAllPlayers = true;
	def.m_bAllowThemeItems = false;
	def.m_bAllowThemeTitles = false;
	def.m_bAllowExplanation = false;
	
	Profile &pro = PROFILEMAN->GetLocalProfile( GAMESTATE->m_sLastSelectedProfileID );

	{
		def.m_sName = "Name";
		def.m_vsChoices.clear();
		CString s = pro.m_sDisplayName;
		def.m_vsChoices.push_back( s );
		vDefs.push_back( def );
		vHands.push_back( NULL );
	}

	{
		def.m_sName = "Character";
		def.m_vsChoices.clear();
		vector<Character*> vpCharacters;
		GAMESTATE->GetCharacters( vpCharacters );
		FOREACH_CONST( Character*, vpCharacters, c )
			def.m_vsChoices.push_back( (*c)->m_sName );
		vDefs.push_back( def );
		vHands.push_back( NULL );
	}

	{
		def.m_sName = "Delete";
		def.m_vsChoices.clear();
		vDefs.push_back( def );
		vHands.push_back( NULL );
	}

	InitMenu( vDefs, vHands );
}

ScreenOptionsEditProfile::~ScreenOptionsEditProfile()
{

}

void ScreenOptionsEditProfile::ImportOptions( int iRow, const vector<PlayerNumber> &vpns )
{
	Profile &pro = PROFILEMAN->GetLocalProfile( GAMESTATE->m_sLastSelectedProfileID );
	OptionRow &row = *m_pRows[iRow];

	switch( iRow )
	{
	case ROW_CHARACTER:
		row.SetOneSharedSelectionIfPresent( pro.m_sCharacter );
		break;
	}
}

void ScreenOptionsEditProfile::ExportOptions( int iRow, const vector<PlayerNumber> &vpns )
{
	Profile &pro = PROFILEMAN->GetLocalProfile( GAMESTATE->m_sLastSelectedProfileID );
	OptionRow &row = *m_pRows[iRow];
	int iIndex = row.GetOneSharedSelection( true );
	CString sValue;
	if( iIndex >= 0 )
		sValue = row.GetRowDef().m_vsChoices[ iIndex ];

	switch( iRow )
	{
	case ROW_NAME:
		pro.m_sDisplayName = sValue;
		break;
	case ROW_CHARACTER:
		pro.m_sCharacter = sValue;
		break;
	}
}

void ScreenOptionsEditProfile::GoToNextScreen()
{
	SCREENMAN->SetNewScreen( "ScreenOptionsService" );
}

void ScreenOptionsEditProfile::GoToPrevScreen()
{
	SCREENMAN->SetNewScreen( "ScreenOptionsService" );
}

void ScreenOptionsEditProfile::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_BackFromEnterName )
	{
		if( !ScreenTextEntry::s_bCancelledLast )
		{
			ASSERT( ScreenTextEntry::s_sLastAnswer != "" );	// validate should have assured this
		
			CString sNewName = ScreenTextEntry::s_sLastAnswer;
			ASSERT( !GAMESTATE->m_sLastSelectedProfileID.empty() )

			// rename
			bool bResult = PROFILEMAN->RenameLocalProfile( GAMESTATE->m_sLastSelectedProfileID, sNewName );
			if( bResult )
				SCREENMAN->SetNewScreen( m_sName );	// reload
			else
				ScreenPrompt::Prompt( SM_None, ssprintf("Error renaming profile '%s'.", sNewName.c_str()) );
		}
	}
	else if( SM == SM_BackFromDelete )
	{
		if( ScreenPrompt::s_LastAnswer == ANSWER_YES )
		{
			PROFILEMAN->DeleteLocalProfile( GAMESTATE->m_sLastSelectedProfileID );
			HandleScreenMessage( SM_GoToPrevScreen );
		}
	}

	ScreenOptions::HandleScreenMessage( SM );
}

void ScreenOptionsEditProfile::ProcessMenuStart( PlayerNumber pn, const InputEventType type )
{
	int iRow = GetCurrentRow();;
	OptionRow &row = *m_pRows[iRow];

	switch( iRow )
	{
	case ROW_NAME:
		{
			CString sCurrentProfileName = PROFILEMAN->GetLocalProfile( GAMESTATE->m_sLastSelectedProfileID ).m_sDisplayName;
			ScreenTextEntry::TextEntry( SM_BackFromEnterName, "Enter a name for a new profile.", sCurrentProfileName, PROFILE_MAX_DISPLAY_NAME_LENGTH, ProfileManager::ValidateLocalProfileName );
		}
		break;
	case ROW_CHARACTER:
		{
		}
		break;
	case ROW_DELETE:
		{
			CString sCurrentProfileName = PROFILEMAN->GetLocalProfile( GAMESTATE->m_sLastSelectedProfileID ).m_sDisplayName;
			CString sMessage = ssprintf( "Are you sure you want to delete the profile '%s'?", sCurrentProfileName.c_str() );
			ScreenPrompt::Prompt( SM_BackFromDelete, sMessage, PROMPT_YES_NO );
		}
		break;
	default:
		ScreenOptions::ProcessMenuStart( pn, type );
		break;
	}
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

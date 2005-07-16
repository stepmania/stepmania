#include "global.h"

#include "ScreenOptionsSelectProfile.h"
#include "ScreenMiniMenu.h"
#include "ProfileManager.h"
#include "ScreenTextEntry.h"
#include "ScreenPrompt.h"
#include "RageUtil.h"
#include "GameState.h"
#include "Profile.h"
#include "Character.h"
#include "OptionRowHandler.h"

AutoScreenMessage( SM_BackFromEnterName )
AutoScreenMessage( SM_BackFromDelete )


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


REGISTER_SCREEN_CLASS( ScreenOptionsSelectProfile );
ScreenOptionsSelectProfile::ScreenOptionsSelectProfile( CString sName ) : 
	ScreenOptions( sName )
{

}

void ScreenOptionsSelectProfile::Init()
{
	ScreenOptions::Init();


	vector<OptionRowDefinition> vDefs;
	vector<OptionRowHandler*> vHands;

	OptionRowDefinition def;
	def.m_layoutType = LAYOUT_SHOW_ONE_IN_ROW;
	def.m_selectType = SELECT_NONE;
	def.m_bOneChoiceForAllPlayers = true;
	def.m_bAllowThemeItems = false;
	def.m_bAllowThemeTitles = false;
	def.m_bAllowExplanation = false;
	
	int iIndex = 0;

	{
		def.m_sName = "";
		def.m_vsChoices.clear();
		def.m_vsChoices.push_back( "Create New" );
		vDefs.push_back( def );
		OptionRowHandlerSimple *pHand = new OptionRowHandlerSimple;
		m_OptionRowHandlers.push_back( pHand );
		GameCommand &gc = pHand->m_gc;
		CString sCommand = "screen,ScreenOptionsEditProfile";
		gc.Load( iIndex++, ParseCommands(sCommand) );
		vHands.push_back( pHand );
	}

	vector<CString> vsProfileID;
	PROFILEMAN->GetLocalProfileIDs( vsProfileID );
	FOREACH_CONST( CString, vsProfileID, s )
	{
		Profile &pro = PROFILEMAN->GetLocalProfile( *s );
		def.m_sName = "";
		def.m_vsChoices.clear();
		def.m_vsChoices.push_back( pro.m_sDisplayName );
		vDefs.push_back( def );
		OptionRowHandlerSimple *pHand = new OptionRowHandlerSimple;
		m_OptionRowHandlers.push_back( pHand );
		GameCommand &gc = pHand->m_gc;
		CString sCommand = "profileid," + *s;
		gc.Load( iIndex++, ParseCommands(sCommand) );
		vHands.push_back( pHand );
	}

	InitMenu( vDefs, vHands );
}

ScreenOptionsSelectProfile::~ScreenOptionsSelectProfile()
{
	FOREACH( OptionRow*, m_pRows, r )
		(*r)->DetachHandler();
	FOREACH( OptionRowHandler*, m_OptionRowHandlers, h )
		SAFE_DELETE( *h );
	m_OptionRowHandlers.clear();
}

void ScreenOptionsSelectProfile::ImportOptions( int iRow, const vector<PlayerNumber> &vpns )
{
	OptionRow &row = *m_pRows[iRow];
}

void ScreenOptionsSelectProfile::ExportOptions( int iRow, const vector<PlayerNumber> &vpns )
{
	int iCurRow = m_iCurrentRow[0];
	if( iRow == iCurRow )
	{
		OptionRow &row = *m_pRows[iRow];

		bool bRowHasFocus[NUM_PLAYERS];
		ZERO( bRowHasFocus );
		row.ExportOptions( vpns, bRowHasFocus );
	}
}

void ScreenOptionsSelectProfile::GoToNextScreen()
{
	SCREENMAN->SetNewScreen( "ScreenOptionsService" );
}

void ScreenOptionsSelectProfile::GoToPrevScreen()
{
	SCREENMAN->SetNewScreen( "ScreenOptionsService" );
}

void ScreenOptionsSelectProfile::HandleScreenMessage( const ScreenMessage SM )
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
				SCREENMAN->SetNewScreen( "ScreenOptionsEditProfile" );
			else
				ScreenPrompt::Prompt( SM_None, ssprintf("Error creating profile '%s'.", sNewName.c_str()) );
		}
	}

	ScreenOptions::HandleScreenMessage( SM );
}

void ScreenOptionsSelectProfile::ProcessMenuStart( PlayerNumber pn, const InputEventType type )
{
	int iRow = GetCurrentRow();;
	OptionRow &row = *m_pRows[iRow];

	if( iRow == 0 )
	{
		vector<CString> vProfileIDs;
		PROFILEMAN->GetLocalProfileIDs( vProfileIDs );
		if( vProfileIDs.size() >= MAX_NUM_LOCAL_PROFILES )
		{
			CString sError = ssprintf( "You may only create up to %d profiles.  You must delete an existing profile before creating a new one.", MAX_NUM_LOCAL_PROFILES );
			ScreenPrompt::Prompt( SM_None, sError );
		}
		else
		{
			ScreenTextEntry::TextEntry( SM_BackFromEnterName, "Enter a name for a new profile.", PROFILEMAN->GetNewLocalProfileDefaultName(), PROFILE_MAX_DISPLAY_NAME_LENGTH, ProfileManager::ValidateLocalProfileName );
		}
	}
	else if( row.GetRowType() == OptionRow::ROW_EXIT )
	{
		ScreenOptions::ProcessMenuStart( pn, type );
	}
	else
	{
		// a profile row
		ScreenOptions::BeginFadingOut();
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

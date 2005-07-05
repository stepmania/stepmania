#include "global.h"

#include "ScreenOptionsProfiles.h"
#include "ScreenMiniMenu.h"
#include "ProfileManager.h"
#include "ScreenTextEntry.h"
#include "ScreenPrompt.h"
#include "RageUtil.h"

AutoScreenMessage( SM_BackFromEnterName )
AutoScreenMessage( SM_BackFromProfileContextMenu )
AutoScreenMessage( SM_BackFromDelete )

CString ScreenOptionsProfiles::s_sCurrentProfileID = "";
const int PROFILE_MAX_NAME_LENGTH = 64;

static bool ValidateProfileName( const CString &sAnswer, CString &sErrorOut )
{
	CString sCurrentProfileOldName = PROFILEMAN->ProfileIDToName( ScreenOptionsProfiles::s_sCurrentProfileID );
	vector<CString> vsProfileNames;
	PROFILEMAN->GetLocalProfileNames( vsProfileNames );
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

enum ContextMenuAnswer
{
	A_EDIT, 
	A_RENAME, 
	A_DELETE, 
	A_CANCEL, 
};

static MenuDef g_ProfileContextMenu(
	"ScreenMiniMenuProfiles",
	MenuRowDef( A_EDIT,		"Edit",		true, EDIT_MODE_PRACTICE, 0, "" ),
	MenuRowDef( A_RENAME,	"Rename",	true, EDIT_MODE_PRACTICE, 0, "" ),
	MenuRowDef( A_DELETE,	"Delete",	true, EDIT_MODE_PRACTICE, 0, "" ),
	MenuRowDef( A_CANCEL,	"Cancel",	true, EDIT_MODE_PRACTICE, 0, "" )
);

REGISTER_SCREEN_CLASS( ScreenOptionsProfiles );
ScreenOptionsProfiles::ScreenOptionsProfiles( CString sName ) : 
	ScreenOptions( sName )
{

}

void ScreenOptionsProfiles::Init()
{
	ScreenOptions::Init();


	vector<OptionRowDefinition> vDefs;
	vector<OptionRowHandler*> vHands;

	OptionRowDefinition def;
	def.layoutType = LAYOUT_SHOW_ONE_IN_ROW;
	def.m_bAllowThemeItems = false;
	def.m_bAllowThemeTitles = false;
	def.m_bAllowExplanation = false;
	
	def.name = "Create New";
	def.choices.clear();
	vDefs.push_back( def );
	vHands.push_back( NULL );

	vector<CString> vsProfileNames;
	PROFILEMAN->GetLocalProfileNames( vsProfileNames );

	FOREACH_CONST( CString, vsProfileNames, s )
	{
		def.name = *s;
		def.choices.clear();
		vDefs.push_back( def );
		vHands.push_back( NULL );
	}


	InitMenu( INPUTMODE_SHARE_CURSOR, vDefs, vHands );
}

ScreenOptionsProfiles::~ScreenOptionsProfiles()
{

}

void ScreenOptionsProfiles::ImportOptions( int row, const vector<PlayerNumber> &vpns )
{

}

void ScreenOptionsProfiles::ExportOptions( int row, const vector<PlayerNumber> &vpns )
{

}

void ScreenOptionsProfiles::GoToNextScreen()
{
	SCREENMAN->SetNewScreen( "ScreenOptionsMenu" );
}

void ScreenOptionsProfiles::GoToPrevScreen()
{
	SCREENMAN->SetNewScreen( "ScreenOptionsMenu" );
}

void ScreenOptionsProfiles::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_BackFromEnterName )
	{
		if( !ScreenTextEntry::s_bCancelledLast )
		{
			ASSERT( ScreenTextEntry::s_sLastAnswer != "" );	// validate should have assured this
		
			CString sNewName = ScreenTextEntry::s_sLastAnswer;
			if( s_sCurrentProfileID.empty() )
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
				bool bResult = PROFILEMAN->RenameLocalProfile( s_sCurrentProfileID, sNewName );
				if( bResult )
					SCREENMAN->SetNewScreen( m_sName );	// reload
				else
					ScreenPrompt::Prompt( SM_None, ssprintf("Error renaming profile '%s'.", sNewName.c_str()) );
			}
		}
	}
	else if( SM == SM_BackFromProfileContextMenu )
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
				CString sCurrentProfileName = PROFILEMAN->ProfileIDToName( s_sCurrentProfileID );
				ScreenTextEntry::TextEntry( SM_BackFromEnterName, "Enter a name for a new profile.", sCurrentProfileName, PROFILE_MAX_NAME_LENGTH, ValidateProfileName );
			}
			break;
		case A_DELETE: 
			{
				CString sCurrentProfileName = PROFILEMAN->ProfileIDToName( s_sCurrentProfileID );
				CString sMessage = ssprintf( "Are you sure you want to delete the profile '%s'?", sCurrentProfileName.c_str() );
				ScreenPrompt::Prompt( SM_BackFromDelete, sMessage, PROMPT_YES_NO );
			}
			break;
		case A_CANCEL:
			SCREENMAN->PlayInvalidSound();
			break;
		}
	}
	else if( SM == SM_BackFromDelete )
	{
		if( ScreenPrompt::s_LastAnswer == ANSWER_YES )
		{
			PROFILEMAN->DeleteLocalProfile( s_sCurrentProfileID );
			SCREENMAN->SetNewScreen( m_sName );	// reload
		}
	}
}

void ScreenOptionsProfiles::ProcessMenuStart( PlayerNumber pn, const InputEventType type )
{
	int iRow = GetCurrentRow();;
	OptionRow &row = *m_pRows[iRow];

	if( iRow == 0 )	// "create new"
	{
		s_sCurrentProfileID = "";
		ScreenTextEntry::TextEntry( SM_BackFromEnterName, "Enter a name for a new profile.", PROFILEMAN->GetNewProfileDefaultName(), PROFILE_MAX_NAME_LENGTH, ValidateProfileName );
	}
	else if( row.GetRowType() == OptionRow::ROW_EXIT )
	{
		s_sCurrentProfileID = "";
		ScreenOptions::ProcessMenuStart( pn, type );
	}
	else
	{
		int iProfileIndex = iRow - 1;
		vector<CString> vsProfileIDs;
		PROFILEMAN->GetLocalProfileIDs( vsProfileIDs );
		s_sCurrentProfileID = vsProfileIDs[ iProfileIndex ];
		ScreenMiniMenu::MiniMenu( &g_ProfileContextMenu, SM_BackFromProfileContextMenu );
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

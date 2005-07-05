#include "global.h"

#include "ScreenOptionsProfiles.h"
#include "ScreenMiniMenu.h"
#include "ProfileManager.h"
#include "ScreenTextEntry.h"
#include "RageUtil.h"

AutoScreenMessage( SM_BackFromCreateNewName )
AutoScreenMessage( SM_BackFromProfileContextMenu )

enum ContextMenuAnswer
{
	A_EDIT, 
	A_RENAME, 
	A_DELETE, 
	A_CANCEL, 
};

static MenuDef g_ProfileContextMenu(
	"ScreenMiniMenuProfiles",
	MenuRowDef( -1, "Edit",		true, EDIT_MODE_PRACTICE, 0, "" ),
	MenuRowDef( -1, "Rename",	true, EDIT_MODE_PRACTICE, 0, "" ),
	MenuRowDef( -1, "Delete",	true, EDIT_MODE_PRACTICE, 0, "" ),
	MenuRowDef( -1, "Cancel",	true, EDIT_MODE_PRACTICE, 0, "" )
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
	if( SM == SM_BackFromCreateNewName )
	{
		if( !ScreenTextEntry::s_bCancelledLast && ScreenTextEntry::s_sLastAnswer != "" )
		{
			CString sNewName = ScreenTextEntry::s_sLastAnswer;
			bool bResult = PROFILEMAN->CreateLocalProfile( sNewName );
			if( bResult )
				SCREENMAN->SetNewScreen( m_sName );	// reload
			else
				SCREENMAN->Prompt( SM_None, ssprintf("Error creating profile '%s'.", sNewName.c_str()) );
		}		
	}
	else if( SM == SM_BackFromProfileContextMenu )
	{
		switch( ScreenMiniMenu::s_iLastRowCode )
		{
		default:
			ASSERT(0);
		case A_EDIT:
		case A_RENAME: 
		case A_DELETE: 
		case A_CANCEL:
			SCREENMAN->PlayInvalidSound();
			break;
		}
	}
}

void ScreenOptionsProfiles::ProcessMenuStart( PlayerNumber pn, const InputEventType type )
{
	int iRow = GetCurrentRow();;
	OptionRow &row = *m_pRows[iRow];

	if( iRow == 0 )	// "create new"
	{
		//ScreenOptions::BeginFadingOut();
		SCREENMAN->TextEntry( SM_BackFromCreateNewName, "Enter a name for a new profile.", "", 64 );
	}
	else if( row.GetRowType() == OptionRow::ROW_EXIT )
	{
		ScreenOptions::ProcessMenuStart( pn, type );
	}
	else
	{
		SCREENMAN->MiniMenu( &g_ProfileContextMenu, SM_BackFromProfileContextMenu );

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

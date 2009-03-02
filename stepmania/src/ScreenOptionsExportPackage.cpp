#include "global.h"
#include "ScreenOptionsExportPackage.h"
#include "ScreenManager.h"
#include "RageLog.h"
#include "GameState.h"
#include "CommonMetrics.h"
#include "ScreenPrompt.h"
#include "ScreenMiniMenu.h"
#include "OptionRowHandler.h"
#include "LocalizedString.h"
#include "SpecialFiles.h"
#include "SpecialFiles.h"
#include "ScreenPrompt.h"

REGISTER_SCREEN_CLASS( ScreenOptionsExportPackage );

void ScreenOptionsExportPackage::Init()
{
	ScreenOptions::Init();

	SetNavigation( NAV_THREE_KEY_MENU );
	SetInputMode( INPUTMODE_SHARE_CURSOR );
}

void ScreenOptionsExportPackage::BeginScreen()
{
	// Fill m_vsPossibleDirsToExport
	{
		// Add themes
		{
			GetDirListing( SpecialFiles::THEMES_DIR + "*", m_vsPossibleDirsToExport, true, true );
		}

		// Add NoteSkins
		{
			vector<RString> vs;
			GetDirListing( SpecialFiles::NOTESKINS_DIR + "*", vs, true, true );
			FOREACH_CONST( RString, vs, s )
				GetDirListing( *s + "*", m_vsPossibleDirsToExport, true, true );
		}

		// Add courses.  Only support courses that are in a group folder.  Support for courses not in a group folder should be phased out.
		{
			vector<RString> vs;
			GetDirListing( SpecialFiles::COURSES_DIR + "*", vs, true, true );
			FOREACH_CONST( RString, vs, s )
			{
				m_vsPossibleDirsToExport.push_back( *s );
				GetDirListing( *s + "/*", m_vsPossibleDirsToExport, true, true );
			}
		}

		// Add songs
		{
			vector<RString> vs;
			GetDirListing( SpecialFiles::SONGS_DIR + "*", vs, true, true );
			FOREACH_CONST( RString, vs, s )
			{
				m_vsPossibleDirsToExport.push_back( *s );
				GetDirListing( *s + "/*", m_vsPossibleDirsToExport, true, true );
			}
		}
	
		StripCvsAndSvn( m_vsPossibleDirsToExport );
	}



	vector<OptionRowHandler*> OptionRowHandlers;
	FOREACH_CONST( RString, m_vsPossibleDirsToExport, s )
	{
		OptionRowHandler *pHand = OptionRowHandlerUtil::MakeNull();
		OptionRowDefinition &def = pHand->m_Def;
		def.m_layoutType = LAYOUT_SHOW_ALL_IN_ROW;
		def.m_bAllowThemeTitle = false;
		def.m_bAllowThemeItems = false;
		def.m_bAllowExplanation = false;
		def.m_sName = *s;
		def.m_sExplanationName = "# files, # MB, # subdirs";

		def.m_vsChoices.push_back( "" );
		OptionRowHandlers.push_back( pHand );
	}
	ScreenOptions::InitMenu( OptionRowHandlers );

	ScreenOptions::BeginScreen();
}

void ScreenOptionsExportPackage::ProcessMenuStart( const InputEventPlus &input )
{
	if( IsTransitioning() )
		return;

	int iCurRow = m_iCurrentRow[GAMESTATE->m_MasterPlayerNumber];
	OptionRow &row = *m_pRows[iCurRow];
	RString sDirToExport = row.GetRowDef().m_sName;

	
	bool bSuccess = true; // DoExport();
	if( bSuccess )
		ScreenPrompt::Prompt( SM_None, ssprintf("Exported '%s' to the desktop", sDirToExport.c_str()) );
	else
		ScreenPrompt::Prompt( SM_None, "Failed to export package" );
}

void ScreenOptionsExportPackage::ImportOptions( int iRow, const vector<PlayerNumber> &vpns )
{

}

void ScreenOptionsExportPackage::ExportOptions( int iRow, const vector<PlayerNumber> &vpns )
{

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

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
	// todo: Split these out over multiple screens so the scroller
	// isn't so overloaded. (See ScreenOptionsToggleSongs) -freem
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

		// Add courses. Only support courses that are in a group folder.
		// Support for courses not in a group folder should be phased out.
		{
			vector<RString> vs;
			GetDirListing( SpecialFiles::COURSES_DIR + "*", vs, true, true );
			StripCvsAndSvn( vs );
			StripMacResourceForks( vs );
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
		StripMacResourceForks( m_vsPossibleDirsToExport );
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

static RString ReplaceInvalidFileNameChars( RString sOldFileName )
{
	RString sNewFileName = sOldFileName;
	const char charsToReplace[] = { 
		' ', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', 
		'+', '=', '[', ']', '{', '}', '|', ':', '\"', '\\',
		'<', '>', ',', '?', '/' 
	};
	for( unsigned i=0; i<sizeof(charsToReplace); i++ )
		sNewFileName.Replace( charsToReplace[i], '_' );
	return sNewFileName;
}

static bool ExportPackage( RString sPackageName, RString sDirToExport, RString &sErrorOut )
{
	/*
	RageFile f;
	// TODO: Mount Desktop/ for each OS
	// SpecialDirs::GetDesktopDir() [windows only right now -aj]
	if( !f.Open("Desktop/"+sPackageName, RageFile::WRITE) )
	{
		sErrorOut = ssprintf( "Couldn't open %s for writing: %s", fn.c_str(), f.GetError().c_str() );
		return false;
	}

	RageFileObjZip zip( &f );
	zip.Start();
	zip.SetGlobalComment( sComment );

	vector<RString> vs;
	GetDirListingRecursive( sDirToExport, "*", vs );
	SMPackageUtil::StripIgnoredSmzipFiles( vs );
	FOREACH( RString, vs, s )
	{
		if( !zip.AddFile( *s ) )
		{
			sErrorOut = ssprintf( "Couldn't add file: %s", s->c_str() );
			return false;
		}
	}

	if( zip.Finish() == -1 )
	{
		sErrorOut = ssprintf( "Couldn't write to file %s", fn.c_str(), f.GetError().c_str() );
		return false;
	}

	return true;
	*/
	return false;
}

void ScreenOptionsExportPackage::ProcessMenuStart( const InputEventPlus &input )
{
	if( IsTransitioning() )
		return;

	int iCurRow = m_iCurrentRow[GAMESTATE->m_MasterPlayerNumber];
	if( m_pRows[iCurRow]->GetRowType() == OptionRow::RowType_Exit )
	{
		ScreenOptions::ProcessMenuStart( input );
		return;
	}

	RString sDirToExport = m_vsPossibleDirsToExport[ iCurRow ];
	RString sPackageName = ReplaceInvalidFileNameChars( sDirToExport + ".smzip" );

	RString sError;
	if( ExportPackage(sPackageName, sDirToExport, sError) )
		ScreenPrompt::Prompt( SM_None, ssprintf("Exported '%s' to the desktop", sDirToExport.c_str()) );
	else
		ScreenPrompt::Prompt( SM_None, ssprintf("Failed to export package: %s",sError.c_str()) );
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

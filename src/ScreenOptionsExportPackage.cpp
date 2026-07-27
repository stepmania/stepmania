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
#include "ScreenPrompt.h"
#include "SongManager.h"
#include "RageFile.h"
#include "archutils/SpecialDirs.h"

#include <vector>


// main page (type list)
REGISTER_SCREEN_CLASS( ScreenOptionsExportPackage );

void ScreenOptionsExportPackage::Init()
{
	ScreenOptions::Init();

	SetNavigation( NAV_THREE_KEY_MENU );
	SetInputMode( INPUTMODE_SHARE_CURSOR );
}

void ScreenOptionsExportPackage::BeginScreen()
{
	// Fill m_vsPackageTypes:
	m_vsPackageTypes.push_back("Themes");
	m_vsPackageTypes.push_back("NoteSkins");
	m_vsPackageTypes.push_back("Courses");
	m_vsPackageTypes.push_back("Songs");
	// announcers, characters, others?

	std::vector<OptionRowHandler*> OptionRowHandlers;
	for (RString const &s : m_vsPackageTypes)
	{
		OptionRowHandler *pHand = OptionRowHandlerUtil::MakeNull();
		OptionRowDefinition &def = pHand->m_Def;

		def.m_sName = s;
		def.m_bAllowExplanation = false;
		//def.m_sExplanationName = "# files, # MB, # subdirs";
		def.m_bAllowThemeTitle = false;
		def.m_bAllowThemeItems = false;
		def.m_layoutType = LAYOUT_SHOW_ALL_IN_ROW;
		def.m_bOneChoiceForAllPlayers = true;
		def.m_vsChoices.clear();
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

	// switch to the subpage with the specified type
	//int iCurRow = m_iCurrentRow[GAMESTATE->GetMasterPlayerNumber()];
	int iRow = GetCurrentRow();
	if( m_pRows[iRow]->GetRowType() == OptionRow::RowType_Exit )
	{
		ScreenOptions::ProcessMenuStart( input );
		return;
	}

	ExportPackages::m_sPackageType = m_vsPackageTypes[iRow];

	SCREENMAN->PlayStartSound();
	this->BeginFadingOut();
	// todo: find a way to make this transition not be instant.
	SCREENMAN->SetNewScreen("ScreenOptionsExportPackageSubPage");
}

// todo: process menu back in SubGroup mode

void ScreenOptionsExportPackage::ImportOptions( int /* iRow */, const std::vector<PlayerNumber> & /* vpns */ )
{

}

void ScreenOptionsExportPackage::ExportOptions( int /* iRow */, const std::vector<PlayerNumber> & /* vpns */ )
{

}


// subpage (has all folders for the specified type)
REGISTER_SCREEN_CLASS( ScreenOptionsExportPackageSubPage );
void ScreenOptionsExportPackageSubPage::Init()
{
	ScreenOptions::Init();

	SetNavigation( NAV_THREE_KEY_MENU );
	SetInputMode( INPUTMODE_SHARE_CURSOR );
}

void ScreenOptionsExportPackageSubPage::BeginScreen()
{
	ScreenWithMenuElements::BeginScreen();

	// Check type and fill m_vsPossibleDirsToExport
	const RString *s_packageType = &ExportPackages::m_sPackageType;
	if( *s_packageType == "Themes" )
	{
		// add themes
		GetDirListing( SpecialFiles::THEMES_DIR + "*", m_vsPossibleDirsToExport, true, true );
	}
	else if( *s_packageType == "NoteSkins" )
	{
		// add noteskins
		std::vector<RString> vs;
		GetDirListing( SpecialFiles::NOTESKINS_DIR + "*", vs, true, true );
		for (RString const &s : vs)
			GetDirListing( s + "*", m_vsPossibleDirsToExport, true, true );
	}
	else if( *s_packageType == "Courses" )
	{
		// Add courses. Only support courses that are in a group folder.
		// Support for courses not in a group folder should be phased out.
		std::vector<RString> vs;
		GetDirListing( SpecialFiles::COURSES_DIR + "*", vs, true, true );
		StripCvsAndSvn( vs );
		StripMacResourceForks( vs );
		for (RString const &s : vs)
		{
			m_vsPossibleDirsToExport.push_back( s );
			GetDirListing( s + "/*", m_vsPossibleDirsToExport, true, true );
		}
	}
	else if( *s_packageType == "Songs" )
	{
		// Add song groups
		std::vector<RString> asAllGroups;
		SONGMAN->GetSongGroupNames(asAllGroups);
		for (RString const &s : asAllGroups)
		{
			m_vsPossibleDirsToExport.push_back(s);
		}
	}
	else if( *s_packageType == "SubGroup" )
	{
		//ExportPackages::m_sFolder
		std::vector<RString> vs;
		GetDirListing( SpecialFiles::SONGS_DIR + "/" + ExportPackages::m_sFolder + "/*", vs, true, true );
		for (RString const &s : vs)
		{
			m_vsPossibleDirsToExport.push_back( s );
			GetDirListing( s + "/*", m_vsPossibleDirsToExport, true, true );
		}
	}
	StripCvsAndSvn( m_vsPossibleDirsToExport );
	StripMacResourceForks( m_vsPossibleDirsToExport );

	std::vector<OptionRowHandler*> OptionRowHandlers;
	for (RString const &s : m_vsPossibleDirsToExport)
	{
		OptionRowHandler *pHand = OptionRowHandlerUtil::MakeNull();
		OptionRowDefinition &def = pHand->m_Def;
		def.m_layoutType = LAYOUT_SHOW_ALL_IN_ROW;
		def.m_bAllowThemeTitle = false;
		def.m_bAllowThemeItems = false;
		def.m_bAllowExplanation = false;
		def.m_sName = s;
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
	// Mount Desktop/ for each OS.
	RString sDesktopDir = SpecialDirs::GetDesktopDir();
	RString fn = sDesktopDir+sPackageName;
	RageFile f;
	if( !f.Open(fn, RageFile::WRITE) )
	{
		sErrorOut = ssprintf( "Couldn't open %s for writing: %s", fn.c_str(), f.GetError().c_str() );
		return false;
	}

	// XXX: totally doesn't work. -aj
	/*
	RageFileObjZip zip( &f );
	zip.Start();
	zip.SetGlobalComment( sComment );

	std::vector<RString> vs;
	GetDirListingRecursive( sDirToExport, "*", vs );
	SMPackageUtil::StripIgnoredSmzipFiles( vs );
	LOG->Trace("Adding files...");
	for (RString &s : vs)
	{
		if( !zip.AddFile( s ) )
		{
			sErrorOut = ssprintf( "Couldn't add file: %s", s.c_str() );
			return false;
		}
	}

	LOG->Trace("Writing zip...");
	if( zip.Finish() == -1 )
	{
		sErrorOut = ssprintf( "Couldn't write to file %s", fn.c_str(), f.GetError().c_str() );
		return false;
	}

	return true;
	*/
	return false;
}

void ScreenOptionsExportPackageSubPage::ProcessMenuStart( const InputEventPlus &input )
{
	if( IsTransitioning() )
		return;

	int iCurRow = m_iCurrentRow[GAMESTATE->GetMasterPlayerNumber()];
	if( m_pRows[iCurRow]->GetRowType() == OptionRow::RowType_Exit )
	{
		ScreenOptions::ProcessMenuStart( input );
		return;
	}

	if( ExportPackages::m_sPackageType == "Courses"
		|| ExportPackages::m_sPackageType == "NoteSkins"
		|| ExportPackages::m_sPackageType == "Songs" )
	{
		// find folder name
		ExportPackages::m_sPackageType = "SubGroup";
		ExportPackages::m_sFolder = m_vsPossibleDirsToExport[iCurRow];
		SCREENMAN->SetNewScreen("ScreenOptionsExportPackageSubPage");
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

void ScreenOptionsExportPackageSubPage::ImportOptions( int iRow, const std::vector<PlayerNumber> &vpns )
{

}

void ScreenOptionsExportPackageSubPage::ExportOptions( int iRow, const std::vector<PlayerNumber> &vpns )
{

}

/*
 * (c) 2002-2014 Chris Danford, AJ Kelly, Renaud Lepage
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

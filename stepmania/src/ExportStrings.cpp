#include "global.h"
#include "ExportStrings.h"
#include "RageFile.h"
#include "RageUtil.h"
#include "IniFile.h"
#include "XmlFile.h"
#include "XmlFileUtil.h"
#include "LuaManager.h"
#include "ProductInfo.h"
#include "DateTime.h"
#include "Foreach.h"
#include "arch/Dialog/Dialog.h"
#include "RageFileManager.h"
#include "SpecialFiles.h"

const RString INSTALLER_LANGUAGES_DIR = "Themes/_Installer/Languages/";

static const RString TEMP_MOUNT_POINT = "/@tempinstallpackage/";

static bool IsPackageFile( RString sFile )
{
	RString sOsDir, sFilename, sExt;
	splitpath( sFile, sOsDir, sFilename, sExt );

	return sExt.EqualsNoCase(".smzip")  ||  sExt.EqualsNoCase(".zip");
}

static void GetPackageFilesToInstall( vector<RString> &vs )
{
	int argc;
	char **argv;
	GetCommandLineArguments( argc, argv );

	for( int i = 1; i<argc; ++i )
		vs.push_back( argv[i] );
	
	bool bFoundOne = false;
	FOREACH( RString, vs, s )
	{
		if( IsPackageFile(*s) )
		{
			bFoundOne = true;
			break;
		}
	}
	if( !bFoundOne )
		vs.clear();
}

bool ExportStrings::AnyPackageFilesInCommandLine()
{
	vector<RString> vs;
	GetPackageFilesToInstall( vs );
	return !vs.empty();
}


struct FileCopyResult
{
	FileCopyResult( RString _sFile, RString _sComment ) : sFile(_sFile), sComment(_sComment) {}
	RString sFile, sComment;
};

void ExportStrings::Install()
{
	vector<FileCopyResult> vSucceeded;
	vector<FileCopyResult> vFailed;

	vector<RString> vs;
	GetPackageFilesToInstall( vs );
	FOREACH_CONST( RString, vs, s )
	{
		if( !IsPackageFile(*s) )
		{
			vFailed.push_back( FileCopyResult(*s,"wrong file extension") );
			continue;
		}

		RString sOsDir, sFilename, sExt;
		splitpath( *s, sOsDir, sFilename, sExt );

		FILEMAN->Mount( "dir", sOsDir, TEMP_MOUNT_POINT );

		// TODO: Validate that this zip contains files for this version of StepMania

		bool bFileExists = DoesFileExist( SpecialFiles::USER_PACKAGES_DIR + sFilename + sExt );
		if( FileCopy( TEMP_MOUNT_POINT + sFilename + sExt, SpecialFiles::USER_PACKAGES_DIR + sFilename + sExt ) )
			vSucceeded.push_back( FileCopyResult(*s,bFileExists ? "overwrote existing file" : "") );
		else
			vFailed.push_back( FileCopyResult(*s,ssprintf("error copying file to '%s'",sOsDir.c_str())) );

		FILEMAN->Unmount( "dir", sOsDir, TEMP_MOUNT_POINT );
	}
	if( vSucceeded.empty()  &&  vFailed.empty() )
	{
		Dialog::OK( "Install: no files specified" );
		return;
	}


	RString sMessage;
	for( int i=0; i<2; i++ )
	{
		const vector<FileCopyResult> &v = (i==0) ? vSucceeded : vFailed;
		if( !v.empty() )
		{
			sMessage += (i==0) ? "Install succeeded for:\n" : "Install failed for:\n";
			FOREACH_CONST( FileCopyResult, v, iter )
			{
				sMessage += "  - " + iter->sFile;
				if( !iter->sComment.empty() )
					sMessage += " : " + iter->sComment;
				sMessage += "\n";
			}
			sMessage += "\n";
		}
	}
	Dialog::OK( sMessage );
}

void ExportStrings::Nsis()
{
	RageFile out;
	if( !out.Open( "nsis_strings_temp.inc", RageFile::WRITE ) )
		RageException::Throw( "Error opening file for write." );
	
	vector<RString> vs;
	GetDirListing( INSTALLER_LANGUAGES_DIR + "*.ini", vs, false, false );
	FOREACH_CONST( RString, vs, s )
	{
		RString sThrowAway, sLangCode;
		splitpath( *s, sThrowAway, sLangCode, sThrowAway );
		const LanguageInfo *pLI = GetLanguageInfo( sLangCode );
		
		RString sLangNameUpper = pLI->szEnglishName;
		sLangNameUpper.MakeUpper();
		
		IniFile ini;
		if( !ini.ReadFile( INSTALLER_LANGUAGES_DIR + *s ) )
			RageException::Throw( "Error opening file for read." );
		FOREACH_CONST_Child( &ini, child )
		{
			FOREACH_CONST_Attr( child, attr )
			{
				RString sName = attr->first;
				RString sValue = attr->second->GetValue<RString>();
				sValue.Replace( "\\n", "$\\n" );
				RString sLine = ssprintf( "LangString %s ${LANG_%s} \"%s\"", sName.c_str(), sLangNameUpper.c_str(), sValue.c_str() );
				out.PutLine( sLine );
			}
		}
	}
}

void ExportStrings::LuaInformation()
{
	XNode *pNode = LuaHelpers::GetLuaInformation();
	pNode->AppendAttr( "xmlns", "http://www.stepmania.com" );
	pNode->AppendAttr( "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance" );
	pNode->AppendAttr( "xsi:schemaLocation", "http://www.stepmania.com Lua.xsd" );

	pNode->AppendChild( "Version", PRODUCT_ID_VER );
	pNode->AppendChild( "Date", DateTime::GetNowDate().GetString() );

	XmlFileUtil::SaveToFile( pNode, "Lua.xml", "Lua.xsl" );

	delete pNode;
}

/*
 * (c) 2006 Chris Danford, Steve Checkoway
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


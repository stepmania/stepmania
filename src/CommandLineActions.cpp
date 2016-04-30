#include "global.h"
#include "CommandLineActions.h"
#include "RageFile.h"
#include "RageUtil.h"
#include "IniFile.h"
#include "XmlFile.h"
#include "XmlFileUtil.h"
#include "LuaManager.h"
#include "ProductInfo.h"
#include "DateTime.h"
#include "arch/Dialog/Dialog.h"
#include "RageFileManager.h"
#include "SpecialFiles.h"
#include "FileDownload.h"
#include "arch/LoadingWindow/LoadingWindow.h"
#include "Preference.h"
#include "JsonUtil.h"
#include "ScreenInstallOverlay.h"
#include "ver.h"

// only used for Version()
#if defined(_WINDOWS)
#include <windows.h>
#include <conio.h>
#endif

using std::string;
using std::vector;

/** @brief The directory where languages should be installed. */
const std::string INSTALLER_LANGUAGES_DIR = "Themes/_Installer/Languages/";

vector<CommandLineActions::CommandLineArgs> CommandLineActions::ToProcess;

static void Nsis()
{
	RageFile out;
	if(!out.Open("nsis_strings_temp.inc", RageFile::WRITE))
	{
		RageException::Throw("Error opening file for write.");
	}
	vector<std::string> vs;
	GetDirListing(INSTALLER_LANGUAGES_DIR + "*.ini", vs, false, false);
	for (auto const &s: vs)
	{
		std::string sThrowAway, sLangCode;
		splitpath(s, sThrowAway, sLangCode, sThrowAway);
		const LanguageInfo *pLI = GetLanguageInfo(sLangCode);

		std::string sLangNameUpper = Rage::make_upper(pLI->englishName);

		IniFile ini;
		if(!ini.ReadFile(INSTALLER_LANGUAGES_DIR + s))
		{
			RageException::Throw("Error opening file for read.");
		}
		for (auto const *child: ini)
		{
			for (auto const &attr: child->m_attrs)
			{
				std::string sName = attr.first;
				std::string sValue = attr.second->GetValue<std::string>();
				Rage::replace(sValue, "\\n", "$\\n");
				std::string sLine = fmt::sprintf("LangString %s ${LANG_%s} \"%s\"", sName.c_str(), sLangNameUpper.c_str(), sValue.c_str());
				out.PutLine(sLine);
			}
		}
	}
}
static void LuaInformation()
{
	XNode *pNode = LuaHelpers::GetLuaInformation();
	pNode->AppendAttr("xmlns", "http://www.stepmania.com");
	pNode->AppendAttr("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
	pNode->AppendAttr("xsi:schemaLocation", "http://www.stepmania.com Lua.xsd");

	pNode->AppendChild("Version", string(PRODUCT_FAMILY) + product_version);
	pNode->AppendChild("Date", DateTime::GetNowDate().GetString());

	XmlFileUtil::SaveToFile(pNode, "Lua.xml", "Lua.xsl");

	delete pNode;
}

/**
 * @brief Print out version information.
 *
 * HACK: This function is primarily needed for Windows users.
 * Mac OS X and Linux print out version information on the command line
 * regardless of any preferences (tested by shakesoda on Mac). -aj */
static void Version()
{
	#if defined(WIN32)
		std::string sProductID = fmt::sprintf("%s", (string(PRODUCT_FAMILY) + product_version).c_str() );
		std::string sVersion = fmt::sprintf("build %s\nCompile Date: %s @ %s", ::sm_version_git_hash, version_date, version_time);

		AllocConsole();
		freopen("CONOUT$","wb", stdout);
		freopen("CONOUT$","wb", stderr);

		fprintf(stdout, "Version Information:\n%s %s\n", sProductID.c_str(), sVersion.c_str());
		fprintf(stdout, "Press any key to exit.");
		_getch();
	#endif // WIN32
}

void CommandLineActions::Handle(LoadingWindow*)
{
	CommandLineArgs args;
	for(int i=0; i<g_argc; ++i)
	{
		args.argv.push_back(g_argv[i]);
	}
	ToProcess.push_back(args);

	bool bExitAfter = false;
	if( GetCommandlineArgument("ExportNsisStrings") )
	{
		Nsis();
		bExitAfter = true;
	}
	if( GetCommandlineArgument("ExportLuaInformation") )
	{
		LuaInformation();
		bExitAfter = true;
	}
	if( GetCommandlineArgument("version") )
	{
		Version();
		bExitAfter = true;
	}
	if( bExitAfter )
		exit(0);
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


#define CO_EXIST_WITH_MFC
#include "global.h"
#include "stdafx.h"
#include "smpackage.h"
#include "smpackageExportDlg.h"
#include "smpackageInstallDlg.h"
#include "RageUtil.h"
#include "smpackageUtil.h"
#include "MainMenuDlg.h"
#include "RageFileManager.h"
#include "arch/arch.h"
#include "LuaManager.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CSmpackageApp, CWinApp)
	//{{AFX_MSG_MAP(CSmpackageApp)
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

CSmpackageApp::CSmpackageApp()
{
	// Place all significant initialization in InitInstance
}

CSmpackageApp theApp;

#include "RageLog.h"
#include "RageFileManager.h"
#include "archutils/Win32/SpecialDirs.h"
#include "ProductInfo.h"
extern RString GetLastErrorString();

BOOL CSmpackageApp::InitInstance()
{
	TCHAR szCurrentDirectory[MAX_PATH];
	GetCurrentDirectory( ARRAYSIZE(szCurrentDirectory), szCurrentDirectory );

	/* Almost everything uses this to read and write files.  Load this early. */
	FILEMAN = new RageFileManager( szCurrentDirectory );
	FILEMAN->MountInitialFilesystems();

	/* Set this up next.  Do this early, since it's needed for RageException::Throw. */
	LOG			= new RageLog();


	if( DoesFileExist("Songs") )	// this is a SM program directory
		SMPackageUtil::AddStepManiaInstallDir( szCurrentDirectory );	// add this if it doesn't already exist
	

	// check if there's a .smzip command line argument
	vector<RString> arrayCommandLineBits;
	split( ::GetCommandLine(), " ", arrayCommandLineBits );
	for( unsigned i=0; i<arrayCommandLineBits.size(); i++ )
	{
		CString sArg = arrayCommandLineBits[i];
		if( sArg == "--machine-profile-stats" )
		{
			RString sPersonalDir = SpecialDirs::GetMyDocumentsDir();
			RString sFile = sPersonalDir + PRODUCT_ID +"/Save/MachineProfile/Stats.xml";
			if( NULL == ::ShellExecute( NULL, "open", sFile, "", "", SW_SHOWNORMAL ) )
				AfxMessageBox( "Failed to open '" + sFile + "': " + GetLastErrorString() );
			exit(0);
		}
	}
	
	split( ::GetCommandLine(), "\"", arrayCommandLineBits );
	for( unsigned i=0; i<arrayCommandLineBits.size(); i++ )
	{
		RString sPath = arrayCommandLineBits[i];
		TrimLeft( sPath );
		TrimRight( sPath );
		RString sPathLower = sPath;
		sPathLower.MakeLower();

		// test to see if this is a smzip file
		if( sPathLower.Right(3) == "zip" )
		{
			if( !FILEMAN->DoesFileExist(sPath) )
			{
				AfxMessageBox( ssprintf("The file '%s' does not exist.  Aborting installation.",sPath), MB_ICONERROR );
				exit(0);
			}

			// We found a zip package.  Prompt the user to install it!
			CSMPackageInstallDlg dlg( CString(sPath.c_str()) );
			int nResponse = dlg.DoModal();
			if( nResponse == IDOK )
			{
				CSmpackageExportDlg dlg;
				int nResponse = dlg.DoModal();
				// Since the dialog has been closed, return FALSE so that we exit the
				//  application, rather than start the application's message pump.
				return FALSE;
			}
			else if (nResponse == IDCANCEL)
			{
				// the user cancelled.  Don't fall through to the Manager.
				exit(0);
			}
		}
	}

	FILEMAN = new RageFileManager( "" );
	LUA = new LuaManager();


	// Show the Manager Dialog
	MainMenuDlg dlg;
	int nResponse = dlg.DoModal();
//	if (nResponse == IDOK)


	SAFE_DELETE( LUA );
	SAFE_DELETE( FILEMAN );


	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

namespace StepMania
{
	void NORETURN HandleException( RString sErr )
	{
		MessageBox( NULL, "exception", sErr, MB_ICONERROR );
	}

}

/*
 * (c) 2002-2005 Chris Danford
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

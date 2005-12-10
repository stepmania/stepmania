// smpackage.cpp : Defines the class behaviors for the application.
//

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


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSmpackageApp

BEGIN_MESSAGE_MAP(CSmpackageApp, CWinApp)
	//{{AFX_MSG_MAP(CSmpackageApp)
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSmpackageApp construction

CSmpackageApp::CSmpackageApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CSmpackageApp object

CSmpackageApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CSmpackageApp initialization

BOOL CSmpackageApp::InitInstance()
{
	// Make sure the current directory is the root program directory

	// change dir to path of the execuctable
	TCHAR szFullAppPath[MAX_PATH];
	GetModuleFileName(NULL, szFullAppPath, MAX_PATH);
		
	// strip off executable name
	LPSTR pLastBackslash = strrchr(szFullAppPath, '\\');
	*pLastBackslash = '\0';	// terminate the string

	/* If "Program" is the top-level directory, strip it off. */
	pLastBackslash = strrchr( szFullAppPath, '\\' );
	if( pLastBackslash && !stricmp(pLastBackslash, "\\Program") )
		*pLastBackslash = '\0';

	SetCurrentDirectory(szFullAppPath);

	if( DoesFileExist("Songs") )	// this is a SM or DWI program directory
	{
		// make sure it's in the list of install directories
		TCHAR szCurrentDirectory[MAX_PATH];
		GetCurrentDirectory( MAX_PATH, szCurrentDirectory );
		SMPackageUtil::AddStepManiaInstallDir( szCurrentDirectory );
	}
	

	// check if there's a .smzip command line argument
	vector<RString> arrayCommandLineBits;
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


	// Show the Manager Dialog
	MainMenuDlg dlg;
	int nResponse = dlg.DoModal();
//	if (nResponse == IDOK)


	SAFE_DELETE( FILEMAN );


	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

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

BOOL CSmpackageApp::InitInstance()
{
	/* Almost everything uses this to read and write files.  Load this early. */
	FILEMAN = new RageFileManager( "" );
	FILEMAN->MountInitialFilesystems();

	/* Set this up next.  Do this early, since it's needed for RageException::Throw. */
	LOG			= new RageLog();


	if( DoesFileExist("Songs") )	// this is a SM program directory
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

namespace StepMania
{
	void NORETURN HandleException( RString sErr )
	{
		MessageBox( NULL, "exception", sErr, MB_ICONERROR );
	}

}

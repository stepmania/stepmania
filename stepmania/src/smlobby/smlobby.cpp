// smlobby.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "smlobby.h"
#include "smlobbyDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSmlobbyApp

BEGIN_MESSAGE_MAP(CSmlobbyApp, CWinApp)
	//{{AFX_MSG_MAP(CSmlobbyApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSmlobbyApp construction

CSmlobbyApp::CSmlobbyApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CSmlobbyApp object

CSmlobbyApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CSmlobbyApp initialization

BOOL CSmlobbyApp::InitInstance()
{
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	m_dlg = new CSmlobbyDlg(NULL);
	m_pMainWnd = m_dlg;

	m_dlg->DoModal();

	m_dlg->DestroyWindow();
	delete m_dlg;
	
	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;

}

// SMPackageInstallDlg.cpp : implementation file
//

#include "stdafx.h"
#include "smpackage.h"
#include "SMPackageInstallDlg.h"
#include "RageUtil.h"
#include "smpackageUtil.h"
#include "EditInsallations.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSMPackageInstallDlg dialog


CSMPackageInstallDlg::CSMPackageInstallDlg(CString sPackagePath, CWnd* pParent /*=NULL*/)
	: CDialog(CSMPackageInstallDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSMPackageInstallDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_sPackagePath = sPackagePath;
}


void CSMPackageInstallDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSMPackageInstallDlg)
	DDX_Control(pDX, IDC_BUTTON_EDIT, m_buttonEdit);
	DDX_Control(pDX, IDC_COMBO_DIR, m_comboDir);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSMPackageInstallDlg, CDialog)
	//{{AFX_MSG_MAP(CSMPackageInstallDlg)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BUTTON_EDIT, OnButtonEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSMPackageInstallDlg message handlers

BOOL CSMPackageInstallDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	int i;

	//
	// Set the text of the first Edit box
	//
	CString sMessage1 = ssprintf(
		"You have chosen to install the Stepmania package:\r\n"
		"\r\n"
		"\t%s\r\n"
		"\r\n"
		"This package contains the following files:\r\n",
		m_sPackagePath
	);
	CEdit* pEdit1 = (CEdit*)GetDlgItem(IDC_EDIT_MESSAGE1);
	pEdit1->SetWindowText( sMessage1 );


	//
	// Set the text of the second Edit box
	//
	CString sMessage2;
	try
	{	
		m_zip.Open( m_sPackagePath, CZipArchive::zipOpenReadOnly );
	}
	catch (CException* e)
	{
		AfxMessageBox( ssprintf("'%s' is not a valid zip archive.", m_sPackagePath), MB_ICONSTOP );
		e->Delete();
		exit( 1 );
	}

	for( i=0; i<m_zip.GetCount(); i++ )
	{
		CZipFileHeader fh;
		m_zip.GetFileInfo(fh, (WORD)i);

		if( fh.IsDirectory() )
			continue;

		sMessage2 += ssprintf( "\t%s\r\n", fh.GetFileName() );
	}
	CEdit* pEdit2 = (CEdit*)GetDlgItem(IDC_EDIT_MESSAGE2);
	pEdit2->SetWindowText( sMessage2 );


	//
	// Set the text of the third Edit box
	//
	CString sMessage3 = "The package will be installed in the following Stepmania program folder:\r\n";

	// Set the message
	CEdit* pEdit3 = (CEdit*)GetDlgItem(IDC_EDIT_MESSAGE3);
	pEdit3->SetWindowText( sMessage3 );



	TCHAR szCurrentDirectory[MAX_PATH];
	GetCurrentDirectory( MAX_PATH, szCurrentDirectory );
	AddStepManiaInstallDir( szCurrentDirectory );


	RefreshInstallationList();

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CSMPackageInstallDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}


void CSMPackageInstallDlg::OnOK() 
{
	// TODO: Add extra validation here

	m_comboDir.EnableWindow( FALSE );
	m_buttonEdit.EnableWindow( FALSE );

	CString sInstallDir;
	m_comboDir.GetWindowText( sInstallDir );
	
	// Unzip the SMzip package into the Stepmania installation folder
	for( int i=0; i<m_zip.GetCount(); i++ )
	{
		// Throw some text up so the user has something to look at during the long pause.
		CEdit* pEdit1 = (CEdit*)GetDlgItem(IDC_EDIT_MESSAGE1);
		pEdit1->SetWindowText( ssprintf("Installing '%s'.  Please wait...", m_sPackagePath) );
		CEdit* pEdit2 = (CEdit*)GetDlgItem(IDC_EDIT_MESSAGE2);
		pEdit2->SetWindowText( "" );
		CEdit* pEdit3 = (CEdit*)GetDlgItem(IDC_EDIT_MESSAGE3);
		pEdit3->SetWindowText( "" );
		SendMessage( WM_PAINT );
		UpdateWindow();		// Force the silly thing to hadle WM_PAINT now!


		// Extract the files
		try
		{	
			m_zip.ExtractFile( (WORD)i, sInstallDir, true );	// extract file to current directory
		}
		catch (CException* e)
		{
			e->ReportError();
			e->Delete();
			exit( 1 );
		}
	}

	AfxMessageBox( "Package installed successfully!" );

	// close the dialog
	CDialog::OnOK();
}


void CSMPackageInstallDlg::OnButtonEdit() 
{
	// TODO: Add your control notification handler code here
	EditInsallations dlg;
	int nResponse = dlg.DoModal();
	if( nResponse == IDOK )
	{
		WriteStepManiaInstallDirs( dlg.m_asReturnedInstallDirs );
		RefreshInstallationList();
	}
}


void CSMPackageInstallDlg::RefreshInstallationList() 
{
	m_comboDir.ResetContent();

	CStringArray asInstallDirs;
	GetStepManiaInstallDirs( asInstallDirs );
	for( int i=0; i<asInstallDirs.GetSize(); i++ )
	{
		m_comboDir.AddString( asInstallDirs[i] );
	}
	m_comboDir.SetCurSel( 0 );	// guaranteed to be at least one item
}


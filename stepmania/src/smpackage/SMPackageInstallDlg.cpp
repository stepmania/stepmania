// SMPackageInstallDlg.cpp : implementation file
//

#include "stdafx.h"
#include "smpackage.h"
#include "SMPackageInstallDlg.h"
#include "../RageUtil.h"

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
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSMPackageInstallDlg, CDialog)
	//{{AFX_MSG_MAP(CSMPackageInstallDlg)
	ON_WM_PAINT()
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

	CString sMessage;
	sMessage += ssprintf(
		"You have chosen to install the Stepmania package:\r\n"
		"\t%s\r\n"
		"\r\n"
		"This package contains the following files:\r\n",
		m_sPackagePath
	);

	// add all the files in the zip to the message
	try
	{	
		m_zip.Open( m_sPackagePath );
	}
	catch (CException* e)
	{
		AfxMessageBox( ssprintf("'%s' is not a valid zip archive.", m_sPackagePath), MB_ICONSTOP );
		e->Delete();
		exit( 1 );
	}

	for( int i=0; i<m_zip.GetNoEntries(); i++ )
	{
		if( m_zip.IsFileDirectory((WORD)i) )
			continue;

		CZipFileHeader fh;
		m_zip.GetFileInfo(fh, (WORD)i);
		sMessage += ssprintf( "\to %s\r\n", fh.GetFileName() );
	}

	TCHAR szCurrentDirectory[MAX_PATH];
	GetCurrentDirectory( MAX_PATH, szCurrentDirectory );
	sMessage += ssprintf(
		"\r\n"
		"The package will be installed in the Stepmania program folder:\r\n"
		"\t%s",
		szCurrentDirectory
	);

	// Set the message
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_MESSAGE);
	pEdit->SetWindowText( sMessage );


	
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

	TCHAR szCurrentDirectory[MAX_PATH];
	GetCurrentDirectory( MAX_PATH, szCurrentDirectory );

	// Unzip the SMzip package into the Stepmania installation folder
	for( int i=0; i<m_zip.GetNoEntries(); i++ )
	{
		CZipFileHeader fh;
		m_zip.GetFileInfo(fh, (WORD)i);
		//AfxMessageBox( ssprintf( "Trying to extract '%s'", fh.GetFileName()) );

		try
		{	
			m_zip.ExtractFile( (WORD)i, szCurrentDirectory, true );	// extract file to current directory
		}
		catch (CException* e)
		{
			AfxMessageBox( "Error extracting files", MB_ICONSTOP );
			e->Delete();
			exit( 1 );
		}
	}

	AfxMessageBox( "Package installed successfully!" );

	// close the dialog
	CDialog::OnOK();
}

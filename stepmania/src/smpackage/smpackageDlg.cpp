// smpackageDlg.cpp : implementation file
//

#include "stdafx.h"
#include "smpackage.h"
#include "smpackageDlg.h"
#include "../RageUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSmpackageDlg dialog

CSmpackageDlg::CSmpackageDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSmpackageDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSmpackageDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSmpackageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSmpackageDlg)
	DDX_Control(pDX, IDC_BUTTON_EXPORT, m_buttonExport);
	DDX_Control(pDX, IDC_LIST_SONGS, m_listBoxSongs);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSmpackageDlg, CDialog)
	//{{AFX_MSG_MAP(CSmpackageDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_LBN_SELCHANGE(IDC_LIST_SONGS, OnSelchangeListSongs)
	ON_BN_CLICKED(IDC_BUTTON_EXPORT, OnButtonExport)
	ON_BN_CLICKED(IDC_BUTTON_PLAY, OnButtonPlay)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSmpackageDlg message handlers

BOOL CSmpackageDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	//
	// TODO: Add extra initialization here
	//

	// Find all song directories
	CStringArray arrayGroupDirs;
	GetDirListing( "Songs\\*.*", arrayGroupDirs, true );
	
	for( int i=0; i<arrayGroupDirs.GetSize(); i++ )
	{
		CString sGroupName = arrayGroupDirs[i];

		CStringArray arraySongDirs;
		GetDirListing( "Songs\\" + sGroupName + "\\*.*", arraySongDirs, true );

		for( int j=0; j<arraySongDirs.GetSize(); j++ )
		{
			CString sSongName = arraySongDirs[j];

			m_listBoxSongs.AddString( sGroupName + "\\" + sSongName );
		}
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSmpackageDlg::OnPaint() 
{
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

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSmpackageDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CSmpackageDlg::OnSelchangeListSongs() 
{
	// TODO: Add your control notification handler code here
	int iCurSel = m_listBoxSongs.GetCurSel();
	if( iCurSel == LB_ERR )		// no song is selected
	{
		m_buttonExport.EnableWindow( false );
		return;
	}

	m_buttonExport.EnableWindow( true );
	
}

void CSmpackageDlg::OnButtonExport() 
{
	// TODO: Add your control notification handler code here

	// Get the path to the selected song folder
	int iCurSel = m_listBoxSongs.GetCurSel();
	CString sSongDir;
	m_listBoxSongs.GetText( iCurSel, sSongDir );
	sSongDir = "Songs\\" + sSongDir;
	//MessageBox( ssprintf("sSongDir is '%s'", sSongDir), "", MB_OK );


	// Extract the group name and song name
	CStringArray arrayPathBits;
	split( sSongDir, "\\", arrayPathBits );
	CString sGroupName = arrayPathBits[1];
	CString sSongName = arrayPathBits[2];
	CString sGroupDir = "Songs\\" + sGroupName;


	// Get path to desktop
	CString sDesktopPath = getenv("USERPROFILE");
	sDesktopPath += "\\Desktop\\";
	//MessageBox( ssprintf("Desktop path is '%s'", sDesktopPath), "", MB_OK );


	// Create a new zip file on the desktop
	CString sZipFileName = sSongName + ".smzip";
	sZipFileName.Replace( " ", "_" );
	sZipFileName.Replace( "!", "_" );
	sZipFileName.Replace( "@", "_" );
	sZipFileName.Replace( "#", "_" );
	sZipFileName.Replace( "$", "_" );
	sZipFileName.Replace( "%", "_" );
	sZipFileName.Replace( "^", "_" );
	sZipFileName.Replace( "&", "_" );
	sZipFileName.Replace( "*", "_" );
	sZipFileName.Replace( "(", "_" );
	sZipFileName.Replace( ")", "_" );
	sZipFileName.Replace( "+", "_" );
	sZipFileName.Replace( "=", "_" );
	sZipFileName.Replace( "[", "_" );
	sZipFileName.Replace( "]", "_" );
	sZipFileName.Replace( "{", "_" );
	sZipFileName.Replace( "}", "_" );
	sZipFileName.Replace( "|", "_" );
	sZipFileName.Replace( ":", "_" );
	sZipFileName.Replace( "\'","_" );
	sZipFileName.Replace( "\"","_" );
	sZipFileName.Replace( "<", "_" );
	sZipFileName.Replace( ">", "_" );
	sZipFileName.Replace( ",", "_" );
	sZipFileName.Replace( "?", "_" );
	sZipFileName.Replace( "/", "_" );
	CString sZipFilePath = sDesktopPath + sZipFileName;
	//MessageBox( ssprintf("sZipFilePath is '%s'", sZipFilePath), "", MB_OK );
	try
	{
		m_zip.Open( sZipFilePath, CZipArchive::create );
	}
	catch( CException* e )
	{
		MessageBox( ssprintf("Error creating zip file '%s'", sDesktopPath), "", MB_OK );
		m_zip.Close();
		e->Delete();
		return;
	}


	CStringArray arrayFiles;
	GetDirListing( sSongDir + "\\*.*", arrayFiles );
	//m_zip.AddNewFile( sRelPathToSelectedSong, 0, true );		// add the directory

	m_zip.AddNewFile( "Songs", 0, true );
	m_zip.AddNewFile( sGroupDir, 0, true );
	m_zip.AddNewFile( sSongDir, 0, true );

	for( int i=0; i<arrayFiles.GetSize(); i++ )
	{
		CString sFileName = arrayFiles[i];
		CString sFilePath = sSongDir + "\\" + sFileName;
		try
		{
			m_zip.AddNewFile( sFilePath, 9, true );
		}
		catch (CException* e)
		{
			MessageBox( ssprintf("Error adding file '%s'.", sFilePath), "", MB_OK );
			m_zip.Close();
			e->Delete();
			return;
		}	
	}

	AfxMessageBox( ssprintf("Successfully exported '%s' to '%s' on your Desktop.", sSongDir, sZipFileName) );


	m_zip.Close();

}



void CSmpackageDlg::OnButtonPlay() 
{
	// TODO: Add your control notification handler code here

	PROCESS_INFORMATION pi;
	STARTUPINFO	si;
	ZeroMemory( &si, sizeof(si) );

	CreateProcess(
		NULL,		// pointer to name of executable module
		"stepmania.exe",		// pointer to command line string
		NULL,  // process security attributes
		NULL,   // thread security attributes
		false,  // handle inheritance flag
		0, // creation flags
		NULL,  // pointer to new environment block
		NULL,   // pointer to current directory name
		&si,  // pointer to STARTUPINFO
		&pi  // pointer to PROCESS_INFORMATION
	);

}




// smpackageDlg.cpp : implementation file
//

#include "stdafx.h"
#include "smpackage.h"
#include "smpackageDlg.h"
#include "../RageUtil.h"
#include "ZipArchive\ZipArchive.h"	


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
	DDX_Control(pDX, IDC_BUTTON_EXPORT_AS_ONE, m_buttonExportAsOne);
	DDX_Control(pDX, IDC_BUTTON_EXPORT_AS_INDIVIDUAL, m_buttonExportAsIndividual);
	DDX_Control(pDX, IDC_LIST, m_listBox);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSmpackageDlg, CDialog)
	//{{AFX_MSG_MAP(CSmpackageDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_LBN_SELCHANGE(IDC_LIST, OnSelchangeList)
	ON_BN_CLICKED(IDC_BUTTON_EXPORT_AS_ONE, OnButtonExportAsOne)
	ON_BN_CLICKED(IDC_BUTTON_EXPORT_AS_INDIVIDUAL, OnButtonExportAsIndividual)
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

	//
	// Add announcers, themes, BGAnimations, random movies, and visualizations
	//
	{
		CStringArray as1;
		GetDirListing( "Announcers\\*.*", as1, true, true );
		GetDirListing( "Themes\\*.*", as1, true, true );
		GetDirListing( "BGAnimations\\*.*", as1, true, true );
		GetDirListing( "RandomMovies\\*.avi", as1, false, true );
		GetDirListing( "RandomMovies\\*.mpg", as1, false, true );
		GetDirListing( "RandomMovies\\*.mpeg", as1, false, true );
		GetDirListing( "Visualizations\\*.avi", as1, false, true );
		GetDirListing( "Visualizations\\*.mpg", as1, false, true );
		GetDirListing( "Visualizations\\*.mpeg", as1, false, true );
		for( int i=0; i<as1.GetSize(); i++ )
			m_listBox.AddString( as1[i] );
	}

	m_listBox.AddString( "Courses\\" );

	//
	// Add NoteSkins
	//
	{
		CStringArray as1;
		GetDirListing( "NoteSkins\\*.*", as1, true, true );
		for( int i=0; i<as1.GetSize(); i++ )
		{
			CStringArray as2;
			GetDirListing( as1[i] + "\\*.*", as2, true, true );
			for( int j=0; j<as2.GetSize(); j++ )
				m_listBox.AddString( as2[j] );
		}
	}

	//
	// Add Songs
	//
	{
		CStringArray as1;
		GetDirListing( "Songs\\*.*", as1, true, true );
		for( int i=0; i<as1.GetSize(); i++ )
		{
			CStringArray as2;
			GetDirListing( as1[i] + "\\*.*", as2, true, true );
			for( int j=0; j<as2.GetSize(); j++ )
				m_listBox.AddString( as2[j] );
		}
	}

	// Strip out "CVS"
	for( int i=m_listBox.GetCount()-1; i>=0; i-- )
	{
		CString sItemName;
		m_listBox.GetText( i, sItemName );
		if( -1!=sItemName.Find("CVS") )
			m_listBox.DeleteString( i );
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

void CSmpackageDlg::OnSelchangeList() 
{
	// TODO: Add your control notification handler code here

	if( m_listBox.GetSelCount() == LB_ERR )		// no song is selected
	{
		m_buttonExportAsOne.EnableWindow( false );
		m_buttonExportAsIndividual.EnableWindow( false );
	}
	else
	{
		m_buttonExportAsOne.EnableWindow( true );
		m_buttonExportAsIndividual.EnableWindow( true );
	}	
}

CString ReplaceInvalidFileNameChars( CString sOldFileName )
{
	CString sNewFileName = sOldFileName;
	const char charsToReplace[] = { 
		' ', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', 
		'+', '=', '[', ']', '{', '}', '|', ':', '\"', '\\',
		'<', '>', ',', '?', '/' 
	};
	for( int i=0; i<sizeof(charsToReplace); i++ )
		sNewFileName.Replace( charsToReplace[i], '_' );
	return sNewFileName;
}

void GetFilePathsInDirectory( CString sDir, CStringArray& asPathToFilesOut )
{
	CStringArray asDirectoriesToExplore;

	// HACK:
	// Must use backslashes in the path, or else WinZip and WinRAR don't see the files.
	// Not sure if this is ZipArchive's fault.

	GetDirListing( sDir + "\\*.*", asPathToFilesOut, false, true );
	GetDirListing( sDir + "\\*.*", asDirectoriesToExplore, true, true );
	while( asDirectoriesToExplore.GetSize() > 0 )
	{
		GetDirListing( asDirectoriesToExplore[0] + "\\*.*", asPathToFilesOut, false, true );
		GetDirListing( asDirectoriesToExplore[0] + "\\*.*", asDirectoriesToExplore, true, true );
		asDirectoriesToExplore.RemoveAt( 0 );
	}
}

bool ExportPackage( CString sPackageName, const CStringArray& asDirectoriesToExport )	
{
	CZipArchive zip;
	
	const CString sDesktopPath = CString(getenv("USERPROFILE")) + "/Desktop/";

	//
	// Create the package zip file
	//
	const CString sPackagePath = sDesktopPath + sPackageName;
	try
	{
		zip.Open( sPackagePath, CZipArchive::zipCreate );
	}
	catch( CException* e )
	{
		AfxMessageBox( ssprintf("Error creating zip file '%s'", sPackagePath) );
		zip.Close();
		e->Delete();
		return false;
	}


	//
	// Add files to zip
	//
	for( int i=0; i<asDirectoriesToExport.GetSize(); i++ )
	{
		CStringArray asFilePaths;
		GetFilePathsInDirectory( asDirectoriesToExport[i], asFilePaths );

		for( int j=0; j<asFilePaths.GetSize(); j++ )
		{
			CString sFilePath = asFilePaths[j];
			
			// don't export "thumbs.db" files or "CVS folders
			CString sDir, sFName, sExt;
			splitrelpath( sFilePath, sDir, sFName, sExt );
			if( 0==stricmp(sFName,"thumbs.db") )
				continue;	// skip
			if( 0==stricmp(sFName,"thumbs.db") )
				continue;	// skip

			try
			{
				zip.AddNewFile( sFilePath, Z_BEST_COMPRESSION, true );
			}
			catch (CException* e)
			{
				AfxMessageBox( ssprintf("Error adding file '%s'.", sFilePath) );
				zip.Close();
				e->Delete();
				return false;
			}
		}
	}

/*	CStringArray arrayFiles;
	GetDirListing( sSongDir + "\\*.*", arrayFiles );
	//m_zip.AddNewFile( sRelPathToSelectedSong, 0, true );		// add the directory

	m_zip.AddNewFile( "Songs", 0, true );
	m_zip.AddNewFile( sGroupDir, 0, true );
	m_zip.AddNewFile( sSongDir, 0, true );

	for( i=0; i<arrayFiles.GetSize(); i++ )
	{
		CString sFileName = arrayFiles[i];
		CString sFilePath = sSongDir + "/" + sFileName;
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
*/
	zip.Close();
	return true;
}


void CSmpackageDlg::OnButtonExportAsOne() 
{
	// TODO: Add your control notification handler code here

	ASSERT( m_listBox.GetSelCount() != LB_ERR );

	if( m_listBox.GetSelCount() == 1 )
	{
		OnButtonExportAsIndividual();
		return;
	}

	int i;

	// Generate a package name
	CString sPackageName;
	int iNumNameTries = 0;
try_another_name:
	iNumNameTries++;
	sPackageName = ReplaceInvalidFileNameChars( ssprintf("New Package %d.smzip",iNumNameTries) );
	CStringArray asFilesOnDesktop;
	const CString sDesktopPath = CString(getenv("USERPROFILE")) + "\\Desktop";
	GetDirListing( sDesktopPath + "\\*.*", asFilesOnDesktop );
	for( i=0; i<asFilesOnDesktop.GetSize(); i++ )
		if( stricmp(asFilesOnDesktop[i],sPackageName) == 0 )
			goto try_another_name;


	CStringArray asPathsToExport;
	int iSelectedItems[200];
	int iNumSelectedItems = m_listBox.GetSelItems( 200, iSelectedItems );
	for( i=0; i<iNumSelectedItems; i++ )
	{
		CString sPath;
		m_listBox.GetText( iSelectedItems[i], sPath );
		asPathsToExport.Add( sPath );
	}

	if( ExportPackage( sPackageName, asPathsToExport ) )
		AfxMessageBox( ssprintf("Successfully exported package '%s' to your Desktop.",sPackageName) );

	m_listBox.SetSel( -1, FALSE );
}

void CSmpackageDlg::OnButtonExportAsIndividual() 
{
	// TODO: Add your control notification handler code here

	ASSERT( m_listBox.GetSelCount() != LB_ERR );

	
	int iSelectedItems[200];
	int iNumSelectedItems = m_listBox.GetSelItems( 200, iSelectedItems );
	bool bAllSucceeded = true;
	CStringArray asExportedPackages;
	CStringArray asFailedPackages;
	for( int i=0; i<iNumSelectedItems; i++ )
	{		
		// Generate a package name for every path
		CString sPath;
		m_listBox.GetText( iSelectedItems[i], sPath );

		CString sPackageName;
		CStringArray asPathBits;
		split( sPath, "/", asPathBits, true );
		sPackageName = asPathBits[ asPathBits.GetSize()-1 ] + ".smzip";
		sPackageName = ReplaceInvalidFileNameChars( sPackageName );

		CStringArray asPathsToExport;
		asPathsToExport.Add( sPath );
		
		if( ExportPackage( sPackageName, asPathsToExport ) )
			asExportedPackages.Add( sPackageName );
		else
			asFailedPackages.Add( sPackageName );
	}

	CString sMessage = ssprintf("Successfully exported the packages %s to your Desktop.", join(", ",asExportedPackages) );
	if( asFailedPackages.GetSize() > 0 )
		sMessage += ssprintf("  The packages %s failed to export.", join(", ",asFailedPackages) );
	AfxMessageBox( sMessage );

	m_listBox.SetSel( -1, FALSE );
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




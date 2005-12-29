// SmpackageExportDlg.cpp : implementation file
//

#define CO_EXIST_WITH_MFC
#include "global.h"
#include "stdafx.h"
#include "smpackage.h"
#include "SmpackageExportDlg.h"
#include "RageUtil.h"
#include "ZipArchive\ZipArchive.h"	
#include "EnterName.h"	
#include "EnterComment.h"	
#include "smpackageUtil.h"	
#include "EditInsallations.h"	
#include "IniFile.h"	
#include "RageFileDriverMemory.h"
#include "archutils/Win32/SpecialDirs.h"

#include <vector>
#include <algorithm>
#include <set>
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSmpackageExportDlg dialog


CSmpackageExportDlg::CSmpackageExportDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSmpackageExportDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSmpackageExportDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSmpackageExportDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSmpackageExportDlg)
	DDX_Control(pDX, IDC_COMBO_DIR, m_comboDir);
	DDX_Control(pDX, IDC_BUTTON_EXPORT_AS_INDIVIDUAL, m_buttonExportAsIndividual);
	DDX_Control(pDX, IDC_BUTTON_EXPORT_AS_ONE, m_buttonExportAsOne);
	DDX_Control(pDX, IDC_TREE, m_tree);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSmpackageExportDlg, CDialog)
	//{{AFX_MSG_MAP(CSmpackageExportDlg)
	ON_BN_CLICKED(IDC_BUTTON_EXPORT_AS_ONE, OnButtonExportAsOne)
	ON_BN_CLICKED(IDC_BUTTON_EXPORT_AS_INDIVIDUAL, OnButtonExportAsIndividual)
	ON_BN_CLICKED(IDC_BUTTON_PLAY, OnButtonPlay)
	ON_BN_CLICKED(IDC_BUTTON_EDIT, OnButtonEdit)
	ON_CBN_SELCHANGE(IDC_COMBO_DIR, OnSelchangeComboDir)
	ON_BN_CLICKED(IDC_BUTTON_OPEN, OnButtonOpen)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSmpackageExportDlg message handlers

BOOL CSmpackageExportDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	//

	RefreshInstallationList();
	
	RefreshTree();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

RString ReplaceInvalidFileNameChars( RString sOldFileName )
{
	RString sNewFileName = sOldFileName;
	const char charsToReplace[] = { 
		' ', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', 
		'+', '=', '[', ']', '{', '}', '|', ':', '\"', '\\',
		'<', '>', ',', '?', '/' 
	};
	for( int i=0; i<sizeof(charsToReplace); i++ )
		sNewFileName.Replace( charsToReplace[i], '_' );
	return sNewFileName;
}

static bool ExportPackage( RString sPackageName, const vector<RString>& asDirectoriesToExport, RString sComment )	
{
	CZipArchive zip;
	
	//
	// Create the package zip file
	//
	const RString sPackagePath = "Desktop/" + sPackageName;
	try
	{
		zip.Open( sPackagePath, CZipArchive::zipCreate );
	}
	catch( CException* e )
	{
		e->ReportError();
		zip.Close();
		e->Delete();
		return false;
	}


	zip.SetGlobalComment( sComment );

	/* Find files to add to zip. */
	vector<RString> asFilePaths;
	for( unsigned i=0; i<asDirectoriesToExport.size(); i++ )
		GetDirListingRecursive( asDirectoriesToExport[i], "*.*", asFilePaths );

	// Must use backslashes in the path, or else WinZip and WinRAR don't see the files.
	// Not sure if this is ZipArchive's fault.
	//;XXX

	{
		IniFile ini;
		ini.SetValue( "SMZIP", "Version", 1 );

		set<RString> Directories;
		for( i=0; i<asFilePaths.size(); i++ )
		{
			const RString name = SMPackageUtil::GetPackageDirectory( asFilePaths[i] );
			if( name != "" )
				Directories.insert( name );
		}

		set<RString>::const_iterator it;
		int num = 0;
		for( it = Directories.begin(); it != Directories.end(); ++it )
			ini.SetValue( "Packages", ssprintf("%i", num++), *it );
		ini.SetValue( "Packages", "NumPackages", num );

		RageFileObjMem f;
		ini.WriteFile( f );
		RString buf = f.GetString();

		CZipMemFile control;
		control.Write( buf.GetBuffer(0), buf.size() );

		control.Seek( 0, CZipAbstractFile::begin );
		zip.AddNewFile( control, "smzip.ctl" );
	}

	//
	// Add files to zip
	//
	for( unsigned j=0; j<asFilePaths.size(); j++ )
	{
		RString sFilePath = asFilePaths[j];
		
		// don't export "thumbs.db" files or "CVS" folders
		if( sFilePath.find("CVS") != string::npos )
			continue;	// skip
		if( sFilePath.find("Thumbs.db") != string::npos )
			continue;	// skip

		RString sExt = GetExtension( sFilePath );
		bool bUseCompression = true;
		if( sExt.CompareNoCase("avi")==0 ||
			sExt.CompareNoCase("mpeg")==0 ||
			sExt.CompareNoCase("mpg")==0 ||
			sExt.CompareNoCase("mp3")==0 ||
			sExt.CompareNoCase("ogg")==0 ||
			sExt.CompareNoCase("gif")==0 ||
			sExt.CompareNoCase("jpg")==0 ||
			sExt.CompareNoCase("png")==0 )
			bUseCompression = false;

		try
		{
			zip.AddNewFile( sFilePath, bUseCompression?Z_BEST_COMPRESSION:Z_NO_COMPRESSION, true );
		}
		catch (CException* e)
		{
			AfxMessageBox( ssprintf("Error adding file '%s'.", sFilePath) );
			zip.Close();
			e->Delete();
			return false;
		}
	}

	zip.Close();
	return true;
}

bool CSmpackageExportDlg::MakeComment( RString &comment )
{
	bool DontAskForComment;
	if( SMPackageUtil::GetPref("DontAskForComment", DontAskForComment) && DontAskForComment )
	{
		comment = "";
		return true;
	}

	EnterComment commentDlg;
	int nResponse = commentDlg.DoModal();
	if( nResponse != IDOK )
		return false;	// cancelled

	comment = commentDlg.m_sEnteredComment;
	if( commentDlg.m_bDontAsk )
		SMPackageUtil::SetPref( "DontAskForComment", true );

	return true;
}

void CSmpackageExportDlg::OnButtonExportAsOne() 
{
	vector<RString> asPaths;
	GetCheckedPaths( asPaths );

	if( asPaths.size() == 0 )
	{
		AfxMessageBox( "No items are checked" );
		return;
	}
	else if( asPaths.size() == 1 )
	{
		OnButtonExportAsIndividual();
		return;
	}

	// Generate a package name
	RString sPackageName;
	EnterName nameDlg;
	int nResponse = nameDlg.DoModal();
	if( nResponse != IDOK )
		return;	// cancelled
	sPackageName = nameDlg.m_sEnteredName;
	sPackageName = ReplaceInvalidFileNameChars( sPackageName+".smzip" );

	// Generate a comment
	RString sComment;
	if( !MakeComment(sComment) )
		return;		// cancelled

	if( ExportPackage( sPackageName, asPaths, sComment ) )
		AfxMessageBox( ssprintf("Successfully exported package '%s' to your Desktop.",sPackageName) );
}

void CSmpackageExportDlg::OnButtonExportAsIndividual() 
{
	vector<RString> asPaths;
	GetCheckedPaths( asPaths );

	if( asPaths.size() == 0 )
	{
		AfxMessageBox( "No items are checked" );
		return;
	}
	
	// Generate a comment
	RString sComment;
	if( !MakeComment(sComment) )
		return;		// cancelled

	vector<RString> asExportedPackages;
	vector<RString> asFailedPackages;
	for( unsigned i=0; i<asPaths.size(); i++ )
	{		
		// Generate a package name for every path
		RString sPath = asPaths[i];

		RString sPackageName;
		vector<RString> asPathBits;
		split( sPath, "\\", asPathBits, true );
		sPackageName = asPathBits[ asPathBits.size()-1 ] + ".smzip";
		sPackageName = ReplaceInvalidFileNameChars( sPackageName );

		vector<RString> asPathsToExport;
		asPathsToExport.push_back( sPath );
		
		if( ExportPackage( sPackageName, asPathsToExport, sComment ) )
			asExportedPackages.push_back( sPackageName );
		else
			asFailedPackages.push_back( sPackageName );
	}

	RString sMessage;
	if( asFailedPackages.size() == 0 )
		sMessage = ssprintf("Successfully exported the package%s '%s' to your Desktop.", asFailedPackages.size()>1?"s":"", join("', '",asExportedPackages) );
	else
		sMessage = ssprintf("  The packages %s failed to export.", join(", ",asFailedPackages) );
	AfxMessageBox( sMessage );
}

void CSmpackageExportDlg::OnButtonPlay() 
{
	// TODO: Add your control notification handler code here
	SMPackageUtil::LaunchGame();
	exit(0);
}

void CSmpackageExportDlg::GetTreeItems( CArray<HTREEITEM,HTREEITEM>& aItemsOut )
{
	CArray<HTREEITEM,HTREEITEM> aRootsToExplore;	
	
	// add all top-level roots
	HTREEITEM item = m_tree.GetRootItem();
	while( item != NULL )
	{
		aRootsToExplore.Add( item );
		item = m_tree.GetNextSiblingItem( item );
	}

	while( aRootsToExplore.GetSize() > 0 )
	{
		HTREEITEM item = aRootsToExplore[0];
		aRootsToExplore.RemoveAt( 0 );
		aItemsOut.Add( item );

		HTREEITEM child = m_tree.GetChildItem( item );
		while( child != NULL )
		{
			aRootsToExplore.Add( child );
			child = m_tree.GetNextSiblingItem( child );
		}
	}
}

void CSmpackageExportDlg::GetCheckedTreeItems( CArray<HTREEITEM,HTREEITEM>& aCheckedItemsOut )
{
	CArray<HTREEITEM,HTREEITEM> aItems;	

	GetTreeItems( aItems );
	for( int i=0; i<aItems.GetSize(); i++ )
		if( m_tree.GetCheck(aItems[i]) )
			aCheckedItemsOut.Add( aItems[i] );
}

void CSmpackageExportDlg::GetCheckedPaths( vector<RString>& aPathsOut )
{
	CArray<HTREEITEM,HTREEITEM> aItems;	

	GetCheckedTreeItems( aItems );
	for( int i=0; i<aItems.GetSize(); i++ )
	{
		HTREEITEM item = aItems[i];

		RString sPath;
		
		while( item )
		{
			sPath = (LPCTSTR)m_tree.GetItemText(item) + '\\' + sPath;
			item = m_tree.GetParentItem(item);
		}

		TrimRight( sPath, "\\" );	// strip off last slash

		aPathsOut.push_back( sPath );
	}
}


void CSmpackageExportDlg::OnButtonEdit() 
{
	// TODO: Add your control notification handler code here
	EditInsallations dlg;
	int nResponse = dlg.DoModal();
	if( nResponse == IDOK )
	{
		SMPackageUtil::WriteStepManiaInstallDirs( dlg.m_vsReturnedInstallDirs );
		RefreshInstallationList();
		RefreshTree();
	}
}

void CSmpackageExportDlg::RefreshInstallationList() 
{
	m_comboDir.ResetContent();

	vector<RString> asInstallDirs;
	SMPackageUtil::GetStepManiaInstallDirs( asInstallDirs );
	for( unsigned i=0; i<asInstallDirs.size(); i++ )
	{
		m_comboDir.AddString( asInstallDirs[i] );
	}
	m_comboDir.SetCurSel( 0 );	// guaranteed to be at least one item
}

void CSmpackageExportDlg::OnSelchangeComboDir() 
{
	// TODO: Add your control notification handler code here
	RefreshTree();
}

void CSmpackageExportDlg::RefreshTree()
{
	m_tree.DeleteAllItems();

	RString sDir;
	{
		CString s;
		m_comboDir.GetWindowText( s );
		sDir = s;
	}

	SetCurrentDirectory( sDir );

	// Add announcers
	{
		vector<RString> as1;
		HTREEITEM item1 = m_tree.InsertItem( "Announcers" );
		GetDirListing( "Announcers\\*.*", as1, true, false );
		for( unsigned i=0; i<as1.size(); i++ )
			m_tree.InsertItem( as1[i], item1 );
	}

	// Add characters
	{
		vector<RString> as1;
		HTREEITEM item1 = m_tree.InsertItem( "Characters" );
		GetDirListing( "Characters\\*.*", as1, true, false );
		for( unsigned i=0; i<as1.size(); i++ )
			m_tree.InsertItem( as1[i], item1 );
	}

	// Add themes
	{
		vector<RString> as1;
		HTREEITEM item1 = m_tree.InsertItem( "Themes" );
		GetDirListing( "Themes\\*.*", as1, true, false );
		for( unsigned i=0; i<as1.size(); i++ )
			m_tree.InsertItem( as1[i], item1 );
	}

	// Add BGAnimations
	{
		vector<RString> as1;
		HTREEITEM item1 = m_tree.InsertItem( "BGAnimations" );
		GetDirListing( "BGAnimations\\*.*", as1, true, false );
		for( unsigned i=0; i<as1.size(); i++ )
			m_tree.InsertItem( as1[i], item1 );
	}

	// Add RandomMovies
	{
		vector<RString> as1;
		HTREEITEM item1 = m_tree.InsertItem( "RandomMovies" );
		GetDirListing( "RandomMovies\\*.avi", as1, false, false );
		GetDirListing( "RandomMovies\\*.mpg", as1, false, false );
		GetDirListing( "RandomMovies\\*.mpeg", as1, false, false );
		for( unsigned i=0; i<as1.size(); i++ )
			m_tree.InsertItem( as1[i], item1 );
	}

	// Add visualizations
	{
		vector<RString> as1;
		HTREEITEM item1 = m_tree.InsertItem( "Visualizations" );
		GetDirListing( "Visualizations\\*.avi", as1, false, false );
		GetDirListing( "Visualizations\\*.mpg", as1, false, false );
		GetDirListing( "Visualizations\\*.mpeg", as1, false, false );
		for( unsigned i=0; i<as1.size(); i++ )
			m_tree.InsertItem( as1[i], item1 );
	}

	// Add courses
	{
		vector<RString> as1;
		HTREEITEM item1 = m_tree.InsertItem( "Courses" );
		GetDirListing( "Courses\\*.crs", as1, false, false );
		for( unsigned i=0; i<as1.size(); i++ )
		{
			as1[i] = as1[i].Left(as1[i].size()-4);	// strip off ".crs"
			m_tree.InsertItem( as1[i], item1 );
		}
	}


	//
	// Add NoteSkins
	//
	{
		vector<RString> as1;
		HTREEITEM item1 = m_tree.InsertItem( "NoteSkins" );
		GetDirListing( "NoteSkins\\*.*", as1, true, false );
		for( unsigned i=0; i<as1.size(); i++ )
		{
			vector<RString> as2;
			HTREEITEM item2 = m_tree.InsertItem( as1[i], item1 );
			GetDirListing( "NoteSkins\\" + as1[i] + "\\*.*", as2, true, false );
			for( unsigned j=0; j<as2.size(); j++ )
				m_tree.InsertItem( as2[j], item2 );
		}
	}

	//
	// Add Songs
	//
	{
		vector<RString> as1;
		HTREEITEM item1 = m_tree.InsertItem( "Songs" );
		GetDirListing( "Songs\\*.*", as1, true, false );
		for( unsigned i=0; i<as1.size(); i++ )
		{
			vector<RString> as2;
			HTREEITEM item2 = m_tree.InsertItem( as1[i], item1 );
			GetDirListing( "Songs\\" + as1[i] + "\\*.*", as2, true, false );
			for( unsigned j=0; j<as2.size(); j++ )
				m_tree.InsertItem( as2[j], item2 );
		}
	}


	// Strip out any "CVS" items
	CArray<HTREEITEM,HTREEITEM> aItems;
	GetTreeItems( aItems );
	for( int i=0; i<aItems.GetSize(); i++ )
		if( m_tree.GetItemText(aItems[i]).CompareNoCase("CVS")==0 )
			m_tree.DeleteItem( aItems[i] );
}

void CSmpackageExportDlg::OnButtonOpen() 
{
	// TODO: Add your control notification handler code here
	
	char szCurDir[MAX_PATH];
	GetCurrentDirectory( MAX_PATH, szCurDir );

	char szCommandToExecute[MAX_PATH] = "explorer ";
	strcat( szCommandToExecute, szCurDir );


	PROCESS_INFORMATION pi;
	STARTUPINFO	si;
	ZeroMemory( &si, sizeof(si) );

	CreateProcess(
		NULL,		// pointer to name of executable module
		szCommandToExecute,		// pointer to command line string
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

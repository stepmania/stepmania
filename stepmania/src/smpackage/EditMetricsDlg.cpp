// EditMetricsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "smpackage.h"
#include "EditMetricsDlg.h"
#include "IniFile.h"
#include "RageUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define BLACK	RGB(0,0,0)
#define RED		RGB(255,0,0)
#define SET_ITEM_STYLE(item,bInBase,bInTheme) {m_tree.SetItemColor(item,bInBase?BLACK:RED);m_tree.SetItemBold(item,bInTheme);}

/////////////////////////////////////////////////////////////////////////////
// EditMetricsDlg dialog


EditMetricsDlg::EditMetricsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(EditMetricsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(EditMetricsDlg)
	//}}AFX_DATA_INIT
}


void EditMetricsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(EditMetricsDlg)
	DDX_Control(pDX, IDC_EDIT_VALUE, m_editValue);
	DDX_Control(pDX, IDC_EDIT_DEFAULT, m_editDefault);
	DDX_Control(pDX, IDC_BUTTON_REMOVE, m_buttonRemove);
	DDX_Control(pDX, IDC_BUTTON_OVERRIDE, m_buttonOverride);
	DDX_Control(pDX, IDC_TREE, m_tree);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(EditMetricsDlg, CDialog)
	//{{AFX_MSG_MAP(EditMetricsDlg)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE, OnSelchangedTree)
	ON_BN_CLICKED(IDC_BUTTON_OVERRIDE, OnButtonOverride)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, OnButtonRemove)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnButtonSave)
	ON_EN_CHANGE(IDC_EDIT_VALUE, OnChangeEditValue)
	ON_BN_CLICKED(IDC_BUTTON_HELP, OnButtonHelp)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE, OnDblclkTree)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, OnButtonClose)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// EditMetricsDlg message handlers

BOOL EditMetricsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here

	RefreshTree();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

IniFile iniBase;
IniFile iniTheme;

void EditMetricsDlg::RefreshTree()
{
	m_tree.DeleteAllItems();

	iniBase.Reset();
	iniBase.SetPath( "Themes\\default\\metrics.ini" );
	iniBase.ReadFile();

	iniTheme.Reset();
	iniTheme.SetPath( "Themes\\"+m_sTheme+"\\metrics.ini" );
	iniTheme.ReadFile();

	IniFile iniCombined;
	iniCombined.SetPath( "Themes\\default\\metrics.ini" );
	iniCombined.ReadFile();
	iniCombined.SetPath( "Themes\\"+m_sTheme+"\\metrics.ini" );
	iniCombined.ReadFile();

	CStringArray asKeys;
	IniFile::const_iterator it;
	for( it = iniCombined.begin(); it != iniCombined.end(); ++it )
		asKeys.push_back( it->first );
	SortCStringArray( asKeys );

	for( unsigned i=0; i<asKeys.size(); i++ )
	{
		CString sKey = asKeys[i];
		bool bInBase = iniBase.GetKey(sKey) != NULL;
		bool bInTheme = iniTheme.GetKey(sKey) != NULL;

		HTREEITEM item1 = m_tree.InsertItem( sKey );
		SET_ITEM_STYLE( item1, bInBase, bInTheme );

		const IniFile::key* pKey = iniCombined.GetKey( sKey );
		CStringArray asNames;
		for( IniFile::key::const_iterator val = pKey->begin(); val != pKey->end(); ++val )
		{
			CString sName = val->first, sValue = val->second;
			asNames.push_back( sName );
		}
	
		SortCStringArray( asNames );

		for( unsigned j=0; j<asNames.size(); j++ )
		{
			CString sName = asNames[j];
			CString sValue;
			iniCombined.GetValue( sKey, sName, sValue );

			CString sThrowAway;
			bool bInBase = !!iniBase.GetValue( sKey, sName, sThrowAway );
			bool bInTheme = !!iniTheme.GetValue( sKey, sName, sThrowAway );

			HTREEITEM item2 = m_tree.InsertItem( sName, item1 );
			SET_ITEM_STYLE( item2, bInBase, bInTheme );
		}
	}
}

void EditMetricsDlg::GetSelectedKeyAndName( CString& sKeyOut, CString& sNameOut )
{
	sKeyOut = "";
	sNameOut = "";

	HTREEITEM itemSel = m_tree.GetSelectedItem();
	if( itemSel == NULL )
		return;

	HTREEITEM itemParent = m_tree.GetParentItem( itemSel );
	if( itemParent == NULL )
	{
		sKeyOut = m_tree.GetItemText( itemSel );
		return;
	}

	sKeyOut = m_tree.GetItemText( itemParent );
	sNameOut = m_tree.GetItemText( itemSel );
}

void EditMetricsDlg::OnSelchangedTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	
	CString sKey, sName;
	GetSelectedKeyAndName( sKey, sName );

	if( sName == "" )
	{
		m_buttonOverride.EnableWindow( false );
		m_buttonRemove.EnableWindow( false );
		m_editValue.EnableWindow( false );
		m_editValue.SetWindowText( "" );
		m_editDefault.SetWindowText( "" );
		return;
	}
	
	CString sThrowAway;
	bool bInBase = !!iniBase.GetValue( sKey, sName, sThrowAway );
	bool bInTheme = !!iniTheme.GetValue( sKey, sName, sThrowAway );

	m_buttonOverride.EnableWindow( bInBase && !bInTheme );
	m_buttonRemove.EnableWindow( bInTheme );
	m_editValue.EnableWindow( bInTheme );
	if( bInTheme )
	{
		CString n;
		iniTheme.GetValue(sKey,sName, n);
		m_editValue.SetWindowText( n );
		m_editDefault.SetWindowText( n );
	} else {
		m_editValue.SetWindowText( "" );
		m_editDefault.SetWindowText( "" );
	}

	*pResult = 0;
}

void EditMetricsDlg::OnButtonOverride() 
{
	// TODO: Add your control notification handler code here
	CString sKey, sName;
	GetSelectedKeyAndName( sKey, sName );

	HTREEITEM itemSel = m_tree.GetSelectedItem();
	HTREEITEM itemParent = m_tree.GetParentItem( itemSel );
	
	SET_ITEM_STYLE( itemSel, true, true );
	m_tree.RedrawWindow();

	m_editValue.EnableWindow( true );
	m_editValue.SetFocus();
	m_buttonOverride.EnableWindow( false );
	m_buttonRemove.EnableWindow( true );

	iniTheme.SetValue( sKey, sName, "" );
}

void EditMetricsDlg::OnButtonRemove() 
{
	// TODO: Add your control notification handler code here
	CString sKey, sName;
	GetSelectedKeyAndName( sKey, sName );

	HTREEITEM itemSel = m_tree.GetSelectedItem();
	
	CString sThrowAway;
	bool bInBase = !!iniBase.GetValue( sKey, sName, sThrowAway );

	if( bInBase )
	{
		m_tree.SetItemColor( itemSel, BLACK );
		m_tree.SetItemBold( itemSel, false );
		m_tree.RedrawWindow();
		m_buttonOverride.EnableWindow( true );
		m_buttonRemove.EnableWindow( false );
		m_editValue.EnableWindow( false );
	}
	else
		m_tree.DeleteItem( itemSel );

	iniTheme.DeleteValue( sKey, sName );
}

void EditMetricsDlg::OnButtonSave() 
{
	// TODO: Add your control notification handler code here
	iniTheme.WriteFile();
}

void EditMetricsDlg::OnChangeEditValue() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	CString sKey, sName;
	GetSelectedKeyAndName( sKey, sName );

	CString sText;
	m_editValue.GetWindowText( sText );
	iniTheme.SetValue( sKey, sName, sText );	
}

void EditMetricsDlg::OnButtonHelp() 
{
	// TODO: Add your control notification handler code here
	AfxMessageBox( 
		"Bold = A metric that is overridden by the current theme (exists in both the base theme, and the current theme)\n\n"
		"Not Bold = A metric that is not overridden by the current theme (exists in the base theme but not in the current theme)\n\n"
		"Red = A metric that exists in the current theme but not in the base theme (check to see if the metric name is misspelled)"
		 );	
}

void EditMetricsDlg::OnDblclkTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	
	OnButtonOverride();

	*pResult = 0;
}

void EditMetricsDlg::OnButtonClose() 
{
	// TODO: Add your control notification handler code here
	EndDialog( IDOK );	
}

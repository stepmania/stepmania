// EditMetricsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "smpackage.h"
#include "EditMetricsDlg.h"
#include "IniFile.h"

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
	ON_EN_KILLFOCUS(IDC_EDIT_VALUE, OnKillfocusEditValue)
	ON_BN_CLICKED(IDC_BUTTON_OVERRIDE, OnButtonOverride)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, OnButtonRemove)
	ON_BN_CLICKED(IDC_BUTTON_REFRESH, OnButtonRefresh)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnButtonSave)
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

	for( int i=0; i<iniCombined.names.GetSize(); i++ )
	{
		CString sKey = iniCombined.names[i];
		bool bInBase = iniBase.FindKey(sKey) != -1;
		bool bInTheme = iniTheme.FindKey(sKey) != -1;

		HTREEITEM item1 = m_tree.InsertItem( sKey );
		SET_ITEM_STYLE( item1, bInBase, bInTheme );

		IniFile::key &key = iniCombined.keys[i];
		for( POSITION pos=key.GetStartPosition(); pos != NULL; )
		{
			CString sName, sValue;
			key.GetNextAssoc( pos, sName, sValue );

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
	m_editValue.SetWindowText( bInTheme ? iniTheme.GetValue(sKey,sName) : "" );
	m_editDefault.SetWindowText( bInBase ? iniBase.GetValue(sKey,sName) : "" );

	*pResult = 0;
}

void EditMetricsDlg::OnKillfocusEditValue() 
{
	// TODO: Add your control notification handler code here
	CString sKey, sName;
	GetSelectedKeyAndName( sKey, sName );

	CString sText;
	m_editValue.GetWindowText( sText );
	iniBase.SetValue( sKey, sName, sText );
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
	}
	else
		m_tree.DeleteItem( itemSel );

	iniTheme.DeleteValue( sKey, sName );
}

void EditMetricsDlg::OnButtonRefresh() 
{
	// TODO: Add your control notification handler code here
	OnButtonSave();
	RefreshTree();
	m_buttonOverride.EnableWindow( false );
	m_buttonRemove.EnableWindow( false );
	m_editValue.EnableWindow( false );
}

void EditMetricsDlg::OnButtonSave() 
{
	// TODO: Add your control notification handler code here
	iniTheme.WriteFile();
}

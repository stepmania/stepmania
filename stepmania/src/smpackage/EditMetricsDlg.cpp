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
	DDX_Control(pDX, IDC_BUTTON_REMOVE, m_buttonRemove);
	DDX_Control(pDX, IDC_BUTTON_OVERRIDE, m_buttonOverride);
	DDX_Control(pDX, IDC_BUTTON_NEW, m_buttonNew);
	DDX_Control(pDX, IDC_TREE, m_tree);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(EditMetricsDlg, CDialog)
	//{{AFX_MSG_MAP(EditMetricsDlg)
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

unsigned long GetColor( bool bInBase, bool bInTheme )
{
	if( bInBase && bInTheme )
		return RGB(0,0,0);
	else if( bInBase && !bInTheme )
		return RGB(127,127,127);
	else if( !bInBase && bInTheme )
		return RGB(255,0,0);
	else
		ASSERT(0);
	return RGB(0,0,0);
}

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
		m_tree.SetItemColor( item1, GetColor(bInBase,bInTheme) );

		IniFile::key &key = iniCombined.keys[i];
		for( POSITION pos=key.GetStartPosition(); pos != NULL; )
		{
			CString sName, sValue;
			key.GetNextAssoc( pos, sName, sValue );

			CString sThrowAway;
			bool bInBase = !!iniBase.GetValue( sKey, sName, sThrowAway );
			bool bInTheme = !!iniTheme.GetValue( sKey, sName, sThrowAway );

			HTREEITEM item2 = m_tree.InsertItem( sName, item1 );
			m_tree.SetItemColor( item2, GetColor(bInBase,bInTheme) );
		}
	}
}

// CreateLanguageDlg.cpp : implementation file
//

#define CO_EXIST_WITH_MFC
#include "global.h"
#include "stdafx.h"
#include "smpackage.h"
#include "CreateLanguageDlg.h"
#include "RageUtil.h"
#include "SMPackageUtil.h"
#include ".\createlanguagedlg.h"
#include "archutils/Win32/DialogUtil.h"

// CreateLanguageDlg dialog

IMPLEMENT_DYNAMIC(CreateLanguageDlg, CDialog)
CreateLanguageDlg::CreateLanguageDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CreateLanguageDlg::IDD, pParent)
{
}

CreateLanguageDlg::~CreateLanguageDlg()
{
}

void CreateLanguageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_LANGUAGES, m_comboLanguages);
}

BOOL CreateLanguageDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO: Add extra initialization here
	DialogUtil::LocalizeDialogAndContents( *this );

	vector<const LanguageInfo*> v;
	GetLanguageInfos( v );
	FOREACH_CONST( const LanguageInfo*, v, i )
	{
		RString s = SMPackageUtil::GetLanguageDisplayString((*i)->szIsoCode);
		m_comboLanguages.AddString( s );
	}
	ASSERT( !v.empty() );
	m_comboLanguages.SetCurSel( 0 );
	return TRUE;  // return TRUE  unless you set the focus to a control
}

BEGIN_MESSAGE_MAP(CreateLanguageDlg, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CreateLanguageDlg message handlers

void CreateLanguageDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here

	int iIndex = m_comboLanguages.GetCurSel();
	ASSERT( iIndex != LB_ERR );

	vector<const LanguageInfo*> v;
	GetLanguageInfos( v );
	m_sChosenLanguageCode = v[iIndex]->szIsoCode;

	OnOK();
}

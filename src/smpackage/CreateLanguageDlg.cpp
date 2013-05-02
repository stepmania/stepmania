// CreateLanguageDlg.cpp : implementation file

#define CO_EXIST_WITH_MFC
#include "global.h"
#include "stdafx.h"
#include "smpackage.h"
#include "CreateLanguageDlg.h"
#include "RageUtil.h"
#include "SMPackageUtil.h"
#include ".\createlanguagedlg.h"
#include "archutils/Win32/DialogUtil.h"
#include "archutils/Win32/ErrorStrings.h"

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

static const char *const g_aListedLanguageCodes[] =
{
	"ab", "af", "ar", "az", "be", "bg", "ca", "cs", "cy", "da", "de", "el", "en", "es", "et", "eu",
	"fa", "fi", "fr", "ga", "gl", "ha", "he", "hi", "hr", "hu", "hy", "id", "in", "is", "it", "iw",
	"ja", "kk", "kn", "ko", "ky", "lt", "lv", "mk", "ms", "mt", "nl", "no", "pl", "ps", "pt", "rn",
	"ro", "ru", "rw", "sk", "sl", "so", "sq", "sr", "sv", "sw", "te", "th", "tr", "uk", "ur", "uz",
	"vi", "wo", "xh", "yo", "zh", "zu",
};

BOOL CreateLanguageDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO: Add extra initialization here
	DialogUtil::LocalizeDialogAndContents( *this );

	for( int i = 0; i < ARRAYLEN(g_aListedLanguageCodes); ++i )
	{
		RString s = SMPackageUtil::GetLanguageDisplayString(g_aListedLanguageCodes[i]);
		RString sLanguage = ConvertUTF8ToACP( s );
		m_comboLanguages.AddString( sLanguage );
	}
	m_comboLanguages.SetCurSel( 0 );
	return TRUE;  // return TRUE  unless you set the focus to a control
}

BEGIN_MESSAGE_MAP(CreateLanguageDlg, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()

void CreateLanguageDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here

	int iIndex = m_comboLanguages.GetCurSel();
	ASSERT( iIndex != LB_ERR );

	m_sChosenLanguageCode = g_aListedLanguageCodes[iIndex];

	OnOK();
}

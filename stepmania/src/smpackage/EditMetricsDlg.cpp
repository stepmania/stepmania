// EditMetricsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "smpackage.h"
#include "EditMetricsDlg.h"

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
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void EditMetricsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(EditMetricsDlg)
	DDX_Control(pDX, IDC_LIST_NAME, m_listName);
	DDX_Control(pDX, IDC_LIST_CLASS, m_listClass);
	DDX_Control(pDX, IDC_EDIT_VALUE, m_editValue);
	DDX_Control(pDX, IDC_EDIT_DEFAULT, m_editDefault);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(EditMetricsDlg, CDialog)
	//{{AFX_MSG_MAP(EditMetricsDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// EditMetricsDlg message handlers

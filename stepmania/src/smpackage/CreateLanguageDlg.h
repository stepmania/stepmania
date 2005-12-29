#pragma once
#include "afxwin.h"


// CreateLanguageDlg dialog

class CreateLanguageDlg : public CDialog
{
	DECLARE_DYNAMIC(CreateLanguageDlg)

public:
	CreateLanguageDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CreateLanguageDlg();

// Dialog Data
	enum { IDD = IDD_CREATE_LANGUAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	CComboBox m_comboLanguages;
	afx_msg void OnBnClickedOk();

	CString m_sChosenLanguageCode;
};

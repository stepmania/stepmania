#pragma once
#include "afxwin.h"


// LanguagesDlg dialog

class LanguagesDlg : public CDialog
{
	DECLARE_DYNAMIC(LanguagesDlg)

public:
	LanguagesDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~LanguagesDlg();

// Dialog Data
	enum { IDD = IDD_THEMES };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	void OnSelchangeListThemes();
	void OnSelchangeListLanguages();

	DECLARE_MESSAGE_MAP()
public:
	CListBox m_listThemes;
	CListBox m_listLanguages;
	afx_msg void OnBnClickedButtonCreate();
	afx_msg void OnBnClickedButtonDelete();
};

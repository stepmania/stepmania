#pragma once


// UninstallOld dialog

class UninstallOld : public CDialog
{
public:
	UninstallOld(CWnd* pParent = NULL);   // standard constructor
	virtual ~UninstallOld();
// Overrides

// Dialog Data
	enum { IDD = IDD_UNINSTALL_OLD_PACKAGES };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	void OnOK();
	void OnCancel();

	DECLARE_MESSAGE_MAP()
public:
	CString m_sPackages;
	afx_msg void OnBnClickedNo();
};

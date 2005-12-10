#if !defined(AFX_SMPACKAGEINSTALLDLG_H__5E362C4C_CA11_4071_A8AB_A0231E985DAF__INCLUDED_)
#define AFX_SMPACKAGEINSTALLDLG_H__5E362C4C_CA11_4071_A8AB_A0231E985DAF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SMPackageInstallDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSMPackageInstallDlg dialog

class CSMPackageInstallDlg : public CDialog
{
// Construction
public:
	CSMPackageInstallDlg(CString sPackagePath, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSMPackageInstallDlg)
	enum { IDD = IDD_INSTALL };
	CButton	m_buttonEdit;
	CComboBox	m_comboDir;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSMPackageInstallDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void RefreshInstallationList();
	bool CheckPackages();

	HICON m_hIcon;
	RString m_sPackagePath;

	// Generated message map functions
	//{{AFX_MSG(CSMPackageInstallDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnPaint();
	afx_msg void OnButtonEdit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SMPACKAGEINSTALLDLG_H__5E362C4C_CA11_4071_A8AB_A0231E985DAF__INCLUDED_)

#if !defined(AFX_CONNECTDLG_H__04D8BFFC_9C8D_4610_8AFF_4736EB7397BE__INCLUDED_)
#define AFX_CONNECTDLG_H__04D8BFFC_9C8D_4610_8AFF_4736EB7397BE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConnectDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CConnectDlg dialog

class CConnectDlg : public CDialog
{
// Construction
public:
	CConnectDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CConnectDlg)
	enum { IDD = IDD_CONNECT };
	CString	m_sFullName;
	CString	m_sNick;
	CString	m_sPassword;
	UINT	m_uiPort;
	CString	m_sServer;
	CString	m_sUserID;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConnectDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void UpdateButtons();

	// Generated message map functions
	//{{AFX_MSG(CConnectDlg)
	afx_msg void OnChange();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONNECTDLG_H__04D8BFFC_9C8D_4610_8AFF_4736EB7397BE__INCLUDED_)

#if !defined(AFX_ENTERNAME_H__0AB99274_EB79_4A25_B95C_627E0A99C6BA__INCLUDED_)
#define AFX_ENTERNAME_H__0AB99274_EB79_4A25_B95C_627E0A99C6BA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EnterName.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// EnterName dialog

class EnterName : public CDialog
{
// Construction
public:
	EnterName(CWnd* pParent = NULL);   // standard constructor

	CString m_sEnteredName;

// Dialog Data
	//{{AFX_DATA(EnterName)
	enum { IDD = IDD_DIALOG_NAME };
	CEdit	m_edit;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(EnterName)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL


// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(EnterName)
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ENTERNAME_H__0AB99274_EB79_4A25_B95C_627E0A99C6BA__INCLUDED_)

#include "afxwin.h"
#if !defined(AFX_ENTERCOMMENT_H__DE628EA8_0FE7_4594_B2A2_47BDB283FA1E__INCLUDED_)
#define AFX_ENTERCOMMENT_H__DE628EA8_0FE7_4594_B2A2_47BDB283FA1E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EnterComment.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// EnterComment dialog

class EnterComment : public CDialog
{
// Construction
public:
	EnterComment(CWnd* pParent = NULL);   // standard constructor

	CString m_sEnteredComment;

// Dialog Data
	//{{AFX_DATA(EnterComment)
	enum { IDD = IDD_ENTER_COMMENT };
	CEdit	m_edit;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(EnterComment)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(EnterComment)
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	BOOL m_bDontAsk;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ENTERCOMMENT_H__DE628EA8_0FE7_4594_B2A2_47BDB283FA1E__INCLUDED_)

#if !defined(AFX_SHOWCOMMENT_H__1FA603E3_5CCD_4D9E_9EC9_88AD55A06270__INCLUDED_)
#define AFX_SHOWCOMMENT_H__1FA603E3_5CCD_4D9E_9EC9_88AD55A06270__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ShowComment.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// ShowComment dialog

class ShowComment : public CDialog
{
// Construction
public:
	ShowComment(CWnd* pParent = NULL);   // standard constructor

	CString m_sComment;

// Dialog Data
	//{{AFX_DATA(ShowComment)
	enum { IDD = IDD_SHOW_COMMENT };
	CEdit	m_edit;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ShowComment)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(ShowComment)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	BOOL m_bDontShow;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SHOWCOMMENT_H__1FA603E3_5CCD_4D9E_9EC9_88AD55A06270__INCLUDED_)

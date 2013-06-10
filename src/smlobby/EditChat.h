#if !defined(AFX_EDITCHAT_H__68A3B363_25AE_45B1_8519_9FE70839EFC4__INCLUDED_)
#define AFX_EDITCHAT_H__68A3B363_25AE_45B1_8519_9FE70839EFC4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditChat.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditChat window

class CEditChat : public CEdit
{
// Construction
public:
	CEditChat();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditChat)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CEditChat();
	void SendChatMesg();

	// Generated message map functions
protected:
	//{{AFX_MSG(CEditChat)
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITCHAT_H__68A3B363_25AE_45B1_8519_9FE70839EFC4__INCLUDED_)

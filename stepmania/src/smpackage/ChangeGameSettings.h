#if !defined(AFX_CHANGEGAMESETTINGS_H__BFA59E6F_E70A_49A7_90A6_F04D3C321CA6__INCLUDED_)
#define AFX_CHANGEGAMESETTINGS_H__BFA59E6F_E70A_49A7_90A6_F04D3C321CA6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChangeGameSettings.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// ChangeGameSettings dialog

class ChangeGameSettings : public CDialog
{
// Construction
public:
	ChangeGameSettings(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(ChangeGameSettings)
	enum { IDD = IDD_CHANGE_GAME_SETTINGS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ChangeGameSettings)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(ChangeGameSettings)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHANGEGAMESETTINGS_H__BFA59E6F_E70A_49A7_90A6_F04D3C321CA6__INCLUDED_)

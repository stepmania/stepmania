#if !defined(AFX_MAINMENUDLG_H__3CED6142_18E5_4267_B678_BE5B63735C6C__INCLUDED_)
#define AFX_MAINMENUDLG_H__3CED6142_18E5_4267_B678_BE5B63735C6C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MainMenuDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// MainMenuDlg dialog

class MainMenuDlg : public CDialog
{
// Construction
public:
	MainMenuDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(MainMenuDlg)
	enum { IDD = IDD_MENU };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(MainMenuDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(MainMenuDlg)
	afx_msg void OnExportPackages();
	afx_msg void OnEditInstallations();
	afx_msg void OnAnalyzeElements();
	afx_msg void OnChangeApi();
	afx_msg void OnCreateSong();
	virtual BOOL OnInitDialog();
	afx_msg void OnOpenStepmaniaIni();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINMENUDLG_H__3CED6142_18E5_4267_B678_BE5B63735C6C__INCLUDED_)

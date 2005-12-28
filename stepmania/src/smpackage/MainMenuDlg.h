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
	afx_msg void OnChangeApi();
	afx_msg void OnCreateSong();
	virtual BOOL OnInitDialog();
	afx_msg void OnOpenStepmaniaIni();
	afx_msg void OnBnClickedClearKeymaps();
	afx_msg void OnBnClickedChangePreferences();
	afx_msg void OnBnClickedOpenPreferences();
	afx_msg void OnBnClickedClearPreferences();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonLaunchGame();
	afx_msg void OnBnClickedViewStatistics();
	afx_msg void OnBnClickedClearCache();
	afx_msg void OnBnClickedLanguages();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINMENUDLG_H__3CED6142_18E5_4267_B678_BE5B63735C6C__INCLUDED_)

/*
 * (c) 2002-2005 Chris Danford
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#if !defined(AFX_SMPACKAGEINSTALLDLG_H__5E362C4C_CA11_4071_A8AB_A0231E985DAF__INCLUDED_)
#define AFX_SMPACKAGEINSTALLDLG_H__5E362C4C_CA11_4071_A8AB_A0231E985DAF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SMPackageInstallDlg.h : header file
//

#include "TransparentStatic.h"

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
	CTransparentStatic	m_staticHeaderText;

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

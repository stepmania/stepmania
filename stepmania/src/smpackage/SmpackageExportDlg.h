#if !defined(AFX_SMPACKAGEEXPORTDLG_H__3E19CBFB_E8F6_4C18_B0A4_636979B80A4D__INCLUDED_)
#define AFX_SMPACKAGEEXPORTDLG_H__3E19CBFB_E8F6_4C18_B0A4_636979B80A4D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "afxtempl.h"

// SmpackageExportDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSmpackageExportDlg dialog

class CSmpackageExportDlg : public CDialog
{
// Construction
public:
	CSmpackageExportDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSmpackageExportDlg)
	enum { IDD = IDD_EXPORTER };
	CComboBox	m_comboDir;
	CButton	m_buttonExportAsIndividual;
	CButton	m_buttonExportAsOne;
	CTreeCtrl	m_tree;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSmpackageExportDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void RefreshInstallationList();
	void RefreshTree();
	void GetTreeItems( CArray<HTREEITEM,HTREEITEM>& aItemsOut );
	void GetCheckedTreeItems( CArray<HTREEITEM,HTREEITEM>& aCheckedItemsOut );
	void GetCheckedPaths( vector<RString>& aCheckedItemsOut );
	bool MakeComment( RString &comment );

	// Generated message map functions
	//{{AFX_MSG(CSmpackageExportDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonExportAsOne();
	afx_msg void OnButtonExportAsIndividual();
	afx_msg void OnButtonPlay();
	afx_msg void OnButtonEdit();
	afx_msg void OnSelchangeComboDir();
	afx_msg void OnButtonOpen();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SMPACKAGEEXPORTDLG_H__3E19CBFB_E8F6_4C18_B0A4_636979B80A4D__INCLUDED_)

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

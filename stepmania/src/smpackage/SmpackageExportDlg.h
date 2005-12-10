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

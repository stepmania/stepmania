#if !defined(AFX_ONVERTTHEMEDLG_H__4A6650E2_F5C6_4914_8610_2118AB81455F__INCLUDED_)
#define AFX_ONVERTTHEMEDLG_H__4A6650E2_F5C6_4914_8610_2118AB81455F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// onvertThemeDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// ConvertThemeDlg dialog

class ConvertThemeDlg : public CDialog
{
// Construction
public:
	ConvertThemeDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(ConvertThemeDlg)
	enum { IDD = IDD_CONVERT_THEME };
	CButton	m_buttonAnalyzeMetrics;
	CButton	m_buttonEditMetrics;
	CButton	m_buttonAnalyze;
	CListBox	m_listThemes;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ConvertThemeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(ConvertThemeDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeListThemes();
	afx_msg void OnButtonAnalyze();
	afx_msg void OnButtonEditMetrics();
	afx_msg void OnButtonAnalyzeMetrics();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ONVERTTHEMEDLG_H__4A6650E2_F5C6_4914_8610_2118AB81455F__INCLUDED_)

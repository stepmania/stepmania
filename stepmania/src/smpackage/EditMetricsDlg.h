#if !defined(AFX_EDITMETRICSDLG_H__39D944CD_BCE4_4C30_876E_0B5A0CE42931__INCLUDED_)
#define AFX_EDITMETRICSDLG_H__39D944CD_BCE4_4C30_876E_0B5A0CE42931__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditMetricsDlg.h : header file
//

#include "ColorListBox.h"

/////////////////////////////////////////////////////////////////////////////
// EditMetricsDlg dialog

class EditMetricsDlg : public CDialog
{
// Construction
public:
	EditMetricsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(EditMetricsDlg)
	enum { IDD = IDD_EDIT_METRICS };
	CColorListBox	m_listName;
	CColorListBox	m_listClass;
	CEdit	m_editValue;
	CEdit	m_editDefault;
	//}}AFX_DATA

	CString m_sTheme;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(EditMetricsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(EditMetricsDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITMETRICSDLG_H__39D944CD_BCE4_4C30_876E_0B5A0CE42931__INCLUDED_)

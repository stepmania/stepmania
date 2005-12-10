#if !defined(AFX_EDITINSALLATIONS_H__CD328CB4_8E35_4B12_BCD1_ADF5CE7EABC7__INCLUDED_)
#define AFX_EDITINSALLATIONS_H__CD328CB4_8E35_4B12_BCD1_ADF5CE7EABC7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditInsallations.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// EditInsallations dialog

class EditInsallations : public CDialog
{
// Construction
public:
	EditInsallations(CWnd* pParent = NULL);   // standard constructor

	vector<RString>	m_vsReturnedInstallDirs;

// Dialog Data
	//{{AFX_DATA(EditInsallations)
	enum { IDD = IDD_EDIT_INSTALLATIONS };
	CListBox	m_list;
	CEdit	m_edit;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(EditInsallations)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(EditInsallations)
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonRemove();
	afx_msg void OnButtonMakeDefault();
	afx_msg void OnButtonAdd();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITINSALLATIONS_H__CD328CB4_8E35_4B12_BCD1_ADF5CE7EABC7__INCLUDED_)

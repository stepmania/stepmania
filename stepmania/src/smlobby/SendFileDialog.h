#if !defined(AFX_SENDFILEDIALOG_H__6AA4B205_EDA8_4960_9424_1C5743093120__INCLUDED_)
#define AFX_SENDFILEDIALOG_H__6AA4B205_EDA8_4960_9424_1C5743093120__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SendFileDialog.h : header file
//

#include "resource2.h"

/////////////////////////////////////////////////////////////////////////////
// CSendFileDialog dialog

class CSendFileDialog : public CDialog
{
// Construction
public:
	CSendFileDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSendFileDialog)
	enum { IDD = IDD_FILEXFER };
	CProgressCtrl	m_ProgressFile;
	CString	m_BytesSent;
	CString	m_Filesize;
	CString	m_FolderName;
	CString	m_FileName;
	CString	m_RecvrName;
	CString	m_TimeLeft;
	CString	m_XferRate;
	CString	m_XferStatus;
	CString	m_SentRecvd;
	CString	m_ToFrom;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSendFileDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	bool isCanceled()	{ return m_bIsCanceled; }

protected:
	bool m_bIsCanceled;

protected:
	
	// Generated message map functions
	//{{AFX_MSG(CSendFileDialog)
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SENDFILEDIALOG_H__6AA4B205_EDA8_4960_9424_1C5743093120__INCLUDED_)

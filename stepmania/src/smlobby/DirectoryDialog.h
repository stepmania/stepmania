#if !defined(AFX_DIRECTORYDIALOG_H__1692F649_A71E_4EA5_9DE0_0A3F2A30B1E6__INCLUDED_)
#define AFX_DIRECTORYDIALOG_H__1692F649_A71E_4EA5_9DE0_0A3F2A30B1E6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DirectoryDialog.h : header file
//

#include "resource2.h"

/////////////////////////////////////////////////////////////////////////////
// CDirectoryDialog dialog

class CDirectoryDialog : public CFileDialog
{
	DECLARE_DYNAMIC(CDirectoryDialog)

public:
   BOOL m_bDlgJustCameUp;

public:
	CDirectoryDialog(BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter = NULL,
		CWnd* pParentWnd = NULL);

protected:
	//{{AFX_MSG(CDirectoryDialog)
	afx_msg void OnPaint();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIRECTORYDIALOG_H__1692F649_A71E_4EA5_9DE0_0A3F2A30B1E6__INCLUDED_)

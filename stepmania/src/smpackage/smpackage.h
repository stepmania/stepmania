// smpackage.h : main header file for the SMPACKAGE application
//

#if !defined(AFX_SMPACKAGE_H__FBCA9E6C_86A9_4271_8304_83CC34A31687__INCLUDED_)
#define AFX_SMPACKAGE_H__FBCA9E6C_86A9_4271_8304_83CC34A31687__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CSmpackageApp:
// See smpackage.cpp for the implementation of this class
//

class CSmpackageApp : public CWinApp
{
public:
	CSmpackageApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSmpackageApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CSmpackageApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SMPACKAGE_H__FBCA9E6C_86A9_4271_8304_83CC34A31687__INCLUDED_)

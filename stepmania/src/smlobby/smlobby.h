// smlobby.h : main header file for the Smlobby application
//

#if !defined(AFX_Smlobby_H__0D0E3C89_DD15_4558_AEA0_3711BD2EC0AA__INCLUDED_)
#define AFX_Smlobby_H__0D0E3C89_DD15_4558_AEA0_3711BD2EC0AA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource2.h"		// main symbols

#include "smlobbyDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CSmlobbyApp:
// See smlobby.cpp for the implementation of this class
//

class CSmlobbyApp : public CWinApp
{
private:
	CSmlobbyDlg *m_dlg;

public:
	CSmlobbyApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSmlobbyApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CSmlobbyApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_Smlobby_H__0D0E3C89_DD15_4558_AEA0_3711BD2EC0AA__INCLUDED_)

// DirectoryDialog.cpp : implementation file
//

#include "stdafx.h"
#include "smlobby.h"
#include "DirectoryDialog.h"
#include "dlgs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDirectoryDialog

IMPLEMENT_DYNAMIC(CDirectoryDialog, CFileDialog)

CDirectoryDialog::CDirectoryDialog(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
		DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd) :
		CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd)
{
}


BEGIN_MESSAGE_MAP(CDirectoryDialog, CFileDialog)
	//{{AFX_MSG_MAP(CDirectoryDialog)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CDirectoryDialog::OnPaint() 
{
    CPaintDC dc(this); // device context for painting
    
    // TODO: Add your message handler code here

//This code makes the directory listbox "highlight" an entry when it first
//comes up.  W/O this code, the focus is on the directory listbox, but no
//focus rectangle is drawn and no entries are selected.  Ho hum.

     if (m_bDlgJustCameUp)
     {
        m_bDlgJustCameUp=FALSE;
        SendDlgItemMessage(lst2, LB_SETCURSEL, 0, 0L);
     }
    
    // Do not call CFileDialog::OnPaint() for painting messages
}

BOOL CDirectoryDialog::OnInitDialog() 
{
   CenterWindow();

//Let's hide these windows so the user cannot tab to them.  Note that in
//the private template (in cddemo.dlg) the coordinates for these guys are
//*outside* the coordinates of the dlg window itself.  Without the following
//ShowWindow()'s you would not see them, but could still tab to them.
    
   GetDlgItem(stc2)->ShowWindow(SW_HIDE);
   GetDlgItem(stc3)->ShowWindow(SW_HIDE);
   GetDlgItem(edt1)->ShowWindow(SW_HIDE);
   GetDlgItem(lst1)->ShowWindow(SW_HIDE);
   GetDlgItem(cmb1)->ShowWindow(SW_HIDE);
    
//We must put something in this field, even though it is hidden.  This is
//because if this field is empty, or has something like "*.txt" in it,
//and the user hits OK, the dlg will NOT close.  We'll jam something in
//there (like "Junk") so when the user hits OK, the dlg terminates.
//Note that we'll deal with the "Junk" during return processing (see below)

   SetDlgItemText(edt1, "Junk");

//Now set the focus to the directories listbox.  Due to some painting
//problems, we *must* also process the first WM_PAINT that comes through
//and set the current selection at that point.  Setting the selection
//here will NOT work.  See comment below in the on paint handler.
            
   GetDlgItem(lst2)->SetFocus();
            
   m_bDlgJustCameUp=TRUE;
             
   CFileDialog::OnInitDialog();
   
   return(FALSE);
}

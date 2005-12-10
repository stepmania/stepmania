/////////////////////////////////////////////////////////////
//	
//	Author:		Sami (M.ALSAMSAM), ittiger@ittiger.net
//
//	Filename:	TreeCtrlEx.cpp
//
//	http	 :	www.ittiger.net
//
//////////////////////////////////////////////////////////////
#define CO_EXIST_WITH_MFC
#include "global.h"
#include "stdafx.h"
#include "TreeCtrlEx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////
CTreeCtrlEx::CTreeCtrlEx()
{
}

CTreeCtrlEx::~CTreeCtrlEx()
{
}

//////////////////////////////////
IMPLEMENT_DYNAMIC(CTreeCtrlEx, CTreeCtrl)

BEGIN_MESSAGE_MAP(CTreeCtrlEx, CTreeCtrl)
	//{{AFX_MSG_MAP(CTreeCtrlEx)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////
void CTreeCtrlEx::SetItemFont(HTREEITEM hItem, LOGFONT& logfont)
{
	Color_Font cf;
	if( !m_mapColorFont.Lookup( hItem, cf ) )
		cf.color = (COLORREF)-1;
	cf.logfont = logfont;
	m_mapColorFont[hItem] = cf;
}

//////////////////////////////////////////////////////////////////////
void CTreeCtrlEx::SetItemBold(HTREEITEM hItem, BOOL bBold)
{
	SetItemState(hItem, bBold ? TVIS_BOLD: 0, TVIS_BOLD);
}

//////////////////////////////////////////////////////////////////////
void CTreeCtrlEx::SetItemColor(HTREEITEM hItem, COLORREF color)
{
	Color_Font cf;
	if(!m_mapColorFont.Lookup(hItem, cf))
		cf.logfont.lfFaceName[0] = '\0';
	cf.color = color;
	m_mapColorFont[hItem] = cf;
}

//////////////////////////////////////////////////////////////////////
BOOL CTreeCtrlEx::GetItemFont(HTREEITEM hItem, LOGFONT * plogfont)
{
	Color_Font cf;
	if(!m_mapColorFont.Lookup(hItem, cf))
		return FALSE;
	if(cf.logfont.lfFaceName[0] == '\0') 
		return FALSE;
	*plogfont = cf.logfont;
	return TRUE;

}

//////////////////////////////////////////////////////////////////////
BOOL CTreeCtrlEx::GetItemBold(HTREEITEM hItem)
{
	return GetItemState(hItem, TVIS_BOLD) & TVIS_BOLD;
}

//////////////////////////////////////////////////////////////////////
COLORREF CTreeCtrlEx::GetItemColor(HTREEITEM hItem)
{
	// Returns (COLORREF)-1 if color was not set
	Color_Font cf;
	if(!m_mapColorFont.Lookup(hItem, cf))
		return (COLORREF) - 1;
	return cf.color;

}

//////////////////////////////////////////////////////////////////////
void CTreeCtrlEx::OnPaint() 
{
	CPaintDC dc(this);

	// Create a memory DC compatible with the paint DC
	CDC memDC;
	memDC.CreateCompatibleDC(&dc);

	CRect rcClip, rcClient;
	dc.GetClipBox( &rcClip );
	GetClientRect(&rcClient);

	// Select a compatible bitmap into the memory DC
	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap( &dc, rcClient.Width(), rcClient.Height() );
	memDC.SelectObject( &bitmap );
	
	// Set clip region to be same as that in paint DC
	CRgn rgn;
	rgn.CreateRectRgnIndirect( &rcClip );
	memDC.SelectClipRgn(&rgn);
	rgn.DeleteObject();
	
	// First let the control do its default drawing.
	CWnd::DefWindowProc(WM_PAINT, (WPARAM)memDC.m_hDC, 0);

	HTREEITEM hItem = GetFirstVisibleItem();

	int iItemCount = GetVisibleCount() + 1;
	while(hItem && iItemCount--)
	{		
		CRect rect;

		// Do not meddle with selected items or drop highlighted items
		UINT selflag = TVIS_DROPHILITED | TVIS_SELECTED;
		Color_Font cf;
	
		//if ( !(GetTreeCtrl().GetItemState( hItem, selflag ) & selflag ) 
		//	&& m_mapColorFont.Lookup( hItem, cf ))
		
		if ((GetItemState(hItem, selflag) & selflag) 
			&& ::GetFocus() == m_hWnd)
			;
		else if (m_mapColorFont.Lookup(hItem, cf))
		{
			CFont *pFontDC;
			CFont fontDC;
			LOGFONT logfont;

			if(cf.logfont.lfFaceName[0] != '\0') 
				logfont = cf.logfont;
			else {
				// No font specified, so use window font
				CFont *pFont = GetFont();
				pFont->GetLogFont( &logfont );
			}

			if(GetItemBold(hItem))
				logfont.lfWeight = 700;

			fontDC.CreateFontIndirect(&logfont);
			pFontDC = memDC.SelectObject(&fontDC );

			if(cf.color != (COLORREF) - 1)
				memDC.SetTextColor(cf.color);
			else
				memDC.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));


			CString sItem = GetItemText(hItem);

			GetItemRect(hItem, &rect, TRUE);
			memDC.SetBkColor( GetSysColor(COLOR_WINDOW));
			memDC.TextOut(rect.left + 2, rect.top + 1, sItem);
			
			memDC.SelectObject(pFontDC);
		}
		hItem = GetNextVisibleItem(hItem);
	}


	dc.BitBlt(rcClip.left, rcClip.top, rcClip.Width(), rcClip.Height(), &memDC, 
				rcClip.left, rcClip.top, SRCCOPY);

	memDC.DeleteDC();
}


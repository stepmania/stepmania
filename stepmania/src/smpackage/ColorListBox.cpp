// ColorListBox.cpp : implementation file

//-------------------------------------------------------------------
//
//	CColorListBox class - 
//		A CListBox-derived class with optional colored items.
//
//		Version: 1.0	01/10/1998 Copyright © Patrice Godard
//
//		Version: 2.0	09/17/1999 Copyright © Paul M. Meidinger
//
//-------------------------------------------------------------------

#include "stdafx.h"
#include "ColorListBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CColorListBox

//-------------------------------------------------------------------
//
CColorListBox::CColorListBox()
//
// Return Value:	None.
//
// Parameters	:	None.
//
// Remarks		:	Standard constructor.
//
{
}	// CColorListBox

//-------------------------------------------------------------------
//
CColorListBox::~CColorListBox()
//
// Return Value:	None.
//
// Parameters	:	None.
//
// Remarks		:	Destructor.
//
{
}	// ~CColorListBox()


BEGIN_MESSAGE_MAP(CColorListBox, CListBox)
	//{{AFX_MSG_MAP(CColorListBox)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CColorListBox message handlers

//-------------------------------------------------------------------
//
void CColorListBox::DrawItem(LPDRAWITEMSTRUCT lpDIS) 
//
// Return Value:	None.
//
// Parameters	:	lpDIS - A long pointer to a DRAWITEMSTRUCT structure 
//							that contains information about the type of drawing required.
//
// Remarks		:	Called by the framework when a visual aspect of 
//						an owner-draw list box changes. 
//
{
	if ((int)lpDIS->itemID < 0)
		return; 

	CDC* pDC = CDC::FromHandle(lpDIS->hDC);

	COLORREF crText;
	CString sText;
	COLORREF crNorm = (COLORREF)lpDIS->itemData;		// Color information is in item data.
	COLORREF crHilite = RGB(255-GetRValue(crNorm), 255-GetGValue(crNorm), 255-GetBValue(crNorm));

	// If item has been selected, draw the highlight rectangle using the item's color.
	if ((lpDIS->itemState & ODS_SELECTED) &&
		 (lpDIS->itemAction & (ODA_SELECT | ODA_DRAWENTIRE)))
	{
		CBrush brush(crNorm);
		pDC->FillRect(&lpDIS->rcItem, &brush);
	}

	// If item has been deselected, draw the rectangle using the window color.
	if (!(lpDIS->itemState & ODS_SELECTED) &&	(lpDIS->itemAction & ODA_SELECT))
	{
		CBrush brush(::GetSysColor(COLOR_WINDOW));
		pDC->FillRect(&lpDIS->rcItem, &brush);
	}	 	

	// If item has focus, draw the focus rect.
	if ((lpDIS->itemAction & ODA_FOCUS) && (lpDIS->itemState & ODS_FOCUS))
		pDC->DrawFocusRect(&lpDIS->rcItem); 

	// If item does not have focus, redraw (erase) the focus rect.
	if ((lpDIS->itemAction & ODA_FOCUS) &&	!(lpDIS->itemState & ODS_FOCUS))
		pDC->DrawFocusRect(&lpDIS->rcItem); 


	// Set the background mode to TRANSPARENT to draw the text.
	int nBkMode = pDC->SetBkMode(TRANSPARENT);

	// If the item's color information is set, use the highlight color
	// gray text color, or normal color for the text.
	if (lpDIS->itemData)		
	{
		if (lpDIS->itemState & ODS_SELECTED)
			crText = pDC->SetTextColor(crHilite);
		else if (lpDIS->itemState & ODS_DISABLED)
			crText = pDC->SetTextColor(::GetSysColor(COLOR_GRAYTEXT));
		else
			crText = pDC->SetTextColor(crNorm);
	}
	// Else the item's color information is not set, so use the
	// system colors for the text.
	else
	{
		if (lpDIS->itemState & ODS_SELECTED)
			crText = pDC->SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
		else if (lpDIS->itemState & ODS_DISABLED)
			crText = pDC->SetTextColor(::GetSysColor(COLOR_GRAYTEXT));
		else
			crText = pDC->SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));
	}


	// Get and display item text.
	GetText(lpDIS->itemID, sText);
	CRect rect = lpDIS->rcItem;

	// Setup the text format.
	UINT nFormat = DT_LEFT | DT_SINGLELINE | DT_VCENTER;
	if (GetStyle() & LBS_USETABSTOPS)
		nFormat |= DT_EXPANDTABS;
	
	// Calculate the rectangle size before drawing the text.
	pDC->DrawText(sText, -1, &rect, nFormat | DT_CALCRECT);
	pDC->DrawText(sText, -1, &rect, nFormat);

	pDC->SetTextColor(crText); 
	pDC->SetBkMode(nBkMode);
}	// DrawItem

//-------------------------------------------------------------------
//
void CColorListBox::MeasureItem(LPMEASUREITEMSTRUCT lpMIS)
//
// Return Value:	None.
//
// Parameters	:	lpMIS - A long pointer to a 
//							MEASUREITEMSTRUCT structure.
//
// Remarks		:	Called by the framework when a list box with 
//						an owner-draw style is created. 
//
{
	// ### Is the default list box item height the same as
	// the menu check height???
	lpMIS->itemHeight = ::GetSystemMetrics(SM_CYMENUCHECK);
}	// MeasureItem

//-------------------------------------------------------------------
//
int CColorListBox::AddString(LPCTSTR lpszItem)
//
// Return Value:	The zero-based index to the string in the list box. 
//						The return value is LB_ERR if an error occurs; the 
//						return value is LB_ERRSPACE if insufficient space 
//						is available to store the new string.
//
// Parameters	:	lpszItem - Points to the null-terminated 
//							string that is to be added.
//
// Remarks		:	Call this member function to add a string to a list 
//						box. Provided because CListBox::AddString is NOT
//						a virtual function.
//
{
	return ((CListBox*)this)->AddString(lpszItem);
}	// AddString

//-------------------------------------------------------------------
//
int CColorListBox::AddString(LPCTSTR lpszItem, COLORREF rgb)
//
// Return Value:	The zero-based index to the string in the list box. 
//						The return value is LB_ERR if an error occurs; the 
//						return value is LB_ERRSPACE if insufficient space 
//						is available to store the new string.
//
// Parameters	:	lpszItem - Points to the null-terminated 
//							string that is to be added.
//						rgb - Specifies the color to be associated with the item.
//
// Remarks		:	Call this member function to add a string to a list 
//						box with a custom color.
//
{
	int nItem = AddString(lpszItem);
	if (nItem >= 0)
		SetItemData(nItem, rgb);
	return nItem;
}	// AddString

//-------------------------------------------------------------------
//
int CColorListBox::InsertString(int nIndex, LPCTSTR lpszItem)
//
// Return Value:	The zero-based index of the position at which the 
//						string was inserted. The return value is LB_ERR if 
//						an error occurs; the return value is LB_ERRSPACE if 
//						insufficient space is available to store the new string.
//
// Parameters	:	nIndex - Specifies the zero-based index of the position
//							to insert the string. If this parameter is –1, the string
//							is added to the end of the list.
//						lpszItem - Points to the null-terminated string that 
//							is to be inserted.
//
// Remarks		:	Inserts a string into the list box.	Provided because 
//						CListBox::InsertString is NOT a virtual function.
//
{
	return ((CListBox*)this)->InsertString(nIndex, lpszItem);
}	// InsertString

//-------------------------------------------------------------------
//
int CColorListBox::InsertString(int nIndex, LPCTSTR lpszItem, COLORREF rgb)
//
// Return Value:	The zero-based index of the position at which the 
//						string was inserted. The return value is LB_ERR if 
//						an error occurs; the return value is LB_ERRSPACE if 
//						insufficient space is available to store the new string.
//
// Parameters	:	nIndex - Specifies the zero-based index of the position
//							to insert the string. If this parameter is –1, the string
//							is added to the end of the list.
//						lpszItem - Points to the null-terminated string that 
//							is to be inserted.
//						rgb - Specifies the color to be associated with the item.
//
// Remarks		:	Inserts a colored string into the list box.
//
{
	int nItem = ((CListBox*)this)->InsertString(nIndex,lpszItem);
	if (nItem >= 0)
		SetItemData(nItem, rgb);
	return nItem;
}	// InsertString

//-------------------------------------------------------------------
//
void CColorListBox::SetItemColor(int nIndex, COLORREF rgb)
//
// Return Value:	None.
//
// Parameters	:	nIndex - Specifies the zero-based index of the item.
//						rgb - Specifies the color to be associated with the item.
//
// Remarks		:	Sets the 32-bit value associated with the specified
//						item in the list box.
//
{
	SetItemData(nIndex, rgb);	
	RedrawWindow();
}	// SetItemColor

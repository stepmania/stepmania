#include "global.h"
#include "DialogUtil.h"
#include "RageUtil.h"
#include "ThemeManager.h"

// Create*Font copied from MFC's CFont

// pLogFont->nHeight is interpreted as PointSize * 10
static HFONT CreatePointFontIndirect(const LOGFONT* lpLogFont)
{
	HDC hDC = ::GetDC(NULL);

	// convert nPointSize to logical units based on pDC
	LOGFONT logFont = *lpLogFont;
	POINT pt;
	pt.y = ::GetDeviceCaps(hDC, LOGPIXELSY) * logFont.lfHeight;
	pt.y /= 720;    // 72 points/inch, 10 decipoints/point
	pt.x = 0;
	::DPtoLP(hDC, &pt, 1);
	POINT ptOrg = { 0, 0 };
	::DPtoLP(hDC, &ptOrg, 1);
	logFont.lfHeight = -abs(pt.y - ptOrg.y);

	ReleaseDC(NULL, hDC);

	return ::CreateFontIndirect(&logFont);
}

// nPointSize is actually scaled 10x
static HFONT CreatePointFont(int nPointSize, LPCTSTR lpszFaceName)
{
	ASSERT(lpszFaceName);

	LOGFONT logFont;
	memset(&logFont, 0, sizeof(LOGFONT));
	logFont.lfCharSet = DEFAULT_CHARSET;
	logFont.lfHeight = nPointSize;
	lstrcpyn(logFont.lfFaceName, lpszFaceName, strlen(logFont.lfFaceName));

	return ::CreatePointFontIndirect(&logFont);
}

void DialogUtil::SetHeaderFont( HWND hdlg, int nID )
{
	ASSERT( hdlg );

	HWND hControl = ::GetDlgItem( hdlg, nID );
	ASSERT( hControl );

	// TODO: Fix font leak
	const int FONT_POINTS = 16;
	HFONT hfont = CreatePointFont( FONT_POINTS*10, "Arial Black" );
	::SendMessage( hControl, WM_SETFONT, (WPARAM)hfont, TRUE );
}

void DialogUtil::LocalizeDialogAndContents( HWND hdlg )
{
	const int LARGE_STRING = 256;
	char szTemp[LARGE_STRING] = "";
	RString sGroup;

	{
		::GetWindowText( hdlg, szTemp, ARRAYSIZE(szTemp) );
		RString s = szTemp;
		sGroup = "Tools-"+s;
		s = THEME->GetString( sGroup, s );
		::SetWindowText( hdlg, ConvertUTF8ToACP(s).c_str() );
	}

	for( HWND hwndChild = ::GetTopWindow(hdlg); hwndChild != NULL; hwndChild = ::GetNextWindow(hwndChild,GW_HWNDNEXT) )
	{
		::GetWindowText( hwndChild, szTemp, ARRAYSIZE(szTemp) );
		RString s = szTemp;
		if( s.empty() )
			continue;
		s = THEME->GetString( sGroup, s );
		::SetWindowText( hwndChild, ConvertUTF8ToACP(s).c_str() );
	}
}


/*
 * (c) 2002-2004 Chris Danford
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */


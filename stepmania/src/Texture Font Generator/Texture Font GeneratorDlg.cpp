#include "stdafx.h"
#include "Texture Font Generator.h"
#include "Texture Font GeneratorDlg.h"
#include "TextureFont.h"
#include "Utils.h"

#include <vector>
#include <fstream>
#include <set>
using namespace std;

#include <math.h>

static TextureFont *g_pTextureFont = NULL;


IMPLEMENT_DYNAMIC(CTextureFontGeneratorDlg, CDialog);
CTextureFontGeneratorDlg::CTextureFontGeneratorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTextureFontGeneratorDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	g_pTextureFont = new TextureFont;
}
CTextureFontGeneratorDlg::~CTextureFontGeneratorDlg()
{
	delete g_pTextureFont;
}

void CTextureFontGeneratorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PICTURE, m_FontView);
	DDX_Control(pDX, IDC_SHOWN_PAGE, m_ShownPage);
	DDX_Control(pDX, IDC_FAMILY_LIST, m_FamilyList);
	DDX_Control(pDX, IDC_TEXT_OVERLAP, m_TextOverlap);
	DDX_Control(pDX, IDC_FONT_SIZE, m_FontSize);
	DDX_Control(pDX, IDC_ERROR_OR_WARNING, m_ErrorOrWarning);
	DDX_Control(pDX, IDC_CLOSEUP, m_CloseUp);
	DDX_Control(pDX, IDC_SPIN_TOP, m_SpinTop);
	DDX_Control(pDX, IDC_SPIN_BASELINE, m_SpinBaseline);
	DDX_Control(pDX, IDC_PADDING, m_Padding);
}

BEGIN_MESSAGE_MAP(CTextureFontGeneratorDlg, CDialog)
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_MOUSEWHEEL()
	//}}AFX_MSG_MAP
	ON_CBN_SELCHANGE(IDC_SHOWN_PAGE, OnCbnSelchangeShownPage)
	ON_CBN_SELCHANGE(IDC_FAMILY_LIST, OnCbnSelchangeFamilyList)
	ON_EN_CHANGE(IDC_FONT_SIZE, OnEnChangeFontSize)
	ON_COMMAND(ID_STYLE_BOLD, OnStyleBold)
	ON_COMMAND(ID_STYLE_ITALIC, OnStyleItalic)
	ON_COMMAND(ID_STYLE_ANTIALIASED, OnStyleAntialiased)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_TOP, OnDeltaposSpinTop)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_BASELINE, OnDeltaposSpinBaseline)
	ON_EN_CHANGE(IDC_PADDING, OnEnChangePadding)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_FILE_EXIT, OnFileExit)
	ON_COMMAND(ID_ACCELERATOR_ITALIC, OnStyleItalic)
	ON_COMMAND(ID_ACCELERATOR_BOLD, OnStyleBold)
	ON_COMMAND(ID_OPTIONS_DOUBLERES, &CTextureFontGeneratorDlg::OnOptionsDoubleres)
	ON_COMMAND(ID_OPTIONS_EXPORTSTROKETEMPLATES, &CTextureFontGeneratorDlg::OnOptionsExportstroketemplates)
	ON_COMMAND(ID_OPTIONS_NUMBERSONLY, &CTextureFontGeneratorDlg::OnOptionsNumbersonly)
END_MESSAGE_MAP()


static const wchar_t map_cp1252[] = {
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
	0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
	0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
	0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
	0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
	0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0xFFFF,
	0x20AC, 0xFFFF, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0xFFFF, 0x017D, 0xFFFF,
	0xFFFF, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x017E, 0xFFFF, 0x0178,
	0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7, 0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
	0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7, 0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
	0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7, 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
	0x00D0, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7, 0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF,
	0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7, 0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
	0x00F0, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7, 0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x00FF,
	0
};

static const wchar_t map_iso_8859_2[] = {
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0x00A0, 0x0104, 0x02D8, 0x0141, 0x00A4, 0x013D, 0x015A, 0x00A7, 0x00A8, 0x0160, 0x015E, 0x0164, 0x0179, 0x00AD, 0x017D, 0x017B,
	0x00B0, 0x0105, 0x02DB, 0x0142, 0x00B4, 0x013E, 0x015B, 0x02C7, 0x00B8, 0x0161, 0x015F, 0x0165, 0x017A, 0x02DD, 0x017E, 0x017C,
	0x0154, 0x00C1, 0x00C2, 0x0102, 0x00C4, 0x0139, 0x0106, 0x00C7, 0x010C, 0x00C9, 0x0118, 0x00CB, 0x011A, 0x00CD, 0x00CE, 0x010E,
	0x0110, 0x0143, 0x0147, 0x00D3, 0x00D4, 0x0150, 0x00D6, 0x00D7, 0x0158, 0x016E, 0x00DA, 0x0170, 0x00DC, 0x00DD, 0x0162, 0x00DF,
	0x0155, 0x00E1, 0x00E2, 0x0103, 0x00E4, 0x013A, 0x0107, 0x00E7, 0x010D, 0x00E9, 0x0119, 0x00EB, 0x011B, 0x00ED, 0x00EE, 0x010F,
	0x0111, 0x0144, 0x0148, 0x00F3, 0x00F4, 0x0151, 0x00F6, 0x00F7, 0x0159, 0x016F, 0x00FA, 0x0171, 0x00FC, 0x00FD, 0x0163, 0x02D9,
	0
};

static const wchar_t map_numbers[] = {
	0x0030, 0x0031, 0x0032, 0x0033, 
	0x0034, 0x0035, 0x0036, 0x0037, 
	0x0038, 0x0039, 0x0025, 0x002E, 
	0x0020, 0x003A, 0x0078, 0x002C,
	0
};

/* Regenerate the font, pulling in settings from widgets. */
void CTextureFontGeneratorDlg::UpdateFont( bool bSavingDoubleRes )
{
	m_bUpdateFontNeeded = false;

	m_FontView.SetBitmap( NULL );

	CString sOld;
	{
		int iOldSel = m_ShownPage.GetCurSel();
		if( iOldSel != -1 && iOldSel < (int) g_pTextureFont->m_PagesToGenerate.size() )
			sOld = g_pTextureFont->m_PagesToGenerate[iOldSel].name;
	}

	/* Get the selected font m_sFamily. */
	CString sText;
	m_FamilyList.GetWindowText( sText );
	g_pTextureFont->m_sFamily = sText;
	
	m_FontSize.GetWindowText(sText);
	g_pTextureFont->m_fFontSizePixels = (float) atof(sText);
	if( bSavingDoubleRes )
		g_pTextureFont->m_fFontSizePixels *= 2;
	
	m_Padding.GetWindowText(sText);
	g_pTextureFont->m_iPadding = atoi(sText);
	if( bSavingDoubleRes )
		g_pTextureFont->m_iPadding *= 2;

	CMenu *pMenu = GetMenu();
	g_pTextureFont->m_bBold = !!( pMenu->GetMenuState(ID_STYLE_BOLD, 0) & MF_CHECKED );
	g_pTextureFont->m_bItalic = !!( pMenu->GetMenuState(ID_STYLE_ITALIC, 0) & MF_CHECKED );
	g_pTextureFont->m_bAntiAlias = !!( pMenu->GetMenuState(ID_STYLE_ANTIALIASED, 0) & MF_CHECKED );

	g_pTextureFont->m_PagesToGenerate.clear();
	FontPageDescription desc;

	bool bNumbersOnly = !!( pMenu->GetMenuState(ID_OPTIONS_NUMBERSONLY, 0) & MF_CHECKED );

	if( !bNumbersOnly )
	{
		desc.name = "main";
		for( int i = 0; map_cp1252[i]; ++i )
		{
			wchar_t wc = map_cp1252[i];
			if( wc != 0xFFFF )
				desc.chars.push_back(wc);
		}
		g_pTextureFont->m_PagesToGenerate.push_back( desc );

		desc.name = "alt";
		desc.chars.clear();
		for( int i = 0; map_iso_8859_2[i]; ++i )
		{
			wchar_t wc = map_iso_8859_2[i];
			if( wc != 0xFFFF )
				desc.chars.push_back(wc);
		}
		g_pTextureFont->m_PagesToGenerate.push_back( desc );
	}
	else
	{
		desc.name = "numbers";
		desc.chars.clear();
		for( int i = 0; map_numbers[i]; ++i )
		{
			wchar_t wc = map_numbers[i];
			if( wc != 0xFFFF )
				desc.chars.push_back(wc);
		}
		g_pTextureFont->m_PagesToGenerate.push_back( desc );
	}

	/* Go: */
	g_pTextureFont->FormatFontPages();

	m_SpinTop.SetPos( g_pTextureFont->m_iCharTop );
	m_SpinBaseline.SetPos( g_pTextureFont->m_iCharBaseline );

	m_ShownPage.ResetContent();
	for( unsigned p = 0; p < g_pTextureFont->m_PagesToGenerate.size(); ++p )
		m_ShownPage.AddString( g_pTextureFont->m_PagesToGenerate[p].name.GetString() );
	int iRet = m_ShownPage.FindStringExact( -1, sOld );
	if( iRet == CB_ERR )
		iRet = 0;
	m_ShownPage.SetCurSel( iRet );
}

void CTextureFontGeneratorDlg::UpdateCloseUp()
{
	HBITMAP hBitmap = m_CloseUp.GetBitmap();
	m_CloseUp.SetBitmap( NULL );
	if( hBitmap )
		DeleteObject( hBitmap );

	HBITMAP hSample = g_pTextureFont->m_Characters[L'A'];
	if( hSample == NULL )
		return;

	HDC hCharacterDC = CreateCompatibleDC( NULL );
	HGDIOBJ hOldCharacterBitmap = SelectObject( hCharacterDC, hSample );

	BITMAPINFO CharacterInfo;
	memset( &CharacterInfo, 0, sizeof(CharacterInfo) );
	CharacterInfo.bmiHeader.biSize = sizeof(CharacterInfo.bmiHeader);
	GetDIBits( hCharacterDC, hSample, 0, 0, NULL, &CharacterInfo, DIB_RGB_COLORS );

	int iWidth = CharacterInfo.bmiHeader.biWidth;
	int iHeight = CharacterInfo.bmiHeader.biHeight;

	/* Set up a bitmap to zoom the image into. */
	int iSourceHeight = g_pTextureFont->m_BoundingRect.bottom;
	const int iZoomFactor = 4;

	HBITMAP hZoom;
	{
		HDC hTempDC = ::GetDC(NULL);
		hZoom = CreateCompatibleBitmap( hTempDC, iWidth * iZoomFactor, iSourceHeight * iZoomFactor );
		::ReleaseDC( NULL, hTempDC );
	}
	HDC hZoomDC = CreateCompatibleDC( NULL );
	HGDIOBJ hOldZoomBitmap = SelectObject( hZoomDC, hZoom );

	StretchBlt(
		hZoomDC, 0, 0,
		iWidth * iZoomFactor, iHeight * iZoomFactor,
		hCharacterDC, 0, 0,
		iWidth, iHeight,
		SRCCOPY
	);
	SelectObject( hCharacterDC, hOldCharacterBitmap );
	::DeleteDC( hCharacterDC );

	HPEN hPen = CreatePen( PS_SOLID, 1, RGB(0, 255, 255) );
	HGDIOBJ hOldPen = SelectObject( hZoomDC, hPen );

	/* Align the line with the top of the unit, so, when correct, the font
	 * "sits" on the line. */
	int iY = iZoomFactor*g_pTextureFont->m_iCharBaseline - 1;
	MoveToEx( hZoomDC, 0, iY, NULL );
	LineTo( hZoomDC, iWidth * iZoomFactor, iY );

	SelectObject( hZoomDC, hOldPen );
	DeleteObject( hPen );

	hPen = CreatePen( PS_SOLID, 1, RGB(255, 255, 50) );
	hOldPen = SelectObject( hZoomDC, hPen );
	
	/* Align the line with the bottom of the unit, so, when correct, the line
	 * "sits" on the font. */
	iY = iZoomFactor*g_pTextureFont->m_iCharTop - 1;
	MoveToEx( hZoomDC, 0, iY, NULL );
	LineTo( hZoomDC, iWidth * iZoomFactor, iY );

	SelectObject( hZoomDC, hOldPen );
	DeleteObject( hPen );

	SelectObject( hZoomDC, hOldZoomBitmap );
	::DeleteDC( hZoomDC );

	m_CloseUp.SetBitmap( hZoom );
}


int CALLBACK CTextureFontGeneratorDlg::EnumFontFamiliesCallback( const LOGFONTA *pLogicalFontData, const TEXTMETRICA *pPhysicalFontData, DWORD FontType, LPARAM lParam )
{
	CTextureFontGeneratorDlg *pThis = (CTextureFontGeneratorDlg *) lParam;
	set<CString> *pSet = (set<CString> *) lParam;
	pSet->insert( pLogicalFontData->lfFaceName );
	return 1;
}

BOOL CTextureFontGeneratorDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// XXX: AddFontResourceEx
	// AddFontMemResourceEx
	// drag drop
	{
		LOGFONT font;
		memset( &font, 0, sizeof(font) );
		font.lfCharSet = DEFAULT_CHARSET;
		CPaintDC dc(this);
		set<CString> setFamilies;
		EnumFontFamiliesEx( dc.GetSafeHdc(), &font, EnumFontFamiliesCallback, (LPARAM) &setFamilies, 0 );
		for( set<CString>::const_iterator it = setFamilies.begin(); it != setFamilies.end(); ++it )
			m_FamilyList.AddString( *it );
		m_FamilyList.SetCurSel( 0 );
	}

	CMenu *pMenu = GetMenu();
	pMenu->CheckMenuItem( ID_STYLE_ANTIALIASED, MF_CHECKED );

	m_FontSize.SetWindowText( "20" );
	m_Padding.SetWindowText( "2" );
	m_SpinTop.SetRange( 64, 0 );
	m_SpinBaseline.SetRange( 64, 0 );

	m_bUpdateFontNeeded = true;
	m_bUpdateFontViewAndCloseUpNeeded = true;

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTextureFontGeneratorDlg::UpdateFontViewAndCloseUp()
{
	m_bUpdateFontViewAndCloseUpNeeded = false;
	m_FontView.SetBitmap( NULL );

	m_SpinTop.EnableWindow( true );
	m_SpinBaseline.EnableWindow( true );
	GetMenu()->EnableMenuItem( ID_FILE_SAVE, MF_ENABLED );
	
	if( g_pTextureFont->m_sError != "" )
	{
		m_SpinTop.EnableWindow(false);
		m_SpinBaseline.EnableWindow(false);
		GetMenu()->EnableMenuItem( ID_FILE_SAVE, MF_GRAYED );

		m_ErrorOrWarning.SetWindowText( "Error: " + g_pTextureFont->m_sError );
		return;
	}

	m_ErrorOrWarning.SetWindowText( g_pTextureFont->m_sWarnings );

	const int iSelectedPage = m_ShownPage.GetCurSel();
	ASSERT( iSelectedPage < (int) g_pTextureFont->m_apPages.size() );

	g_pTextureFont->m_iCharTop = LOWORD(m_SpinTop.GetPos());
	g_pTextureFont->m_iCharBaseline = LOWORD(m_SpinBaseline.GetPos());

	HBITMAP hBitmap = g_pTextureFont->m_apPages[iSelectedPage]->m_hPage;
	m_FontView.SetBitmap( hBitmap );
	
	UpdateCloseUp();

	CString sStr;
	sStr.Format("Overlap: %i, %i\nMaximum size: %ix%i", 
		g_pTextureFont->m_iCharLeftOverlap, g_pTextureFont->m_iCharRightOverlap,
		g_pTextureFont->m_BoundingRect.right - g_pTextureFont->m_BoundingRect.left,
		g_pTextureFont->m_BoundingRect.bottom - g_pTextureFont->m_BoundingRect.top
		);
	m_TextOverlap.SetWindowText( sStr );
}

void CTextureFontGeneratorDlg::OnDestroy()
{
	WinHelp(0L, HELP_QUIT);
	CDialog::OnDestroy();
}

void CTextureFontGeneratorDlg::OnPaint() 
{
	if( IsIconic() )
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
		return;
	}

	if( m_bUpdateFontNeeded )
	{
		m_bUpdateFontViewAndCloseUpNeeded = true;
		UpdateFont( false );
	}

	if( m_bUpdateFontViewAndCloseUpNeeded )
		UpdateFontViewAndCloseUp();

	CDialog::OnPaint();
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CTextureFontGeneratorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CTextureFontGeneratorDlg::OnClose() 
{
	if( CanExit() )
		CDialog::OnClose();
}

void CTextureFontGeneratorDlg::OnOK() 
{
	if( CanExit() )
		CDialog::OnOK();
}

void CTextureFontGeneratorDlg::OnCancel() 
{
	if (CanExit())
		CDialog::OnCancel();
}

BOOL CTextureFontGeneratorDlg::OnMouseWheel( UINT nFlags, short zDelta, CPoint pt )
{
	CWnd *pFocus = this->GetFocus();
	if( nFlags & MK_CONTROL )
		zDelta *= 5;
	if( pFocus == &m_Padding || pFocus == &m_FontSize )
	{
		CEdit *pFocusedEdit = dynamic_cast<CEdit *>(pFocus);
		CString sText;
		pFocusedEdit->GetWindowText( sText );
		int i = atoi( sText );
		i += zDelta / WHEEL_DELTA;
		if( pFocus == &m_Padding )
			i = max( i, 0 );
		else if( pFocus == &m_FontSize )
			i = max( i, 1 );

		sText.Format( "%i", i );
		pFocusedEdit->SetWindowText( sText );
		return FALSE;
	}

	return CDialog::OnMouseWheel( nFlags, zDelta, pt );
}

BOOL CTextureFontGeneratorDlg::CanExit()
{
	return TRUE;
}

void CTextureFontGeneratorDlg::OnCbnSelchangeShownPage()
{
	m_bUpdateFontViewAndCloseUpNeeded = true;
	Invalidate( FALSE );
	UpdateWindow();
}

void CTextureFontGeneratorDlg::OnCbnSelchangeFamilyList()
{
	m_bUpdateFontNeeded = true;
	Invalidate( FALSE );
	UpdateWindow();
}

void CTextureFontGeneratorDlg::OnEnChangeFontSize()
{
	m_bUpdateFontNeeded = true;
	Invalidate( FALSE );
	UpdateWindow();
}

void CTextureFontGeneratorDlg::OnEnChangePadding()
{
	m_bUpdateFontNeeded = true;
	Invalidate( FALSE );
}

void CTextureFontGeneratorDlg::OnStyleAntialiased()
{
	CMenu *pMenu = GetMenu();
	int Checked = pMenu->GetMenuState(ID_STYLE_ANTIALIASED, 0) & MF_CHECKED;
	Checked ^= MF_CHECKED;
	pMenu->CheckMenuItem( ID_STYLE_ANTIALIASED, Checked );
	m_bUpdateFontNeeded = true;
	Invalidate( FALSE );
}

void CTextureFontGeneratorDlg::OnStyleBold()
{
	CMenu *pMenu = GetMenu();
	int Checked = pMenu->GetMenuState(ID_STYLE_BOLD, 0) & MF_CHECKED;
	Checked ^= MF_CHECKED;
	pMenu->CheckMenuItem( ID_STYLE_BOLD, Checked );
	m_bUpdateFontNeeded = true;
	Invalidate( FALSE );
}

void CTextureFontGeneratorDlg::OnStyleItalic()
{
	CMenu *pMenu = GetMenu();
	int Checked = pMenu->GetMenuState(ID_STYLE_ITALIC, 0) & MF_CHECKED;
	Checked ^= MF_CHECKED;
	pMenu->CheckMenuItem( ID_STYLE_ITALIC, Checked );
	m_bUpdateFontNeeded = true;
	Invalidate( FALSE );
}

void CTextureFontGeneratorDlg::OnOptionsDoubleres()
{
	CMenu *pMenu = GetMenu();
	int Checked = pMenu->GetMenuState(ID_OPTIONS_DOUBLERES, 0) & MF_CHECKED;
	Checked ^= MF_CHECKED;
	pMenu->CheckMenuItem( ID_OPTIONS_DOUBLERES, Checked );
	m_bUpdateFontNeeded = true;
	Invalidate( FALSE );
}

void CTextureFontGeneratorDlg::OnOptionsExportstroketemplates()
{
	CMenu *pMenu = GetMenu();
	int Checked = pMenu->GetMenuState(ID_OPTIONS_EXPORTSTROKETEMPLATES, 0) & MF_CHECKED;
	Checked ^= MF_CHECKED;
	pMenu->CheckMenuItem( ID_OPTIONS_EXPORTSTROKETEMPLATES, Checked );
}

void CTextureFontGeneratorDlg::OnOptionsNumbersonly()
{
	CMenu *pMenu = GetMenu();
	int Checked = pMenu->GetMenuState(ID_OPTIONS_NUMBERSONLY, 0) & MF_CHECKED;
	Checked ^= MF_CHECKED;
	pMenu->CheckMenuItem( ID_OPTIONS_NUMBERSONLY, Checked );
	m_bUpdateFontNeeded = true;
	Invalidate( FALSE );
}

void CTextureFontGeneratorDlg::OnDeltaposSpinTop(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	*pResult = 0;

	m_bUpdateFontViewAndCloseUpNeeded = true;
	Invalidate( FALSE );
}

void CTextureFontGeneratorDlg::OnDeltaposSpinBaseline(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	*pResult = 0;

	m_bUpdateFontViewAndCloseUpNeeded = true;
	Invalidate( FALSE );
}

void CTextureFontGeneratorDlg::OnFileSave()
{
	// XXX
	OPENFILENAME ofn;
	memset( &ofn, 0, sizeof(ofn) );
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = m_hWnd;
	ofn.lpstrFilter = "*.ini\000Font control files\000\000";

	char szFile[1024] = "";
	ofn.lpstrFile = szFile;
	{
		CString sName = g_pTextureFont->m_sFamily;
		sName.MakeLower();
		_snprintf( szFile, 1023, "_%s%s %gpx", 
			(const char *) sName, 
			g_pTextureFont->m_bBold ? " Bold" : "",
			g_pTextureFont->m_fFontSizePixels );
	}
	ofn.nMaxFile = sizeof(szFile);

	// lpstrInitialDir
	ofn.Flags = OFN_HIDEREADONLY|OFN_LONGNAMES;

	if( !GetSaveFileName(&ofn) )
		return;

	/* If the filename provided has an extension, remove it. */
	{
		char *pExt = strrchr( szFile, '.' );
		if( pExt )
			*pExt = 0;
	}


/*	{
		vector<CString> asOldFiles;
		WIN32_FIND_DATA fd;

		CString sPath = szFile;
		HANDLE hFind = FindFirstFile( sPath+"*", &fd );

		if( hFind != INVALID_HANDLE_VALUE )
		{
			do {
				if( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
					continue;
				asOldFiles.push_back( fd.cFileName );
			} while( FindNextFile(hFind, &fd) );
			FindClose( hFind );
		}
	}
*/

	CMenu *pMenu = GetMenu();
	bool bExportStrokeTemplates = !!( pMenu->GetMenuState(ID_OPTIONS_EXPORTSTROKETEMPLATES, 0) & MF_CHECKED );
	bool bDoubleRes = !!( pMenu->GetMenuState(ID_OPTIONS_DOUBLERES, 0) & MF_CHECKED );
	if( bDoubleRes )
	{
		g_pTextureFont->Save( szFile, "", true, false, bExportStrokeTemplates );	// save metrics
		UpdateFont( true );	// generate DoubleRes bitmaps
		g_pTextureFont->Save( szFile, " (doubleres)", false, true, bExportStrokeTemplates );	// save bitmaps
		// reset to normal, non-DoubleRes font
		m_bUpdateFontNeeded = true;
		Invalidate( FALSE );
		UpdateWindow();
	}
	else	// normal res
	{
		g_pTextureFont->Save( szFile, "", true, true, bExportStrokeTemplates );	// save metrics and bitmaps
	}
}

void CTextureFontGeneratorDlg::OnFileExit()
{
	DestroyWindow();
}

/*
 * Copyright (c) 2003-2007 Glenn Maynard
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

#include "stdafx.h"
#include "TextureFont.h"
#include "Utils.h"

#include <fstream>
#include <math.h>
#include <cassert>

TextureFont::TextureFont()
{
	m_iCharDescent = 0;
	m_iCharLeftOverlap = 0;
	m_iCharRightOverlap = 0;
	m_iCharBaseline = 0;
	m_iCharTop = 0;
	m_iCharVertSpacing = 0;
}

TextureFont::~TextureFont()
{
}

void TextureFont::FormatFontPages()
{
	for( unsigned i = 0; i < m_apPages.size(); ++i )
		delete m_apPages[i];
	m_apPages.clear();
	for( map<wchar_t, HBITMAP>::iterator i = m_Characters.begin(); i != m_Characters.end(); ++i )
	{
		if( i->second == NULL )
			continue;
		int b = DeleteObject( i->second );
		ASSERT( b );
	}
	m_Characters.clear();
	
	m_sError = m_sWarnings = "";

	/*
	 * Create the system font.
	 */
	LOGFONT font;
	memset( &font, 0, sizeof(font) );
	strncpy( &font.lfFaceName[0], (const char *) m_sFamily, 31 );
	font.lfFaceName[31] = 0;
	font.lfCharSet = DEFAULT_CHARSET;
	if( m_bBold )
		font.lfWeight = FW_BOLD;
	if( m_bItalic )
		font.lfItalic = TRUE;

//	(fPoints / 72.0f) * 90
	font.lfHeight = (LONG) -m_fFontSizePixels;
	font.lfQuality = m_bAntiAlias? ANTIALIASED_QUALITY: NONANTIALIASED_QUALITY;
	font.lfPitchAndFamily = DEFAULT_PITCH;

	HFONT hFont = CreateFontIndirect( &font );
	if( hFont == NULL )
	{
		m_sError = "Font isn't available";
		return;
	}

	HDC hDC = CreateCompatibleDC( NULL );
	HGDIOBJ hOldFont = SelectObject( hDC, hFont );

	/*
	 * Read high-level text metrics.
	 */
	TEXTMETRIC TextMetrics;
	GetTextMetrics( hDC, &TextMetrics );

	m_iCharBaseline = TextMetrics.tmAscent;
	m_iCharDescent = TextMetrics.tmDescent;
	m_iCharVertSpacing = TextMetrics.tmHeight + TextMetrics.tmExternalLeading;
	m_iCharTop = TextMetrics.tmInternalLeading;
	m_iCharLeftOverlap = m_iCharRightOverlap = 0;
	m_BoundingRect.top = m_BoundingRect.left = 0;
	m_BoundingRect.bottom = m_BoundingRect.right = 0;


	int n = GetKerningPairs( hDC, 0, NULL );
	KERNINGPAIR *kp = new KERNINGPAIR[n];
	GetKerningPairs( hDC, n, kp );

	for( int i = 0; i < n; ++i )
	{
		if( kp[i].wFirst == 'A' && kp[i].wSecond == 'A' )
			int q = 1;
		if( kp[i].wFirst == 'A' && kp[i].wSecond == 'j' )
			int q = 1;
		if( kp[i].wFirst == 'j' && kp[i].wSecond == 'j' )
			int q = 1;
	}

	m_RealBounds.clear();

	for( unsigned i = 0; i < m_PagesToGenerate.size(); ++i )
	{
		m_apPages.push_back( new FontPage );
		FormatFontPage( i, hDC );
	}

/*	OUTLINETEXTMETRIC *tm = NULL;
	int i = GetOutlineTextMetrics( hDC, 0, NULL );
	if( i )
	{
		tm = (OUTLINETEXTMETRIC *) new char[i];
		GetOutlineTextMetrics( hDC, i, tm );
	}
	delete [] tm;
*/

	SelectObject( hDC, hOldFont );
	DeleteObject( hFont );
	DeleteDC( hDC );
}

void TextureFont::FormatCharacter( wchar_t c, HDC hDC )
{
	if( m_Characters.find(c) != m_Characters.end() )
		return;

	WORD gi;
	if( GetGlyphIndicesW(hDC, &c, 1, &gi, GGI_MARK_NONEXISTING_GLYPHS) && gi == 0xFFFF )
		return;

/*		int ii = GetFontUnicodeRanges( hDC, NULL );
		GLYPHSET *gs = (GLYPHSET *) alloca(ii);
		GetFontUnicodeRanges( hDC, gs );

		GLYPHMETRICS gm;
		MAT2 mat;
		memset( &mat, 0, sizeof(mat) );
		mat.eM11.value = 1;
		mat.eM22.value = 1;

		ii = GetGlyphOutline( hDC, c, GGO_BEZIER, &gm, 0, NULL, &mat );
		DWORD *data = (DWORD *) alloca(ii);
		ii = GetGlyphOutline( hDC, c, GGO_BEZIER, &gm, ii, data, &mat );
*/

	// lol what
	if(c == L'j')
		int q = 1;

	ABC &abc = m_ABC[c];
	{
		/* Use GetCharABCWidthsFloatW, since it works on all types of fonts; GetCharABCWidths
		 * only works on TrueType fonts. */
		ABCFLOAT abcf;
		GetCharABCWidthsFloatW( hDC, c, c, &abcf );

		abc.abcA = lrintf( abcf.abcfA );
		abc.abcB = lrintf( abcf.abcfB );
		abc.abcC = lrintf( abcf.abcfC );
	}

	/*
	 * If the A or C widths are positive, it's simply extra space to add to either side.
	 * Move this into the B width.  The only time this actually matters is to be able to
	 * omit the A width from the first letter of a string which is left-aligned, or the
	 * last C width from a right-aligned string.  The exported fonts don't support explicit
	 * ABC widths, instead representing overhang and underhang with global m_iCharLeftOverlap
	 * and m_iCharRightOverlap settings.
	 *
	 * After making this adjustment, the glyphs' B region is left-aligned in the bitmap.
	 */
	if( abc.abcA > 0 )
	{
		abc.abcB += abc.abcA;
		abc.abcA = 0;
	}
	if( abc.abcC > 0 )
	{
		abc.abcB += abc.abcC;
		abc.abcC = 0;
	}

	/* Render the character into an empty bitmap.  Since we don't know how
	 * large the character will be, this is somewhat oversized. */
	HBITMAP hBitmap;
	{
		HDC hTempDC = GetDC(NULL);
		hBitmap = CreateCompatibleBitmap( hTempDC, abc.abcB, 128 );
		ReleaseDC( NULL, hTempDC );
	}
	HGDIOBJ hOldBitmap = SelectObject( hDC, hBitmap );

	SetTextColor( hDC, RGB(0xFF,0xFF,0xFF) );
	SetBkColor( hDC, RGB(0,0,0) );
	SetBkMode( hDC, OPAQUE );

	TextOutW( hDC, -abc.abcA, 0, &c, 1 );

	/* Determine the real bounding box: */
	RECT &realbounds = m_RealBounds[c];
	realbounds.top = 0;
	realbounds.bottom = 10;

/*	if(c == L'j')
	{
		Surface surf;
		BitmapToSurface( hBitmap, &surf );
		GrayScaleToAlpha( &surf );

		FILE *f = fopen( "c:/foo5.png", "w+b" );
		char szErrorbuf[1024];
		SavePNG( f, szErrorbuf, &surf );
		fclose( f );
	}
*/
	{
		Surface surf;
		BitmapToSurface( hBitmap, &surf );
		GetBounds( &surf, &realbounds );
	}
	realbounds.left = 0;
	realbounds.right = abc.abcB;


/*	{
//		Graphics blit( pBitmap );
//		blit.DrawLine(&pen, abc.abcA, 5, abc.abcA+abc.abcB, 5);
		const SolidBrush solidBrush1(Color(128, 255, 0, 255));
		Pen pen( &solidBrush1, 1 );
		graphics.DrawRectangle( &pen, 0, 0, (-abc.abcA) - 1, 10 );

		pen.SetColor(Color(128, 0, 255, 255));
		graphics.DrawRectangle( &pen, 0, 0, abc.abcB - 1, 10 );

		pen.SetColor(Color(128, 255, 255, 0));
		graphics.DrawRectangle( &pen, abc.abcA + abc.abcB, 10, abc.abcC - 1, 20 );
	}
*/

	/*
	 * The bitmap is probably too big.  Resize it: remove empty space on
	 * the left, right and bottom.  Don't move it up; that'll confuse offsets.
	 */
	BitBlt( hDC, 0, 0, realbounds.right, realbounds.bottom,
		hDC, 0, 0, SRCCOPY );
	SelectObject( hDC, hOldBitmap );


	m_Characters[c] = hBitmap;

	if( realbounds.left != realbounds.right && realbounds.top != realbounds.bottom )
	{
		m_BoundingRect.top = min( m_BoundingRect.top, (LONG) realbounds.top );
		m_BoundingRect.left = min( m_BoundingRect.left, (LONG) realbounds.left );
		m_BoundingRect.right = max( m_BoundingRect.right, (LONG) realbounds.right );
		m_BoundingRect.bottom = max( m_BoundingRect.bottom, (LONG) realbounds.bottom );
		if( m_BoundingRect.left == m_BoundingRect.right && m_BoundingRect.top == m_BoundingRect.bottom )
			m_BoundingRect = realbounds;
	}

	m_iCharLeftOverlap = max( m_iCharLeftOverlap, -int(abc.abcA) );
	m_iCharRightOverlap = max( m_iCharRightOverlap, int(abc.abcC) - int(abc.abcB) );

//	const SolidBrush solidBrush(Color(128, 255, 0, 255));
//	Pen pen(&solidBrush, 1);
//	graphics.DrawRectangle(&pen, bounds);

//	Graphics blit( pBitmap );
//	blit.DrawLine(&pen, abc.abcA, 5, abc.abcA+abc.abcB, 5);
//	blit.DrawRectangle( &pen, 1, 10, abc.abcA, 10 );

//	pen.SetColor(Color(255, 0, 255, 0));
//	graphics.DrawRectangle(&pen, realbounds);
}

/* Return the number of pixels the characters are shifted downwards in the final
 * page for m_iPadding. */
int TextureFont::GetTopPadding() const
{
	return m_iPadding/2;
}

void TextureFont::FormatFontPage( int iPage, HDC hDC )
{
	const FontPageDescription &Desc = m_PagesToGenerate[iPage];

	/* First, generate bitmaps for all characters in this page. */
	for( unsigned i = 0; i < Desc.chars.size(); ++i )
		FormatCharacter( Desc.chars[i], hDC );

	FontPage *pPage = m_apPages[iPage];
	pPage->m_iFrameWidth = (m_BoundingRect.right - m_BoundingRect.left) + m_iPadding;
	pPage->m_iFrameHeight = (m_BoundingRect.bottom - m_BoundingRect.top) + m_iPadding;
	int iDimensionMultiple = 4;	// TODO: This only needs to be 4 for doubleres textures.  It could be 2 otherwise and use less space
	pPage->m_iFrameWidth = (int)ceil( pPage->m_iFrameWidth /(double)iDimensionMultiple ) * iDimensionMultiple;
	pPage->m_iFrameHeight = (int)ceil( pPage->m_iFrameHeight /(double)iDimensionMultiple ) * iDimensionMultiple;

	pPage->m_iNumFramesX = (int) ceil( powf( (float) Desc.chars.size(), 0.5f ) );
	pPage->m_iNumFramesY = (int) ceil( (float) Desc.chars.size() / pPage->m_iNumFramesX );

	pPage->Create( pPage->m_iNumFramesX*pPage->m_iFrameWidth, pPage->m_iNumFramesY*pPage->m_iFrameHeight );
	
	HGDIOBJ hOldBitmap = SelectObject( hDC, pPage->m_hPage );

	HDC hSrcDC = CreateCompatibleDC( NULL );

	int iRow = 0, iCol = 0;
	for( unsigned CurChar = 0; CurChar < Desc.chars.size(); ++CurChar )
	{
		const wchar_t c = Desc.chars[CurChar];
		const ABC &abc = m_ABC[c];
		
		/* The current frame is at fOffsetX/fOffsetY.  Center the character
		 * horizontally in the frame.  We can align it however we want
		 * vertically, as long as we align the baselines. */
		float fOffsetX = (float) pPage->m_iFrameWidth*iCol; /* origin -> frame top-left */
		fOffsetX += pPage->m_iFrameWidth/2.0f; /* frame top-left -> frame center */
		fOffsetX -= (abc.abcA+abc.abcB+abc.abcC)/2.0f;
		fOffsetX += abc.abcA;

		/* Truncate, so we don't blit to half a pixel: */
		fOffsetX = float(int(fOffsetX));

		float fOffsetY = (float) pPage->m_iFrameHeight*iRow;
		fOffsetY += GetTopPadding();

		if( m_Characters[c] != NULL )
		{
			HBITMAP hCharacterBitmap = m_Characters[c];
			HGDIOBJ hOldSrcBitmap = SelectObject( hSrcDC, hCharacterBitmap );

			const RECT &realbounds = m_RealBounds[c];
			BitBlt( hDC, int(fOffsetX), int(fOffsetY),
				m_ABC[c].abcB, realbounds.bottom,
				hSrcDC, 0, 0, SRCCOPY );

			SelectObject( hSrcDC, hOldSrcBitmap );
		}

		++iCol;
		if( iCol == pPage->m_iNumFramesY )
		{
			iCol = 0;
			++iRow;
		}
	}

	DeleteDC( hSrcDC );
	SelectObject( hDC, hOldBitmap );
}

/* UTF-8 encode ch and append to out. */
void wchar_to_utf8( wchar_t ch, string &out )
{
	if( ch < 0x80 ) { out.append( 1, (char) ch ); return; }

	int cbytes = 0;
	if( ch < 0x800 ) cbytes = 1;
	else if( ch < 0x10000 )    cbytes = 2;
	else if( ch < 0x200000 )   cbytes = 3;
	else if( ch < 0x4000000 )  cbytes = 4;
	else cbytes = 5;

	{
		int shift = cbytes*6;
		const int init_masks[] = { 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };
		out.append( 1, (char) (init_masks[cbytes-1] | (ch>>shift)) );
	}

	for( int i = 0; i < cbytes; ++i )
	{
		int shift = (cbytes-i-1)*6;
		out.append( 1, (char) (0x80 | ((ch>>shift)&0x3F)) );
	}
}

#include <iomanip>

static bool IsNumberChar( wchar_t c )
{
	return c >= 0x0030  &&  c <= 0x0039;
}

void TextureFont::Save( CString sBasePath, CString sBitmapAppendBeforeExtension, bool bSaveMetrics, bool bSaveBitmaps, bool bExportStrokeTemplates )
{
	if( m_sError != "" )
		return;

	const CString inipath = sBasePath + ".ini";

	ofstream f;

	if( bSaveMetrics )
	{
		f.open(inipath.GetString());

		/* Write global properties: */
		f << "[common]\n";

		f << "Baseline=" << m_iCharBaseline + GetTopPadding() << "\n";
		f << "Top=" << m_iCharTop + GetTopPadding() << "\n";
		f << "LineSpacing=" << m_iCharVertSpacing << "\n";
		f << "DrawExtraPixelsLeft=" << m_iCharLeftOverlap << "\n";
		f << "DrawExtraPixelsRight=" << m_iCharRightOverlap << "\n";
		f << "AdvanceExtraPixels=0\n";
	}

	for( unsigned i = 0; i < m_apPages.size(); ++i )
	{
		const FontPageDescription &desc = m_PagesToGenerate[i];
		ASSERT( m_apPages[i]->m_hPage );
		FontPage &page = *m_apPages[i];

		if( bSaveMetrics )
		{
			f << "\n" << "[" << desc.name.GetString() << "]\n";

			{
				int iWidth = 1;
				if( desc.chars.size() / page.m_iNumFramesX > 10 )
					iWidth = 2;

				unsigned iChar = 0;
				unsigned iLine = 0;
				while( iChar < desc.chars.size() )
				{
					f << "Line "  << setw(iWidth) << iLine << "=";
					f << setw(1);
					for( int iX = 0; iX < page.m_iNumFramesX && iChar < desc.chars.size(); ++iX, ++iChar )
					{
						const wchar_t c = desc.chars[iChar];
						string sUTF8;
						wchar_to_utf8( c, sUTF8 );
						f << sUTF8.c_str();
					}
					f << "\n";
					++iLine;
				}
			}

			f << "\n";


			/* export character widths.  "numbers" page has fixed with for all number characters. */
			vector<int> viCharWidth;
			int iMaxNumberCharWidth = 0;
			for( unsigned j = 0; j < desc.chars.size(); ++j )
			{
				/* This is the total width to advance for the whole character, which is the
				 * sum of the ABC widths. */
				const wchar_t c = desc.chars[j];
				ABC &abc = m_ABC[c];
				int iCharWidth = abc.abcA + int(abc.abcB) + int(abc.abcC);
				viCharWidth.push_back( iCharWidth );

				if( IsNumberChar( c ) )
					iMaxNumberCharWidth = max( iMaxNumberCharWidth, iCharWidth );
			}
			for( unsigned j = 0; j < desc.chars.size(); ++j )
			{
				const wchar_t c = desc.chars[j];
				int iCharWidth = viCharWidth[j];
				if( desc.name == "numbers"  &&  IsNumberChar( c ) )
					iCharWidth = iMaxNumberCharWidth;
				f << j << "=" << iCharWidth << "\n";
			}
		}

		if( bSaveBitmaps )
		{
			Surface surf;
			BitmapToSurface( m_apPages[i]->m_hPage, &surf );

			GrayScaleToAlpha( &surf );

			for( int j=0; j<2; j++ )
			{
				CString sPageName = m_PagesToGenerate[i].name.GetString();
				switch( j )
				{
				case 0:
					break;
				case 1:
					if( !bExportStrokeTemplates )
						continue;
					sPageName += "-stroke";
					break;
				default:
					assert(false);
				}

				CString sFile;
				sFile.Format( "%s [%s] %ix%i%s.png",
					sBasePath.GetString(),
					sPageName.GetString(),
					m_apPages[i]->m_iNumFramesX,
					m_apPages[i]->m_iNumFramesY,
					sBitmapAppendBeforeExtension.GetString() );

				FILE *f = fopen( sFile, "w+b" );
				char szErrorbuf[1024];
				SavePNG( f, szErrorbuf, &surf );
				fclose( f );
			}
		}
	}
}


FontPage::FontPage()
{ 
	m_hPage = NULL;
	m_iFrameWidth = 0;
	m_iFrameHeight = 0;
}

FontPage::~FontPage()
{
	DeleteObject( m_hPage );
}

void FontPage::Create( unsigned width, unsigned height )
{
	DeleteObject( m_hPage );
	HDC hDC = GetDC(NULL);
	m_hPage = CreateCompatibleBitmap( hDC, width, height );
	ReleaseDC( NULL, hDC );
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
